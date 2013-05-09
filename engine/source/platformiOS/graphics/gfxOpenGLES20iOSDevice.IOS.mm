//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#import "platformiOS/platformiOS.h"
#include "./gfxOpenGLES20iOSDevice.h"

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
#import <GLKit/GLKit.h>


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
                    mContext(nil),
                    mPixelFormat(NULL),
                    mTextureLoader(NULL)
{
    GFXGLES20iOSEnumTranslate::init();
    
    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
}


GFXOpenGLES20iOSDevice::~GFXOpenGLES20iOSDevice()
{
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


void GFXOpenGLES20iOSDevice::zombify()
{
}

void GFXOpenGLES20iOSDevice::resurrect()
{
}


//GFXCubemap* GFXOpenGLES20iOSDevice::createCubemap()
//{
//    GFXOpenGLESCubemap* cube = new GFXOpenGLESCubemap();
//    cube->registerResourceWithDevice(this);
//    return cube;
//};

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
    xform *= GFX->getViewMatrix();
    MatrixF projMatrix = GFX->getProjectionMatrix();
    
    mBaseEffect.transform.projectionMatrix = GLKMatrix4MakeWithArrayAndTranspose(projMatrix);
    mBaseEffect.transform.modelviewMatrix = GLKMatrix4MakeWithArrayAndTranspose(xform);
    mBaseEffect.useConstantColor = GL_TRUE;
    mBaseEffect.constantColor = GLKVector4Make(1.0, 1.0, 1.0, 1.0);

    switch (type) {
        case GSColor:
            mBaseEffect.texture2d0.enabled = GL_FALSE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
            break;
        case GSTexture:
        case GSModColorTexture:
        case GSAddColorTexture:
            mBaseEffect.texture2d0.enabled = GL_TRUE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
            break;
        case GSPoint:
            mBaseEffect.texture2d0.enabled = GL_TRUE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
            break;
        case GSTest:
            break;
        case GSAlphaTexture:
            mBaseEffect.texture2d0.enabled = GL_TRUE;
            mBaseEffect.texture2d1.enabled = GL_FALSE;
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
class GFXOpenGLESRegisterDevice
{
public:
    GFXOpenGLESRegisterDevice()
    {
        GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGLES20iOSDevice::enumerateAdapters);
    }
};

static GFXOpenGLESRegisterDevice pGLRegisterDevice;

