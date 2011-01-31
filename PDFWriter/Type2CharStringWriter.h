/*
   Source File : Type2CharStringWriter.h


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
#pragma once
#include "EStatusCode.h"
#include "IOBasicTypes.h"

using namespace IOBasicTypes;

class IByteWriter;

class Type2CharStringWriter
{
public:
	Type2CharStringWriter(IByteWriter* inTargetStream);
	~Type2CharStringWriter(void);

	void Assign(IByteWriter* inTargetStream);

	EStatusCode WriteHintMask(unsigned long inMask,unsigned long inMaskSize);
	EStatusCode WriteIntegerOperand(long inOperand);
	EStatusCode WriteOperator(unsigned short inOperatorCode);


private:

	IByteWriter* mTargetStream;

	EStatusCode WriteMaskBytes(unsigned long inMask,unsigned long inMaskByteSize);
	EStatusCode WriteByte(Byte inValue);
};
