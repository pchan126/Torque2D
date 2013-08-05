//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#import "platform/platform.h"
#import "platformOSX/platformOSX.h"
#include "./gfxOpenGL32Device.h"

#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGL32EnumTranslate.h"
#include "./gfxOpenGL32VertexBuffer.h"
#include "./gfxOpenGL32TextureTarget.h"
#include "./gfxOpenGL32TextureManager.h"
#include "./gfxOpenGL32Cubemap.h"
#include "./gfxOpenGL32CardProfiler.h"
#include "./gfxOpenGL32WindowTarget.h"
#import <QuartzCore/CoreImage.h>

GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGL32Device::mCreateDeviceInstance(GFXOpenGL32Device::createInstance);

GFXDevice *GFXOpenGL32Device::createInstance( U32 adapterIndex )
{
    return new GFXOpenGL32Device(adapterIndex);
}


void GFXOpenGL32Device::initGLState()
{
    // Currently targeting OpenGL 3.2 (Mac)
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGL32StateBlock, but if we change our GFXOpenGL32StateBlock strategy, this may
    // need to happen.
    
    // Deal with the card profiler here when we know we have a valid context.
    mCardProfiler = new GFXOpenGL32OSXCardProfiler();
    mCardProfiler->init();

    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


//-----------------------------------------------------------------------------
// Matrix interface
GFXOpenGL32Device::GFXOpenGL32Device( U32 adapterIndex )  : GFXOpenGLDevice( adapterIndex ),
                        mAdapterIndex(adapterIndex),
                        mCurrentVB(NULL),
                        mContext(nil),
                        mPixelFormat(NULL),
                        mPixelShaderVersion(0.0f),
                        mClip(0, 0, 0, 0),
                        mTextureLoader(NULL)
{
    GFXOpenGLEnumTranslate::init();

    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
}


GFXOpenGL32Device::~GFXOpenGL32Device()
{
   if(mInitialized)
   {
      glfwTerminate();
   }
}


void GFXOpenGL32Device::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
    {
       glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
       glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 2);
       glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
       glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
       glfwWindowHint(GLFW_DEPTH_BITS, 0);
       glfwWindowHint(GLFW_STENCIL_BITS, 0);
       
       GLFWWindow* mWindow = dynamic_cast<GLFWWindow*>(WindowManager->createWindow(this, mode));
       mContext = mWindow->getContext();
       mWindow->makeContextCurrent();
       
       CGLContextObj cglContext = (CGLContextObj)[mContext CGLContextObj];
       CGLPixelFormatObj cglPixelFormat = CGLGetPixelFormat(cglContext);

       NSDictionary *opts = @{ kCIContextWorkingColorSpace : [NSNull null] };
       mCIContext = [CIContext contextWithCGLContext: cglContext
                                                 pixelFormat: cglPixelFormat
                                                  colorSpace:nil
                                                     options:opts];
       
       mTextureManager = new GFXOpenGL32TextureManager(mContext);

       initGLState();
       initGenericShaders();
       mInitialized = true;
       deviceInited();
    }
}

void GFXOpenGL32Device::addVideoMode(GFXVideoMode toAdd)
{
    // Only add this resolution if it is not already in the list:
    mVideoModes.push_back_unique( toAdd );
}


void GFXOpenGL32Device::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
    GFXAdapter *toAdd;
    
    Vector<GFXVideoMode> videoModes;

   int monitorCount;
   GLFWmonitor** monitorArray = glfwGetMonitors(&monitorCount);
   for (U32 i = 0; i < monitorCount; i++)
   {
      toAdd = new GFXAdapter();
      toAdd->mType = OpenGL;
      toAdd->mIndex = i;
      toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;
      adapterList.push_back(toAdd);

      int videoModeCount;
      const GLFWvidmode* videoModes = glfwGetVideoModes(monitorArray[i], &videoModeCount);
      for (U32 j = 0; j < videoModeCount; j++)
      {
         GFXVideoMode toVMAdd;
         
         toVMAdd.resolution.x = videoModes[j].width;
         toVMAdd.resolution.y = videoModes[j].height;
         toVMAdd.refreshRate = videoModes[j].refreshRate;
         toVMAdd.fullScreen = false;
         toAdd->mAvailableModes.push_back(toVMAdd);
      }
   }
}

void GFXOpenGL32Device::enumerateVideoModes()
{
   mVideoModes.clear();
   
   int videoModeCount;
   const GLFWvidmode* videoModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &videoModeCount);
   for (U32 j = 0; j < videoModeCount; j++)
   {
      GFXVideoMode toVMAdd;
      
      toVMAdd.resolution.x = videoModes[j].width;
      toVMAdd.resolution.y = videoModes[j].height;
      toVMAdd.refreshRate = videoModes[j].refreshRate;
      toVMAdd.fullScreen = false;
      mVideoModes.push_back(toVMAdd);
   }
}

