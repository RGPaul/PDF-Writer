/*
   Source File : CharStringType1Interpreter.cpp


   Copyright 2011 Gal Kahana PDFWriter

   Licensed under the Apache License, Version 2.0 (the "License");
   you may not use this file except in compliance with the License.
   You may obtain a copy of the License at

       http://www.apache.org/licenses/LICENSE-2.0

   Unless required by applicable law or agreed to in writing, software
   distributed under the License is distributed on an "AS IS" BASIS,
   WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
   See the License for the specific language governing permissions and
   limitations under the License.

   
*/
#include "CharStringType1Interpreter.h"
#include "InputByteArrayStream.h"
#include "Trace.h"

CharStringType1Interpreter::CharStringType1Interpreter(void)
{
}

CharStringType1Interpreter::~CharStringType1Interpreter(void)
{
}

EStatusCode CharStringType1Interpreter::Intepret(const Type1CharString& inCharStringToIntepret, IType1InterpreterImplementation* inImplementationHelper)
{
	EStatusCode status;

	do
	{
		mImplementationHelper = inImplementationHelper;
		InputByteArrayStream byteArrayStream(inCharStringToIntepret.Code,inCharStringToIntepret.CodeLength);
		InputCharStringDecodeStream charStringStream(&byteArrayStream,inImplementationHelper->GetLenIV());

		mGotEndChar = false;
		if(!inImplementationHelper)
		{
			TRACE_LOG("CharStringType1Interpreter::Intepret, null implementation helper passed. pass a proper pointer!!");
			status = eFailure;
			break;
		}

		status = ProcessCharString(&charStringStream);

	}while(false);

	return status;
}

EStatusCode CharStringType1Interpreter::ProcessCharString(InputCharStringDecodeStream* inCharStringToIntepret)
{
	EStatusCode status = eSuccess;
	bool gotEndExecutionOperator = false;
	Byte buffer;

	while(inCharStringToIntepret->NotEnded() &&
			eSuccess == status && 
			!gotEndExecutionOperator &&
			!mGotEndChar)
	{
		status = (inCharStringToIntepret->Read(&buffer,1) == 1) ? eSuccess:eFailure;
		if(status != eSuccess)
			break;

		if(IsOperator(buffer))
			status = InterpretOperator(buffer,inCharStringToIntepret,gotEndExecutionOperator);
		else
			status = InterpretNumber(buffer,inCharStringToIntepret);
	}
	return status;
}

bool CharStringType1Interpreter::IsOperator(Byte inBuffer)
{
	return  (0 <= inBuffer && inBuffer <= 31);
			
}

EStatusCode CharStringType1Interpreter::InterpretOperator(Byte inBuffer,InputCharStringDecodeStream* inCharStringToIntepret,bool& outGotEndExecutionCommand)
{
	EStatusCode status = eFailure;
	unsigned short operatorValue;
	outGotEndExecutionCommand = false;
	Byte buffer;
	
	if(12 == inBuffer)
	{
		if(inCharStringToIntepret->Read(&buffer,1) != 1)
			return eFailure;

		operatorValue = 0x0c00 + buffer;
	}
	else
	{
		operatorValue = inBuffer;
	}

	switch(operatorValue)
	{
		case 1: // hstem
			status = InterpretHStem();
			break;
		case 3: // vstem
			status = InterpretVStem();
			break;
		case 4: // vmoveto
			status = InterpretVMoveto();
			break;
		case 5: // rlineto
			status = InterpretRLineto();
			break;
		case 6: // hlineto
			status = InterpretHLineto();
			break;
		case 7: // vlineto
			status = InterpretVLineto();
			break;
		case 8: // rrcurveto
			status = InterpretRRCurveto();
			break;
		case 9: // closepath
			status = InterpretClosePath();
			break;
		case 10: // callsubr
			status = InterpretCallSubr();
			break;
		case 11: // return
			status = InterpretReturn();
			outGotEndExecutionCommand = true;
			break;
		case 13: // hsbw
			status = InterpretHsbw();
			break;
		case 14: // endchar
			status = InterpretEndChar();
			break;
		case 21: // rmoveto
			status = InterpretRMoveto();
			break;
		case 22: // hmoveto
			status = InterpretHMoveto();
			break;
		case 30: // vhcurveto
			status = InterpretVHCurveto();
			break;
		case 31: // hvcurveto
			status = InterpretHVCurveto();
			break;
		
		case 0x0c00: // dotsection
			status = InterpretDotSection();
			break;
		case 0x0c01: // vstem3
			status = InterpretVStem3();
			break;
		case 0x0c02: // hstem3
			status = InterpretHStem3();
			break;
		case 0x0c06: // seac
			status = InterpretSeac();
			break;
		case 0x0c07: // sbw
			status = InterpretSbw();
			break;
		case 0x0c0c: // div
			status = InterpretDiv();
			break;
		case 0x0c10: // callothersubr
			status = InterpretCallOtherSubr();
			break;
		case 0x0c11: // pop
			status = InterpretPop();
			break;
		case 0x0c21: // setcurrentpoint
			status = InterpretSetCurrentPoint();
			break;
	}
	return status;	
}

