//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#import "platformiOS/platformiOS.h"
#include "./gfxOpenGLESDevice.h"
#import <GLKit/GLKit.h>

#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGLESEnumTranslate.h"
#include "./gfxOpenGLESVertexBuffer.h"
#include "./gfxOpenGLESTextureTarget.h"
#include "./gfxOpenGLESTextureManager.h"
#include "./gfxOpenGLESTextureObject.h"
#include "./gfxOpenGLESCardProfiler.h"
#include "./gfxOpenGLESWindowTarget.h"

#include "./gfxOpenGLESShader.h"
#include "graphics/primBuilder.h"
#include "console/console.h"
#import <UIKit/UIKit.h>


GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGLESDevice::mCreateDeviceInstance(GFXOpenGLESDevice::createInstance);

GFXDevice *GFXOpenGLESDevice::createInstance( U32 adapterIndex )
{
    return new GFXOpenGLESDevice(adapterIndex);
}


static String _getRendererForDisplay(UIScreen* display)
{
    EAGLContext* ctx = [[EAGLContext alloc]
                        initWithAPI:kEAGLRenderingAPIOpenGLES2];
    
    // Save the current context, just in case
    EAGLContext* currCtx = [EAGLContext currentContext];
    [EAGLContext setCurrentContext: ctx];
    
    // get the renderer string
    String ret((const char*)glGetString(GL_RENDERER));
    
    // Restore our old context, release the context and pixel format.
    [EAGLContext setCurrentContext: currCtx];
    return ret;
}


void GFXOpenGLESDevice::initGLState()
{
    // Currently targeting OpenGL ES 2.0
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGLESStateBlock, but if we change our GFXOpenGLESStateBlock strategy, this may
    // need to happen.
    
    // Deal with the card profiler here when we know we have a valid context.
    mCardProfiler = new GFXOpenGLESCardProfiler();
    mCardProfiler->init();

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&mMaxShaderTextures);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


//-----------------------------------------------------------------------------
// Matrix interface
GFXOpenGLESDevice::GFXOpenGLESDevice( U32 adapterIndex ) : GFXOpenGLDevice( adapterIndex ),
                    mAdapterIndex(adapterIndex),
                    mCurrentVB(NULL),
                    m_mCurrentView(true),
                    mContext(nil),
                    mPixelFormat(NULL),
                    mPixelShaderVersion(0.0f),
                    mMaxShaderTextures(2),
                    mClip(0, 0, 0, 0),
                    mTextureLoader(NULL)
{
    GFXGLESEnumTranslate::init();
    
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
    
//    m_WorldStackRef = GLKMatrixStackCreate(kCFAllocatorDefault);
//    m_ProjectionStackRef = GLKMatrixStackCreate(kCFAllocatorDefault);
    m_WorldStack.push_back(MatrixF(true));
    m_ProjectionStack.push_back(MatrixF(true));
    
    mDeviceName = "OpenGLES";
    mFullScreenOnly = false;
}


GFXOpenGLESDevice::~GFXOpenGLESDevice()
{
//    CFRelease(m_WorldStackRef);
//    CFRelease(m_ProjectionStackRef);
}


void GFXOpenGLESDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
    {
        AssertFatal(!mContext && !mPixelFormat, "_createInitialContextAndFormat - Already created initial context and format");
        
        mContext = [[EAGLContext alloc]
                    initWithAPI:kEAGLRenderingAPIOpenGLES2];

        [EAGLContext setCurrentContext:mContext];
        
        mTextureManager = new GFXOpenGLESTextureManager();
        
        initGLState();
        initGenericShaders();
        mInitialized = true;
        deviceInited();
    }
}


void GFXOpenGLESDevice::_handleTextureLoaded(GFXTexNotifyCode code)
{
    mTexturesDirty = true;
}

void GFXOpenGLESDevice::enumerateVideoModes()
{
    mVideoModes.clear();
    

    //    CGDirectDisplayID display = CGMainDisplayID();
//    
//    // Enumerate all available resolutions:
//    CFArrayRef modeArray = CGDisplayCopyAllDisplayModes( display, NULL );
//    CFArrayApplyFunction(modeArray, CFRangeMake(0,CFArrayGetCount(modeArray)), addVideoModeCallback, &mVideoModes);
}

void GFXOpenGLESDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
{
    GFXAdapter *toAdd;
    
    Vector<GFXVideoMode> videoModes;
    
	NSArray			*screens;
	UIScreen		*aScreen;
	UIScreenMode	*mode;
    
	screens = [UIScreen screens];
	uint32_t screenNum = 1;
	for (aScreen in screens)
    {
		NSArray *displayModes;
		
		NSLog(@"\tScreen %d\n", screenNum);
		
		displayModes = [aScreen availableModes];
		for (mode in displayModes)
        {
            NSLog(@"\tScreen mode %@\n", mode);
            GFXVideoMode toAdd;
            
            toAdd.resolution.x = mode.size.width;  
            toAdd.resolution.y = mode.size.height; 
            toAdd.bitDepth = 32; 
            toAdd.refreshRate = 30; 
            toAdd.fullScreen = true;
            
            videoModes.push_back_unique(toAdd);
		}
		
        toAdd = new GFXAdapter();
        toAdd->mType = OpenGLES;
        toAdd->mIndex = screenNum-1;
        toAdd->mCreateDeviceInstanceDelegate = mCreateDeviceInstance;
        String renderer = _getRendererForDisplay(aScreen);
        AssertFatal(dStrlen(renderer.c_str()) < GFXAdapter::MaxAdapterNameLen, "GFXGLDevice::enumerateAdapter - renderer name too long, increae the size of GFXAdapter::MaxAdapterNameLen (or use String!)");
        dStrncpy(toAdd->mName, renderer.c_str(), GFXAdapter::MaxAdapterNameLen);
        
        adapterList.push_back(toAdd);

        for (S32 j = videoModes.size() - 1 ; j >= 0 ; j--)
            toAdd->mAvailableModes.push_back(videoModes[j]);
        
		screenNum++;
	}
}



//inline void GFXOpenGLESDevice::pushWorldMatrix()
//{
////    mWorldMatrixDirty = true;
////    mStateDirty = true;
//    GLKMatrixStackPush(m_WorldStackRef);
//}
//
//inline void GFXOpenGLESDevice::popWorldMatrix()
//{
////    mWorldMatrixDirty = true;
////    mStateDirty = true;
//    GLKMatrixStackPop(m_WorldStackRef);
//}
//
//inline void GFXOpenGLESDevice::pushProjectionMatrix()
//{
//    //    mWorldMatrixDirty = true;
//    //    mStateDirty = true;
//    GLKMatrixStackPush(m_WorldStackRef);
//}
//
//inline void GFXOpenGLESDevice::popProjectionMatrix()
//{
//    //    mWorldMatrixDirty = true;
//    //    mStateDirty = true;
//    GLKMatrixStackPop(m_WorldStackRef);
//}

inline void GFXOpenGLESDevice::pushWorldMatrix()
{
    MatrixF newMatrix = m_WorldStack.last();
    m_WorldStack.push_back(newMatrix);
}

inline void GFXOpenGLESDevice::popWorldMatrix()
{
    m_WorldStack.pop_back();
}

inline void GFXOpenGLESDevice::pushProjectionMatrix()
{
    MatrixF newMatrix = m_ProjectionStack.last();
    m_ProjectionStack.push_back(newMatrix);
}

inline void GFXOpenGLESDevice::popProjectionMatrix()
{
    m_ProjectionStack.pop_back();
}


inline void GFXOpenGLESDevice::multWorld( const MatrixF &mat )
{
//    mWorldMatrixDirty = true;
//    mStateDirty = true;
//    GLKMatrixStackMultiplyMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArray(mat));
    MatrixF newMatrix = m_WorldStack.last();
    newMatrix*=mat;
    m_WorldStack.last() = newMatrix;
}


void GFXOpenGLESDevice::zombify()
{
}

void GFXOpenGLESDevice::resurrect()
{
}


GFXVertexBuffer* GFXOpenGLESDevice::findVolatileVBO(U32 vertexCount, const GFXVertexFormat *vertexFormat, U32 vertSize, void* vertexData, U32 indexSize, void* indexData)
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
    StrongRefPtr<GFXOpenGLESVertexBuffer> buf(new GFXOpenGLESVertexBuffer(GFX, vertexCount, vertexFormat, vertSize, GFXBufferTypeVolatile, vertexData, indexSize, indexData));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}

