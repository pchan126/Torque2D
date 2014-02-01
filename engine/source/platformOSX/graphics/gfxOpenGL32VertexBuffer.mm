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
#include "./gfxOpenGL32VertexBuffer.h"

#include "./gfxOpenGL32Device.h"
#include "./gfxOpenGL32EnumTranslate.h"
#include "./gfxOpenGL32Utils.h"
#include "platform/platformGL.h"

GFXOpenGL32VertexBuffer::GFXOpenGL32VertexBuffer(  GFXDevice *device,
                                       dsize_t vertexCount,
                                       const GFXVertexFormat *vertexFormat, 
                                       dsize_t vertexSize,
                                       GFXBufferType bufferType,
                                       const GLvoid *vertexBuffer,
                                       dsize_t indexCount,
                                       const GLvoid *indexBuffer)
   :  GFXOpenGLVertexBuffer( device, vertexCount, vertexFormat, vertexSize, bufferType ),
      mZombieCache(NULL),
      elementBufferName(0)
{
    glGenVertexArrays(1, &mVertexArrayObject);
    glBindVertexArray(mVertexArrayObject);

    glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, vertexBuffer, GFXGLBufferType[bufferType]);
    
    U8* buffer = (U8*)getBuffer();
    
    // Configure the attributes in the VAO.
    U32 texCoordIndex = 0;
    for ( U32 i=0; i < mVertexFormat.getElementCount(); i++ )
    {
        const GFXVertexElement &element = mVertexFormat.getElement( i );
        
        if ( element.getSemantic() == GFXSemantic::POSITION )
        {
            glVertexAttribPointer(GLKVertexAttribPosition, element.getSizeInBytes()/4, GL_FLOAT, GL_FALSE, (GLsizei)mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribPosition);
            buffer += element.getSizeInBytes();
        }
        else if ( element.getSemantic() == GFXSemantic::NORMAL )
        {
            glVertexAttribPointer(GLKVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, (GLsizei)mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribNormal);
            buffer += element.getSizeInBytes();
        }
        else if ( element.getSemantic() == GFXSemantic::COLOR)
        {
            glVertexAttribPointer(GLKVertexAttribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei)mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribColor);
            buffer += element.getSizeInBytes();
        }
        else // Everything else is a texture coordinate.
        {
            glVertexAttribPointer(GLKVertexAttribTexCoord0+texCoordIndex, 2, GL_FLOAT, GL_FALSE, (GLsizei)mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribTexCoord0+texCoordIndex);
            buffer += element.getSizeInBytes();
            ++texCoordIndex;
        }
    }

    if (indexCount)
    {
        // This also attaches the element array buffer to the VAO
        glGenBuffers(1, &elementBufferName);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferName);

        // Allocate and load vertex array element data into VBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(U16), indexBuffer, GL_STATIC_DRAW);
    }
}

GFXOpenGL32VertexBuffer::~GFXOpenGL32VertexBuffer()
{
	// While heavy handed, this d delete the buffer and frees the associated memory.
   glDeleteBuffers(1, &mBuffer);
    glDeleteBuffers(1, &elementBufferName);
    glDeleteVertexArrays(1, &mVertexArrayObject);
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXOpenGL32VertexBuffer::lock( dsize_t vertexStart, dsize_t vertexEnd, void **vertexPtr )
{
   PRESERVE_VERTEX_BUFFER();
	// Bind us, get a pointer into the buffer, then
	// offset it by vertexStart so we act like the D3D layer.
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * mVertexSize, NULL, GFXGLBufferType[mBufferType]);
	*vertexPtr = (void*)((U8*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) + (vertexStart * mVertexSize));
	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXOpenGL32VertexBuffer::set( void* data, dsize_t dataSize, dsize_t indexCount, void* indexBuffer)
{
    glBindVertexArray(mVertexArrayObject);
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, GFXGLBufferType[GFXBufferTypeVolatile]);

    if (indexCount)
    {
       if (elementBufferName == 0)
          glGenBuffers(1, &elementBufferName);

       // This also attaches the element array buffer to the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferName);
        
        // Allocate and load vertex array element data into VBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(U16), indexBuffer, GL_STATIC_DRAW);
    }
    else
    {
       glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

}


void GFXOpenGL32VertexBuffer::unlock()
{
   PRESERVE_VERTEX_BUFFER();
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBuffer(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGL32VertexBuffer::unlock - shouldn't fail!");

    lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}


void GFXOpenGL32VertexBuffer::prepare()
{
    glBindVertexArray(mVertexArrayObject);
}

void GFXOpenGL32VertexBuffer::finish()
{
    glBindVertexArray(0);
}


void GFXOpenGL32VertexBuffer::zombify()
{
   if(mZombieCache || !mBuffer)
      return;
      
   mZombieCache = new U8[mVertexCount * mVertexSize];
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glGetBufferSubData(GL_ARRAY_BUFFER, 0, mVertexCount * mVertexSize, mZombieCache);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &mBuffer);
   mBuffer = 0;
}

void GFXOpenGL32VertexBuffer::resurrect()
{
   if(!mZombieCache)
      return;
   
   glGenBuffers(1, &mBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ARRAY_BUFFER, mVertexCount * mVertexSize, mZombieCache, GFXGLBufferType[mBufferType]);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   
   delete[] mZombieCache;
   mZombieCache = NULL;
}