EStatusCode CharStringType1Interpreter::InterpretNumber(Byte inBuffer,InputCharStringDecodeStream* inCharStringToIntepret)
{
	long operand;

	if(32 <= inBuffer && inBuffer <= 246)
	{
		operand = (short)inBuffer - 139;
	}
	else if(247 <= inBuffer && inBuffer <= 250)
	{
		Byte byte1;
		if(inCharStringToIntepret->Read(&byte1,1) != 1)
			return eFailure;

		operand = (inBuffer - 247) * 256 + byte1 + 108;
	}
	else if(251 <= inBuffer && inBuffer <= 254)
	{
		Byte byte1;
		if(inCharStringToIntepret->Read(&byte1,1) != 1)
			return eFailure;

		operand = -(short)(inBuffer - 251) * 256 - byte1 - 108;
	}
	else if(255 == inBuffer)
	{
		Byte byte1,byte2,byte3,byte4;
		
		if(inCharStringToIntepret->Read(&byte1,1) != 1)
			return eFailure;
		if(inCharStringToIntepret->Read(&byte2,1) != 1)
			return eFailure;
		if(inCharStringToIntepret->Read(&byte3,1) != 1)
			return eFailure;
		if(inCharStringToIntepret->Read(&byte4,1) != 1)
			return eFailure;


		operand = (long)(
						((unsigned long)(byte1) << 24) + 
						((unsigned long)(byte2) << 16) + 
						((unsigned long)(byte3) << 8) + 
						(byte4));
	}
	else
		return eFailure;

	mOperandStack.push_back(operand);
	return mImplementationHelper->Type1InterpretNumber(operand);
}

void CharStringType1Interpreter::ClearStack()
{
	mOperandStack.clear();
}

