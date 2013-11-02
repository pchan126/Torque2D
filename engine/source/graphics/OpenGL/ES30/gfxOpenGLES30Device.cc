//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "gfxOpenGLES30Device.h"
#include "gfxOpenGLES30TextureObject.h"

//-----------------------------------------------------------------------------
GFXOpenGLES30Device::GFXOpenGLES30Device( U32 adapterIndex ) : GFXOpenGLDevice( adapterIndex )
{
}


GFXOpenGLES30Device::~GFXOpenGLES30Device()
{
}

GFXShader* GFXOpenGLES30Device::createShader()
{
    GFXOpenGLES30Shader* shader = new GFXOpenGLES30Shader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGLES30Device::initGenericShaders()
{
    Vector<GFXShaderMacro> macros;
    char vertBuffer[1024];
    char fragBuffer[1024];
    //  #Color Shader
    
    const char* shaderDirectory = Con::getVariable("$GUI::shaderDirectory");
    Con::printf("loading shaders from %s", shaderDirectory);
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/C.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/C.fsh", shaderDirectory);
    
    mGenericShader[0] = dynamic_cast<GFXOpenGLES30Shader*>(createShader());
    mGenericShader[0]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[0] = mGenericShader[0]->allocConstBuffer();
    
    //  #Texture Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/simple.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/simple.fsh", shaderDirectory);
    
    mGenericShader[1] = dynamic_cast<GFXOpenGLES30Shader*>(createShader());
    mGenericShader[1]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[1] = mGenericShader[1]->allocConstBuffer();
    
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/point.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/point.fsh", shaderDirectory);
    
    mGenericShader[2] = dynamic_cast<GFXOpenGLES30Shader*>(createShader());
    mGenericShader[2]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[2] = mGenericShader[2]->allocConstBuffer();
    
    //    GFXShaderConstHandle* hand = mGenericShader[0]->getShaderConstHandle("$mvp_matrix");
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/test.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/test.fsh", shaderDirectory);
    
    mGenericShader[3] = dynamic_cast<GFXOpenGLES30Shader*>(createShader());
    mGenericShader[3]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[3] = mGenericShader[3]->allocConstBuffer();
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/alpha.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/alpha.fsh", shaderDirectory);
    
    mGenericShader[4] = dynamic_cast<GFXOpenGLES30Shader*>(createShader());
    mGenericShader[4]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[4] = mGenericShader[4]->allocConstBuffer();
}



void GFXOpenGLES30Device::setupGenericShaders( GenericShaderType type )
{
    MatrixF xform(GFX->getWorldMatrix());
    xform *= GFX->getViewMatrix();
    xform *= GFX->getProjectionMatrix();

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

void GFXOpenGLES30Device::clear(U32 flags, ColorI color, F32 z, U32 stencil)
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

void GFXOpenGLES30Device::setFillMode(GFXFillMode fillMode) {
    if (mFillMode != fillMode)
    {
        mFillMode = fillMode;
//        glPolygonMode(GL_FRONT_AND_BACK, GFXGLFillMode[mFillMode]);
    }
}


//GFXVertexBuffer* GFXOpenGLES30Device::findVolatileVBO(U32 vertexCount, const GFXVertexFormat *vertexFormat, U32 vertSize, void* vertexData, U32 indexSize, void* indexData)
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
//    StrongRefPtr<GFXOpenGLES30VertexBuffer> buf(new GFXOpenGLES30VertexBuffer(GFX, vertexCount, vertexFormat, vertSize, GFXBufferTypeVolatile, vertexData, indexSize, indexData));
//    buf->registerResourceWithDevice(this);
//    mVolatileVBs.push_back(buf);
//    return buf.getPointer();
//}
//
//GFXVertexBuffer *GFXOpenGLES30Device::allocVertexBuffer(   U32 numVerts,
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
//    GFXOpenGLES30VertexBuffer* buf = new GFXOpenGLES30VertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
//    buf->registerResourceWithDevice(this);
//    return buf;
//}


