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
                        dsize_t numVerts,
                        const GFXVertexFormat *vertexFormat, 
                        dsize_t vertexSize,
                        GFXBufferType bufferType,
                        const GLvoid * data = nullptr,
                        dsize_t indexCount = 0,
                        const GLvoid *indexBuffer = nullptr);

	~GFXOpenGLES30iOSVertexBuffer();

	virtual void lock(dsize_t vertexStart, dsize_t vertexEnd, void **vertexPtr); ///< calls glMapBuffer and offsets the pointer by vertex start
    virtual void set( void* data, dsize_t dataSize, dsize_t indexCount, void* indexData );
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
   dsize_t mIndexCount;
   
   U8* mZombieCache;
};

#endif
