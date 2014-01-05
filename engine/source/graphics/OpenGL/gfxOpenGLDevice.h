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

#ifndef _GFXOPENGLDEVICE_H_
#define _GFXOPENGLDEVICE_H_

#include "graphics/gfxDevice.h"
#include "windowManager/platformWindow.h"
#include "graphics/gfxResource.h"
#include "./gfxOpenGLStateBlock.h"
#include "./gfxOpenGLShader.h"

class GFXOpenGLVertexBuffer;
class GFXOpenGLTextureTarget;
class GFXOpenGLCubemap;

class GFXOpenGLDevice : public GFXDevice
{
public:
   GFXOpenGLDevice( U32 adapterIndex );
   virtual ~GFXOpenGLDevice() {};

   virtual void init( const GFXVideoMode &mode, PlatformWindow *window = NULL ) = 0;

   virtual void activate() { }
   virtual void deactivate() { }

   virtual void enterDebugEvent(ColorI color, const char *name) { }
   virtual void leaveDebugEvent() { }
   virtual void setDebugMarker(ColorI color, const char *name) { }

   virtual void enumerateVideoModes() = 0;

   virtual F32 getFillConventionOffset() const { return 0.0f; }

   ///@}

   /// @name Render Target functions
   /// @{

   virtual GFXTextureTarget *allocRenderToTextureTarget() = 0;
   virtual GFXWindowTarget *allocWindowTarget(PlatformWindow *window) = 0;
   virtual void _updateRenderTargets() = 0;

   ///@}
    GFXOpenGLShader* mpCurrentShader;
    GFXOpenGLShader* mGenericShader[5];

    virtual void setShader(GFXShader* shd);
    virtual void disableShaders(); ///< Equivalent to setShader(NULL)
   
   /// @attention GL cannot check if the given format supports blending or filtering!
   virtual GFXFormat selectSupportedFormat(GFXTextureProfile *profile,
                                           const Vector<GFXFormat> &formats, bool texture, bool mustblend, bool mustfilter);
    
   /// Returns the number of texture samplers that can be used in a shader rendering pass
    virtual U32 getNumSamplers() const { return mMaxShaderTextures; }

   /// Returns the number of simultaneous render targets supported by the device.
    virtual U32 getNumRenderTargets() const { return 1; };

   virtual void clear( U32 flags, ColorI color, F32 z, U32 stencil ) = 0;
   virtual bool beginSceneInternal();
   virtual void endSceneInternal();

   virtual void drawPrimitive( GFXPrimitiveType primType, size_t vertexStart, size_t primitiveCount );

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
    
   virtual void setupGenericShaders( GenericShaderType type = GSColor ) = 0;
   
   ///
   bool supportsAnisotropic() const { return mSupportsAnisotropic; }
   GLsizei primCountToIndexCount(GFXPrimitiveType primType, size_t primitiveCount);

    typedef Vector<MatrixF> MatrixStack;
    
    /// Since GL does not have separate world and view matrices we need to track them
    MatrixStack m_ViewStack;
    MatrixStack m_WorldStack;
    MatrixStack m_ProjectionStack;
    
    virtual void pushWorldMatrix();
    virtual void popWorldMatrix();
    virtual void pushProjectionMatrix();
    virtual void popProjectionMatrix();
    virtual void pushViewMatrix();
    virtual void popViewMatrix();

    virtual void multWorld( const MatrixF &mat );

    virtual F32 getPixelShaderVersion() const { return mPixelShaderVersion; }
    virtual void  setPixelShaderVersion( F32 version ) { mPixelShaderVersion = version; }
    
    virtual const GFXOpenGLStateBlockRef getCurrentStateBlock() { return mCurrentGLStateBlock;};

    virtual void updateStates(bool forceSetAll = false);

/// OpenGL state calls
   virtual void setTextureUnit( U32 texUnit);

protected:
   U32 mActiveTextureUnit;
   
protected:

   virtual void preDrawPrimitive();
    void postDrawPrimitive(size_t primitiveCount);

    /// Called by GFXDevice to create a device specific stateblock
   virtual GFXStateBlockRef createStateBlockInternal(const GFXStateBlockDesc& desc);
   /// Called by GFXDevice to actually set a stateblock.
   virtual void setStateBlockInternal(GFXStateBlock* block, bool force);

    virtual void setCullMode( GFXCullMode );
    virtual void setBlending( bool DoesItBlend );
    virtual void setBlendFunc( GFXBlend blendSrc, GFXBlend blendDest);
    virtual void setBlendEquation( GFXBlendOp blendOp);
    virtual void setBlendFuncSeparate( GFXBlend srcRGB, GFXBlend dstRGB, GFXBlend srcAlpha, GFXBlend dstAlpha);
    virtual void setBlendEquationSeparate( GFXBlendOp opRGB, GFXBlendOp opAlpha);

