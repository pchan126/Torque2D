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

#include "platform/platform.h"
#include "platform/platformGL.h"
#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "./gfxOpenGLTextureObject.h"
#include "./gfxOpenGLCubemap.h"
#include "./gfxOpenGLVertexBuffer.h"

GFXOpenGLDevice::GFXOpenGLDevice( U32 adapterIndex ) :
            currentCullMode(GFXCullNone),
            mIsBlending( false ),
            mMaxShaderTextures(2),
            mClip(0, 0, 0, 0),
            mPixelShaderVersion(0.0f),
            mBlendSrcState(GFXBlendSrcAlpha),
            mBlendDestState(GFXBlendInvSrcAlpha),
            mBlendOp(GFXBlendOpAdd),
            separateAlphaBlendDefined(false),
            separateAlphaBlendEnable(false),
            separateAlphaBlendSrc(GFXBlendOne),
            separateAlphaBlendDest(GFXBlendZero),
            separateAlphaBlendOp(GFXBlendOpAdd),
            m_globalAmbientColor(1.0, 1.0, 1.0, 1.0),
            mActiveTextureUnit(0)
{
    m_WorldStack.push_back(MatrixF(true));
    m_ProjectionStack.push_back(MatrixF(true));
    m_ViewStack.push_back(MatrixF(true));
    m_lightStack.setSize(LIGHT_STAGE_COUNT);

    for (int i = 0; i < TEXTURE_STAGE_COUNT; i++)
        mActiveTextureType[i] = GL_TEXTURE_2D;
}


void GFXOpenGLDevice::setCullMode(GFXCullMode mode)
{
    if (mode == currentCullMode)
        return;
    
    // Culling
    currentCullMode = mode;
    if (mode == GFXCullNone)
    {
        glDisable(GL_CULL_FACE);
    }
    else
    {
        glEnable(GL_CULL_FACE);
        glCullFace(GFXGLCullMode[mode]);
    }
}


void GFXOpenGLDevice::setBlending( bool DoesItBlend )
{
   if (mIsBlending != DoesItBlend)
   {
      if (DoesItBlend)
      {
         glEnable(GL_BLEND);
      }
      else
      {
         glDisable(GL_BLEND);
      }
      mIsBlending = DoesItBlend;
   }
}

void GFXOpenGLDevice::setBlendFunc( GFXBlend blendSrc, GFXBlend blendDest )
{
    if (mBlendSrcState != blendSrc || mBlendDestState != blendDest)
    {
        mBlendSrcState = blendSrc;
        mBlendDestState = blendDest;
        glBlendFunc(GFXGLBlend[mBlendSrcState], GFXGLBlend[mBlendDestState]);
    }
}

void GFXOpenGLDevice::setBlendFuncSeparate( GFXBlend srcRGB, GFXBlend dstRGB, GFXBlend srcAlpha, GFXBlend dstAlpha)
{
    if (mBlendSrcState != srcRGB || mBlendDestState != dstRGB)
    {
        mBlendSrcState = srcRGB;
        mBlendDestState = dstRGB;
        glBlendFuncSeparate(GFXGLBlend[mBlendSrcState], GFXGLBlend[mBlendDestState], GFXGLBlend[separateAlphaBlendSrc], GFXGLBlend[separateAlphaBlendDest]);
    }
}

void GFXOpenGLDevice::setBlendEquation( GFXBlendOp blendOp)
{
    if (mBlendOp != blendOp)
    {
        mBlendOp = blendOp;
        glBlendEquation(GFXGLBlendOp[mBlendOp]);
    }
}

void GFXOpenGLDevice::setBlendEquationSeparate( GFXBlendOp opRGB, GFXBlendOp opAlpha)
{
    if (mBlendOp != opRGB)
    {
        mBlendOp = opRGB;
        separateAlphaBlendOp = opAlpha;
        glBlendEquationSeparate(GFXGLBlendOp[mBlendOp], GFXGLBlendOp[separateAlphaBlendOp]);
    }
}

void GFXOpenGLDevice::setDepthTest(bool enable)
{
    if (depthTest != enable)
    {
        if (enable)
        {
            glEnable(GL_DEPTH_TEST);
        }
        else
        {
            glDisable(GL_DEPTH_TEST);
        }
        depthTest = enable;
    }
}