void GFXOpenGL32Device::zombify()
{
    mTextureManager->zombify();
    if(mCurrentVB)
        mCurrentVB->finish();
    //mVolatileVBs.clear();
    //mVolatilePBs.clear();
    GFXResource* walk = mResourceListHead;
    while(walk)
    {
        walk->zombify();
        walk = walk->getNextResource();
    }
}

void GFXOpenGL32Device::resurrect()
{
    GFXResource* walk = mResourceListHead;
    while(walk)
    {
        walk->resurrect();
        walk = walk->getNextResource();
    }
    if(mCurrentVB)
        mCurrentVB->prepare();
    mTextureManager->resurrect();
}


GFXVertexBuffer* GFXOpenGL32Device::findVolatileVBO(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize, void* data, U32 indexSize, void* indexData)
{
    for(U32 i = 0; i < mVolatileVBs.size(); i++)
        if (  mVolatileVBs[i]->mVertexCount >= numVerts &&
            mVolatileVBs[i]->mVertexFormat.isEqual( *vertexFormat ) &&
            mVolatileVBs[i]->mVertexSize == vertSize &&
            mVolatileVBs[i]->getRefCount() == 1 )
        {
            mVolatileVBs[i].getPointer()->set(data, numVerts*vertSize, indexSize, indexData);
            return mVolatileVBs[i];
        }
    
    // No existing VB, so create one
    StrongRefPtr<GFXOpenGL32VertexBuffer> buf(new GFXOpenGL32VertexBuffer(GFX, numVerts, vertexFormat, vertSize, GFXBufferTypeVolatile, data, indexSize, indexData));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}


