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

#include "platform/platform.h"
#include "platformWin32/platformWin32.h"
#include "./gfxOpenGL33WinDevice.h"

#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGL33WinEnumTranslate.h"
#include "./gfxOpenGL33WinVertexBuffer.h"
#include "./gfxOpenGL33WinTextureTarget.h"
#include "./gfxOpenGL33WinTextureManager.h"
#include "./gfxOpenGL33WinTextureObject.h"
#include "./gfxOpenGL33WinCardProfiler.h"
#include "./gfxOpenGL33WinWindowTarget.h"

#include "./gfxOpenGL33WinShader.h"
#include "graphics/primBuilder.h"
#include "console/console.h"

GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGL33WinDevice::mCreateDeviceInstance(GFXOpenGL33WinDevice::createInstance);


// yonked from winWindow.cc
void CreatePixelFormat( PIXELFORMATDESCRIPTOR *pPFD, S32 colorBits, S32 depthBits, S32 stencilBits, bool stereo )
{
   PIXELFORMATDESCRIPTOR src =
   {
      sizeof(PIXELFORMATDESCRIPTOR),   // size of this pfd
      1,                      // version number
      PFD_DRAW_TO_WINDOW |    // support window
      PFD_SUPPORT_OPENGL |    // support OpenGL
      PFD_DOUBLEBUFFER,       // double buffered
      PFD_TYPE_RGBA,          // RGBA type
      colorBits,              // color depth
      0, 0, 0, 0, 0, 0,       // color bits ignored
      0,                      // no alpha buffer
      0,                      // shift bit ignored
      0,                      // no accumulation buffer
      0, 0, 0, 0,             // accum bits ignored
      depthBits,              // z-buffer
      stencilBits,            // stencil buffer
      0,                      // no auxiliary buffer
      PFD_MAIN_PLANE,         // main layer
      0,                      // reserved
      0, 0, 0                 // layer masks ignored
    };

   if ( stereo )
   {
      //ri.Printf( PRINT_ALL, "...attempting to use stereo\n" );
      src.dwFlags |= PFD_STEREO;
      //glConfig.stereoEnabled = true;
   }
   else
   {
      //glConfig.stereoEnabled = qfalse;
   }
   *pPFD = src;
}


GFXDevice *GFXOpenGL33WinDevice::createInstance( U32 adapterIndex )
{
    return new GFXOpenGL33WinDevice(adapterIndex);
}

void GFXOpenGL33WinDevice::initGLState()
{
    // Currently targeting OpenGL 3.2 (Mac)
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGL33WinStateBlock, but if we change our GFXOpenGL33WinStateBlock strategy, this may
    // need to happen.
    
    // Deal with the card profiler here when we know we have a valid context.
    mCardProfiler = new GFXOpenGLCardProfiler();
    mCardProfiler->init();

    glGetIntegerv(GL_MAX_TEXTURE_IMAGE_UNITS, (GLint*)&mMaxShaderTextures);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


//-----------------------------------------------------------------------------
// Matrix interface
GFXOpenGL33WinDevice::GFXOpenGL33WinDevice( U32 adapterIndex )  : GFXOpenGLDevice( adapterIndex ),
                        mAdapterIndex(adapterIndex),
                        mCurrentVB(NULL),
                        mContext(NULL),
                        mPixelFormat(NULL),
                        mPixelShaderVersion(0.0f),
                        mMaxShaderTextures(2),
                        mClip(0, 0, 0, 0)
{
    GFXOpenGLEnumTranslate::init();
   
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;

//	InitAPI();
}


GFXOpenGL33WinDevice::~GFXOpenGL33WinDevice()
{
	if (mInitialized)
		glfwTerminate();
}




void GFXOpenGL33WinDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
	{
		glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
		glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
		glfwWindowHint(GLFW_OPENGL_FORWARD_COMPAT, 1);
		glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);
#if defined(TORQUE_DEBUG)
		glfwWindowHint(GLFW_OPENGL_DEBUG_CONTEXT, GL_TRUE);
#endif
		GLFWWindow* mWindow = dynamic_cast<GLFWWindow*>(WindowManager->createWindow(this,mode));
		mContext = mWindow->getContext();
		mWindow->makeContextCurrent();
		glfwSwapInterval(1);

		glewExperimental = GL_TRUE;
		GLenum err = glewInit();
	    if (GLEW_OK != err)
		{
			Con::printf("glewInit error %s", glewGetErrorString(err));
		}
		Con::printf("Using GLEW %s", glewGetString(GLEW_VERSION));

        mTextureManager = new GFXOpenGL33WinTextureManager();
	    initGLState();
	    initGenericShaders();
		mInitialized = true;
		deviceInited();
	}
}

void GFXOpenGL33WinDevice::addVideoMode(GFXVideoMode toAdd)
{
    // Only add this resolution if it is not already in the list:
    mVideoModes.push_back_unique( toAdd );
}

void addVideoModeCallback( const void *value, void *context )
{
}

void GFXOpenGL33WinDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
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
		for (U32 j= 0; j< videoModeCount; j++)
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

