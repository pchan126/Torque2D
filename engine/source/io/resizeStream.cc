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

#include "resizeStream.h"

//ResizeFilterStream::ResizeFilterStream()
//{
   //
//}

ResizeFilterStream::~ResizeFilterStream()
{
//   detachStream();
}

bool ResizeFilterStream::attachStream(std::iostream* io_pSlaveStream)
{
   AssertFatal(io_pSlaveStream != NULL, "NULL Slave stream?");

   m_pStream     = io_pSlaveStream;
   m_startOffset = 0;
   m_pStream->seekg(0, m_pStream->end);
   m_streamLen   = (U32)m_pStream->tellg();
   m_currOffset  = 0;
//   setStatus(EOS);
   return true;
}

void ResizeFilterStream::detachStream()
{
   m_pStream     = NULL;
   m_startOffset = 0;
   m_streamLen   = 0;
   m_currOffset  = 0;
//   setStatus(Closed);
}

std::iostream* ResizeFilterStream::getStream()
{
   return m_pStream;
}

bool ResizeFilterStream::setStreamOffset(const U32 in_startOffset, const U32 in_streamLen)
{
   AssertFatal(m_pStream != NULL, "stream not attached!");
   if (m_pStream == NULL)
      return false;

   U32 start  = in_startOffset;
   U32 end    = in_startOffset + in_streamLen;
//   U32 actual = m_pStream->getStreamSize();
   m_pStream->seekg(0, m_pStream->end);
   auto actual = m_pStream->tellg();

   if (start >= actual || end > actual)
      return false;

   m_startOffset = start;
   m_streamLen   = in_streamLen;
   m_currOffset  = 0;

   //if (m_streamLen != 0)
   //   setStatus(Ok);
   //else
   //   setStatus(EOS);

   return true;
}

U32 ResizeFilterStream::getPosition()
{
   AssertFatal(m_pStream != NULL, "Error, stream not attached");
   if (m_pStream == NULL)
      return 0;

   return m_currOffset;
}

bool ResizeFilterStream::setPosition(const U32 in_newPosition)
{
   AssertFatal(m_pStream != NULL, "Error, stream not attached");
   if (m_pStream == NULL)
      return false;

   if (in_newPosition < m_streamLen) {
      m_currOffset = in_newPosition;
      return true;
   } else {
      m_currOffset = m_streamLen;
      return false;
   }
}

U32 ResizeFilterStream::getStreamSize()
{
   AssertFatal(m_pStream != NULL, "Error, stream not attached");

   return m_streamLen;
}

bool ResizeFilterStream::_read(char* out_pBuffer, const U32 in_numBytes)
{

}

