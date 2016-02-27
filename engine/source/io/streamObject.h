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

#include "sim/simBase.h"
#include <iostream>
#include "io/StreamFn.h"

#ifndef _STREAMOBJECT_H_
#define _STREAMOBJECT_H_

/// @addtogroup zip_group
// @{

//////////////////////////////////////////////////////////////////////////
/// @brief Script wrapper for the Stream class
/// 
/// It is not possible to instantiate StreamObject in script. Instead,
/// it is instantiated in C++ code and returned to script.
/// 
/// This was mainly intended to allow the \ref zip_group "zip code" to
/// provide the stream interface to script.
//////////////////////////////////////////////////////////////////////////
class StreamObject : public SimObject
{
   typedef SimObject Parent;

protected:
   std::iostream *mStream;

public:
   StreamObject();
   StreamObject(std::iostream *stream);
   virtual ~StreamObject();

   DECLARE_CONOBJECT(StreamObject);

   virtual bool onAdd();

   /// Set the stream to allow reuse of the object
   void setStream(std::iostream *stream)      { mStream = stream; }

   /// Get the underlying stream. Used with setStream() to support object reuse
   std::iostream *getStream()           { return mStream; }

   /// Gets a printable string form of the status
   const char* getStatus();

   bool isEOS()                  { return mStream->eof(); }

   /// Gets the position in the stream
   U32  getPosition() const
   {
		int pos = mStream->tellp();
		return pos;
   }

   /// Sets the position of the stream.  Returns if the new position is valid or not
   bool setPosition(const U32 in_newPosition)
   {
	   mStream->seekp(0, mStream->beg + in_newPosition);
	   return mStream->good();
   }

   /// Gets the size of the stream
   U32  getStreamSize()
   {
	   int pos = mStream->tellp();
	   mStream->seekp(0, mStream->end);
	   int size = mStream->tellp();
	   mStream->seekp(0, mStream->beg + pos);
	   return size;
   }

   /// Reads a line from the stream.
   const char * readLine();

   /// Writes a line to the stream
   void writeLine(U8 *buffer)
   {
	   if (mStream)
	   {
		   StreamFn::writeLine(*mStream, (char*)buffer);
	   }
   }

   /// Reads a string and inserts it into the StringTable
   /// @see StringTable
   const char *readSTString(bool casesens = false)
   {
      return mStream ? mStream->readSTString(casesens) : NULL;
   }

   /// Reads a string of maximum 255 characters long
   const char *readString();
   /// Reads a string that could potentially be more than 255 characters long.
   /// @param maxStringLen Maximum length to read.  If the string is longer than maxStringLen, only maxStringLen bytes will be read.
   /// @param stringBuf buffer where data is read into
   const char * readLongString(U32 maxStringLen);
   /// Writes a string to the stream.  This function is slightly unstable.
   /// Only use this if you have a valid string that is not empty.
   /// writeString is safer.
   void writeLongString(U32 maxStringLen, const char *string)
   {
      if(mStream)
         mStream->writeLongString(maxStringLen, string);
   }

   /// Writes a string to the stream.
   void writeString(const char *stringBuf, S32 maxLen=255)
   {
      if(mStream)
         mStream->writeString(stringBuf, maxLen);
   }

   /// Copy the contents of another stream into this one
   bool copyFrom(StreamObject *other)
   {
      if(mStream)
         return mStream->copyFrom(other->getStream());

      return false;
   }
};

// @}

#endif // _STREAMOBJECT_H_
