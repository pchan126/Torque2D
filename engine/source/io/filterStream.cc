//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "filterStream.h"

FilterStream::~FilterStream()
{
   //
}

bool FilterStream::_read(char* out_pBuffer, const U32 in_numBytes)
{
   AssertFatal(getStream() != NULL, "Error no stream to pass to");

   getStream()->read(out_pBuffer, in_numBytes);

   return good();
}


bool FilterStream::_write(const U32 numBytes, const char *buffer )
{
   AssertFatal(false, "No writing allowed to filter");
   return false;
}

//bool FilterStream::hasCapability(const Capability in_streamCap) const
//{
//   // Fool the compiler.  We know better...
//   FilterStream* ncThis = const_cast<FilterStream*>(this);
//   AssertFatal(ncThis->getStream() != NULL, "Error no stream to pass to");
//
//   return ncThis->getStream()->hasCapability(in_streamCap);
//}

U32 FilterStream::getPosition() 
{
	U32 pos = tellg();
	return pos;
}

void FilterStream::setPosition(const U32 in_newPosition)
{
   seekg(0, getStream()->beg+in_newPosition);
}

U32 FilterStream::getStreamSize()
{
   auto x = tellg();
   seekg(0, end);
   U32 length = (U32)tellg();
   seekg(x);

   return length;
}

