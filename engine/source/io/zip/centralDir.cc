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

#include "io/zip/centralDir.h"
#include "io/zip/compressor.h"

#include "memory/safeDelete.h"

namespace Zip
{

//////////////////////////////////////////////////////////////////////////
// CentralDir Class
//////////////////////////////////////////////////////////////////////////

CentralDir::CentralDir()
{
   mHeaderSig = mCentralDirSignature;

   mDiskNumStart = 0;

   mInternalFileAttr = 0;
   mExternalFileAttr = 0;

   mLocalHeadOffset = 0;
   
   mVersionMadeBy = 0;

   mFileComment = NULL;

   mInternalFlags = 0;
}

CentralDir::CentralDir(FileHeader &fh) : FileHeader(fh)
{
   mHeaderSig = mCentralDirSignature;

   mDiskNumStart = 0;

   mInternalFileAttr = 0;
   mExternalFileAttr = 0;

   mLocalHeadOffset = 0;

   mVersionMadeBy = 0;

   mFileComment = NULL;
}

CentralDir::~CentralDir()
{
   SAFE_DELETE_ARRAY(mFileComment);
}

//////////////////////////////////////////////////////////////////////////

bool CentralDir::read(std::iostream &stream)
{
   stream >> mHeaderSig;
   if(mHeaderSig != mCentralDirSignature)
      return false;

   stream >> mVersionMadeBy;
   stream >> mExtractVer;
   stream >> mFlags;
   stream >> mCompressMethod;
   stream >> mModTime;
   stream >> mModDate;
   stream >> mCRC32;
   stream >> mCompressedSize;
   stream >> mUncompressedSize;

   U16 fnLen, efLen, fcLen;
   stream >> fnLen;
   stream >> efLen;
   stream >> fcLen;

   stream >> mDiskNumStart;

   stream >> mInternalFileAttr;
   stream >> mExternalFileAttr;

   stream >> mLocalHeadOffset;

   char *fn = new char[fnLen + 1];
   stream.read(fn, fnLen);
   fn[fnLen] = 0;

   SAFE_DELETE_ARRAY(mFilename);
   mFilename = fn;

   // [tom, 10/28/2006] We currently only need the extra fields when we want to
   // open the file, so we won't bother reading them here. This avoids keeping
   // them in memory twice.

   //readExtraFields(stream, efLen);
   U32 pos = stream.tellg();
   stream.seekg(0, (stream.beg + pos + efLen));
//   stream->setPosition(stream->getPosition() + efLen);

   fn = new char[fcLen + 1];
   stream.read(fn, fcLen);
   fn[fcLen] = 0;

   SAFE_DELETE_ARRAY(mFileComment);
   mFileComment = fn;

   // Sanity checks to make life easier elsewhere
   if(mCompressMethod != Stored && mUncompressedSize == 0 && mCompressedSize == 0)
      mCompressMethod = Stored;

   return true;
}

bool CentralDir::write(std::iostream &stream)
{
   mHeaderSig = mCentralDirSignature;
   stream << mHeaderSig;

   stream << mVersionMadeBy;
   stream << mExtractVer;
   stream << mFlags;
   stream << mCompressMethod;
   stream << mModTime;
   stream << mModDate;
   stream << mCRC32;
   stream << mCompressedSize;
   stream << mUncompressedSize;

   U16 fnLen = mFilename ? (U16)dStrlen(mFilename) : 0,
       efLen = 0,
       fcLen = mFileComment ? (U16)dStrlen(mFileComment) : 0;
   stream << fnLen;
   stream << efLen;
   stream << fcLen;

   stream << mDiskNumStart;

   stream << mInternalFileAttr;
   stream << mExternalFileAttr;

   stream << mLocalHeadOffset;

   if(fnLen)
      stream.write((char*)mFilename, fnLen);

   // FIXME [tom, 10/29/2006] Write extra fields here

   if(fcLen)
      stream.write((char*)mFileComment, fcLen);

   return true;
}

//////////////////////////////////////////////////////////////////////////

void CentralDir::setFileComment(const char *comment)
{
   SAFE_DELETE_ARRAY(mFileComment);
   mFileComment = new char [dStrlen(comment)+1];
   dStrcpy((char *)mFileComment, comment);
}

//////////////////////////////////////////////////////////////////////////
// EndOfCentralDir Class
//////////////////////////////////////////////////////////////////////////

EndOfCentralDir::EndOfCentralDir()
{
   mHeaderSig = mEOCDSignature;

   mDiskNum = 0;
   mStartCDDiskNum = 0;
   mNumEntriesInThisCD = 0;
   mTotalEntriesInCD = 0;
   mCDSize = 0;
   mCDOffset = 0;
   mCommentSize = 0;
   mZipComment = NULL;
}

EndOfCentralDir::~EndOfCentralDir()
{
   SAFE_DELETE_ARRAY(mZipComment);
}

//////////////////////////////////////////////////////////////////////////

bool EndOfCentralDir::read(std::iostream &stream)
{
   stream >> (mHeaderSig);
   if(mHeaderSig != mEOCDSignature)
      return false;

   stream >> mDiskNum;
   stream >> mStartCDDiskNum;
   stream >> mNumEntriesInThisCD;
   stream >> mTotalEntriesInCD;
   stream >> mCDSize;
   stream >> mCDOffset;

   stream >> mCommentSize;
   
   char *comment = new char[mCommentSize + 1];
   stream.read(comment, mCommentSize);
   comment[mCommentSize] = 0;

   SAFE_DELETE_ARRAY(mZipComment);
   mZipComment = comment;

   return true;
}

bool EndOfCentralDir::write(std::iostream &stream)
{
   stream << mHeaderSig;

   stream << mDiskNum;
   stream << mStartCDDiskNum;
   stream << mNumEntriesInThisCD;
   stream << mTotalEntriesInCD;
   stream << mCDSize;
   stream << mCDOffset;

   stream << mCommentSize;
   if(mZipComment && mCommentSize)
      stream.write(mZipComment, mCommentSize);

   return true;
}

//////////////////////////////////////////////////////////////////////////

// [tom, 10/19/2006] I know, i know ... this'll get rewritten.
// [tom, 1/23/2007] Maybe.

bool EndOfCentralDir::findInStream(std::iostream &stream)
{
   U32 initialPos = stream.tellg();
   stream.seekg(0, stream.end);
   U32 size = stream.tellg();
   stream.seekg(0, stream.beg + initialPos);
   U32 pos;
   if(size == 0)
      return false;

   stream.seekg(0, stream.end - mRecordSize);
   if(! stream.good())
      goto hell;

   U32 sig;
   stream >> sig;

   if(sig == mEOCDSignature)
   {
//      stream->setPosition(size - mRecordSize);
	   stream.seekg(0, stream.end - mRecordSize);
       return true;
   }

   // OK, so we couldn't find the EOCD where we expected it. The zip file
   // either has comments or isn't a zip file. We need to search the last
   // 64Kb of the file for the EOCD.

   pos = size > mEOCDSearchSize ? size - mEOCDSearchSize : 0;
   stream.seekg(0, stream.beg + pos);
   if(! stream.good())
      goto hell;

   while(pos < (size - 4))
   {
      stream >> sig;

      if(sig == mEOCDSignature)
      {
		  pos = stream.tellg();
//         stream->setPosition(pos);
         return true;
      }

      pos++;
	  stream.seekg(0, stream.beg + pos);
      if(! stream.good())
         goto hell;
   }

hell:
   stream.seekg(0, stream.beg);
   return false;
}

//////////////////////////////////////////////////////////////////////////

void EndOfCentralDir::setZipComment(U16 commentSize, const char *zipComment)
{
   SAFE_DELETE_ARRAY(mZipComment);
   mZipComment = new char [commentSize];
   dMemcpy((void *)mZipComment, zipComment, commentSize);
   mCommentSize = commentSize;
}

void EndOfCentralDir::setZipComment(const char *zipComment)
{
   setZipComment(dStrlen(zipComment), zipComment);
}

} // end namespace Zip
