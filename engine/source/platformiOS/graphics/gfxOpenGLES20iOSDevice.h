//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLES20IOSDEVICE_H_
#define _GFXGLES20IOSDEVICE_H_

#include "platformiOS/platformiOS.h"
#include "platform/platformGL.h"
#include "graphics/OpenGL/gfxOpenGLDevice.h"

#include "graphics/gfxResource.h"
#include "./gfxOpenGLES20IOSStateBlock.h"
#include "./gfxOpenGLES20IOSShader.h"

class GFXOpenGLES20iOSVertexBuffer;
class GFXOpenGLES20iOSTextureTarget;
//class GFXOpenGLESCubemap;
//class GLKMatrixStackRef;

@class EAGLContext, GLKTextureLoader, GLKBaseEffect;

class GFXOpenGLES20iOSDevice : public GFXOpenGLDevice
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
    
   virtual F32 getPixelShaderVersion() const { return mPixelShaderVersion; }
   virtual void  setPixelShaderVersion( F32 version ) { mPixelShaderVersion = version; }
   
   virtual void setShader(GFXOpenGLES20iOSShader* shd);
   virtual void disableShaders(); ///< Equivalent to setShader(NULL)
   
   /// @attention GL cannot check if the given format supports blending or filtering!
   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
	   const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter);
      
   /// Returns the number of texture samplers that can be used in a shader rendering pass
   virtual U32 getNumSamplers() const;

   /// Returns the number of simultaneous render targets supported by the device.
   virtual U32 getNumRenderTargets() const;

   virtual GFXOpenGLES20iOSShader* createShader();
    
   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil );

   virtual void updateStates(bool forceSetAll = false);
   
   GFXOpenGLES20iOSStateBlockRef getCurrentStateBlock() { return mCurrentGLStateBlock; }
   
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
   ///
   bool supportsAnisotropic() const { return mSupportsAnisotropic; }
   
    EAGLContext* getEAGLContext() const { return mContext; };
    GLKTextureLoader* getTextureLoader() const { return mTextureLoader; };
protected:
   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force);   

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject*texture);
//   virtual void setCubemapInternal(U32 cubemap, const GFXOpenGLESCubemap* texture);

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { }

   virtual void setMatrix( GFXMatrixType mtype, const MatrixF &mat );
   virtual const MatrixF getMatrix (GFXMatrixType mtype );

   virtual inline const MatrixF getWorldMatrix() { return getMatrix(GFXMatrixWorld);};
   virtual inline const MatrixF getProjectionMatrix() { return getMatrix(GFXMatrixProjection);};
   virtual inline const MatrixF getViewMatrix() { return getMatrix(GFXMatrixView);};
   

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

    typedef Vector<MatrixF> MatrixStack;
    
    /// Since GL does not have separate world and view matrices we need to track them
    MatrixF m_mCurrentView;
    MatrixStack m_WorldStack;
    MatrixStack m_ProjectionStack;
    
    virtual void pushWorldMatrix();
    virtual void popWorldMatrix();
    virtual void pushProjectionMatrix();
    virtual void popProjectionMatrix();

    virtual void multWorld( const MatrixF &mat );

    void _handleTextureLoaded(GFXTexNotifyCode code);

    EAGLContext* mContext;
    void* mPixelFormat;
    GLKTextureLoader* mTextureLoader; // GLKTextureLoader
    GLKBaseEffect* mBaseEffect;

   F32 mPixelShaderVersion;
   
   bool mSupportsAnisotropic;
   
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
   void initGenericShaders();
    
    virtual void preDrawPrimitive();
};


#endif