void GFXOpenGLDevice::setDepthFunc( GFXCmpFunc func )
{
   if ( depthFunc != func )
   {
       glDepthFunc(GFXGLCmpFunc[func]);
       depthFunc = func;
   }
}


void GFXOpenGLDevice::setDepthMask(bool flag)
{
    if (depthMask != flag)
    {
        glDepthMask(flag);
        depthMask = flag;
    }
}


void GFXOpenGLDevice::setStencilEnable(bool enable)
{
    if (mStencilEnable != enable)
    {
        if (enable)
        {
            glEnable(GL_STENCIL_TEST);
        }
        else
        {
            glDisable(GL_STENCIL_TEST);
        }
        mStencilEnable = enable;
    }
}

void GFXOpenGLDevice::setStencilFunc( GFXCmpFunc func, U32 ref, U32 mask)
{
    if ( mStencilFunc != func || mStencilRef != ref || mStencilMask != mask )
    {
        glStencilFunc(GFXGLCmpFunc[func], ref, mask);
        mStencilFunc = func;
        mStencilRef = ref;
        mStencilMask = mask;
    }
}

void GFXOpenGLDevice::setStencilOp(GFXStencilOp failOp, GFXStencilOp zFailOp, GFXStencilOp passOp)
{
    if ( mStencilFailOp != failOp || mStencilZFailOp != zFailOp || mStencilPassOp != passOp )
    {
        glStencilOp(GFXGLStencilOp[failOp], GFXGLStencilOp[zFailOp], GFXGLStencilOp[passOp]);
        mStencilFailOp = failOp;
        mStencilZFailOp = zFailOp;
        mStencilPassOp = passOp;
    }
}

void GFXOpenGLDevice::setStencilWriteMask( U32 writeMask )
{
    if (mStencilWriteMask != writeMask)
    {
        mStencilMask = writeMask;
        glStencilMask(writeMask);
    }
}

void GFXOpenGLDevice::setColorMask(bool colorWriteRed, bool colorWriteBlue, bool colorWriteGreen, bool colorWriteAlpha)
{
    // Color write masks
    if (colorWriteRed != mColorWriteRed || colorWriteBlue != mColorWriteBlue || colorWriteGreen != mColorWriteGreen || colorWriteAlpha != mColorWriteAlpha)
    {
        mColorWriteRed = colorWriteRed;
        mColorWriteBlue = colorWriteBlue;
        mColorWriteGreen = colorWriteGreen;
        mColorWriteAlpha = colorWriteAlpha;
        glColorMask(mColorWriteRed, mColorWriteBlue, mColorWriteGreen, mColorWriteAlpha);
    }
}


void GFXOpenGLDevice::preDrawPrimitive()
{
    if( mStateDirty )
    {
        updateStates();
    }
    
    if(mCurrentShaderConstBuffer)
        setShaderConstBufferInternal(mCurrentShaderConstBuffer);
}

void GFXOpenGLDevice::postDrawPrimitive(size_t primitiveCount)
{
   mDeviceStatistics.mDrawCalls++;
   mDeviceStatistics.mPolyCount += primitiveCount;
}


// Given a primitive type and a number of primitives, return the number of indexes/vertexes used.
GLsizei GFXOpenGLDevice::primCountToIndexCount(GFXPrimitiveType primType, size_t primitiveCount)
{
    switch (primType)
    {
        case GFXPointList :
            return (GLsizei)primitiveCount;
            break;
        case GFXLineList :
            return (GLsizei)primitiveCount * 2;
            break;
        case GFXLineStrip :
            return (GLsizei)primitiveCount + 1;
            break;
        case GFXTriangleList :
            return (GLsizei)primitiveCount * 3;
            break;
        case GFXTriangleStrip :
            return 2 + (GLsizei)primitiveCount;
            break;
        case GFXTriangleFan :
            return 2 + (GLsizei)primitiveCount;
            break;
        default:
            AssertFatal(false, "GFXOpenGLDevice::primCountToIndexCount - unrecognized prim type");
            break;
    }
    
    return 0;
}



