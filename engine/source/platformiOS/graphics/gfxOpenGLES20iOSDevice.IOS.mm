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
#include "./gfxOpenGLES20iOSCubemap.h"

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
                    mContext(nil),
                    mCIContext(nil),
                    mMaxShaderTextures(2),
                    mClip(0, 0, 0, 0),
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
        AssertFatal(!mContext, "_createInitialContextAndFormat - Already created initial context");
        
        mContext = [[EAGLContext alloc]
                    initWithAPI:kEAGLRenderingAPIOpenGLES2];

        [EAGLContext setCurrentContext:mContext];
        
        mCIContext = [CIContext contextWithEAGLContext:mContext];
        
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
        AssertFatal(dStrlen(renderer.c_str()) < GFXAdapter::MaxAdapterNameLen, "GFXOpenGLDevice::enumerateAdapter - renderer name too long, increae the size of GFXAdapter::MaxAdapterNameLen (or use String!)");
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


GFXCubemap* GFXOpenGLES20iOSDevice::createCubemap()
{
    GFXOpenGLES20iOSCubemap* cube = new GFXOpenGLES20iOSCubemap();
    cube->registerResourceWithDevice(this);
    return cube;
};


void GFXOpenGLES20iOSDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
   if(light.mType == GFXLightInfo::Ambient)
   {
      AssertFatal(false, "Instead of setting an ambient light you should set the global ambient color.");
      return;
   }

   Vector4F pos;
   if(light.mType != GFXLightInfo::Vector)
   {
      dMemcpy(pos, &light.mPos, sizeof(light.mPos));
      pos[3] = 1.0;
   }
   else
   {
      dMemcpy(pos, &light.mDirection, sizeof(light.mDirection));
      pos[3] = 0.0;
   }
   
   switch (lightStage) {
      case 0:
         mBaseEffect.light0.enabled = lightEnable;
         mBaseEffect.light0.specularColor = light.specular.mGV;
         mBaseEffect.light0.ambientColor = light.ambient.mGV;
         mBaseEffect.light0.diffuseColor = light.diffuse.mGV;
         mBaseEffect.light0.position = pos.mGV;
         mBaseEffect.light0.linearAttenuation = 0.1;
         break;
         
      case 1:
         mBaseEffect.light1.enabled = lightEnable;
         mBaseEffect.light1.specularColor = light.specular.mGV;
         mBaseEffect.light1.ambientColor = light.ambient.mGV;
         mBaseEffect.light1.diffuseColor = light.diffuse.mGV;
         mBaseEffect.light1.position = pos.mGV;
         mBaseEffect.light1.linearAttenuation = 0.1;
         
      case 2:
         mBaseEffect.light2.enabled = lightEnable;
         mBaseEffect.light2.specularColor = light.specular.mGV;
         mBaseEffect.light2.ambientColor = light.ambient.mGV;
         mBaseEffect.light2.diffuseColor = light.diffuse.mGV;
         mBaseEffect.light2.position = pos.mGV;
         mBaseEffect.light2.linearAttenuation = 0.1;
         
      default:
         Con::printf("GFXOpenGLES20iOSDevice::setLightInternal - Only 3 lights");
         break;
   }

}

void GFXOpenGLES20iOSDevice::setLightMaterialInternal(const GFXLightMaterial mat)
{
   mBaseEffect.material.diffuseColor = mat.diffuse.mGV;
   mBaseEffect.material.emissiveColor = mat.emissive.mGV;
   mBaseEffect.material.specularColor = mat.specular.mGV;
   mBaseEffect.material.ambientColor = mat.ambient.mGV;
   mBaseEffect.material.shininess = mat.shininess;
}

void GFXOpenGLES20iOSDevice::setGlobalAmbientInternal(ColorF color)
{
   mBaseEffect.lightModelAmbientColor = color.mGV;
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
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
    
    glActiveTexture(GL_TEXTURE0);
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


// special immediate function for drawing CIImages
void GFXOpenGLES20iOSDevice::drawImage( CIImage* image, CGRect inRect, CGRect fromRect)
{
    if( mStateDirty )
    {
        updateStates();
    }
    
    [mCIContext drawImage:image inRect:inRect fromRect:fromRect];
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

