//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "./GFXOpenGLES20iOSVertexBuffer.h"

#include "./gfxOpenGLES20iOSDevice.h"
#include "./gfxOpenGLES20iOSEnumTranslate.h"
#include "./gfxOpenGLES20iOSUtils.h"
#import <GLKit/GLKit.h>

GFXOpenGLES20iOSVertexBuffer::GFXOpenGLES20iOSVertexBuffer(  GFXDevice *device, 
                                       dsize_t vertexCount,
                                       const GFXVertexFormat *vertexFormat, 
                                       dsize_t vertexSize,
                                       GFXBufferType bufferType,
                                       const GLvoid * data,
                                       dsize_t indexCount,
                                       const GLvoid *indexBuffer)
   :  GFXOpenGLVertexBuffer( device, vertexCount, vertexFormat, vertexSize, bufferType ),
      mZombieCache(nullptr),
      mIndexCount(indexCount),
      elementBufferName(0)
{
    GFXOpenGLES20iOSDevice *_device = dynamic_cast<GFXOpenGLES20iOSDevice*>(device);
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
        else if ( element.getSemantic() == GFXSemantic::COLOR )
        {
            glVertexAttribPointer(GLKVertexAttribColor, 4, GL_UNSIGNED_BYTE, GL_TRUE, (GLsizei)mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribColor);
            buffer += element.getSizeInBytes();
        }
        else if ( element.getSemantic() == GFXSemantic::SIZE )
        {
            glVertexAttribPointer(ATTRIB_POINTSIZE, 1, GL_FLOAT, GL_FALSE, (GLsizei)mVertexSize, buffer);
            glEnableVertexAttribArray(ATTRIB_POINTSIZE);
            buffer += element.getSizeInBytes();
        }
        else // Everything else is a texture coordinate.
        {
            glVertexAttribPointer(GLKVertexAttribTexCoord0+mTextureCount, 2, GL_FLOAT, GL_FALSE, (GLsizei)mVertexSize, buffer);
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

GFXOpenGLES20iOSVertexBuffer::~GFXOpenGLES20iOSVertexBuffer()
{
	// While heavy handed, this does delete the buffer and frees the associated memory.
    glDeleteBuffers(1, &mBuffer);
    glDeleteBuffers(1, &elementBufferName);
    glDeleteVertexArraysOES(1, &mVertexArrayObject);
   
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXOpenGLES20iOSVertexBuffer::lock( dsize_t vertexStart, dsize_t vertexEnd, void **vertexPtr )
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


void GFXOpenGLES20iOSVertexBuffer::set( void* data, dsize_t dataSize, dsize_t indexCount, void* indexData)
{
    GFXOpenGLES20iOSDevice *_device = dynamic_cast<GFXOpenGLES20iOSDevice*>(GFX);
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


void GFXOpenGLES20iOSVertexBuffer::unlock()
{
   PRESERVE_VERTEX_BUFFER();
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBufferOES(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGLES20iOSVertexBuffer::unlock - shouldn't fail!");

    lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}


void GFXOpenGLES20iOSVertexBuffer::prepare()
{
   glBindVertexArrayOES(mVertexArrayObject);
}

void GFXOpenGLES20iOSVertexBuffer::finish()
{
//   glBindVertexArrayOES(0);
}

void GFXOpenGLES20iOSVertexBuffer::zombify()
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

void GFXOpenGLES20iOSVertexBuffer::resurrect()
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
