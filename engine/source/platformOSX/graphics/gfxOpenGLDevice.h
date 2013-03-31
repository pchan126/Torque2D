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

#ifndef _GFXGLDEVICE_H_
#define _GFXGLDEVICE_H_

#include "platformOSX/platformOSX.h"
#include "platform/platformGL.h"
#include "graphics/gfxDevice.h"


#include "windowManager/platformWindow.h"
#include "graphics/gfxResource.h"
#include "./gfxOpenGLStateBlock.h"
#include "platformOSX/graphics/gfxOpenGLShader.h"

class GFXOpenGLVertexBuffer;
class GFXOpenGLPrimitiveBuffer;
class GFXOpenGLTextureTarget;
class GFXOpenGLCubemap;
//class GLKMatrixStackRef;
typedef uint32_t CGDirectDisplayID;
@class NSOpenGLContext;
@class NSOpenGLPixelFormat;

class GFXOpenGLDevice : public GFXDevice
{
public:
   void zombify();
   void resurrect();
   GFXOpenGLDevice(U32 adapterIndex );
   virtual ~GFXOpenGLDevice();

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
    void* baseEffect;
    GFXOpenGLShader* mpCurrentShader;
    
    GFXOpenGLShader* mGenericShader[5];
    GFXShaderConstBufferRef mGenericShaderConst[4];
    GFXStateBlockRef mGenericShaderStateblock[4];
    
   virtual F32 getPixelShaderVersion() const { return mPixelShaderVersion; }
   virtual void  setPixelShaderVersion( F32 version ) { mPixelShaderVersion = version; }
   
   virtual void setShader(GFXOpenGLShader* shd);
   virtual void disableShaders(); ///< Equivalent to setShader(NULL)
   
   /// @attention GL cannot check if the given format supports blending or filtering!
   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
	   const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter);
    
   /// Returns the number of texture samplers that can be used in a shader rendering pass
   virtual U32 getNumSamplers() const;

   /// Returns the number of simultaneous render targets supported by the device.
   virtual U32 getNumRenderTargets() const;

   virtual GFXOpenGLShader* createShader();
    
   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil );
   virtual bool beginSceneInternal();
   virtual void endSceneInternal();

   virtual void drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount );

   virtual void drawIndexedPrimitive(  GFXPrimitiveType primType, 
                                       U32 startVertex, 
                                       U32 minIndex, 
                                       U32 numVerts, 
                                       U32 startIndex, 
                                       U32 primitiveCount );

   virtual void setClipRect( const RectI &rect );
   virtual const RectI &getClipRect() const { return mClip; }

   virtual void preDestroy() { Parent::preDestroy(); }

   virtual U32 getMaxDynamicVerts() { return MAX_DYNAMIC_VERTS; }
   virtual U32 getMaxDynamicIndices() { return MAX_DYNAMIC_INDICES; }
   
   virtual void updateStates(bool forceSetAll = false);
   
//   GFXOcclusionQuery* createOcclusionQuery();

   GFXOpenGLStateBlockRef getCurrentStateBlock() { return mCurrentGLStateBlock; }
   
   virtual void setupGenericShaders( GenericShaderType type = GSColor );
   
   ///
   bool supportsAnisotropic() const { return mSupportsAnisotropic; }
   
    void* getTextureLoader() const { return mTextureLoader; };
protected:
   /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force);   

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   virtual void setTextureInternal(U32 textureUnit, const GFXTextureObject* texture);
//   virtual void setCubemapInternal(U32 cubemap, const GFXOpenGLCubemap* texture);

   virtual void setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable);
   virtual void setLightMaterialInternal(const GFXLightMaterial mat);
   virtual void setGlobalAmbientInternal(ColorF color);

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
                                                void *data);
   virtual GFXPrimitiveBuffer *allocPrimitiveBuffer( U32 numIndices,
                                                    U32 numPrimitives,
                                                    GFXBufferType bufferType,
                                                    U16 *indexBuffer,
                                                    GFXPrimitive *primitiveBuffer);
   
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
   virtual void setVertexStreamFrequency( U32 stream, U32 frequency );

