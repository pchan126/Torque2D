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
#include "platform/platformGL.h"
#include "./gfxOpenGL33WinVertexBuffer.h"
#include "./gfxOpenGL33WinDevice.h"
#include "./gfxOpenGL33WinEnumTranslate.h"
#include "./gfxOpenGL33WinUtils.h"

GFXOpenGL33WinVertexBuffer::GFXOpenGL33WinVertexBuffer(  GFXDevice *device,
                                       U32 vertexCount,
                                       const GFXVertexFormat *vertexFormat, 
                                       U32 vertexSize, 
                                       GFXBufferType bufferType,
                                       const GLvoid *vertexBuffer,
                                       U32 indexCount,
                                       const GLvoid *indexBuffer)
   :  GFXVertexBuffer( device, vertexCount, vertexFormat, vertexSize, bufferType ),
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
            glVertexAttribPointer(GLKVertexAttribPosition, element.getSizeInBytes()/4, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribPosition);
            buffer += element.getSizeInBytes();
        }
        else if ( element.getSemantic() == GFXSemantic::NORMAL )
        {
            glVertexAttribPointer(GLKVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribNormal);
            buffer += element.getSizeInBytes();
        }
        else if ( element.getSemantic() == GFXSemantic::COLOR )
        {
            glVertexAttribPointer(GLKVertexAttribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribColor);
            buffer += element.getSizeInBytes();
        }
        else // Everything else is a texture coordinate.
        {
            glVertexAttribPointer(GLKVertexAttribTexCoord0+texCoordIndex, 2, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
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

GFXOpenGL33WinVertexBuffer::~GFXOpenGL33WinVertexBuffer()
{
	// While heavy handed, this d delete the buffer and frees the associated memory.
   glDeleteBuffers(1, &mBuffer);
    glDeleteBuffers(1, &elementBufferName);
    glDeleteVertexArrays(1, &mVertexArrayObject);
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXOpenGL33WinVertexBuffer::lock( U32 vertexStart, U32 vertexEnd, void **vertexPtr )
{
	// Bind us, get a pointer into the buffer, then
	// offset it by vertexStart so we act like the D3D layer.
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * mVertexSize, NULL, GFXGLBufferType[mBufferType]);
	*vertexPtr = (void*)((U8*)glMapBuffer(GL_ARRAY_BUFFER, GL_WRITE_ONLY) + (vertexStart * mVertexSize));
	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXOpenGL33WinVertexBuffer::set( void* data, U32 dataSize, U32 indexCount, void* indexBuffer)
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
		//if (elementBufferName != 0)
		//	glDeleteBuffers(1, &elementBufferName);
		glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
	}
}


void GFXOpenGL33WinVertexBuffer::unlock()
{
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBuffer(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGL33WinVertexBuffer::unlock - shouldn't fail!");

    lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}


void GFXOpenGL33WinVertexBuffer::prepare()
{
    glBindVertexArray(mVertexArrayObject);
}

void GFXOpenGL33WinVertexBuffer::finish()
{
    glBindVertexArray(0);

}

GLvoid* GFXOpenGL33WinVertexBuffer::getBuffer()
{
	// NULL specifies no offset into the hardware buffer
	return (GLvoid*)NULL;
}

void GFXOpenGL33WinVertexBuffer::zombify()
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

void GFXOpenGL33WinVertexBuffer::resurrect()
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
