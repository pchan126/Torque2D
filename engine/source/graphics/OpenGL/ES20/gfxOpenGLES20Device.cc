//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "./GFXOpenGLES20Device.h"

//#include "graphics/gfxDrawUtil.h"
//#include "graphics/gfxInit.h"
//
//#include "./GFXOpenGLES20EnumTranslate.h"
#include "./GFXOpenGLES20VertexBuffer.h"
//#include "./GFXOpenGLES20TextureTarget.h"
//#include "./GFXOpenGLES20TextureManager.h"
#include "./GFXOpenGLES20TextureObject.h"
//#include "./GFXOpenGLES20CardProfiler.h"
//#include "./GFXOpenGLES20WindowTarget.h"
//
//#include "./GFXOpenGLES20Shader.h"
//#include "graphics/primBuilder.h"
//#include "console/console.h"

//-----------------------------------------------------------------------------
GFXOpenGLES20Device::GFXOpenGLES20Device( U32 adapterIndex ) : GFXOpenGLDevice( adapterIndex ),
mAdapterIndex(adapterIndex),
mCurrentVB(NULL)
{
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
}


GFXOpenGLES20Device::~GFXOpenGLES20Device()
{
}



GFXVertexBuffer* GFXOpenGLES20Device::findVolatileVBO(U32 vertexCount, const GFXVertexFormat *vertexFormat, U32 vertSize, void* vertexData, U32 indexSize, void* indexData)
{
    for(U32 i = 0; i < mVolatileVBs.size(); i++)
        if (  mVolatileVBs[i]->mVertexCount >= vertexCount &&
            mVolatileVBs[i]->mVertexFormat.isEqual( *vertexFormat ) &&
            mVolatileVBs[i]->mVertexSize == vertSize &&
            mVolatileVBs[i]->getRefCount() == 1 )
        {
            mVolatileVBs[i].getPointer()->set(vertexData, vertexCount*vertSize, indexSize, indexData);
            return mVolatileVBs[i];
        }
    
    // No existing VB, so create one
    StrongRefPtr<GFXOpenGLES20VertexBuffer> buf(new GFXOpenGLES20VertexBuffer(GFX, vertexCount, vertexFormat, vertSize, GFXBufferTypeVolatile, vertexData, indexSize, indexData));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}

GFXVertexBuffer *GFXOpenGLES20Device::allocVertexBuffer(   U32 numVerts,
                                                           const GFXVertexFormat *vertexFormat,
                                                           U32 vertSize,
                                                           GFXBufferType bufferType,
                                                           void *vertexBuffer,
                                                           U32 indexCount,
                                                           void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(numVerts, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
    
    GFXOpenGLES20VertexBuffer* buf = new GFXOpenGLES20VertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