bool GFXOpenGLDevice::beginSceneInternal()
{
    // Nothing to do here for GL.
    mCanCurrentlyRender = true;
    return true;
}

void GFXOpenGLDevice::endSceneInternal()
{
    // nothing to do for opengl
    mCanCurrentlyRender = false;
}

void GFXOpenGLDevice::drawPrimitive( GFXPrimitiveType primType, size_t vertexStart, size_t primitiveCount )
{
    preDrawPrimitive();

    glDrawArrays(GFXGLPrimType[primType], (GLint)vertexStart, primCountToIndexCount(primType, primitiveCount));

    postDrawPrimitive(primitiveCount);
}

void GFXOpenGLDevice::drawIndexedPrimitive(   GFXPrimitiveType primType,
                                             U32 startVertex,
                                             U32 minIndex,
                                             U32 numVerts,
                                             U32 startIndex,
                                             U32 primitiveCount )
{
    AssertFatal( startVertex == 0, "GFXOpenGLDevice::drawIndexedPrimitive() - Non-zero startVertex unsupported!" );
    
    preDrawPrimitive();
    int count = primCountToIndexCount(primType, primitiveCount);
    glDrawElements(GFXGLPrimType[primType], count, GL_UNSIGNED_SHORT, (GLvoid*)0);
    postDrawPrimitive(primitiveCount);
}


void GFXOpenGLDevice::setClipRect( const RectI &inRect )
{
    AssertFatal(mCurrentRT.isValid(), "GFXOpenGLDevice::setClipRect - must have a render target set to do any rendering operations!");
    
    // Clip the rect against the renderable size.
    Point2I size = mCurrentRT->getSize();
    RectI maxRect(Point2I(0,0), size);
    mClip = inRect;
    mClip.intersect(maxRect);
    
    // Create projection matrix.  See http://www.opengl.org/documentation/specs/man_pages/hardcopy/GL/html/gl/ortho.html
    const F32 left = mClip.centre().x - (mClip.extent.x)/2;
    const F32 right = mClip.centre().x + (mClip.extent.x)/2;
    const F32 bottom = mClip.centre().y + mClip.extent.y / 2;
    const F32 top = mClip.centre().y - mClip.extent.y / 2;
    const F32 p_near = 0.0f;
    const F32 p_far = 1.0f;
    
    MatrixF projection(true);
    projection.setOrtho(left, right, bottom, top, p_near, p_far);
    setMatrix(GFXMatrixProjection, projection);
    
    MatrixF mTempMatrix(true);
    setViewMatrix( mTempMatrix );
    setWorldMatrix( mTempMatrix );
    
    // Set the viewport to the clip rect (with y flip)
    RectI viewport(mClip.point.x, size.y - (mClip.point.y + mClip.extent.y), mClip.extent.x, mClip.extent.y);
    setViewport(viewport);
}

const MatrixF GFXOpenGLDevice::getMatrix( GFXMatrixType mtype )
{
    MatrixF ret = MatrixF(true);
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            return m_WorldStack.back();
        }
            break;
        case GFXMatrixView :
        {
            return m_ViewStack.back();
        }
            break;
        case GFXMatrixProjection :
        {
            return m_ProjectionStack.back();
        }
            break;
            // CodeReview - Add support for texture transform matrix types
        default:
            AssertFatal(false, "GFXOpenGLESDevice::setMatrix - Unknown matrix mode!");
    }
    return ret;
}

void GFXOpenGLDevice::setMatrix( GFXMatrixType mtype, const MatrixF &mat )
{
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            m_WorldStack.back() = mat;
        }
            break;
        case GFXMatrixView :
        {
            m_ViewStack.back() = mat;
        }
            break;
        case GFXMatrixProjection :
        {
            m_ProjectionStack.back() = mat;
        }
            break;
            // CodeReview - Add support for texture transform matrix types
        default:
            AssertFatal(false, "GFXOpenGL33WinDevice::setMatrix - Unknown matrix mode!");
            return;
    }
}

