//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#import "platformiOS/platformiOS.h"
#include "./gfxOpenGLES20iOSDevice.h"
#import <GLKit/GLKit.h>

#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGLES20iOSEnumTranslate.h"
#include "./gfxOpenGLES20iOSVertexBuffer.h"
#include "./gfxOpenGLES20iOSTextureTarget.h"
#include "./gfxOpenGLES20iOSTextureManager.h"
#include "./gfxOpenGLES20iOSTextureObject.h"
#include "./gfxOpenGLES20iOSCardProfiler.h"
#include "./gfxOpenGLES20iOSWindowTarget.h"

#include "./gfxOpenGLES20iOSShader.h"
#include "graphics/primBuilder.h"
#include "console/console.h"
#import <UIKit/UIKit.h>


GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGLES20iOSDevice::mCreateDeviceInstance(GFXOpenGLES20iOSDevice::createInstance);

GFXDevice *GFXOpenGLES20iOSDevice::createInstance( U32 adapterIndex )
{
    return new GFXOpenGLES20iOSDevice(adapterIndex);
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


void GFXOpenGLES20iOSDevice::initGLState()
{
    // Currently targeting OpenGL ES 2.0
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGLES20iOSStateBlock, but if we change our GFXOpenGLES20iOSStateBlock strategy, this may
    // need to happen.
    
    // Deal with the card profiler here when we know we have a valid context.
    mCardProfiler = new GFXOpenGLES20iOSCardProfiler();
    mCardProfiler->init();

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&mMaxShaderTextures);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


//-----------------------------------------------------------------------------
// Matrix interface
GFXOpenGLES20iOSDevice::GFXOpenGLES20iOSDevice( U32 adapterIndex ) : GFXOpenGLES20Device( adapterIndex ),
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
    GFXGLES20iOSEnumTranslate::init();
    
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
    
//    m_WorldStackRef = GLKMatrixStackCreate(kCFAllocatorDefault);
//    m_ProjectionStackRef = GLKMatrixStackCreate(kCFAllocatorDefault);
    m_WorldStack.push_back(MatrixF(true));
    m_ProjectionStack.push_back(MatrixF(true));
}


GFXOpenGLES20iOSDevice::~GFXOpenGLES20iOSDevice()
{
//    CFRelease(m_WorldStackRef);
//    CFRelease(m_ProjectionStackRef);
}


void GFXOpenGLES20iOSDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
    {
        AssertFatal(!mContext && !mPixelFormat, "_createInitialContextAndFormat - Already created initial context and format");
        
        mContext = [[EAGLContext alloc]
                    initWithAPI:kEAGLRenderingAPIOpenGLES2];

        [EAGLContext setCurrentContext:mContext];
        
        mTextureManager = new GFXOpenGLES20iOSTextureManager();
        
        initGLState();
        initGenericShaders();
        mInitialized = true;
        deviceInited();
    }
}


void GFXOpenGLES20iOSDevice::_handleTextureLoaded(GFXTexNotifyCode code)
{
    mTexturesDirty = true;
}

void GFXOpenGLES20iOSDevice::enumerateVideoModes()
{
    mVideoModes.clear();
    

    //    CGDirectDisplayID display = CGMainDisplayID();
//    
//    // Enumerate all available resolutions:
//    CFArrayRef modeArray = CGDisplayCopyAllDisplayModes( display, NULL );
//    CFArrayApplyFunction(modeArray, CFRangeMake(0,CFArrayGetCount(modeArray)), addVideoModeCallback, &mVideoModes);
}

void GFXOpenGLES20iOSDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
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



