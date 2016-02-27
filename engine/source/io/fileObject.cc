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

#include "fileObject.h"

#include "fileObject_ScriptBinding.h"

IMPLEMENT_CONOBJECT(FileObject);

bool FileObject::isEOF()
{
   return mCurPos == mBufferSize;
}

FileObject::FileObject()
{
   mFileBuffer = nullptr;
   mBufferSize = 0;
   mCurPos = 0;
}

FileObject::~FileObject()
{
   dFree(mFileBuffer);
}

void FileObject::close()
{
   stream.close();
   dFree(mFileBuffer);
   mFileBuffer = nullptr;
   mBufferSize = mCurPos = 0;
}

bool FileObject::openForWrite(const char *fileName, const bool append)
{
   char buffer[1024];
   Con::expandPath(buffer, sizeof(buffer), fileName);

   close();

   if(buffer == nullptr || *buffer == 0)
      return false;

   if ( !append )
      return( ResourceManager->openFileForWrite(stream, buffer) );

   // Use the WriteAppend flag so it doesn't clobber the existing file:
   if ( !ResourceManager->openFileForWrite(stream, buffer, File::WriteAppend) )
      return( false );

   return( true );
}

bool FileObject::openForRead(const char* /*fileName*/)
{
   AssertFatal(false, "Error, not yet implemented!");
   return false;
}

bool FileObject::readMemory(const char *fileName)
{
   StringTableEntry fileToOpen = nullptr;

   char buffer[1024];
    
   Con::expandPath( buffer, sizeof( buffer ), fileName );

   fileToOpen = StringTable->insert(buffer);

   close();
   std::iostream *s = ResourceManager->openStream(fileToOpen);
   if(!s)
      return false;
   mBufferSize = ResourceManager->getSize(fileToOpen);
   mFileBuffer = (U8 *) dMalloc(mBufferSize + 1);
   mFileBuffer[mBufferSize] = 0;
   s->read((char*)mFileBuffer, mBufferSize);
   ResourceManager->closeStream(s);
   mCurPos = 0;

   return true;
}

const U8 *FileObject::readLine()
{
   if(!mFileBuffer)
      return (U8 *) "";

   U32 tokPos = mCurPos;

   for(;;)
   {
      if(mCurPos == mBufferSize)
         break;

      if(mFileBuffer[mCurPos] == '\r')
      {
         mFileBuffer[mCurPos++] = 0;
         if(mFileBuffer[mCurPos] == '\n')
            mCurPos++;
         break;
      }

      if(mFileBuffer[mCurPos] == '\n')
      {
         mFileBuffer[mCurPos++] = 0;
         break;
      }

      mCurPos++;
   }

   return mFileBuffer + tokPos;
}

void FileObject::peekLine( U8* line, S32 length )
{
   if(!mFileBuffer)
   {
      line[0] = '\0';
      return;
   }

   // Copy the line into the buffer. We can't do this like readLine because
   // we can't modify the file buffer.
   S32 i = 0;
   U32 tokPos = mCurPos;
   while( ( tokPos != mBufferSize ) && ( mFileBuffer[tokPos] != '\r' ) && ( mFileBuffer[tokPos] != '\n' ) && ( i < ( length - 1 ) ) )
      line[i++] = mFileBuffer[tokPos++];

   line[i++] = '\0';

   //if( i == length )
      //Con::warnf( "FileObject::peekLine - The line contents could not fit in the buffer (size %d). Truncating.", length );
}

void FileObject::writeLine(const U8 *line)
{
   stream.write((char*)line, dStrlen((const char *)line));
   stream.write("\r\n", 2);
}

void FileObject::writeObject( SimObject* object, const U8* objectPrepend )
{
   if( objectPrepend == nullptr )
      stream.write("\r\n", 2);
   else
      stream.write((char*)objectPrepend, dStrlen((const char *)objectPrepend) );
   object->write( stream, 0 );
}