    virtual void setDepthTest( bool enable );
    virtual void setDepthMask( bool flag );
    virtual void setDepthFunc( GFXCmpFunc func );

    virtual void setColorMask(bool colorWriteRed, bool colorWriteBlue, bool colorWriteGreen, bool colorWriteAlpha);

    virtual void setStencilEnable(bool enable);
    virtual void setStencilFunc( GFXCmpFunc func, U32 ref, U32 mask);
    virtual void setStencilOp(GFXStencilOp failOp, GFXStencilOp zFailOp, GFXStencilOp passOp);
    virtual void setStencilWriteMask( U32 writeMask );

   /// Called by base GFXDevice to actually set a const buffer
   virtual void setShaderConstBufferInternal(GFXShaderConstBuffer* buffer);

   virtual void setTextureInternal(U32 textureUnit, GFXTextureObject* texture);
   virtual void setCubemapInternal(U32 cubemap, GFXOpenGLCubemap* texture);

   Vector<GFXLightInfo> m_lightStack;
   virtual void setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable);
   GFXLightMaterial mCurrentLightMaterial;
   virtual void setLightMaterialInternal(const GFXLightMaterial mat);
   ColorF m_globalAmbientColor;
   virtual void setGlobalAmbientInternal(ColorF color);

   /// @name State Initalization.
   /// @{

   /// State initalization. This MUST BE CALLED in setVideoMode after the device
   /// is created.
    virtual void initStates() = 0;

   virtual const MatrixF getMatrix( GFXMatrixType mtype );
   virtual void setMatrix( GFXMatrixType mtype, const MatrixF &mat );

   virtual inline const MatrixF getWorldMatrix() { return getMatrix(GFXMatrixWorld);};
   virtual inline const MatrixF getProjectionMatrix() { return getMatrix(GFXMatrixProjection);};
   virtual inline const MatrixF getViewMatrix() { return getMatrix(GFXMatrixView);};
   

   virtual GFXVertexBuffer *allocVertexBuffer(  dsize_t numVerts,
                                                const GFXVertexFormat *vertexFormat,
                                                dsize_t vertSize,
                                                GFXBufferType bufferType,
                                                void *data,
                                              dsize_t indexCount = 0,
                                              void *indexData = nullptr) = 0;
   
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

    GFXCullMode currentCullMode;
    U32 mMaxShaderTextures;

    void _handleTextureLoaded(GFXTexNotifyCode code);
private:
   typedef GFXDevice Parent;

   friend class GFXOpenGLTextureObject;
   friend class GFXOpenGLCubemap;
   friend class GFXOpenGLWindowTarget;
   friend class GFXOpenGLVertexBuffer;

protected:
   static GFXAdapter::CreateDeviceInstanceDelegate mCreateDeviceInstance;

   U32 mAdapterIndex;
   
   StrongRefPtr<GFXOpenGLVertexBuffer> mCurrentVB;
    
   F32 mPixelShaderVersion;
    bool mSupportsAnisotropic;
    bool mIsBlending;
    GFXBlend mBlendSrcState;
    GFXBlend mBlendDestState;
    GFXBlendOp mBlendOp;

   GFXFillMode mFillMode;

   bool separateAlphaBlendDefined;
   bool separateAlphaBlendEnable;
   GFXBlend separateAlphaBlendSrc;
   GFXBlend separateAlphaBlendDest;
   GFXBlendOp separateAlphaBlendOp;

    bool depthTest;
    bool depthMask;
    GFXCmpFunc depthFunc;

   bool mColorWriteRed;
    bool mColorWriteBlue;
    bool mColorWriteGreen;
    bool mColorWriteAlpha;

    bool mStencilEnable;
    GFXStencilOp mStencilFailOp;
    GFXStencilOp mStencilZFailOp;
    GFXStencilOp mStencilPassOp;
    GFXCmpFunc  mStencilFunc;
    U32 mStencilRef;
    U32 mStencilMask;
    U32 mStencilWriteMask;


    U32 mMaxFFTextures;
    RectI mClip;

   GFXOpenGLStateBlockRef mCurrentGLStateBlock;
   
   GLenum mActiveTextureType[TEXTURE_STAGE_COUNT];
   
   Vector< StrongRefPtr<GFXOpenGLVertexBuffer> > mVolatileVBs; ///< Pool of existing volatile VBs so we can reuse previously created ones
   
   virtual void initGLState() = 0; ///< Guaranteed to be called after all extensions have been loaded, use to init card profiler, shader version, max samplers, etc.
   
   virtual void initGenericShaders() = 0;

   virtual void setFillMode( GFXFillMode fillMode ) = 0;
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
