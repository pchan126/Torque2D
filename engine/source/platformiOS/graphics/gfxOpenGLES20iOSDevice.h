//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLES20IOSDEVICE_H_
#define _GFXGLES20IOSDEVICE_H_

#include "platformiOS/platformiOS.h"
#include "platform/platformGL.h"
#include "graphics/gfxResource.h"

#include "graphics/OpenGL/ES20/gfxOpenGLES20Device.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSShader.h"

class GFXOpenGLES20iOSVertexBuffer;
class GFXOpenGLES20iOSTextureTarget;

@class EAGLContext, GLKTextureLoader, GLKBaseEffect;

class GFXOpenGLES20iOSDevice : public GFXOpenGLES20Device
{
public:
   void zombify();
   void resurrect();
   GFXOpenGLES20iOSDevice(U32 adapterIndex );
   virtual ~GFXOpenGLES20iOSDevice();

   static void enumerateAdapters( Vector<GFXAdapter*> &adapterList );
   static GFXDevice *createInstance( U32 adapterIndex );

   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL );

   virtual void activate() { }
   virtual void deactivate() { }
   virtual GFXAdapterType getAdapterType() { return OpenGLES; }

   virtual void enterDebugEvent(ColorI color, const char *name) { }
   virtual void leaveDebugEvent() { }
   virtual void setDebugMarker(ColorI color, const char *name) { }

   virtual void enumerateVideoModes();

//   virtual U32 getTotalVideoMemory();

//   virtual GFXCubemap * createCubemap();

   ///@}

   /// @name Render Target functions
   /// @{

   ///
    
   GFXWindowTarget *gwt;   // single window render target;
    
   virtual GFXTextureTarget *allocRenderToTextureTarget();
   virtual GFXWindowTarget *allocWindowTarget(PlatformWindow *window);
   virtual void _updateRenderTargets();

   ///@}

   /// @name Shader functions
   /// @{
      
   virtual GFXShader* createShader();
    
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
    EAGLContext* getEAGLContext() const { return mContext; };
    GLKTextureLoader* getTextureLoader() const { return mTextureLoader; };
protected:

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject*texture);
//   virtual void setCubemapInternal(U32 cubemap, const GFXOpenGLESCubemap* texture);

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { }

private:
   typedef GFXDevice Parent;
   
   friend class GFXOpenGLES20iOSTextureObject;
//   friend class GFXOpenGLESCubemap;
   friend class GFXOpenGLES20iOSWindowTarget;
   friend class GFXOpenGLES20iOSVertexBuffer;

   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance; 

   U32 mAdapterIndex;
   
   StrongRefPtr<GFXOpenGLES20iOSVertexBuffer> mCurrentVB;

    EAGLContext* mContext;
    void* mPixelFormat;
    GLKTextureLoader* mTextureLoader; // GLKTextureLoader
    GLKBaseEffect* mBaseEffect;
   
   Vector< StrongRefPtr<GFXOpenGLES20iOSVertexBuffer> > mVolatileVBs;
    ///< Pool of existing volatile VBs so we can reuse previously created ones
   
   virtual void initGLState(); ///< Guaranteed to be called after all extensions have been loaded, use to init card profiler, shader version, max samplers, etc.
   virtual void initGenericShaders();
    
    virtual void preDrawPrimitive();
};


#endif