GFXVertexBuffer *GFXOpenGL32Device::allocVertexBuffer(   U32 vertexCount,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *vertexBuffer,
                                                    U32 indexCount,
                                                    void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(vertexCount, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
   
    GFXOpenGL32VertexBuffer* buf = new GFXOpenGL32VertexBuffer( GFX, vertexCount, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


GFXCubemap* GFXOpenGL32Device::createCubemap()
{
    GFXOpenGL32Cubemap* cube = new GFXOpenGL32Cubemap();
    cube->registerResourceWithDevice(this);
    return cube;
};

void GFXOpenGL32Device::clear(U32 flags, ColorI color, F32 z, U32 stencil)
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
    glClearDepth(z);
    glClearStencil(stencil);
    glClearColor(c.red, c.green, c.blue, c.alpha);

    if(!zwrite)
        glDepthMask(false);
}


void GFXOpenGL32Device::setTextureInternal(U32 textureUnit, GFXTextureObject *texture)
{
    GFXOpenGL32TextureObject *tex = static_cast<GFXOpenGL32TextureObject*>(texture);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (tex)
    {
        // GFXOpenGL32TextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
        {
            glBindTexture(mActiveTextureType[textureUnit], 0);
        }
        mActiveTextureType[textureUnit] = tex->getBinding();
        tex->bind(textureUnit);
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
        glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
    glActiveTexture(GL_TEXTURE0);
}


////------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GFXWindowTarget *GFXOpenGL32Device::allocWindowTarget(PlatformWindow *window)
{
   GLFWWindow* thewindow = dynamic_cast<GLFWWindow*>(window);

   if (thewindow == NULL)
      return NULL;
   
    // Allocate the wintarget and create a new context.
    GFXOpenGL32WindowTarget *gwt = new GFXOpenGL32WindowTarget(thewindow, this);
    return gwt;
}


GFXTextureTarget * GFXOpenGL32Device::allocRenderToTextureTarget()
{
    GFXOpenGL32TextureTarget *targ = new GFXOpenGL32TextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}

void GFXOpenGL32Device::initGenericShaders()
{
    Vector<GFXShaderMacro> macros;
    char vertBuffer[1024];
    char fragBuffer[1024];
    //  #Color Shader
    
    const char* shaderDirectory = Con::getVariable("$GUI::shaderDirectory");
    Con::printf("loading shaders from %s", shaderDirectory);
    
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/C.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/C.fsh", shaderDirectory);

    mGenericShader[0] = createShader();
    mGenericShader[0]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[0] = mGenericShader[0]->allocConstBuffer();
    
    //  #Texture Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/simple.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/simple.fsh", shaderDirectory);
    
    mGenericShader[1] = createShader();
    mGenericShader[1]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[1] = mGenericShader[1]->allocConstBuffer();
    
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/point.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/point.fsh", shaderDirectory);
    
    mGenericShader[2] = createShader();
    mGenericShader[2]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[2] = mGenericShader[2]->allocConstBuffer();
    
    //    GFXShaderConstHandle* hand = mGenericShader[0]->getShaderConstHandle("$mvp_matrix");
    //  #Point Shader
    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/test.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/test.fsh", shaderDirectory);
    
    mGenericShader[3] = createShader();
    mGenericShader[3]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[3] = mGenericShader[3]->allocConstBuffer();

    dSprintf(vertBuffer, sizeof(vertBuffer), "%s/alpha.vsh", shaderDirectory);
    dSprintf(fragBuffer, sizeof(fragBuffer), "%s/alpha.fsh", shaderDirectory);
    
    mGenericShader[4] = createShader();
    mGenericShader[4]->init(String(vertBuffer), String(fragBuffer), 0, macros);
    mGenericShaderConst[4] = mGenericShader[4]->allocConstBuffer();
}


void GFXOpenGL32Device::setupGenericShaders( GenericShaderType type )
{
//    Con::printf("setupGenericShaders");

    MatrixF xform(GFX->getWorldMatrix());
    xform *= GFX->getViewMatrix();
    xform *= GFX->getProjectionMatrix();
    xform.transpose();
    
    switch (type) {
        case GSColor:
            setShader(mGenericShader[0]);
            setShaderConstBuffer( mGenericShaderConst[0] );
            mGenericShaderConst[0]->setSafe( mGenericShader[0]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSTexture:
        case GSModColorTexture:
        case GSAddColorTexture:
       case GSBatchTexture:
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
            //        case GSTargetRestore:
            
        default:
            break;
    }
}

GFXOpenGL32Shader* GFXOpenGL32Device::createShader()
{
    GFXOpenGL32Shader* shader = new GFXOpenGL32Shader();
    shader->registerResourceWithDevice( this );
    return shader;
}


void GFXOpenGL32Device::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGL32ShaderConstBuffer*>(buffer)->activate();
}


void GFXOpenGL32Device::_updateRenderTargets()
{
    if ( mRTDirty || mCurrentRT->isPendingState() )
    {
        if ( mRTDeactivate )
        {
            mRTDeactivate->deactivate();
            mRTDeactivate = NULL;
        }
        
        // NOTE: The render target changes is not really accurate
        // as the GFXTextureTarget supports MRT internally.  So when
        // we activate a GFXTarget it could result in multiple calls
        // to SetRenderTarget on the actual device.
        mDeviceStatistics.mRenderTargetChanges++;
       
        GFXOpenGL32TextureTarget *tex = dynamic_cast<GFXOpenGL32TextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGL32WindowTarget *win = dynamic_cast<GFXOpenGL32WindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != NULL,
                        "GFXOpenGL32Device::_updateRenderTargets() - invalid target subclass passed!" );

            win->makeActive();

            if( win->getContext() != static_cast<GFXOpenGL32Device*>(GFX)->mContext )
            {
                mRTDirty = false;
                GFX->updateStates(true);
            }
        }
        
        mRTDirty = false;
    }
    
    if ( mViewport != mNextViewport )
    {
        mViewport = mNextViewport;
        glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y );
    }
}

// special immediate function for drawing CIImages
void GFXOpenGL32Device::drawImage( CIImage* image, CGRect inRect, CGRect fromRect)
{
   updateStates(true);

//    CIContext *context = [[NSGraphicsContext currentContext] CIContext];

   CGLContextObj cglContext = (CGLContextObj)[[NSOpenGLContext currentContext] CGLContextObj];
   CGLPixelFormatObj cglPixelFormat = CGLGetPixelFormat(cglContext);
   
   NSDictionary *opts = @{ kCIContextWorkingColorSpace : [NSNull null] };
   mCIContext = [CIContext contextWithCGLContext: cglContext
                                     pixelFormat: cglPixelFormat
                                      colorSpace:nil
                                         options:opts];
   
//   [context drawImage:image inRect:inRect fromRect:fromRect];
   [mCIContext drawImage:image inRect:inRect fromRect:fromRect];

    glGetError();
}

//
// Register this device with GFXInit
//
class GFXOpenGL32RegisterDevice
{
public:
    GFXOpenGL32RegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGL32Device::enumerateAdapters);
    }
};

static GFXOpenGL32RegisterDevice pGLRegisterDevice;

void GFXOpenGL32Device::setFillMode(GFXFillMode fillMode) {
    if (mFillMode != fillMode)
    {
        mFillMode = fillMode;
        glPolygonMode(GL_FRONT_AND_BACK, GFXGLFillMode[mFillMode]);
    }
}

