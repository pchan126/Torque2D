//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#import "platformiOS/platformiOS.h"
#include "./gfxOpenGLES30iOSDevice.h"

#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxInit.h"

#include "./gfxOpenGLES30iOSEnumTranslate.h"
#include "./gfxOpenGLES30iOSVertexBuffer.h"
#include "./gfxOpenGLES30iOSTextureTarget.h"
#include "./gfxOpenGLES30iOSTextureManager.h"
#include "./gfxOpenGLES30iOSCardProfiler.h"
#include "./gfxOpenGLES30iOSWindowTarget.h"
#include "./gfxOpenGLES30iOSCubemap.h"

#include "lighting/lightInfo.h"
#include "lighting/lightManager.h"

GFXAdapter::CreateDeviceInstanceDelegate GFXOpenGLES30iOSDevice::mCreateDeviceInstance(GFXOpenGLES30iOSDevice::createInstance);

GFXDevice *GFXOpenGLES30iOSDevice::createInstance( U32 adapterIndex )
{
    return new GFXOpenGLES30iOSDevice(adapterIndex);
}


void GFXOpenGLES30iOSDevice::initGLState()
{
    // Currently targeting OpenGL ES 3.0
    
    // We don't currently need to sync device state with a known good place because we are
    // going to set everything in GFXOpenGLES30iOSStateBlock, but if we change our GFXOpenGLES30iOSStateBlock strategy, this may
    // need to happen.
    
    // Deal with the card profiler here when we know we have a valid context.
    mCardProfiler = new GFXOpenGLES30iOSCardProfiler();
    mCardProfiler->init();

    glGetIntegerv(GL_MAX_COMBINED_TEXTURE_IMAGE_UNITS, (GLint*)&mMaxShaderTextures);
    glPixelStorei(GL_UNPACK_ALIGNMENT, 1);
}


//-----------------------------------------------------------------------------
// Matrix interface
GFXOpenGLES30iOSDevice::GFXOpenGLES30iOSDevice( U32 adapterIndex ) : GFXOpenGLES30Device( adapterIndex ),
                    mAdapterIndex(adapterIndex),
                    mCIContext(nil),
                    mClip(0, 0, 0, 0),
                    mTextureLoader(nullptr)
{
    GFXGLES30iOSEnumTranslate::init();
}


GFXOpenGLES30iOSDevice::~GFXOpenGLES30iOSDevice()
{
}


void GFXOpenGLES30iOSDevice::init( const GFXVideoMode &mode, PlatformWindow *window )
{
    if(!mInitialized)
    {
        AssertFatal(!mContext, "_createInitialContextAndFormat - Already created initial context");
        
        mContext = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
        [EAGLContext setCurrentContext:mContext];
        
        mCIContext = [CIContext contextWithEAGLContext:mContext options:@{kCIContextWorkingColorSpace: [NSNull null]}];
       
        mTextureManager = new GFXOpenGLES30iOSTextureManager();
        
        initGLState();
        initGenericShaders();
        mInitialized = true;
        deviceInited();
    }
}

void GFXOpenGLES30iOSDevice::refreshCIContext()
{
   mCIContext = [CIContext contextWithEAGLContext:mContext];
}


void GFXOpenGLES30iOSDevice::_handleTextureLoaded(GFXTexNotifyCode code)
{
    mTexturesDirty = true;
}


void GFXOpenGLES30iOSDevice::enumerateAdapters( Vector<GFXAdapter*> &adapterList )
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


GFXVertexBuffer* GFXOpenGLES30iOSDevice::findVolatileVBO(U32 vertexCount, const GFXVertexFormat *vertexFormat, U32 vertSize, void* vertexData, U32 indexSize, void* indexData)
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
    StrongRefPtr<GFXOpenGLES30iOSVertexBuffer> buf(new GFXOpenGLES30iOSVertexBuffer(GFX, vertexCount, vertexFormat, vertSize, GFXBufferTypeVolatile, vertexData, indexSize, indexData));
    buf->registerResourceWithDevice(this);
    mVolatileVBs.push_back(buf);
    return buf.getPointer();
}

