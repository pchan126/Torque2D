//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES30iOSVertexBuffer_H_
#define _GFXOpenGLES30iOSVertexBuffer_H_

#include "graphics/OpenGL/gfxOpenGLVertexBuffer.h"

#import <OpenGLES/ES3/glext.h>

/// This is a vertex buffer which uses GL_ARB_vertex_buffer_object.
class GFXOpenGLES30iOSVertexBuffer : public GFXOpenGLVertexBuffer
{
public:
	GFXOpenGLES30iOSVertexBuffer(   GFXDevice *device, 
                        U32 numVerts, 
                        const GFXVertexFormat *vertexFormat, 
                        U32 vertexSize, 
                        GFXBufferType bufferType,
                        const GLvoid * data = nullptr,
                        U32 indexCount = 0,
                        const GLvoid *indexBuffer = nullptr);

	~GFXOpenGLES30iOSVertexBuffer();

	virtual void lock(U32 vertexStart, U32 vertexEnd, void **vertexPtr); ///< calls glMapBuffer and offsets the pointer by vertex start
    virtual void set( void* data, U32 dataSize, U32 indexCount, void* indexData );
	virtual void unlock(); ///< calls glUnmapBufferOES, unbinds the buffer
	virtual void prepare(); ///< Binds the buffer
   virtual void finish(); ///< We're done here

   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   
private:
   friend class GFXOpenGLES30iOSDevice;
	/// GL buffer handle
	GLuint mBuffer, mVertexArrayObject, elementBufferName;
   U32 mTextureCount;
   U32 mIndexCount;
   
   U8* mZombieCache;
};

#endif
