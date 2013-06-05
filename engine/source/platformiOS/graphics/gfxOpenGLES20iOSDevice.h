//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLES20IOSDEVICE_H_
#define _GFXGLES20IOSDEVICE_H_

#include "platformiOS/platformiOS.h"
#include "platform/platformGL.h"
#include "graphics/OpenGL/ES20/gfxOpenGLES20Device.h"

#include "graphics/gfxResource.h"
#include "./gfxOpenGLES20IOSStateBlock.h"
#include "./gfxOpenGLES20IOSShader.h"

class GFXOpenGLES20iOSVertexBuffer;
class GFXOpenGLES20iOSTextureTarget;
class GFXOpenGLES20iOSCubemap;

@class EAGLContext, GLKTextureLoader, GLKBaseEffect, CIImage;

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

   virtual GFXCubemap * createCubemap();

   virtual F32 getFillConventionOffset() const { return 0.0f; }

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
    GFXOpenGLES20iOSShader* mpCurrentShader;
    GFXOpenGLES20iOSShader* mGenericShader[5];

    GFXShaderConstBufferRef mGenericShaderConst[4];
    GFXStateBlockRef mGenericShaderStateblock[4];
    
   /// @attention GL cannot check if the given format supports blending or filtering!
   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
	   const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter);
      
   /// Returns the number of texture samplers that can be used in a shader rendering pass
   virtual U32 getNumSamplers() const;

   /// Returns the number of simultaneous render targets supported by the device.
   virtual U32 getNumRenderTargets() const;

   virtual GFXShader* createShader();
    
   GFXOpenGLES20iOSStateBlockRef getCurrentStateBlock() { return mCurrentGLStateBlock; }
   
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
    EAGLContext* getEAGLContext() const { return mContext; };
    GLKTextureLoader* getTextureLoader() const { return mTextureLoader; };

    // special immediate function for drawing CIImages
    void drawImage( CIImage* image, CGRect inRect, CGRect fromRect);


protected:
   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force);   

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject*texture);
//   virtual void setCubemapInternal(U32 cubemap, const GFXOpenGLES20iOSCubemap* texture);

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { }

   virtual GFXVertexBuffer *allocVertexBuffer(  U32 numVerts, 
                                                const GFXVertexFormat *vertexFormat,
                                                U32 vertSize, 
                                                GFXBufferType bufferType,
                                                void *data = NULL,
                                              U32 indexCount = 0,
                                              void *indexData = NULL);
    
   // NOTE: The GL device doesn't need a vertex declaration at
   // this time, but we need to return something to keep the system
   // from retrying to allocate one on every call.
   virtual GFXVertexDecl* allocVertexDecl( const GFXVertexFormat *vertexFormat ) 
   {
      static GFXVertexDecl decl;
      return &decl; 
   }

   virtual void setVertexDecl( const GFXVertexDecl *decl ) { }

   virtual void setVertexStream( U32 stream, GFXVertexBuffer *buffer );

private:
   typedef GFXDevice Parent;
   
   friend class GFXOpenGLES20iOSTextureObject;
//   friend class GFXOpenGLESCubemap;
   friend class GFXOpenGLES20iOSWindowTarget;
   friend class GFXOpenGLES20iOSVertexBuffer;

   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance; 

   U32 mAdapterIndex;
   
   StrongRefPtr<GFXOpenGLES20iOSVertexBuffer> mCurrentVB;

    void _handleTextureLoaded(GFXTexNotifyCode code);

    EAGLContext* mContext;
    CIContext* mCIContext;
    GLKTextureLoader* mTextureLoader; // GLKTextureLoader
    GLKBaseEffect* mBaseEffect;

   U32 mMaxShaderTextures;

   RectI mClip;

   GFXOpenGLES20iOSStateBlockRef mCurrentGLStateBlock;
   
   GLenum mActiveTextureType[TEXTURE_STAGE_COUNT];
   
   Vector< StrongRefPtr<GFXOpenGLES20iOSVertexBuffer> > mVolatileVBs;
    ///< Pool of existing volatile VBs so we can reuse previously created ones

    
    ///< Returns an existing volatile VB which has >= numVerts and the same vert flags/size, or creates a new VB if necessary
   GFXVertexBuffer* findVolatileVBO(U32 numVerts,
                                    const GFXVertexFormat *vertexFormat,
                                    U32 vertSize,
                                    void* vertData = NULL,
                                    U32 numIndex =0,
                                    void* indexData = 0);
   
   void initGLState(); ///< Guaranteed to be called after all extensions have been loaded, use to init card profiler, shader version, max samplers, etc.
   virtual void initGenericShaders();
    
    virtual void preDrawPrimitive();
};


#endif
