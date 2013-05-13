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
//#include "./GFXOpenGLES20VertexBuffer.h"
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
GFXOpenGLES20Device::GFXOpenGLES20Device( U32 adapterIndex ) : GFXOpenGLDevice( adapterIndex )//,
//mAdapterIndex(adapterIndex) ///,
//mCurrentVB(NULL)
{
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
}


GFXOpenGLES20Device::~GFXOpenGLES20Device()
{
}

GFXShader* GFXOpenGLES20Device::createShader()
{
    GFXOpenGLES20Shader* shader = new GFXOpenGLES20Shader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGLES20Device::initGenericShaders()
{
    Vector<GFXShaderMacro> macros;
    char vertBuffer[1024];
    char fragBuffer[1024];
    //  #Color Shader
    
    const char* shaderDirectory = Con::getVariable("$GUI::shaderDirectory");
    Con::printf("loading shaders from %s", shaderDirectory);
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/C.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/C.fsh", shaderDirectory);
    
    mGenericShader[0] = dynamic_cast<GFXOpenGLES20Shader*>(createShader());
    mGenericShader[0]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[0] = mGenericShader[0]->allocConstBuffer();
    
    //  #Texture Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/simple.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/simple.fsh", shaderDirectory);
    
    mGenericShader[1] = dynamic_cast<GFXOpenGLES20Shader*>(createShader());
    mGenericShader[1]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[1] = mGenericShader[1]->allocConstBuffer();
    
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/point.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/point.fsh", shaderDirectory);
    
    mGenericShader[2] = dynamic_cast<GFXOpenGLES20Shader*>(createShader());
    mGenericShader[2]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[2] = mGenericShader[2]->allocConstBuffer();
    
    //    GFXShaderConstHandle* hand = mGenericShader[0]->getShaderConstHandle("$mvp_matrix");
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/test.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/test.fsh", shaderDirectory);
    
    mGenericShader[3] = dynamic_cast<GFXOpenGLES20Shader*>(createShader());
    mGenericShader[3]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[3] = mGenericShader[3]->allocConstBuffer();
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/alpha.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/alpha.fsh", shaderDirectory);
    
    mGenericShader[4] = dynamic_cast<GFXOpenGLES20Shader*>(createShader());
    mGenericShader[4]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[4] = mGenericShader[4]->allocConstBuffer();
}

void GFXOpenGLES20Device::setupGenericShaders( GenericShaderType type )
{
    MatrixF xform(GFX->getWorldMatrix());
    //    Con::printf("worldMatrix");
    //    Con::printf("%f %f %f %f", xform[0], xform[1], xform[2], xform[3]);
    //    Con::printf("%f %f %f %f", xform[4], xform[5], xform[6], xform[7]);
    //    Con::printf("%f %f %f %f", xform[8], xform[9], xform[10], xform[11]);
    //    Con::printf("%f %f %f %f", xform[12], xform[13], xform[14], xform[15]);
    xform *= GFX->getViewMatrix();
    //    Con::printf("viewMatrix");
    //    Con::printf("%f %f %f %f", xform[0], xform[1], xform[2], xform[3]);
    //    Con::printf("%f %f %f %f", xform[4], xform[5], xform[6], xform[7]);
    //    Con::printf("%f %f %f %f", xform[8], xform[9], xform[10], xform[11]);
    //    Con::printf("%f %f %f %f", xform[12], xform[13], xform[14], xform[15]);
    xform *= GFX->getProjectionMatrix();
    //    Con::printf("projectionMatrix");
    //    Con::printf("%f %f %f %f", xform[0], xform[1], xform[2], xform[3]);
    //    Con::printf("%f %f %f %f", xform[4], xform[5], xform[6], xform[7]);
    //    Con::printf("%f %f %f %f", xform[8], xform[9], xform[10], xform[11]);
    //    Con::printf("%f %f %f %f", xform[12], xform[13], xform[14], xform[15]);
    
//    xform.transpose();
//    projMatrix.transpose();
    
    switch (type) {
        case GSColor:
            setShader(mGenericShader[0]);
            setShaderConstBuffer( mGenericShaderConst[0] );
            mGenericShaderConst[0]->setSafe( mGenericShader[0]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSTexture:
        case GSModColorTexture:
        case GSAddColorTexture:
            setShader(mGenericShader[1]);
            setShaderConstBuffer( mGenericShaderConst[1] );
            mGenericShaderConst[1]->setSafe( mGenericShader[1]->getShaderConstHandle("$mvp_matrix"), xform );
            mGenericShaderConst[1]->setSafe( mGenericShader[1]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
        case GSPoint:
            setShader(mGenericShader[2]);
            setShaderConstBuffer( mGenericShaderConst[2] );
            mGenericShaderConst[2]->setSafe( mGenericShader[2]->getShaderConstHandle("$mvp_matrix"), xform );
            mGenericShaderConst[2]->setSafe( mGenericShader[2]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
        case GSTest:
            setShader(mGenericShader[3]);
            setShaderConstBuffer( mGenericShaderConst[3] );
            mGenericShaderConst[3]->setSafe( mGenericShader[3]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSAlphaTexture:
            setShader(mGenericShader[4]);
            setShaderConstBuffer( mGenericShaderConst[4] );
            mGenericShaderConst[4]->setSafe( mGenericShader[4]->getShaderConstHandle("$mvp_matrix"), xform );
            mGenericShaderConst[4]->setSafe( mGenericShader[4]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
            
        default:
            break;
    }
}

void GFXOpenGLES20Device::clear(U32 flags, ColorI color, F32 z, U32 stencil)
{
    // Make sure we have flushed our render target state.
    _updateRenderTargets();
    
    bool zwrite = true;
    //   if (mCurrentGLStateBlock)
    //   {
    //      zwrite = mCurrentGLStateBlock->getDesc().zWriteEnable;
    //   }
    
    glDepthMask(true);
    
    GLbitfield clearflags = 0;
    clearflags |= (flags & GFXClearTarget)   ? GL_COLOR_BUFFER_BIT : 0;
    clearflags |= (flags & GFXClearZBuffer)  ? GL_DEPTH_BUFFER_BIT : 0;
    clearflags |= (flags & GFXClearStencil)  ? GL_STENCIL_BUFFER_BIT : 0;
    
    glClear(clearflags);
    
    ColorF c = color;
    glClearDepthf(z);
    glClearStencil(stencil);
    glClearColor(c.red, c.green, c.blue, c.alpha);
    
    if(!zwrite)
        glDepthMask(false);
}

//GFXVertexBuffer* GFXOpenGLES20Device::findVolatileVBO(U32 vertexCount, const GFXVertexFormat *vertexFormat, U32 vertSize, void* vertexData, U32 indexSize, void* indexData)
//{
//    for(U32 i = 0; i < mVolatileVBs.size(); i++)
//        if (  mVolatileVBs[i]->mVertexCount >= vertexCount &&
//            mVolatileVBs[i]->mVertexFormat.isEqual( *vertexFormat ) &&
//            mVolatileVBs[i]->mVertexSize == vertSize &&
//            mVolatileVBs[i]->getRefCount() == 1 )
//        {
//            mVolatileVBs[i].getPointer()->set(vertexData, vertexCount*vertSize, indexSize, indexData);
//            return mVolatileVBs[i];
//        }
//    
//    // No existing VB, so create one
//    StrongRefPtr<GFXOpenGLES20VertexBuffer> buf(new GFXOpenGLES20VertexBuffer(GFX, vertexCount, vertexFormat, vertSize, GFXBufferTypeVolatile, vertexData, indexSize, indexData));
//    buf->registerResourceWithDevice(this);
//    mVolatileVBs.push_back(buf);
//    return buf.getPointer();
//}
//
//GFXVertexBuffer *GFXOpenGLES20Device::allocVertexBuffer(   U32 numVerts,
//                                                           const GFXVertexFormat *vertexFormat,
//                                                           U32 vertSize,
//                                                           GFXBufferType bufferType,
//                                                           void *vertexBuffer,
//                                                           U32 indexCount,
//                                                           void *indexBuffer)
//{
//    if(bufferType == GFXBufferTypeVolatile)
//        return findVolatileVBO(numVerts, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
//    
//    GFXOpenGLES20VertexBuffer* buf = new GFXOpenGLES20VertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
//    buf->registerResourceWithDevice(this);
//    return buf;
//}