//inline void GFXOpenGLES20iOSDevice::pushWorldMatrix()
//{
////    mWorldMatrixDirty = true;
////    mStateDirty = true;
//    GLKMatrixStackPush(m_WorldStackRef);
//}
//
//inline void GFXOpenGLES20iOSDevice::popWorldMatrix()
//{
////    mWorldMatrixDirty = true;
////    mStateDirty = true;
//    GLKMatrixStackPop(m_WorldStackRef);
//}
//
//inline void GFXOpenGLES20iOSDevice::pushProjectionMatrix()
//{
//    //    mWorldMatrixDirty = true;
//    //    mStateDirty = true;
//    GLKMatrixStackPush(m_WorldStackRef);
//}
//
//inline void GFXOpenGLES20iOSDevice::popProjectionMatrix()
//{
//    //    mWorldMatrixDirty = true;
//    //    mStateDirty = true;
//    GLKMatrixStackPop(m_WorldStackRef);
//}

inline void GFXOpenGLES20iOSDevice::pushWorldMatrix()
{
    MatrixF newMatrix = m_WorldStack.last();
    m_WorldStack.push_back(newMatrix);
}

inline void GFXOpenGLES20iOSDevice::popWorldMatrix()
{
    m_WorldStack.pop_back();
}

inline void GFXOpenGLES20iOSDevice::pushProjectionMatrix()
{
    MatrixF newMatrix = m_ProjectionStack.last();
    m_ProjectionStack.push_back(newMatrix);
}

inline void GFXOpenGLES20iOSDevice::popProjectionMatrix()
{
    m_ProjectionStack.pop_back();
}


inline void GFXOpenGLES20iOSDevice::multWorld( const MatrixF &mat )
{
//    mWorldMatrixDirty = true;
//    mStateDirty = true;
//    GLKMatrixStackMultiplyMatrix4(m_WorldStackRef, GLKMatrix4MakeWithArray(mat));
    MatrixF newMatrix = m_WorldStack.last();
    newMatrix*=mat;
    m_WorldStack.last() = newMatrix;
}


void GFXOpenGLES20iOSDevice::zombify()
{
}

void GFXOpenGLES20iOSDevice::resurrect()
{
}


GFXVertexBuffer* GFXOpenGLES20iOSDevice::findVolatileVBO(U32 vertexCount, const GFXVertexFormat *vertexFormat, U32 vertSize, void* vertexData, U32 indexSize, void* indexData)
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
    StrongRefPtr<GFXOpenGLES20iOSVertexBuffer> buf(new GFXOpenGLES20iOSVertexBuffer(GFX, vertexCount, vertexFormat, vertSize, GFXBufferTypeVolatile, vertexData, indexSize, indexData));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}

GFXVertexBuffer *GFXOpenGLES20iOSDevice::allocVertexBuffer(   U32 numVerts,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *vertexBuffer,
                                                  U32 indexCount,
                                                  void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(numVerts, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
    
    GFXOpenGLES20iOSVertexBuffer* buf = new GFXOpenGLES20iOSVertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


void GFXOpenGLES20iOSDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGLES20iOSDevice::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    if ( mCurrentVB )
        mCurrentVB->finish();
    
    mCurrentVB = static_cast<GFXOpenGLES20iOSVertexBuffer*>( buffer );
    if ( mCurrentVB )
        mCurrentVB->prepare();
}


//GFXCubemap* GFXOpenGLES20iOSDevice::createCubemap()
//{
//    GFXOpenGLESCubemap* cube = new GFXOpenGLESCubemap();
//    cube->registerResourceWithDevice(this);
//    return cube;
//};


void GFXOpenGLES20iOSDevice::clear(U32 flags, ColorI color, F32 z, U32 stencil)
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


void GFXOpenGLES20iOSDevice::updateStates(bool forceSetAll /*=false*/)
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


void GFXOpenGLES20iOSDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject*texture)
{
    const GFXOpenGLES20iOSTextureObject *tex = static_cast<const GFXOpenGLES20iOSTextureObject*>(texture);
    if (tex)
    {
        // GFXOpenGLES20iOSTextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        }
        mActiveTextureType[textureUnit] = tex->getBinding();
        
        switch (textureUnit) {
            case 0:
                mBaseEffect.texture2d0.name = tex->getHandle();
                break;
            case 1:
                mBaseEffect.texture2d1.name = tex->getHandle();
                break;
                
            default:
                break;
        }
//        tex->bind(textureUnit);
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
    
    glActiveTexture(GL_TEXTURE0);
}


void GFXOpenGLES20iOSDevice::setMatrix( GFXMatrixType mtype, const MatrixF &mat )
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
//            AssertFatal(false, "GFXOpenGLES20iOSDevice::setMatrix - Unknown matrix mode!");
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


const MatrixF GFXOpenGLES20iOSDevice::getMatrix( GFXMatrixType mtype )
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
//            AssertFatal(false, "GFXOpenGLES20iOSDevice::setMatrix - Unknown matrix mode!");
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
            AssertFatal(false, "GFXOpenGLES20iOSDevice::setMatrix - Unknown matrix mode!");
    }
    return ret;
}


