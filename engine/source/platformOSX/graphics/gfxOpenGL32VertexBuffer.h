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

#ifndef _GFXOpenGL32VertexBuffer_H_
#define _GFXOpenGL32VertexBuffer_H_

#include "graphics/OpenGL/gfxOpenGLVertexBuffer.h"
#include "platform/platformGL.h"

/// This is a vertex buffer which uses GL_ARB_vertex_buffer_object.
class GFXOpenGL32VertexBuffer : public GFXOpenGLVertexBuffer 
{
public:
	GFXOpenGL32VertexBuffer(   GFXDevice *device, 
                        U32 vertexCount,
                        const GFXVertexFormat *vertexFormat, 
                        U32 vertexSize, 
                        GFXBufferType bufferType,
                        const GLvoid * vertexData = NULL,
                        U32 indexCount = 0,
                        const GLvoid * indexData = NULL);

	~GFXOpenGL32VertexBuffer();

	virtual void lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr); ///< calls glMapBuffer and offsets the pointer by vertex start
    virtual void set( void* data, U32 dataSize, U32 indexCount = 0, void* indexData = NULL );
	virtual void unlock(); 
	virtual void prepare();
   virtual void finish();
   
   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
private:
   friend class GFXOpenGL32Device;
	/// GL buffer handle
	GLuint mBuffer, mVertexArrayObject, elementBufferName;
   
   U8* mZombieCache;
};

#endif