GFXVertexBuffer *GFXOpenGLESDevice::allocVertexBuffer(   U32 numVerts,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *vertexBuffer,
                                                  U32 indexCount,
                                                  void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(numVerts, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
    
    GFXOpenGLESVertexBuffer* buf = new GFXOpenGLESVertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


void GFXOpenGLESDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGLESDevice::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    if ( mCurrentVB )
        mCurrentVB->finish();
    
    mCurrentVB = static_cast<GFXOpenGLESVertexBuffer*>( buffer );
    if ( mCurrentVB )
        mCurrentVB->prepare();
}


//GFXCubemap* GFXOpenGLESDevice::createCubemap()
//{
//    GFXOpenGLESCubemap* cube = new GFXOpenGLESCubemap();
//    cube->registerResourceWithDevice(this);
//    return cube;
//};


void GFXOpenGLESDevice::clear(U32 flags, ColorI color, F32 z, U32 stencil)
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


void GFXOpenGLESDevice::updateStates(bool forceSetAll /*=false*/)
{
    PROFILE_SCOPE(GFXDevice_updateStates);
    
    if(forceSetAll)
    {
        bool rememberToEndScene = false;
        if(!canCurrentlyRender())
        {
            if (!beginScene())
            {
                AssertFatal(false, "GFXDevice::updateStates:  Unable to beginScene!");
            }
            rememberToEndScene = true;
        }
        
        setVertexDecl( mCurrVertexDecl );
        
        for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
        {
            setVertexStream( i, mCurrentVertexBuffer[i] );
        }
        
        /// Stateblocks
        if ( mNewStateBlock )
            setStateBlockInternal(mNewStateBlock, true);
        mCurrentStateBlock = mNewStateBlock;
        
        for(U32 i = 0; i < getNumSamplers(); i++)
        {
            switch (mTexType[i])
            {
                case GFXTDT_Normal :
                {
                    mCurrentTexture[i] = mNewTexture[i];
                    setTextureInternal(i, mCurrentTexture[i]);
                }
                    break;
//                case GFXTDT_Cube :
//                {
//                    mCurrentCubemap[i] = mNewCubemap[i];
//                    if (mCurrentCubemap[i])
//                        mCurrentCubemap[i]->setToTexUnit(i);
//                    else
//                        setTextureInternal(i, NULL);
//                }
//                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }
        
//        // Set our material
//        setLightMaterialInternal(mCurrentLightMaterial);
//        
//        // Set our lights
//        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
//        {
//            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
//        }
        
        _updateRenderTargets();
        
        if(rememberToEndScene)
            endScene();
        
        return;
    }
    
    if (!mStateDirty)
        return;
    
    // Normal update logic begins here.
    mStateDirty = false;
    
    // Update the vertex declaration.
    if ( mVertexDeclDirty )
    {
        setVertexDecl( mCurrVertexDecl );
        mVertexDeclDirty = false;
    }
    
    // Update the vertex buffers.
    for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
    {
        if ( mVertexBufferDirty[i] )
        {
            setVertexStream( i, mCurrentVertexBuffer[i] );
            mVertexBufferDirty[i] = false;
        }
        
        if ( mVertexBufferFrequencyDirty[i] )
        {
            mVertexBufferFrequencyDirty[i] = false;
        }
    }
    
    // NOTE: With state blocks, it's now important to update state before setting textures
    // some devices (e.g. OpenGL) set states on the texture and we need that information before
    // the texture is activated.
    if (mStateBlockDirty)
    {
        setStateBlockInternal(mNewStateBlock, false);
        mCurrentStateBlock = mNewStateBlock;
        mStateBlockDirty = false;
    }
    
    if( mTexturesDirty )
    {
        mTexturesDirty = false;
        for(U32 i = 0; i < getNumSamplers(); i++)
        {
            if(!mTextureDirty[i])
                continue;
            mTextureDirty[i] = false;
            
            switch (mTexType[i])
            {
                case GFXTDT_Normal :
                {
                    mCurrentTexture[i] = mNewTexture[i];
                    setTextureInternal(i, mCurrentTexture[i]);
                }
                    break;
//                case GFXTDT_Cube :
//                {
//                    mCurrentCubemap[i] = mNewCubemap[i];
//                    if (mCurrentCubemap[i])
//                        mCurrentCubemap[i]->setToTexUnit(i);
//                    else
//                        setTextureInternal(i, NULL);
//                }
//                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }
    }
    
//    // Set light material
//    if(mLightMaterialDirty)
//    {
//        setLightMaterialInternal(mCurrentLightMaterial);
//        mLightMaterialDirty = false;
//    }
//    
//    // Set our lights
//    if(mLightsDirty)
//    {
//        mLightsDirty = false;
//        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
//        {
//            if(!mLightDirty[i])
//                continue;
//            
//            mLightDirty[i] = false;
//            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
//        }
//    }
    
    _updateRenderTargets();
    
#ifdef TORQUE_DEBUG_RENDER
    doParanoidStateCheck();
#endif
}


void GFXOpenGLESDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject*texture)
{
    const GFXOpenGLESTextureObject *tex = static_cast<const GFXOpenGLESTextureObject*>(texture);
    glActiveTexture(GL_TEXTURE0 + textureUnit);
    if (tex)
    {
        // GFXOpenGLESTextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
        {
            glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
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


void GFXOpenGLESDevice::setMatrix( GFXMatrixType mtype, const MatrixF &mat )
{
//    switch (mtype)
//    {
//        case GFXMatrixWorld :
//        {
//            GLKMatrixStackLoadMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArrayAndTranspose(mat));
//        }
//            break;
//        case GFXMatrixView :
//        {
//            m_mCurrentView = mat;
//        }
//            break;
//        case GFXMatrixProjection :
//        {
//            GLKMatrixStackLoadMatrix4(m_ProjectionStackRef, GLKMatrix4MakeWithArrayAndTranspose(mat));
//        }
//            break;
//            // CodeReview - Add support for texture transform matrix types
//        default:
//            AssertFatal(false, "GFXOpenGLESDevice::setMatrix - Unknown matrix mode!");
//            return;
//    }
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            m_WorldStack.last() = mat;
        }
            break;
        case GFXMatrixView :
        {
            m_mCurrentView = mat;
        }
            break;
        case GFXMatrixProjection :
        {
            m_ProjectionStack.last() = mat;
        }
            break;
            // CodeReview - Add support for texture transform matrix types
        default:
            AssertFatal(false, "GFXOpenGL32Device::setMatrix - Unknown matrix mode!");
            return;
    }
}


const MatrixF GFXOpenGLESDevice::getMatrix( GFXMatrixType mtype )
{
//    MatrixF ret = MatrixF(true);
//    switch (mtype)
//    {
//        case GFXMatrixWorld :
//        {
//            MatrixF ret = GLKMatrixStackGetMatrix4(m_WorldStackRef);
//            return ret;
//        }
//            break;
//        case GFXMatrixView :
//        {
//            return m_mCurrentView;
//        }
//            break;
//        case GFXMatrixProjection :
//        {
//            MatrixF ret = GLKMatrixStackGetMatrix4(m_ProjectionStackRef);
//            return ret;
//        }
//            break;
//            // CodeReview - Add support for texture transform matrix types
//        default:
//            AssertFatal(false, "GFXOpenGLESDevice::setMatrix - Unknown matrix mode!");
//    }
//    return ret;
    MatrixF ret = MatrixF(true);
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            return m_WorldStack.last();
        }
            break;
        case GFXMatrixView :
        {
            return m_mCurrentView;
        }
            break;
        case GFXMatrixProjection :
        {
            return m_ProjectionStack.last();
        }
            break;
            // CodeReview - Add support for texture transform matrix types
        default:
            AssertFatal(false, "GFXOpenGLESDevice::setMatrix - Unknown matrix mode!");
    }
    return ret;
}


/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXOpenGLESDevice::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
    GFXOpenGLESStateBlockRef ret = new GFXOpenGLESStateBlock(desc);
    ret->setView(getMatrix(GFXMatrixView));
    ret->setModel(getMatrix(GFXMatrixWorld));
    ret->setProjection(getMatrix(GFXMatrixProjection));
    return GFXStateBlockRef(ret);
}

/// Activates a stateblock
void GFXOpenGLESDevice::setStateBlockInternal(GFXStateBlock* block, bool force)
{
    AssertFatal(dynamic_cast<GFXOpenGLESStateBlock*>(block), "GFXOpenGLESDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
    GFXOpenGLESStateBlock* glBlock = static_cast<GFXOpenGLESStateBlock*>(block);
    GFXOpenGLESStateBlock* glCurrent = static_cast<GFXOpenGLESStateBlock*>(mCurrentStateBlock.getPointer());
    if (force)
        glCurrent = NULL;
    
    glBlock->activate(glCurrent); // Doesn't use current yet.
    mCurrentGLStateBlock = glBlock;
}

////------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GFXWindowTarget *GFXOpenGLESDevice::allocWindowTarget(PlatformWindow *window)
{
    // Allocate the wintarget and create a new context.
    GFXOpenGLESWindowTarget *gwt = new GFXOpenGLESWindowTarget(window, this);
    gwt->mContext = this->mContext;
    
    // And return...
    return gwt;
}


GFXTextureTarget * GFXOpenGLESDevice::allocRenderToTextureTarget()
{
    GFXOpenGLESTextureTarget *targ = new GFXOpenGLESTextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}


void GFXOpenGLESDevice::initGenericShaders()
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

void GFXOpenGLESDevice::setupGenericShaders( GenericShaderType type )
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

            
            //        case GSTargetRestore:
            
        default:
            break;
    }
}

GFXOpenGLESShader* GFXOpenGLESDevice::createShader()
{
    GFXOpenGLESShader* shader = new GFXOpenGLESShader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGLESDevice::setShader( GFXOpenGLESShader *shader )
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

void GFXOpenGLESDevice::disableShaders()
{
    setShader(NULL);
    setShaderConstBuffer( NULL );
}

void GFXOpenGLESDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGLESShaderConstBuffer*>(buffer)->activate();
}

U32 GFXOpenGLESDevice::getNumSamplers() const
{
    return mMaxShaderTextures;
}

U32 GFXOpenGLESDevice::getNumRenderTargets() const
{
    return 1;
}


void GFXOpenGLESDevice::_updateRenderTargets()
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
        
        GFXOpenGLESTextureTarget *tex = dynamic_cast<GFXOpenGLESTextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGLESWindowTarget *win = dynamic_cast<GFXOpenGLESWindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != NULL,
                        "GFXOpenGLESDevice::_updateRenderTargets() - invalid target subclass passed!" );
            
            win->makeActive();
            
            if( win->mContext != static_cast<GFXOpenGLESDevice*>(GFX)->mContext )
            {
                mRTDirty = false;
                GFX->updateStates(true);
            }
        }
        
        mRTDirty = false;
    }
    
    if ( mViewportDirty )
    {
        glViewport( mViewport.point.x, mViewport.point.y, mViewport.extent.x, mViewport.extent.y );
        mViewportDirty = false;
    }
}


GFXFormat GFXOpenGLESDevice::selectSupportedFormat(   GFXTextureProfile* profile, 
                                               const Vector<GFXFormat>& formats, 
                                               bool texture, 
                                               bool mustblend,
                                               bool mustfilter )
{
    for(U32 i = 0; i < formats.size(); i++)
    {
        // Single channel textures are not supported by FBOs.
        if(profile->testFlag(GFXTextureProfile::RenderTarget) && (formats[i] == GFXFormatA8 || formats[i] == GFXFormatL8 || formats[i] == GFXFormatL16))
            continue;
        if(GFXGLTextureInternalFormat[formats[i]] == GL_ZERO)
            continue;
        
        return formats[i];
    }
    
    return GFXFormatR8G8B8A8;
}

//
// Register this device with GFXInit
//
class GFXOpenGLESRegisterDevice
{
public:
    GFXOpenGLESRegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGLESDevice::enumerateAdapters);
    }
};

static GFXOpenGLESRegisterDevice pGLRegisterDevice;