GFXVertexBuffer *GFXOpenGLES30iOSDevice::allocVertexBuffer(   U32 numVerts,
                                                  const GFXVertexFormat *vertexFormat,
                                                  U32 vertSize,
                                                  GFXBufferType bufferType,
                                                  void *vertexBuffer,
                                                  U32 indexCount,
                                                  void *indexBuffer)
{
    if(bufferType == GFXBufferTypeVolatile)
        return findVolatileVBO(numVerts, vertexFormat, vertSize, vertexBuffer, indexCount, indexBuffer);
    
    GFXOpenGLES30iOSVertexBuffer* buf = new GFXOpenGLES30iOSVertexBuffer( GFX, numVerts, vertexFormat, vertSize, bufferType, vertexBuffer, indexCount, indexBuffer );
    buf->registerResourceWithDevice(this);
    return buf;
}


GFXCubemap* GFXOpenGLES30iOSDevice::createCubemap()
{
    GFXOpenGLES30iOSCubemap* cube = new GFXOpenGLES30iOSCubemap();
    cube->registerResourceWithDevice(this);
    return cube;
};


void GFXOpenGLES30iOSDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
   if(light.mType == GFXLightInfo::Ambient)
   {
      AssertFatal(false, "Instead of setting an ambient light you should set the global ambient color.");
      return;
   }
   
   if (lightEnable == false)
   {
      switch (lightStage) {
         case 0:
//            mBaseEffect.light0.enabled = lightEnable;
              break;
         case 1:
//            mBaseEffect.light1.enabled = lightEnable;
              break;
         case 2:
//            mBaseEffect.light2.enabled = lightEnable;
              break;
      }
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
//         mBaseEffect.light0.enabled = lightEnable;
//         mBaseEffect.light0.specularColor = light.specular.mGV;
//         mBaseEffect.light0.ambientColor = light.ambient.mGV;
//         mBaseEffect.light0.diffuseColor = light.diffuse.mGV;
//         mBaseEffect.light0.position = pos.mGV;
//         mBaseEffect.light0.constantAttenuation = 1.0;
//         mBaseEffect.light0.linearAttenuation = 0.0;
//         mBaseEffect.light0.quadraticAttenuation = 0.0;
         break;
         
      case 1:
//         mBaseEffect.light1.enabled = lightEnable;
//         mBaseEffect.light1.specularColor = light.specular.mGV;
//         mBaseEffect.light1.ambientColor = light.ambient.mGV;
//         mBaseEffect.light1.diffuseColor = light.diffuse.mGV;
//         mBaseEffect.light1.position = pos.mGV;
//         mBaseEffect.light1.constantAttenuation = 0.9;
//         mBaseEffect.light1.linearAttenuation = 0.1;
//         mBaseEffect.light1.quadraticAttenuation = 0.0;
         break;
      case 2:
//         mBaseEffect.light2.enabled = lightEnable;
//         mBaseEffect.light2.specularColor = light.specular.mGV;
//         mBaseEffect.light2.ambientColor = light.ambient.mGV;
//         mBaseEffect.light2.diffuseColor = light.diffuse.mGV;
//         mBaseEffect.light2.position = pos.mGV;
//         mBaseEffect.light2.constantAttenuation = 0.9;
//         mBaseEffect.light2.linearAttenuation = 0.1;
//         mBaseEffect.light2.quadraticAttenuation = 0.0;
         break;
      default:
         Con::printf("GFXOpenGLES30iOSDevice::setLightInternal - Only 3 lights");
         break;
   }

}

void GFXOpenGLES30iOSDevice::setLightMaterialInternal(const GFXLightMaterial mat)
{
//   mBaseEffect.material.diffuseColor = mat.diffuse.mGV;
//   mBaseEffect.material.emissiveColor = mat.emissive.mGV;
//   mBaseEffect.material.specularColor = mat.specular.mGV;
//   mBaseEffect.material.ambientColor = mat.ambient.mGV;
//   mBaseEffect.material.shininess = mat.shininess;
}

void GFXOpenGLES30iOSDevice::setGlobalAmbientInternal(ColorF color)
{
//   mBaseEffect.lightModelAmbientColor = color.mGV;
}


