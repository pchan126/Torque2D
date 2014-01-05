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
// IMPLIED, INCLUDING BUT NOT LIMITEFD TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLVertexBuffer_H_
#define _GFXOpenGLVertexBuffer_H_

#ifndef _GFXVERTEXBUFFER_H_
#include "graphics/gfxVertexBuffer.h"
#endif

#include "platform/platformGL.h"

class GFXOpenGLVertexBuffer : public GFXVertexBuffer 
{
public:
	GFXOpenGLVertexBuffer(   GFXDevice *device,
                          dsize_t vertexCount,
                          const GFXVertexFormat *vertexFormat,
                          dsize_t vertexSize,
                          GFXBufferType bufferType,
                          const GLvoid * vertexData = nullptr,
                          dsize_t indexCount = 0,
                         const GLvoid * indexData = nullptr)
   :GFXVertexBuffer( device, vertexCount, vertexFormat, vertexSize, bufferType) {} ;
	
   ~GFXOpenGLVertexBuffer() {};

	virtual void lock(dsize_t vertexStart, dsize_t vertexEnd, void **vertexPtr) = 0;
   virtual void set( void* data, dsize_t dataSize, dsize_t indexCount, void* indexData ) = 0;
	virtual void unlock() = 0;
	virtual void prepare() = 0;
   virtual void finish() = 0;

	virtual GLvoid* getBuffer() { return (GLvoid*)nullptr; };
   
   // GFXResource interface
   virtual void zombify() = 0;
   virtual void resurrect() = 0;
   
private:
   friend class GFXOpenGLDevice;
	/// GL buffer handle
	GLuint mBuffer, mVertexArrayObject;
   
   U8* mZombieCache;
};

#endif
