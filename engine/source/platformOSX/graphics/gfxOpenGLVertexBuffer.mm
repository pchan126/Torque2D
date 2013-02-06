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
#include "./gfxOpenGLVertexBuffer.h"

#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "./gfxOpenGLUtils.h"
#include "platform/platformGL.h"

GFXOpenGLVertexBuffer::GFXOpenGLVertexBuffer(  GFXDevice *device,
                                       U32 numVerts, 
                                       const GFXVertexFormat *vertexFormat, 
                                       U32 vertexSize, 
                                       GFXBufferType bufferType,
                                       const GLvoid * data )
   :  GFXVertexBuffer( device, numVerts, vertexFormat, vertexSize, bufferType ), 
      mZombieCache(NULL)
{
//   PRESERVE_VERTEX_BUFFER();
	// Generate a buffer and allocate the needed memory.
    // Create and bind the vertex array object.
    
    glGenVertexArrays(1,&mVertexArrayObject);
    glBindVertexArray(mVertexArrayObject);
    
    glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ARRAY_BUFFER, numVerts * vertexSize, data, GFXGLBufferType[bufferType]);
    
    U8* buffer = (U8*)getBuffer();
    
    // Configure the attributes in the VAO.
    U32 texCoordIndex = 0;
    for ( U32 i=0; i < mVertexFormat.getElementCount(); i++ )
    {
        const GFXVertexElement &element = mVertexFormat.getElement( i );
        
        if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::POSITION.c_str() ) == 0 )
        {
            glVertexAttribPointer(GLKVertexAttribPosition, element.getSizeInBytes()/4, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribPosition);
//            Con::printf("prepare POSITION: %i %i", element.getSizeInBytes(), mVertexSize);
            buffer += element.getSizeInBytes();
        }
        else if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::NORMAL.c_str() ) == 0 )
        {
            glVertexAttribPointer(GLKVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribNormal);
//            Con::printf("prepare NORMAL: %i %i", element.getSizeInBytes(), mVertexSize);
            buffer += element.getSizeInBytes();
        }
        else if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::COLOR.c_str() ) == 0 )
        {
            glVertexAttribPointer(GLKVertexAttribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribColor);
//            Con::printf("prepare COLOR: %i %i", element.getSizeInBytes(), mVertexSize);
            buffer += element.getSizeInBytes();
        }
        else // Everything else is a texture coordinate.
        {
            glVertexAttribPointer(GLKVertexAttribTexCoord0+texCoordIndex, 2, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribTexCoord0+texCoordIndex);
            buffer += element.getSizeInBytes();
//            if (texCoordIndex > 0)
//                Con::printf("prepare TEXCOORD+%i: %i %i", texCoordIndex, element.getSizeInBytes()/4, mVertexSize);
            ++texCoordIndex;
        }
    }
	glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);
}

GFXOpenGLVertexBuffer::~GFXOpenGLVertexBuffer()
{
	// While heavy handed, this d delete the buffer and frees the associated memory.
   glDeleteBuffers(1, &mBuffer);
   
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXOpenGLVertexBuffer::lock( U32 vertexStart, U32 vertexEnd, void **vertexPtr )
{
   PRESERVE_VERTEX_BUFFER();
	// Bind us, get a pointer into the buffer, then
	// offset it by vertexStart so we act like the D3D layer.
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, NULL, GFXGLBufferType[mBufferType]);
	*vertexPtr = (void*)((U8*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) + (vertexStart * mVertexSize));
	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXOpenGLVertexBuffer::set( void* data, U32 dataSize)
{
    PRESERVE_VERTEX_BUFFER();

    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, GFXGLBufferType[GFXBufferTypeVolatile]);

}


void GFXOpenGLVertexBuffer::unlock()
{
   PRESERVE_VERTEX_BUFFER();
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBuffer(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGLVertexBuffer::unlock - shouldn't fail!");

    lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}


void GFXOpenGLVertexBuffer::prepare()
{
    glBindVertexArray(mVertexArrayObject);
}

void GFXOpenGLVertexBuffer::finish()
{
    glBindVertexArray(0);

}

GLvoid* GFXOpenGLVertexBuffer::getBuffer()
{
	// NULL specifies no offset into the hardware buffer
	return (GLvoid*)NULL;
}

void GFXOpenGLVertexBuffer::zombify()
{
   if(mZombieCache || !mBuffer)
      return;
      
   mZombieCache = new U8[mNumVerts * mVertexSize];
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
//   glGetBufferSubData(GL_ARRAY_BUFFER, 0, mNumVerts * mVertexSize, mZombieCache);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &mBuffer);
   mBuffer = 0;
}

void GFXOpenGLVertexBuffer::resurrect()
{
   if(!mZombieCache)
      return;
   
   glGenBuffers(1, &mBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, mZombieCache, GFXGLBufferType[mBufferType]);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   
   delete[] mZombieCache;
   mZombieCache = NULL;
}
