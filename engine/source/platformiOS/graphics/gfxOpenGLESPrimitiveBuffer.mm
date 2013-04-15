//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformiOS/graphics/gfxOpenGLESDevice.h"
#include "platformiOS/graphics/gfxOpenGLESPrimitiveBuffer.h"
#include "platformiOS/graphics/gfxOpenGLESEnumTranslate.h"

#import <OpenGLES/ES2/glext.h>
#include "platformiOS/graphics/gfxOpenGLESUtils.h"

GFXOpenGLESPrimitiveBuffer::GFXOpenGLESPrimitiveBuffer(GFXDevice *device, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType, U16 *indexBuffer, GFXPrimitive *primitiveBuffer) :
    GFXOpenGLPrimitiveBuffer(device, indexCount, primitiveCount, bufferType, indexBuffer, primitiveBuffer)
    , mZombieCache(NULL)
{
//   PRESERVE_INDEX_BUFFER();
	// Generate a buffer and allocate the needed memory
	glGenBuffers(1, &mBuffer);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
	glBufferData(GL_ELEMENT_ARRAY_BUFFER, indexCount * sizeof(U16), indexBuffer, GFXGLBufferType[bufferType]);
}

GFXOpenGLESPrimitiveBuffer::~GFXOpenGLESPrimitiveBuffer()
{
	// This is heavy handed, but it frees the buffer memory
	glDeleteBuffers(1, &mBuffer);
   
   if( mZombieCache )
      delete [] mZombieCache;
}


void GFXOpenGLESPrimitiveBuffer::lock(U32 indexStart, U32 indexEnd, void **indexPtr)
{
	// Preserve previous binding
//   PRESERVE_INDEX_BUFFER();
   
   // Bind ourselves and map
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(U16), NULL, GFXGLBufferType[mBufferType]);
   
   // Offset the buffer to indexStart
	*indexPtr = (void*)((U8*)glMapBufferOES(GL_ELEMENT_ARRAY_BUFFER, GL_WRITE_ONLY_OES) + (indexStart * sizeof(U16)));
}

void GFXOpenGLESPrimitiveBuffer::unlock()
{
	// Preserve previous binding
//   PRESERVE_INDEX_BUFFER();
   
   // Bind ourselves and unmap
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
	bool res = glUnmapBufferOES(GL_ELEMENT_ARRAY_BUFFER);
   AssertFatal(res, "GFXOpenGLESPrimitiveBuffer::unlock - shouldn't fail!");
}

void GFXOpenGLESPrimitiveBuffer::prepare()
{
	// Bind
	static_cast<GFXOpenGLESDevice*>(mDevice)->setPB(this);
	glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
}

void GFXOpenGLESPrimitiveBuffer::finish()
{
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
}

GLvoid* GFXOpenGLESPrimitiveBuffer::getBuffer()
{
	// NULL specifies no offset into the hardware buffer
	return (GLvoid*)NULL;
}

void GFXOpenGLESPrimitiveBuffer::zombify()
{
   if(mZombieCache)
      return;
      
   mZombieCache = new U8[mIndexCount * sizeof(U16)];
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
//   glGetBufferSubData(GL_ELEMENT_ARRAY_BUFFER, 0, mIndexCount * sizeof(U16), mZombieCache);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   glDeleteBuffers(1, &mBuffer);
   mBuffer = 0;
}

void GFXOpenGLESPrimitiveBuffer::resurrect()
{
   if(!mZombieCache)
      return;
   
   glGenBuffers(1, &mBuffer);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, mBuffer);
   glBufferData(GL_ELEMENT_ARRAY_BUFFER, mIndexCount * sizeof(U16), mZombieCache, GFXGLBufferType[mBufferType]);
   glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
   
   delete[] mZombieCache;
   mZombieCache = NULL;
}