void GFXOpenGLDevice::setShader( GFXShader *shader )
{
    GFXOpenGLShader* iOSShader = dynamic_cast<GFXOpenGLShader*>(shader);
    if ( shader )
    {
        if (shader != mpCurrentShader)
        {
            mpCurrentShader = iOSShader;
            iOSShader->useProgram();
        }
    }
//    else
//    {
//        mpCurrentShader = NULL;
//        glUseProgram(0);
//    }
}

void GFXOpenGLDevice::disableShaders()
{
//    mpCurrentShader = NULL;
//    glUseProgram(0);
}

GFXFormat GFXOpenGLDevice::selectSupportedFormat(   GFXTextureProfile* profile,
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


inline void GFXOpenGLDevice::pushWorldMatrix()
{
    MatrixF newMatrix = m_WorldStack.back();
    m_WorldStack.push_back(newMatrix);
}

inline void GFXOpenGLDevice::popWorldMatrix()
{
    m_WorldStack.pop_back();
}

inline void GFXOpenGLDevice::pushProjectionMatrix()
{
    MatrixF newMatrix = m_ProjectionStack.back();
    m_ProjectionStack.push_back(newMatrix);
}

inline void GFXOpenGLDevice::popProjectionMatrix()
{
    m_ProjectionStack.pop_back();
}


inline void GFXOpenGLDevice::pushViewMatrix()
{
    MatrixF newMatrix = m_ViewStack.back();
    m_ViewStack.push_back(newMatrix);
}

inline void GFXOpenGLDevice::popViewMatrix()
{
    m_ViewStack.pop_back();
}


inline void GFXOpenGLDevice::multWorld( const MatrixF &mat )
{
    MatrixF newMatrix = m_WorldStack.back();
    newMatrix*=mat;
    m_WorldStack.back() = newMatrix;
}



/// Creates a state block object based on the desc passed in.  This object
/// represents an immutable state.
GFXStateBlockRef GFXOpenGLDevice::createStateBlockInternal(const GFXStateBlockDesc& desc)
{
    GFXOpenGLStateBlockRef ret = new GFXOpenGLStateBlock(desc);
    ret->setView(getMatrix(GFXMatrixView));
    ret->setModel(getMatrix(GFXMatrixWorld));
    ret->setProjection(getMatrix(GFXMatrixProjection));
    return GFXStateBlockRef(ret);
}

/// Activates a stateblock
void GFXOpenGLDevice::setStateBlockInternal(GFXStateBlock* block, bool force)
{
    AssertFatal(dynamic_cast<GFXOpenGLStateBlock*>(block), "GFXOpenGLDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
    GFXOpenGLStateBlock* glBlock = static_cast<GFXOpenGLStateBlock*>(block);
    GFXOpenGLStateBlock* glCurrent = static_cast<GFXOpenGLStateBlock*>(mCurrentStateBlock.getPointer());
    if (force)
        glCurrent = NULL;

    const GFXStateBlockDesc& desc = glBlock->getDesc();

    // Blending
    setBlending(desc.blendEnable);
    if (desc.separateAlphaBlendEnable)
    {
        setBlendFuncSeparate(desc.blendSrc, desc.blendDest, desc.separateAlphaBlendSrc, desc.separateAlphaBlendDest);
        setBlendEquationSeparate(desc.blendOp, desc.separateAlphaBlendOp);
    }
    else
    {
        setBlendFunc(desc.blendSrc, desc.blendDest);
        setBlendEquation(desc.blendOp);
    }


    setColorMask(desc.colorWriteRed, desc.colorWriteBlue, desc.colorWriteGreen, desc.colorWriteAlpha);
    setCullMode(desc.cullMode);

    // Depth
    if (desc.zDefined)
    {
        setDepthTest(desc.zEnable);
        setDepthFunc(desc.zFunc);
        setDepthMask(desc.zWriteEnable);
    }

    // Stencil
    if (desc.stencilDefined)
    {
        setStencilEnable(desc.stencilEnable);
        setStencilFunc(desc.stencilFunc, desc.stencilRef, desc.stencilMask);
        setStencilOp(desc.stencilFailOp, desc.stencilZFailOp, desc.stencilPassOp);
        setStencilWriteMask(desc.stencilWriteMask);
    }

    setFillMode(desc.fillMode);

    mCurrentGLStateBlock = glBlock;
}

void GFXOpenGLDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGLShaderConstBuffer*>(buffer)->activate();
}

void GFXOpenGLDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
   m_lightStack[lightStage] = light;
//   if(!lightEnable)
//   {
//      glDisable(GL_LIGHT0 + lightStage);
//      return;
//   }
//   
//   if(light.mType == GFXLightInfo::Ambient)
//   {
//      AssertFatal(false, "Instead of setting an ambient light you should set the global ambient color.");
//      return;
//   }
//   
//   GLenum lightEnum = GL_LIGHT0 + lightStage;
//   glLightfv(lightEnum, GL_AMBIENT, (GLfloat*)&light.mAmbient);
//   glLightfv(lightEnum, GL_DIFFUSE, (GLfloat*)&light.mColor);
//   glLightfv(lightEnum, GL_SPECULAR, (GLfloat*)&light.mColor);
//   
//   F32 pos[4];
//   
//   if(light.mType != GFXLightInfo::Vector)
//   {
//      dMemcpy(pos, &light.mPos, sizeof(light.mPos));
//      pos[3] = 1.0;
//   }
//   else
//   {
//      dMemcpy(pos, &light.mDirection, sizeof(light.mDirection));
//      pos[3] = 0.0;
//   }
//   // Harcoded attenuation
//   glLightf(lightEnum, GL_CONSTANT_ATTENUATION, 1.0f);
//   glLightf(lightEnum, GL_LINEAR_ATTENUATION, 0.1f);
//   glLightf(lightEnum, GL_QUADRATIC_ATTENUATION, 0.0f);
//   
//   glLightfv(lightEnum, GL_POSITION, (GLfloat*)&pos);
//   glEnable(lightEnum);
}