private:
   typedef GFXDevice Parent;
   
   friend class GFXOpenGLTextureObject;
   friend class GFXOpenGLCubemap;
   friend class GFXOpenGLWindowTarget;
   friend class GFXOpenGLPrimitiveBuffer;
   friend class GFXOpenGLVertexBuffer;

   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance; 

   U32 mAdapterIndex;
   
   StrongRefPtr<GFXOpenGLVertexBuffer> mCurrentVB;
   StrongRefPtr<GFXOpenGLPrimitiveBuffer> mCurrentPB;
   
   /// Since GL does not have separate world and view matrices we need to track them
    MatrixF m_mCurrentWorld;
    MatrixF m_mCurrentView;
    MatrixF m_mCurrentProj;
    typedef Vector<MatrixF> MatrixStack;
    MatrixStack m_WorldStack;
    MatrixStack m_ProjectionStack;
    
    
//    GLKMatrixStackRef m_WorldStackRef;
//    GLKMatrixStackRef m_ProjectionStackRef;

//    MatrixF mWorldMatrix[WORLD_STACK_MAX];
    bool    mWorldMatrixDirty;
//    S32     mWorldStackSize;
    
//    MatrixF mProjectionMatrix;
    bool    mProjectionMatrixDirty;
    
//    MatrixF mViewMatrix;
    bool    mViewMatrixDirty;
    
//    MatrixF mTextureMatrix[TEXTURE_STAGE_COUNT];
//    bool    mTextureMatrixDirty;
//    bool    mTextureMatrixCheckDirty;
    
    /// Pushes the world matrix stack and copies the current top
    /// matrix to the new top of the stack
    virtual void pushWorldMatrix();
    
    /// Pops the world matrix stack
    virtual void popWorldMatrix();
    
    virtual void multWorld( const MatrixF &mat );

   NSOpenGLContext* mContext;       // NSOpenGLContext
   NSOpenGLPixelFormat* mPixelFormat;   // NSOpenGLPixelFormat
   void* mTextureLoader; // GLKTextureLoader - for OSX version 10.8

   F32 mPixelShaderVersion;
   
   bool mSupportsAnisotropic;
   
    U32 mMaxShaderTextures;
    U32 mMaxFFTextures;

   RectI mClip;

   GFXOpenGLStateBlockRef mCurrentGLStateBlock;
   
   GLenum mActiveTextureType[TEXTURE_STAGE_COUNT];
   
   Vector< StrongRefPtr<GFXOpenGLVertexBuffer> > mVolatileVBs; ///< Pool of existing volatile VBs so we can reuse previously created ones
   Vector< StrongRefPtr<GFXOpenGLPrimitiveBuffer> > mVolatilePBs; ///< Pool of existing volatile PBs so we can reuse previously created ones

   GLsizei primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount);
   void preDrawPrimitive();
   void postDrawPrimitive(U32 primitiveCount);  
   
   GFXVertexBuffer* findVolatileVBO(U32 numVerts, const GFXVertexFormat *vertexFormat, U32 vertSize, void* data = NULL); ///< Returns an existing volatile VB which has >= numVerts and the same vert flags/size, or creates a new VB if necessary
   GFXPrimitiveBuffer* findVolatilePBO(U32 numIndices, U32 numPrimitives, U16 *indexBuffer = NULL, GFXPrimitive *primitiveBuffer = NULL); ///< Returns an existing volatile PB which has >= numIndices, or creates a new PB if necessary
   
   void initGLState(); ///< Guaranteed to be called after all extensions have been loaded, use to init card profiler, shader version, max samplers, etc.
   
   void initGenericShaders();
    
   void setPB(GFXOpenGLPrimitiveBuffer* pb); ///< Sets mCurrentPB
};

void CheckOpenGLError(const char* stmt, const char* fname, int line);

#ifdef TORQUE_DEBUG
    #define GL_CHECK(stmt) do { \
    stmt; \
    CheckOpenGLError(#stmt, __FILE__, __LINE__); \
} while (0)
#else
    #define GL_CHECK(stmt) stmt
#endif

#endif