void GFXOpenGLES30iOSDevice::setTextureInternal(U32 textureUnit, GFXTextureObject*texture)
{
   GFXOpenGLTextureObject *tex = static_cast< GFXOpenGLTextureObject*>(texture);
    if (tex)
    {
        // GFXOpenGLESTextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
        {
            setTextureUnit(textureUnit);
            glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        }
        mActiveTextureType[textureUnit] = tex->getBinding();
        tex->bind(textureUnit);
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
         setTextureUnit(textureUnit);
        glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
   setTextureUnit(0);
}


////------------------------------------------------------------------------------
//
//-----------------------------------------------------------------------------
GFXWindowTarget *GFXOpenGLES30iOSDevice::allocWindowTarget(PlatformWindow *window)
{
    // Allocate the wintarget and create a new context.
    GFXOpenGLES30iOSWindowTarget *gwt = new GFXOpenGLES30iOSWindowTarget(window, this);
    gwt->mContext = this->mContext;
    
    // And return...
    return gwt;
}


GFXTextureTarget * GFXOpenGLES30iOSDevice::allocRenderToTextureTarget()
{
    GFXOpenGLES30TextureTarget *targ = new GFXOpenGLES30TextureTarget();
    targ->registerResourceWithDevice(this);
    return targ;
}


void GFXOpenGLES30iOSDevice::initGenericShaders()
{
//    mBaseEffect = [[GLKBaseEffect alloc] init];
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

void GFXOpenGLES30iOSDevice::setupGenericShaders( GenericShaderType type )
{
	Vector<LightInfo*> mLights;
   LightQuery query;
   GFXLightInfo outLight;

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

        default:
            break;
    }
}

GFXShader* GFXOpenGLES30iOSDevice::createShader()
{
    GFXOpenGLES30Shader* shader = new GFXOpenGLES30Shader();
    shader->registerResourceWithDevice( this );
    return shader;
}

void GFXOpenGLES30iOSDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGLES30ShaderConstBuffer*>(buffer)->activate();
}

void GFXOpenGLES30iOSDevice::_updateRenderTargets()
{
    if ( mRTDirty || mCurrentRT->isPendingState() )
    {
        if ( mRTDeactivate )
        {
            mRTDeactivate->deactivate();
            mRTDeactivate = nullptr;
        }
        
        // NOTE: The render target changes is not really accurate
        // as the GFXTextureTarget supports MRT internally.  So when
        // we activate a GFXTarget it could result in multiple calls
        // to SetRenderTarget on the actual device.
        //      mDeviceStatistics.mRenderTargetChanges++;

        GFXOpenGLES30TextureTarget *tex = dynamic_cast<GFXOpenGLES30TextureTarget*>( mCurrentRT.getPointer() );
        if ( tex )
        {
            tex->applyState();
            tex->makeActive();
        }
        else
        {
            GFXOpenGLES30iOSWindowTarget *win = dynamic_cast<GFXOpenGLES30iOSWindowTarget*>( mCurrentRT.getPointer() );
            AssertFatal( win != nullptr,
                        "GFXOpenGLES30iOSDevice::_updateRenderTargets() - invalid target subclass passed!" );
            
            win->makeActive();
            
            if( win->mContext != static_cast<GFXOpenGLES30iOSDevice*>(GFX)->mContext )
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


GFXFormat GFXOpenGLES30iOSDevice::selectSupportedFormat(   GFXTextureProfile* profile, 
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


// special immediate function for drawing CIImages
void GFXOpenGLES30iOSDevice::drawImage( CIImage* image, CGRect inRect, CGRect fromRect)
{
     updateStates(true);
   CIContext* ciContext = [CIContext contextWithEAGLContext:mContext options:@{kCIContextWorkingColorSpace: [NSNull null]}];
   glFlush();
   [ciContext drawImage:image inRect:inRect fromRect:fromRect];
}


//
// Register this device with GFXInit
//
class GFXOpenGLES30iOSRegisterDevice
{
public:
    GFXOpenGLES30iOSRegisterDevice()
    {
       NSString *osVersion = [[UIDevice currentDevice] systemVersion];
       if ([osVersion compare:@"7.0.0" options:NSNumericSearch])
       {
          EAGLContext* ctx = [[EAGLContext alloc] initWithAPI:kEAGLRenderingAPIOpenGLES3];
          if (ctx != nil)
             GFXInit::getRegisterDeviceSignal().notify(&GFXOpenGLES30iOSDevice::enumerateAdapters);
       }
    }
};

//static GFXOpenGLES30iOSRegisterDevice pGLES3RegisterDevice;

