//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSVertexBuffer_H_
#define _GFXOpenGLES20iOSVertexBuffer_H_

#ifndef _GFXVERTEXBUFFER_H_
#include "graphics/gfxVertexBuffer.h"
#endif

#import <OpenGLES/ES2/glext.h>

/// This is a vertex buffer which uses GL_ARB_vertex_buffer_object.
class GFXOpenGLES20iOSVertexBuffer : public GFXVertexBuffer 
{
public:
	GFXOpenGLES20iOSVertexBuffer(   GFXDevice *device, 
                        U32 numVerts, 
                        const GFXVertexFormat *vertexFormat, 
                        U32 vertexSize, 
                        GFXBufferType bufferType,
                        const GLvoid * data = NULL,
                        U32 indexCount = 0,
                        const GLvoid *indexBuffer = NULL);

	~GFXOpenGLES20iOSVertexBuffer();

	virtual void lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr); ///< calls glMapBuffer and offsets the pointer by vertex start
    virtual void set( void* data, U32 dataSize, U32 indexCount, void* indexData );
	virtual void unlock(); ///< calls glUnmapBufferOES, unbinds the buffer
	virtual void prepare(); ///< Binds the buffer
   virtual void finish(); ///< We're done here

	GLvoid* getBuffer(); ///< returns NULL

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
private:
   friend class GFXOpenGLES20iOSDevice;
	/// GL buffer handle
	GLuint mBuffer, mVertexArrayObject, elementBufferName;
   U32 mTextureCount;
   
   U8* mZombieCache;
};

#endif