/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXOpenGLES20iOSDevice::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
    GFXOpenGLES20iOSStateBlockRef ret = new GFXOpenGLES20iOSStateBlock(desc);
    ret->setView(getMatrix(GFXMatrixView));
    ret->setModel(getMatrix(GFXMatrixWorld));
    ret->setProjection(getMatrix(GFXMatrixProjection));
    return GFXStateBlockRef(ret);
}

/// Activates a stateblock
void GFXOpenGLES20iOSDevice::setStateBlockInternal(GFXStateBlock* block, bool force)
{
    AssertFatal(dynamic_cast<GFXOpenGLES20iOSStateBlock*>(block), "GFXOpenGLES20iOSDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
    GFXOpenGLES20iOSStateBlock* glBlock = static_cast<GFXOpenGLES20iOSStateBlock*>(block);
    GFXOpenGLES20iOSStateBlock* glCurrent = static_cast<GFXOpenGLES20iOSStateBlock*>(mCurrentStateBlock.getPointer());
    if (force)
        glCurrent = NULL;
    
    glBlock->activate(glCurrent); // Doesn't use current yet.
    mCurrentGLStateBlock = glBlock;
}

////------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GFXWindowTarget *GFXOpenGLES20iOSDevice::allocWindowTarget(PlatformWindow *window)
{
    // Allocate the wintarget and create a new context.
    GFXOpenGLES20iOSWindowTarget *gwt = new GFXOpenGLES20iOSWindowTarget(window, this);
    gwt->mContext = this->mContext;
    
    // And return...
    return gwt;
}


GFXTextureTarget * GFXOpenGLES20iOSDevice::allocRenderToTextureTarget()
{
    GFXOpenGLES20iOSTextureTarget *targ = new GFXOpenGLES20iOSTextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}


void GFXOpenGLES20iOSDevice::initGenericShaders()
{
    mBaseEffect = [[GLKBaseEffect alloc] init];
}

void GFXOpenGLES20iOSDevice::setupGenericShaders( GenericShaderType type )
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
    MatrixF projMatrix = GFX->getProjectionMatrix();
//    Con::printf("projectionMatrix");
//    Con::printf("%f %f %f %f", xform[0], xform[1], xform[2], xform[3]);
//    Con::printf("%f %f %f %f", xform[4], xform[5], xform[6], xform[7]);
//    Con::printf("%f %f %f %f", xform[8], xform[9], xform[10], xform[11]);
//    Con::printf("%f %f %f %f", xform[12], xform[13], xform[14], xform[15]);
   
//    xform.transpose();
//    projMatrix.transpose();
    
    mBaseEffect.transform.projectionMatrix = GLKMatrix4MakeWithArrayAndTranspose(projMatrix);
    mBaseEffect.transform.modelviewMatrix = GLKMatrix4MakeWithArrayAndTranspose(xform);
    mBaseEffect.useConstantColor = GL_TRUE;
    mBaseEffect.constantColor = GLKVector4Make(1.0, 1.0, 1.0, 1.0);

    switch (type) {
        case GSColor:
            mBaseEffect.texture2d0.enabled = GL_FALSE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
//            setShader(mGenericShader[0]);
//            setShaderConstBuffer( mGenericShaderConst[0] );
//            mGenericShaderConst[0]->setSafe( mGenericShader[0]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSTexture:
        case GSModColorTexture:
        case GSAddColorTexture:
            mBaseEffect.texture2d0.enabled = GL_TRUE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
//            setShader(mGenericShader[1]);
//            setShaderConstBuffer( mGenericShaderConst[1] );
//            mGenericShaderConst[1]->setSafe( mGenericShader[1]->getShaderConstHandle("$mvp_matrix"), xform );
//            mGenericShaderConst[1]->setSafe( mGenericShader[1]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
        case GSPoint:
            mBaseEffect.texture2d0.enabled = GL_TRUE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
//            setShader(mGenericShader[2]);
//            setShaderConstBuffer( mGenericShaderConst[2] );
//            mGenericShaderConst[2]->setSafe( mGenericShader[2]->getShaderConstHandle("$mvp_matrix"), xform );
//            mGenericShaderConst[2]->setSafe( mGenericShader[2]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
        case GSTest:
//            setShader(mGenericShader[3]);
//            setShaderConstBuffer( mGenericShaderConst[3] );
//            mGenericShaderConst[3]->setSafe( mGenericShader[3]->getShaderConstHandle("$mvp_matrix"), xform );
            break;
        case GSAlphaTexture:
            mBaseEffect.texture2d0.enabled = GL_TRUE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
//            setShader(mGenericShader[4]);
//            setShaderConstBuffer( mGenericShaderConst[4] );
//            mGenericShaderConst[4]->setSafe( mGenericShader[4]->getShaderConstHandle("$mvp_matrix"), xform );
//            mGenericShaderConst[4]->setSafe( mGenericShader[4]->getShaderConstHandle("$sampler2d_0"), 0);
            break;
            
        default:
            break;
    }
}