void GFXOpenGLDevice::setLightMaterialInternal(const GFXLightMaterial mat)
{
   mCurrentLightMaterial = mat;
}

void GFXOpenGLDevice::setGlobalAmbientInternal(ColorF color)
{
   m_globalAmbientColor = color;
}

void GFXOpenGLDevice::setTextureInternal(U32 textureUnit, GFXTextureObject*texture)
{
    GFXOpenGLTextureObject *tex = static_cast<GFXOpenGLTextureObject*>(texture);
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


void GFXOpenGLDevice::setCubemapInternal(U32 textureUnit, GFXOpenGLCubemap* texture)
{
   setTextureUnit(textureUnit);
   if(texture)
   {
      if(mActiveTextureType[textureUnit] != GL_TEXTURE_CUBE_MAP && mActiveTextureType[textureUnit] != GL_ZERO)
      {
         glBindTexture(mActiveTextureType[textureUnit], 0);
         glDisable(mActiveTextureType[textureUnit]);
      }
      mActiveTextureType[textureUnit] = GL_TEXTURE_CUBE_MAP;
      texture->bind(textureUnit);
   }
   else if(mActiveTextureType[textureUnit] != GL_ZERO)
   {
      glBindTexture(mActiveTextureType[textureUnit], 0);
      glDisable(mActiveTextureType[textureUnit]);
      mActiveTextureType[textureUnit] = GL_ZERO;
   }
   setTextureUnit(0);
}

void GFXOpenGLDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGLDevice::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    GFXOpenGLVertexBuffer* nextVB = static_cast<GFXOpenGLVertexBuffer*>( buffer );
   if (nextVB == mCurrentVB)
      return;

    if ( mCurrentVB )
        mCurrentVB->finish();
   
    mCurrentVB = nextVB;
    
    if ( mCurrentVB )
        mCurrentVB->prepare();
}

void GFXOpenGLDevice::_handleTextureLoaded(GFXTexNotifyCode code)
{
    mTexturesDirty = true;
}


