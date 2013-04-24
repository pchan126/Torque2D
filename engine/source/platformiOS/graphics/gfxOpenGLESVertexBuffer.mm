//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platformiOS/graphics/gfxOpenGLESVertexBuffer.h"

#include "platformiOS/graphics/gfxOpenGLESDevice.h"
#include "platformiOS/graphics/gfxOpenGLESEnumTranslate.h"
#include "platformiOS/graphics/gfxOpenGLESUtils.h"
#import <GLKit/GLKit.h>

GFXOpenGLESVertexBuffer::GFXOpenGLESVertexBuffer(  GFXDevice *device, 
                                       U32 vertexCount,
                                       const GFXVertexFormat *vertexFormat, 
                                       U32 vertexSize, 
                                       GFXBufferType bufferType,
                                       const GLvoid * data,
                                       U32 indexCount,
                                       const GLvoid *indexBuffer)
   :  GFXVertexBuffer( device, vertexCount, vertexFormat, vertexSize, bufferType ),
      mZombieCache(NULL)
{
    glGenVertexArraysOES(1,&mVertexArrayObject);
    glBindVertexArrayOES(mVertexArrayObject);
    
    glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ARRAY_BUFFER, vertexCount * vertexSize, data, GFXGLBufferType[bufferType]);
    
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
            buffer += element.getSizeInBytes();
        }
        else if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::NORMAL.c_str() ) == 0 )
        {
            glVertexAttribPointer(GLKVertexAttribNormal, 3, GL_FLOAT, GL_FALSE, mVertexSize, buffer);
            glEnableVertexAttribArray(GLKVertexAttribNormal);
            buffer += element.getSizeInBytes();
        }
        else if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::COLOR.c_str() ) == 0 )
        {
            glVertexAttribPointer(GLKVertexAttribColor, 4, GL_FLOAT, GL_TRUE, mVertexSize, buffer);
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
    // This also attaches the element array buffer to the VAO
    glGenBuffers(1, &elementBufferName);
    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, elementBufferName);
    
    // Allocate and load vertex array element data into VBO
    glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount*sizeof(U16), indexBuffer, GL_STATIC_DRAW);
}

GFXOpenGLESVertexBuffer::~GFXOpenGLESVertexBuffer()
{
	// While heavy handed, this does delete the buffer and frees the associated memory.
    glDeleteBuffers(1, &mBuffer);
    glDeleteBuffers(1, &elementBufferName);
    glDeleteVertexArraysOES(1, &mVertexArrayObject);
   
   if( mZombieCache )
      delete [] mZombieCache;
}

void GFXOpenGLESVertexBuffer::lock( U32 vertexStart, U32 vertexEnd, void **vertexPtr )
{
   PRESERVE_VERTEX_BUFFER();
	// Bind us, get a pointer into the buffer, then
	// offset it by vertexStart so we act like the D3D layer.
	glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, mNumVerts * mVertexSize, NULL, GFXGLBufferType[mBufferType]);
	*vertexPtr = (void*)((U8*)glMapBufferOES(GL_ARRAY_BUFFER, GL_WRITE_ONLY_OES) + (vertexStart * mVertexSize));
	lockedVertexStart = vertexStart;
	lockedVertexEnd   = vertexEnd;
}

void GFXOpenGLESVertexBuffer::set( void* data, U32 dataSize)
{
    PRESERVE_VERTEX_BUFFER();
    glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
    glBufferData(GL_ARRAY_BUFFER, dataSize, data, GFXGLBufferType[GFXBufferTypeVolatile]);
}


void GFXOpenGLESVertexBuffer::unlock()
{
   PRESERVE_VERTEX_BUFFER();
	// Unmap the buffer and bind 0 to GL_ARRAY_BUFFER
   glBindBuffer(GL_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBufferOES(GL_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGLESVertexBuffer::unlock - shouldn't fail!");

    lockedVertexStart = 0;
	lockedVertexEnd   = 0;
}


void GFXOpenGLESVertexBuffer::prepare()
{
    glBindVertexArrayOES(mVertexArrayObject);

//    U32 texCoordIndex = 0;
//    for ( U32 i=0; i < mVertexFormat.getElementCount(); i++ )
//    {
//        const GFXVertexElement &element = mVertexFormat.getElement( i );
//        
//        if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::POSITION.c_str() ) == 0 )
//        {
//            glEnableVertexAttribArray(ATTRIB_POSITION);
//        }
//        else if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::NORMAL.c_str() ) == 0 )
//        {
//            glEnableVertexAttribArray(ATTRIB_NORMAL);
//        }
//        else if ( dStrcmp (element.getSemantic().c_str(), GFXSemantic::COLOR.c_str() ) == 0 )
//        {
//            glEnableVertexAttribArray(ATTRIB_COLOR);
//        }
//        else // Everything else is a texture coordinate.
//        {
//            glEnableVertexAttribArray(ATTRIB_TEXCOORD0+texCoordIndex);
//            ++texCoordIndex;
//        }
//    }
    
}

void GFXOpenGLESVertexBuffer::finish()
{
    glBindVertexArrayOES(0);

}

GLvoid* GFXOpenGLESVertexBuffer::getBuffer()
{
	// NULL specifies no offset into the hardware buffer
	return (GLvoid*)NULL;
}

void GFXOpenGLESVertexBuffer::zombify()
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

void GFXOpenGLESVertexBuffer::resurrect()
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
