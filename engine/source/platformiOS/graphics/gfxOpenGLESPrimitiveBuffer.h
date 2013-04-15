//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLESPrimitiveBuffer_H_
#define _GFXOpenGLESPrimitiveBuffer_H_

#include "graphics/OpenGL/gfxOpenGLPrimitiveBuffer.h"

/// This is a primitive buffer (index buffer to GL users) which uses VBOs.
class GFXOpenGLESPrimitiveBuffer : public GFXOpenGLPrimitiveBuffer
{
public:
	GFXOpenGLESPrimitiveBuffer(GFXDevice *device, U32 indexCount, U32 primitiveCount, GFXBufferType bufferType, U16 *indexBuffer, GFXPrimitive *primitiveBuffer = NULL);
	~GFXOpenGLESPrimitiveBuffer();

	virtual void lock(U32 indexStart, U32 indexEnd, void **indexPtr); ///< calls glMapBuffer, offets pointer by indexStart
	virtual void unlock(); ///< calls glUnmapBufferOES, unbinds the buffer
	virtual void prepare();  ///< binds the buffer
   virtual void finish(); ///< We're done with this buffer

	virtual void* getBuffer(); ///< returns NULL

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
private:
	/// Handle to our GL buffer object
	GLuint mBuffer;
   
   U8* mZombieCache;
};

#endif