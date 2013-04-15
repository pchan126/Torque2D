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

#ifndef _GFXOpenGLPrimitiveBuffer_H_
#define _GFXOpenGLPrimitiveBuffer_H_

#include "graphics/gfxPrimitiveBuffer.h"

class GFXDevice;

/// This is a primitive buffer (index buffer to GL users) which uses VBOs.
class GFXOpenGLPrimitiveBuffer : public GFXPrimitiveBuffer
{
public:
    GFXOpenGLPrimitiveBuffer(GFXDevice *device, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType, U16 *indexBuffer, GFXPrimitive *primitiveBuffer) :
    GFXPrimitiveBuffer(device, indexCount, primitiveCount, bufferType, primitiveBuffer)
    { };
	~GFXOpenGLPrimitiveBuffer() {};

	virtual void lock(U32 indexStart, U32 indexEnd, void **indexPtr) = 0;
	virtual void unlock() = 0;
	virtual void prepare() = 0;
    virtual void finish() = 0;

	virtual void* getBuffer() = 0;

   // GFXResource interface
   virtual void zombify() = 0;
   virtual void resurrect() = 0;
};

#endif