EStatusCode CharStringType1Interpreter::InterpretHStem()
{
	EStatusCode status = mImplementationHelper->Type1Hstem(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretVStem()
{
	EStatusCode status = mImplementationHelper->Type1Vstem(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretVMoveto()
{
	EStatusCode status = mImplementationHelper->Type1VMoveto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretRLineto()
{
	EStatusCode status = mImplementationHelper->Type1RLineto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretHLineto()
{
	EStatusCode status = mImplementationHelper->Type1HLineto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretVLineto()
{
	EStatusCode status = mImplementationHelper->Type1VLineto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretRRCurveto()
{
	EStatusCode status = mImplementationHelper->Type1RRCurveto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretClosePath()
{
	EStatusCode status = mImplementationHelper->Type1ClosePath(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretCallSubr()
{
	Type1CharString* aCharString = mImplementationHelper->GetSubr(mOperandStack.back());
	mOperandStack.pop_back();

	if(aCharString != NULL)
	{
		InputByteArrayStream byteArrayStream(aCharString->Code,aCharString->CodeLength);
		InputCharStringDecodeStream charStringStream(&byteArrayStream,mImplementationHelper->GetLenIV());

		EStatusCode status = ProcessCharString(&charStringStream);
		if(status != eSuccess)
			return eFailure;
		else
			return eSuccess;
	}
	else
	{
		return eFailure;
	}

}

EStatusCode CharStringType1Interpreter::InterpretReturn()
{
	EStatusCode status = mImplementationHelper->Type1Return(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretHsbw()
{
	EStatusCode status = mImplementationHelper->Type1Hsbw(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretEndChar()
{
	EStatusCode status = mImplementationHelper->Type1Endchar(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	mGotEndChar = true;
	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretRMoveto()
{
	EStatusCode status = mImplementationHelper->Type1RMoveto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretHMoveto()
{
	EStatusCode status = mImplementationHelper->Type1HMoveto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretVHCurveto()
{
	EStatusCode status = mImplementationHelper->Type1VHCurveto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretHVCurveto()
{
	EStatusCode status = mImplementationHelper->Type1HVCurveto(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretDotSection()
{
	EStatusCode status = mImplementationHelper->Type1DotSection(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretVStem3()
{
	EStatusCode status = mImplementationHelper->Type1VStem3(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretHStem3()
{
	EStatusCode status = mImplementationHelper->Type1HStem3(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretSeac()
{
	EStatusCode status = mImplementationHelper->Type1Seac(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretSbw()
{
	EStatusCode status = mImplementationHelper->Type1Sbw(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretDiv()
{
	EStatusCode status = mImplementationHelper->Type1Div(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	long valueA;
	long valueB;

	valueB = mOperandStack.back();
	mOperandStack.pop_back();
	valueA = mOperandStack.back();
	mOperandStack.pop_back();
	mOperandStack.push_back(valueA/valueB);
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretCallOtherSubr()
{
	long otherSubrIndex = mOperandStack.back();
	EStatusCode status;

	if(mImplementationHelper->IsOtherSubrSupported(otherSubrIndex))
		status = mImplementationHelper->CallOtherSubr(mOperandStack,mPostScriptOperandStack);
	else
		status = DefaultCallOtherSubr();
	if(status != eSuccess)
		return eFailure;

	mOperandStack.pop_back();
	long argumentsCount = mOperandStack.back();
	mOperandStack.pop_back();
	for(long i=0;i<argumentsCount;++i)
		mOperandStack.pop_back();

	return eSuccess;
}

EStatusCode CharStringType1Interpreter::DefaultCallOtherSubr()
{
	/*
		K. at first i thought of actually implementing first 4 othersubrs- flex mechanism (hint replacement just does basically nothing).
		but then i figured that for the purpose of running this interpreter - meaning getting the stack right, judging by how these 4 are used - 
		i don't reallly need to do anything to get the stack right, other than do the default behavior for unknown subrs. the only damage is that 
		setcurrentpoint won't necesserily get the right points after calling 0 othersubr. well...as long as it's getting the 2 parameter i'm happy,
		and it does. actual flex implementation is only necessery if drawing the thing. so i'll leave it to implementation, and by default
		just behave like it's unknown.

	*/

	LongList::reverse_iterator it = mOperandStack.rbegin();
	long otherSubrIndex = *it;
	++it;
	long argumentsCount = *it;
	++it;

	for(long i=0;i<argumentsCount;++i)
	{
		mPostScriptOperandStack.push_back(*it);
		++it;
	}
	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretPop()
{
	EStatusCode status = mImplementationHelper->Type1Pop(mOperandStack,mPostScriptOperandStack);
	if(status != eSuccess)
		return eFailure;

	long parameter = mPostScriptOperandStack.back();
	mPostScriptOperandStack.pop_back();
	mOperandStack.push_back(parameter);

	return eSuccess;
}

EStatusCode CharStringType1Interpreter::InterpretSetCurrentPoint()
{
	EStatusCode status = mImplementationHelper->Type1SetCurrentPoint(mOperandStack);
	if(status != eSuccess)
		return eFailure;

	ClearStack();
	return eSuccess;
}

