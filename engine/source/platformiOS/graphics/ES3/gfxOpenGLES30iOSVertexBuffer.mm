//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "./GFXOpenGLES30iOSVertexBuffer.h"

#include "./gfxOpenGLES30iOSDevice.h"
#include "./gfxOpenGLES30iOSEnumTranslate.h"
#include "./gfxOpenGLES30iOSUtils.h"
#import <GLKit/GLKit.h>

GFXOpenGLES30iOSVertexBuffer::GFXOpenGLES30iOSVertexBuffer(  GFXDevice *device, 
                                       U32 vertexCount,
                                       const GFXVertexFormat *vertexFormat, 
                                       U32 vertexSize, 
                                       GFXBufferType bufferType,
                                       const GLvoid * data,
                                       U32 indexCount,
                                       const GLvoid *indexBuffer)
   :  GFXOpenGLVertexBuffer( device, vertexCount, vertexFormat, vertexSize, bufferType ),
      mZombieCache(nullptr),
      mIndexCount(indexCount),
      elementBufferName(0)
{
    GFXOpenGLES30iOSDevice *_device = dynamic_cast<GFXOpenGLES30iOSDevice*>(device);
    mIndexCount = indexCount;
    glGenVertexArraysOES(1,&mVertexArrayObject);
    _device->setVertexStream( 0, this );
   
    glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, data, GFXGLBufferType[bufferType]);
    
    U8* buffer = (U8*)getBuffer();
    
    // Configure the attributes in the VAO.
    mTextureCount = 0;
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
        else if ( element.getSemantic() == GFXSemantic::SIZE )
        {
            glVertexAttribPointer(ATTRIB_POINTSIZE, 1, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(ATTRIB_POINTSIZE);
            buffer += element.getSizeInBytes();
        }
        else // Everything else is a texture coordinate.
        {
            glVertexAttribPointer(GLKVertexAttribTexCoord0+mTextureCount, 2, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribTexCoord0+mTextureCount);
            buffer += element.getSizeInBytes();
            ++mTextureCount;
        }
    }
    
    if (indexCount)
    {
        // This also attaches the element array buffer to the VAO
        glGenBuffers(1, &elementBufferName);
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferName);
        
        // Allocate and load vertex array element data into VBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(U16), indexBuffer, GFXGLBufferType[bufferType]);
    }
}

GFXOpenGLES30iOSVertexBuffer::~GFXOpenGLES30iOSVertexBuffer()
{
	// While heavy handed, this does delete the buffer and frees the associated memory.
    glDeleteBuffers(1, &mBuffer);
    glDeleteBuffers(1, &elementBufferName);
    glDeleteVertexArraysOES(1, &mVertexArrayObject);
   
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXOpenGLES30iOSVertexBuffer::lock( U32 vertexStart, U32 vertexEnd, void **vertexPtr )
{
   PRESERVE_VERTEX_BUFFER();
	// Bind us, get a pointer into the buffer, then
	// offset it by vertexStart so we act like the D3D layer.
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, mVertexCount * mVertexSize, nullptr, GFXGLBufferType[mBufferType]);
	*vertexPtr = (void*)((U8*)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES) + (vertexStart * mVertexSize));
	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}


void GFXOpenGLES30iOSVertexBuffer::set( void* data, U32 dataSize, U32 indexCount, void* indexData)
{
    GFXOpenGLES30iOSDevice *_device = dynamic_cast<GFXOpenGLES30iOSDevice*>(GFX);
    _device->setVertexStream( 0, this );

   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, GFXGLBufferType[GFXBufferTypeVolatile]);

    if (indexCount)
    {
        if (elementBufferName == 0)
            glGenBuffers(1, &elementBufferName);

        // This also attaches the element array buffer to the VAO
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferName);
        
        // Allocate and load vertex array element data into VBO
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(U16), indexData, GFXGLBufferType[mBufferType]);
    }
    else
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }
}


void GFXOpenGLES30iOSVertexBuffer::unlock()
{
   PRESERVE_VERTEX_BUFFER();
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBufferOES(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGLES30iOSVertexBuffer::unlock - shouldn't fail!");

    lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}


void GFXOpenGLES30iOSVertexBuffer::prepare()
{
   glBindVertexArrayOES(mVertexArrayObject);
}

void GFXOpenGLES30iOSVertexBuffer::finish()
{
//   glBindVertexArrayOES(0);
}

void GFXOpenGLES30iOSVertexBuffer::zombify()
{
   if(mZombieCache || !mBuffer)
      return;
      
   mZombieCache = new U8[mVertexCount * mVertexSize];
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
//   glGetBufferSubData(GL_ARRAY_BUFFER, 0, mNumVerts * mVertexSize, mZombieCache);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &mBuffer);
   mBuffer = 0;
}

void GFXOpenGLES30iOSVertexBuffer::resurrect()
{
   if(!mZombieCache)
      return;
   
   glGenBuffers(1, &mBuffer);
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ARRAY_BUFFER, mVertexCount * mVertexSize, mZombieCache, GFXGLBufferType[mBufferType]);
   glBindBuffer(GL_ARRAY_BUFFER, 0);
   
   delete[] mZombieCache;
   mZombieCache = nullptr;
}
