//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLES30DEVICE_H_
#define _GFXGLES30DEVICE_H_

#include "platform/platformGL.h"
#include "graphics/OpenGL/gfxOpenGLDevice.h"
#include "graphics/gfxResource.h"

#include "gfxOpenGLES30Shader.h"

class GFXOpenGLES30VertexBuffer;
class GFXOpenGLES30TextureTarget;

// virtual class for OpenGLES 2.0 devices (Mobile)

class GFXOpenGLES30Device : public GFXOpenGLDevice
{
public:
   void zombify() {};
   void resurrect() {};
   GFXOpenGLES30Device(U32 adapterIndex );
   virtual ~GFXOpenGLES30Device();

//   static void enumerateAdapters( Vector<GFXAdapter*> &adapterList ) = 0;
//   static GFXDevice *createInstance( U32 adapterIndex ) = 0;

   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL ) = 0;

   virtual void activate() { }
   virtual void deactivate() { }
   virtual GFXAdapterType getAdapterType() { return OpenGLES; }

   virtual void enterDebugEvent(ColorI color, const char *name) { }
   virtual void leaveDebugEvent() { }
   virtual void setDebugMarker(ColorI color, const char *name) { }

   virtual void enumerateVideoModes() {    mVideoModes.clear();   }
   ///@}

   /// @name Render Target functions
   /// @{

   ///
    
   GFXWindowTarget *gwt;   // single window render target;
    
   virtual GFXTextureTarget *allocRenderToTextureTarget() = 0;
   virtual GFXWindowTarget *allocWindowTarget(PlatformWindow *window) = 0;
   virtual void _updateRenderTargets() = 0;

   ///@}

   /// @name Shader functions
   /// @{
    GFXShaderConstBufferRef mGenericShaderConst[4];
    GFXStateBlockRef mGenericShaderStateblock[4];
    
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
   virtual void clear(U32 flags, ColorI color, F32 z, U32 stencil);
    
   virtual GFXShader* createShader();

protected:

    virtual void setFillMode( GFXFillMode fillMode );

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { }

//   virtual GFXVertexBuffer *allocVertexBuffer(  U32 numVerts, 
//                                                const GFXVertexFormat *vertexFormat,
//                                                U32 vertSize, 
//                                                GFXBufferType bufferType,
//                                                void *data = NULL,
//                                              U32 indexCount = 0,
//                                              void *indexData = NULL);

    virtual void initGenericShaders();

private:
   typedef GFXDevice Parent;
   
   friend class GFXOpenGLES30TextureObject;
   friend class GFXOpenGLES30WindowTarget;
   friend class GFXOpenGLES30VertexBuffer;
    


//   U32 mAdapterIndex;
//   
//   StrongRefPtr<GFXOpenGLES30VertexBuffer> mCurrentVB;
//   
//   Vector< StrongRefPtr<GFXOpenGLES30VertexBuffer> > mVolatileVBs;
//    ///< Pool of existing volatile VBs so we can reuse previously created ones
//
//    
//    ///< Returns an existing volatile VB which has >= numVerts and the same vert flags/size, or creates a new VB if necessary
//   virtual GFXVertexBuffer* findVolatileVBO(U32 numVerts,
//                                    const GFXVertexFormat *vertexFormat,
//                                    U32 vertSize,
//                                    void* vertData = NULL,
//                                    U32 numIndex =0,
//                                    void* indexData = 0);
};


#endif
