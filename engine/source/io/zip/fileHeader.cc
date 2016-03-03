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

#include "platform/platform.h"
#include "io/stream.h"

#include "io/zip/fileHeader.h"
#include "io/zip/compressor.h"

#include "memory/safeDelete.h"

#include "io/resizeStream.h"
#include "memory/frameAllocator.h"

namespace Zip
{

//////////////////////////////////////////////////////////////////////////
// Constructor/Destructor
//////////////////////////////////////////////////////////////////////////

FileHeader::FileHeader()
{
   mHeaderSig = mFileHeaderSignature;

   mExtractVer = 20;
   mFlags = 0;
   mCompressMethod = Stored;

   mModTime = 0;
   mModDate = 0;

   mCRC32 = 0;

   mCompressedSize = 0;
   mUncompressedSize = 0;

   mFilename = NULL;
}

FileHeader::~FileHeader()
{
   SAFE_DELETE_ARRAY(mFilename);

   for(S32 i = 0;i < mExtraFields.size();i++)
   {
      SAFE_DELETE(mExtraFields[i]);
   }
}

//////////////////////////////////////////////////////////////////////////
// Protected Methods
//////////////////////////////////////////////////////////////////////////

bool FileHeader::readExtraFields(std::iostream &stream, U16 efLen)
{
   bool ret = true;

   U32 pos = stream.tellg();
   U32 end = pos + efLen;

   while(stream.tellg() < end)
   {
      U16 fieldSig, fieldSize;

      ret = false;

      stream >> fieldSig;
      stream >> fieldSize;
      if(! stream.good())
         break;

      pos = stream.tellg();

      ExtraField *ef = ExtraField::create(fieldSig);
      if(ef)
      {
         ret |= ef->read(stream);

         if(! ret)
            delete ef;
         else
            mExtraFields.push_back(ef);
      }

      stream.seekg(0, stream.beg + pos + fieldSize);
   }
   
   return ret;
}

//////////////////////////////////////////////////////////////////////////
// Public Methods
//////////////////////////////////////////////////////////////////////////

bool FileHeader::read(std::iostream &stream)
{
   stream >> mHeaderSig;
   if(mHeaderSig != mFileHeaderSignature)
      return false;

   stream >> mExtractVer;
   stream >> mFlags;
   stream >> mCompressMethod;
   stream >> mModTime;
   stream >> mModDate;
   stream >> mCRC32;
   stream >> mCompressedSize;
   stream >> mUncompressedSize;
   
   U16 fnLen, efLen;
   stream >> fnLen;
   stream >> efLen;

   char *fn = new char[fnLen + 1];
   stream.read( fn, fnLen);
   fn[fnLen] = 0;

   SAFE_DELETE_ARRAY(mFilename);
   mFilename = fn;

   return readExtraFields(stream, efLen);
}

bool FileHeader::write(std::iostream &stream)
{
   mHeaderSig = mFileHeaderSignature;

   stream << mHeaderSig;
   
   stream << mExtractVer;
   stream << mFlags;
   stream << mCompressMethod;
   stream << mModTime;
   stream << mModDate;
   stream << mCRC32;
   stream << mCompressedSize;
   stream << mUncompressedSize;

   U16 fnLen = mFilename ? (U16)dStrlen(mFilename) : 0,
      efLen = 0;
   stream << fnLen;
   stream << efLen;

   if(fnLen)
      stream.write(mFilename, fnLen);

   // FIXME [tom, 1/23/2007] Write extra fields here

   return true;
}

//////////////////////////////////////////////////////////////////////////

ExtraField *FileHeader::findExtraField(U16 id)
{
   for(S32 i = 0;i < mExtraFields.size();++i)
   {
      if(mExtraFields[i]->getID() == id)
         return mExtraFields[i];
   }

   return NULL;
}

//////////////////////////////////////////////////////////////////////////

void FileHeader::setFilename(const char *filename)
{
   SAFE_DELETE_ARRAY(mFilename);
   mFilename = new char [dStrlen(filename)+1];
   dStrcpy((char *)mFilename, filename);
}

} // end namespace Zip
