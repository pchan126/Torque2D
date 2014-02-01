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

#ifndef _GFXGL32DEVICE_H_
#define _GFXGL32DEVICE_H_

#include "platformOSX/platformOSX.h"
#include "platform/platformGL.h"
#include "graphics/OpenGL/gfxOpenGLDevice.h"
#include "./gfxOpenGL32EnumTranslate.h"
#include "windowManager/platformWindow.h"
#include "graphics/gfxResource.h"
#include "./gfxOpenGL32Shader.h"

class GFXOpenGL32VertexBuffer;
class GFXOpenGL32TextureTarget;
class GFXOpenGL32Cubemap;
//class GLKMatrixStackRef;

typedef uint32_t CGDirectDisplayID;
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;

class GFXOpenGL32Device : public GFXOpenGLDevice
{
public:
   void zombify();
   void resurrect();
   GFXOpenGL32Device(U32 adapterIndex );
   virtual ~GFXOpenGL32Device();

   static void enumerateAdapters( Vector<GFXAdapter*> &adapterList );
   static GFXDevice *createInstance( U32 adapterIndex );

   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL );

   virtual void activate() { }
   virtual void deactivate() { }
   virtual GFXAdapterType getAdapterType() { return OpenGL; }

   virtual void enterDebugEvent(ColorI color, const char *name) { }
   virtual void leaveDebugEvent() { }
   virtual void setDebugMarker(ColorI color, const char *name) { }

   virtual void enumerateVideoModes();
   void addVideoMode(const GFXVideoMode toAdd);

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
    GFXOpenGL32Shader* mpCurrentShader;
    GFXOpenGL32Shader* mGenericShader[5];
    
    GFXShaderConstBufferRef mGenericShaderConst[4];
    GFXStateBlockRef mGenericShaderStateblock[4];
    
   virtual F32 getPixelShaderVersion() const { return mPixelShaderVersion; }
   virtual void  setPixelShaderVersion( F32 version ) { mPixelShaderVersion = version; }

   virtual GFXOpenGL32Shader* createShader();
    
   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil );
      
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
   ///
   bool supportsAnisotropic() const { return mSupportsAnisotropic; }
   
    void* getTextureLoader() const { return mTextureLoader; };
   
   // special immediate function for drawing CIImages
   void drawImage( CIImage* image, CGRect inRect, CGRect fromRect);

protected:
    virtual void setFillMode( GFXFillMode fillMode );

    /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   virtual void setTextureInternal(U32 textureUnit, GFXTextureObject* texture);
//   virtual void setCubemapInternal(U32 cubemap, const GFXOpenGLCubemap* texture);

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
   virtual void initStates() { }

   virtual GFXVertexBuffer *allocVertexBuffer(  dsize_t numVerts,
                                                const GFXVertexFormat *vertexFormat,
                                                dsize_t vertSize,
                                                GFXBufferType bufferType,
                                                void *vertexData,
                                                dsize_t indexCount,
                                                void *indexData);
   
   // NOTE: The GL device doesn't need a vertex declaration at
   // this time, but we need to return something to keep the system
   // from retrying to allocate one on every call.
   virtual GFXVertexDecl* allocVertexDecl( const GFXVertexFormat *vertexFormat ) 
   {
      static GFXVertexDecl decl;
      return &decl; 
   }

   virtual void setVertexDecl( const GFXVertexDecl *decl ) { }

private:
   typedef GFXOpenGLDevice Parent;
   
   friend class GFXOpenGL32TextureObject;
   friend class GFXOpenGL32Cubemap;
   friend class GFXOpenGL32WindowTarget;
   friend class GFXOpenGL32VertexBuffer;

   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance; 

   U32 mAdapterIndex;
   
   StrongRefPtr<GFXOpenGL32VertexBuffer> mCurrentVB;
   
   NSOpenGLContext* mContext;       // NSOpenGLContext
   NSOpenGLPixelFormat* mPixelFormat;   // NSOpenGLPixelFormat
   CIContext *mCIContext;
   void* mTextureLoader; // GLKTextureLoader - for OSX version 10.8

   F32 mPixelShaderVersion;
   
   bool mSupportsAnisotropic;

   RectI mClip;

   GLenum mActiveTextureType[TEXTURE_STAGE_COUNT];
   
   Vector< StrongRefPtr<GFXOpenGL32VertexBuffer> > mVolatileVBs; ///< Pool of existing volatile VBs so we can reuse previously created ones
    
   GFXVertexBuffer* findVolatileVBO(U32 numVerts,
                                    const GFXVertexFormat *vertexFormat,
                                    U32 vertSize,
                                    void* data = NULL, U32 indexSize = 0, void* indexData = NULL); ///< Returns an existing volatile VB which has >= numVerts and the same vert flags/size, or creates a new VB if necessary
   
   void initGLState(); ///< Guaranteed to be called after all extensions have been loaded, use to init card profiler, shader version, max samplers, etc.
   
   void initGenericShaders();
    
};


#endif