void GFXOpenGL33WinDevice::enumerateVideoModes()
{
    mVideoModes.clear();

	int videoModeCount;
	const GLFWvidmode* videoModes = glfwGetVideoModes(glfwGetPrimaryMonitor(), &videoModeCount);
	for (U32 j= 0; j < videoModeCount; j++)
	{
		GFXVideoMode toVMAdd;
		toVMAdd.resolution.x = videoModes[j].width;
		toVMAdd.resolution.y = videoModes[j].height;
		toVMAdd.refreshRate = videoModes[j].refreshRate;
		toVMAdd.fullScreen = false;
		mVideoModes.push_back(toVMAdd);
	}
}

void GFXOpenGL33WinDevice::zombify()
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

void GFXOpenGL33WinDevice::resurrect()
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


GFXVertexBuffer* GFXOpenGL33WinDevice::findVolatileVBO(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize, void* data, U32 indexSize, void* indexData)
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
    StrongRefPtr<GFXOpenGL33WinVertexBuffer> buf(new GFXOpenGL33WinVertexBuffer(GFX, numVerts, vertexFormat, vertSize, GFXBufferTypeVolatile, data, indexSize, indexData));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}


GFXVertexBuffer *GFXOpenGL33WinDevice::allocVertexBuffer(   U32 vertexCount,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *vertexBuffer,
                                                    U32 indexCount,
                                                    void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(vertexCount, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
   
    GFXOpenGL33WinVertexBuffer* buf = new GFXOpenGL33WinVertexBuffer( GFX, vertexCount, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


void GFXOpenGL33WinDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGL33WinDevice::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    if ( mCurrentVB )
        mCurrentVB->finish();
    
    mCurrentVB = static_cast<GFXOpenGL33WinVertexBuffer*>( buffer );
    if ( mCurrentVB )
        mCurrentVB->prepare();
}

GFXCubemap* GFXOpenGL33WinDevice::createCubemap()
{
    //GFXOpenGLCubemap* cube = new GFXOpenGLCubemap();
    //cube->registerResourceWithDevice(this);
    //return cube;
	return NULL;
};

void GFXOpenGL33WinDevice::clear(U32 flags, ColorI color, F32 z, U32 stencil)
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


void GFXOpenGL33WinDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject *texture)
{
    const GFXOpenGL33WinTextureObject *tex = static_cast<const GFXOpenGL33WinTextureObject*>(texture);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (tex)
    {
        // GFXOpenGL33WinTextureObject::bind also handles applying the current sampler state.
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
GFXWindowTarget *GFXOpenGL33WinDevice::allocWindowTarget(PlatformWindow *window)
{
   if (window == NULL)
      return NULL;
   
    // Allocate the wintarget and create a new context.
    GFXOpenGL33WinWindowTarget *gwt = new GFXOpenGL33WinWindowTarget(window, this);
    return gwt;
}


GFXTextureTarget * GFXOpenGL33WinDevice::allocRenderToTextureTarget()
{
    GFXOpenGL33WinTextureTarget *targ = new GFXOpenGL33WinTextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}

void GFXOpenGL33WinDevice::initGenericShaders()
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


void GFXOpenGL33WinDevice::setupGenericShaders( GenericShaderType type )
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

GFXOpenGL33WinShader* GFXOpenGL33WinDevice::createShader()
{
    GFXOpenGL33WinShader* shader = new GFXOpenGL33WinShader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGL33WinDevice::setShader( GFXOpenGL33WinShader *shader )
{
    if ( shader )
    {
        if (shader != mpCurrentShader)
        {
            mpCurrentShader = shader;
            shader->useProgram();
        }
    }
    else
    {
        mpCurrentShader = NULL;
        glUseProgram(0);
    }
}

void GFXOpenGL33WinDevice::disableShaders()
{
    setShader(NULL);
    setShaderConstBuffer( NULL );
}

void GFXOpenGL33WinDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGL33WinShaderConstBuffer*>(buffer)->activate();
}

U32 GFXOpenGL33WinDevice::getNumSamplers() const
{
    return mMaxShaderTextures;
}

U32 GFXOpenGL33WinDevice::getNumRenderTargets() const
{
    return 1;
}


void GFXOpenGL33WinDevice::_updateRenderTargets()
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
        //      mDeviceStatistics.mRenderTargetChanges++;
        
        GFXOpenGL33WinTextureTarget *tex = dynamic_cast<GFXOpenGL33WinTextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGL33WinWindowTarget *win = dynamic_cast<GFXOpenGL33WinWindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != NULL,
                        "GFXOpenGL33WinDevice::_updateRenderTargets() - invalid target subclass passed!" );
            
            win->makeActive();
            
            //if( win->getContext() != *(static_cast<GFXOpenGL33WinDevice*>(GFX)->mContext ))
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

void GFXOpenGL33WinDevice::setFillMode(GFXFillMode fillMode) {
   if (mFillMode != fillMode)
   {
      mFillMode = fillMode;
      glPolygonMode(GL_FRONT_AND_BACK, GFXGLFillMode[mFillMode]);
   }
}
//
// Register this device with GFXInit
//
class GFXOpenGL33WinRegisterDevice
{
public:
    GFXOpenGL33WinRegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGL33WinDevice::enumerateAdapters);
    }
};

static GFXOpenGL33WinRegisterDevice pGLRegisterDevice;