GFXShader* GFXOpenGLES20iOSDevice::createShader()
{
    GFXOpenGLES20iOSShader* shader = new GFXOpenGLES20iOSShader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGLES20iOSDevice::setShader( GFXOpenGLES20iOSShader *shader )
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

void GFXOpenGLES20iOSDevice::disableShaders()
{
//    setShader(NULL);
//    setShaderConstBuffer( NULL );
}

void GFXOpenGLES20iOSDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGLES20iOSShaderConstBuffer*>(buffer)->activate();
}

U32 GFXOpenGLES20iOSDevice::getNumSamplers() const
{
    return mMaxShaderTextures;
}

U32 GFXOpenGLES20iOSDevice::getNumRenderTargets() const
{
    return 1;
}


void GFXOpenGLES20iOSDevice::_updateRenderTargets()
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
        
        GFXOpenGLES20iOSTextureTarget *tex = dynamic_cast<GFXOpenGLES20iOSTextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGLES20iOSWindowTarget *win = dynamic_cast<GFXOpenGLES20iOSWindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != NULL,
                        "GFXOpenGLES20iOSDevice::_updateRenderTargets() - invalid target subclass passed!" );
            
            win->makeActive();
            
            if( win->mContext != static_cast<GFXOpenGLES20iOSDevice*>(GFX)->mContext )
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


GFXFormat GFXOpenGLES20iOSDevice::selectSupportedFormat(   GFXTextureProfile* profile, 
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


void GFXOpenGLES20iOSDevice::preDrawPrimitive()
{
    if( mStateDirty )
    {
        updateStates();
    }
    
//    if(mCurrentShaderConstBuffer)
//        setShaderConstBufferInternal(mCurrentShaderConstBuffer);
    
    [mBaseEffect prepareToDraw];
}

//
// Register this device with GFXInit
//
class GFXOpenGLES20iOSRegisterDevice
{
public:
    GFXOpenGLES20iOSRegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGLES20iOSDevice::enumerateAdapters);
    }
};

static GFXOpenGLES20iOSRegisterDevice pGLRegisterDevice;