void GFXOpenGLDevice::updateStates(bool forceSetAll /*=false*/)
{
    PROFILE_SCOPE(GFXDevice_updateStates);

    if(forceSetAll)
    {
        bool rememberToEndScene = false;
        if(!canCurrentlyRender())
        {
            if (!beginScene())
            {
                AssertFatal(false, "GFXDevice::updateStates:  Unable to beginScene!");
            }
            rememberToEndScene = true;
        }

        setVertexDecl( mCurrVertexDecl );

        for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
        {
            setVertexStream( i, mCurrentVertexBuffer[i] );
        }

        /// Stateblocks
        if ( mNewStateBlock )
            setStateBlockInternal(mNewStateBlock, true);
        mCurrentStateBlock = mNewStateBlock;

        for(U32 i = 0; i < getNumSamplers(); i++)
        {
            switch (mTexType[i])
            {
                case GFXTDT_Normal :
                {
                    mCurrentTexture[i] = mNewTexture[i];
                    setTextureInternal(i, mCurrentTexture[i]);
                }
                    break;
                case GFXTDT_Cube :
                {
                    mCurrentCubemap[i] = mNewCubemap[i];
                    if (mCurrentCubemap[i])
                       static_cast<GFXOpenGLCubemap*>(mCurrentCubemap[i].getPointer())->setToTexUnit(i);
                    else
                        setTextureInternal(i, NULL);
                }
                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }

        // Set our material
        setLightMaterialInternal(mCurrentLightMaterial);

        // Set our lights
        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
        {
            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
        }

        _updateRenderTargets();

        if(rememberToEndScene)
            endScene();

        return;
    }

    if (!mStateDirty)
        return;

    // Normal update logic begins here.
    mStateDirty = false;

    // Update the vertex declaration.
    if ( mVertexDeclDirty )
    {
        setVertexDecl( mCurrVertexDecl );
        mVertexDeclDirty = false;
    }

    // Update the vertex buffers.
    for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
    {
        if ( mVertexBufferDirty[i] )
        {
            setVertexStream( i, mCurrentVertexBuffer[i] );
            mVertexBufferDirty[i] = false;
        }

        if ( mVertexBufferFrequencyDirty[i] )
        {
            mVertexBufferFrequencyDirty[i] = false;
        }
    }

    // NOTE: With state blocks, it's now important to update state before setting textures
    // some devices (e.g. OpenGL) set states on the texture and we need that information before
    // the texture is activated.
    if (mStateBlockDirty)
    {
        setStateBlockInternal(mNewStateBlock, false);
        mCurrentStateBlock = mNewStateBlock;
        mStateBlockDirty = false;
    }

    if( mTexturesDirty )
    {
        mTexturesDirty = false;
        for(U32 i = 0; i < getNumSamplers(); i++)
        {
            if(!mTextureDirty[i])
                continue;
            mTextureDirty[i] = false;

            switch (mTexType[i])
            {
                case GFXTDT_Normal :
                {
                    mCurrentTexture[i] = mNewTexture[i];
                    setTextureInternal(i, mCurrentTexture[i]);
                }
                    break;
                case GFXTDT_Cube :
                {
                    mCurrentCubemap[i] = mNewCubemap[i];
                    if (mCurrentCubemap[i])
                        static_cast<GFXOpenGLCubemap*>(mCurrentCubemap[i].getPointer())->setToTexUnit(i);
                    else
                        setTextureInternal(i, NULL);
                }
                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }
    }

    // Set light material
    if(mLightMaterialDirty)
    {
        setLightMaterialInternal(mCurrentLightMaterial);
        mLightMaterialDirty = false;
    }

    // Set our lights
    if(mLightsDirty)
    {
        mLightsDirty = false;
        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
        {
            if(!mLightDirty[i])
                continue;

            mLightDirty[i] = false;
            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
        }
    }

    _updateRenderTargets();

#ifdef TORQUE_DEBUG_RENDER
    doParanoidStateCheck();
#endif
}

void GFXOpenGLDevice::setTextureUnit(U32 texUnit)
{
   if (mActiveTextureUnit != texUnit)
   {
      mActiveTextureUnit = texUnit;
      glActiveTexture(GL_TEXTURE0 + texUnit);
   }
}




//-----------------------------------------------------------------------------


void CheckOpenGLError(const char* stmt, const char* fname, int line)
{
    GLenum err = glGetError();
    if (err != GL_NO_ERROR)
    {
        printf("OpenGL error %08x, at %s:%i - for %s\n", err, fname, line, stmt);
        abort();
    }
}

