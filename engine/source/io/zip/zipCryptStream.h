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

#ifndef _ZIPCRYPTSTREAM_H_
#define _ZIPCRYPTSTREAM_H_

#ifndef _FILTERSTREAM_H_
#include "io/filterStream.h"
#endif

class ZipCryptRStream : public FilterStream
{
   typedef FilterStream Parent;

   std::iostream *mStream;

   S32 mStreamStartPos;
   S32 mFileStartPos;
   S32 mFileEndPos;
   
   U32 mKeys[3]; // mKeys and it's usage is very unclear and has a ton of magic numbers -patw
   
   const char *mPassword;

   U32 fillBuffer(char* pBuffer, const U32 in_attemptSize);

public:
   ZipCryptRStream();
   virtual ~ZipCryptRStream();

   void setPassword(const char *password);
   inline void setFileEndPos(S32 pos)        { mFileEndPos = pos; }

   // Overrides of FilterStream
   bool attachStream(std::iostream *io_pSlaveStream);
   void detachStream();
   std::iostream *getStream()                       { return mStream; }

   U32  getPosition() const;
   bool setPosition(const U32 in_newPosition);

protected:
   bool _read(char* out_pBuffer, const U32 in_numBytes);

   void updateKeys(const U8 c);
   U8 decryptByte();
};

#endif // _ZIPCRYPTSTREAM_H_
