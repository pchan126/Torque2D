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
#include "./gfxOpenGLDevice.h"
#include "./gfxOpenGLEnumTranslate.h"
#include "./gfxOpenGLTextureObject.h"
#include "./gfxOpenGLVertexBuffer.h"

GFXOpenGLDevice::GFXOpenGLDevice( U32 adapterIndex ) :
            currentCullMode(GFXCullNone),
            mIsBlending( false ),
            mMaxShaderTextures(2),
            m_mCurrentView(true),
            mClip(0, 0, 0, 0),
            mPixelShaderVersion(0.0f)
{
    m_WorldStack.push_back(MatrixF(true));
    m_ProjectionStack.push_back(MatrixF(true));
}


void GFXOpenGLDevice::setCullMode(GFXCullMode mode)
{
    if (mode == currentCullMode)
        return;
    
    // Culling
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

void GFXOpenGLDevice::preDrawPrimitive()
{
    if( mStateDirty )
    {
        updateStates();
    }
    
    if(mCurrentShaderConstBuffer)
        setShaderConstBufferInternal(mCurrentShaderConstBuffer);
}

void GFXOpenGLDevice::postDrawPrimitive(U32 primitiveCount)
{
    //   mDeviceStatistics.mDrawCalls++;
    //   mDeviceStatistics.mPolyCount += primitiveCount;
}


// Given a primitive type and a number of primitives, return the number of indexes/vertexes used.
GLsizei GFXOpenGLDevice::primCountToIndexCount(GFXPrimitiveType primType, U32 primitiveCount)
{
    switch (primType)
    {
        case GFXPointList :
            return primitiveCount;
            break;
        case GFXLineList :
            return primitiveCount * 2;
            break;
        case GFXLineStrip :
            return primitiveCount + 1;
            break;
        case GFXTriangleList :
            return primitiveCount * 3;
            break;
        case GFXTriangleStrip :
            return 2 + primitiveCount;
            break;
        case GFXTriangleFan :
            return 2 + primitiveCount;
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

void GFXOpenGLDevice::drawPrimitive( GFXPrimitiveType primType, U32 vertexStart, U32 primitiveCount )
{
    preDrawPrimitive();

    glDrawArrays(GFXGLPrimType[primType], vertexStart, primCountToIndexCount(primType, primitiveCount));

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
    glDrawElements(GFXGLPrimType[primType], primCountToIndexCount(primType, primitiveCount), GL_UNSIGNED_SHORT, (GLvoid*)0);
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


void GFXOpenGLDevice::setLightInternal(U32 lightStage, const GFXLightInfo light, bool lightEnable)
{
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
    //   // CodeReview - Setting these for front and back is unnecessary.  We should consider
    //   // checking what faces we're culling and setting this only for the unculled faces.
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_AMBIENT, (GLfloat*)&mat.ambient);
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_DIFFUSE, (GLfloat*)&mat.diffuse);
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_SPECULAR, (GLfloat*)&mat.specular);
    //   glMaterialfv(GL_FRONT_AND_BACK, GL_EMISSION, (GLfloat*)&mat.emissive);
    //   glMaterialf(GL_FRONT_AND_BACK, GL_SHININESS, mat.shininess);
}

void GFXOpenGLDevice::setGlobalAmbientInternal(ColorF color)
{
    //   glLightModelfv(GL_LIGHT_MODEL_AMBIENT, (GLfloat*)&color);
}


const MatrixF GFXOpenGLDevice::getMatrix( GFXMatrixType mtype )
{
    MatrixF ret = MatrixF(true);
    switch (mtype)
    {
        case GFXMatrixWorld :
        {
            return m_WorldStack.last();
        }
            break;
        case GFXMatrixView :
        {
            return m_mCurrentView;
        }
            break;
        case GFXMatrixProjection :
        {
            return m_ProjectionStack.last();
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
            m_WorldStack.last() = mat;
        }
            break;
        case GFXMatrixView :
        {
            m_mCurrentView = mat;
        }
            break;
        case GFXMatrixProjection :
        {
            m_ProjectionStack.last() = mat;
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
    else
    {
        mpCurrentShader = NULL;
        glUseProgram(0);
    }
}

void GFXOpenGLDevice::disableShaders()
{
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
    MatrixF newMatrix = m_WorldStack.last();
    m_WorldStack.push_back(newMatrix);
}

inline void GFXOpenGLDevice::popWorldMatrix()
{
    m_WorldStack.pop_back();
}

inline void GFXOpenGLDevice::pushProjectionMatrix()
{
    MatrixF newMatrix = m_ProjectionStack.last();
    m_ProjectionStack.push_back(newMatrix);
}

inline void GFXOpenGLDevice::popProjectionMatrix()
{
    m_ProjectionStack.pop_back();
}


inline void GFXOpenGLDevice::multWorld( const MatrixF &mat )
{
    MatrixF newMatrix = m_WorldStack.last();
    newMatrix*=mat;
    m_WorldStack.last() = newMatrix;
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
    AssertFatal(dynamic_cast<GFXOpenGLStateBlock*>(block), "GFXOpenGLESDevice::setStateBlockInternal - Incorrect stateblock type for this device!");
    GFXOpenGLStateBlock* glBlock = static_cast<GFXOpenGLStateBlock*>(block);
    GFXOpenGLStateBlock* glCurrent = static_cast<GFXOpenGLStateBlock*>(mCurrentStateBlock.getPointer());
    if (force)
        glCurrent = NULL;
    
    glBlock->activate(glCurrent); // Doesn't use current yet.
    mCurrentGLStateBlock = glBlock;
}

void GFXOpenGLDevice::setShaderConstBufferInternal(GFXShaderConstBuffer* buffer)
{
    static_cast<GFXOpenGLShaderConstBuffer*>(buffer)->activate();
}

void GFXOpenGLDevice::setTextureInternal(U32 textureUnit, const GFXTextureObject*texture)
{
    const GFXOpenGLTextureObject *tex = static_cast<const GFXOpenGLTextureObject*>(texture);
    if (tex)
    {
        // GFXOpenGLESTextureObject::bind also handles applying the current sampler state.
        if(mActiveTextureType[textureUnit] != tex->getBinding() && mActiveTextureType[textureUnit] != GL_ZERO)
        {
            glActiveTexture(GL_TEXTURE0 + textureUnit);
            glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        }
        mActiveTextureType[textureUnit] = tex->getBinding();
        tex->bind(textureUnit);
    }
    else if(mActiveTextureType[textureUnit] != GL_ZERO)
    {
        glActiveTexture(GL_TEXTURE0 + textureUnit);
        glBindTexture(mActiveTextureType[textureUnit], GL_ZERO);
        mActiveTextureType[textureUnit] = GL_ZERO;
    }
    glActiveTexture(GL_TEXTURE0);
}

void GFXOpenGLDevice::setVertexStream( U32 stream, GFXVertexBuffer *buffer )
{
    if (stream > 0) return;
    
    AssertFatal( stream == 0, "GFXOpenGLES20Device::setVertexStream - We don't support multiple vertex streams!" );
    
    // Reset the state the old VB required, then set the state the new VB requires.
    if ( mCurrentVB )
        mCurrentVB->finish();
    
    mCurrentVB = static_cast<GFXOpenGLVertexBuffer*>( buffer );
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
//                case GFXTDT_Cube :
//                {
//                    mCurrentCubemap[i] = mNewCubemap[i];
//                    if (mCurrentCubemap[i])
//                        mCurrentCubemap[i]->setToTexUnit(i);
//                    else
//                        setTextureInternal(i, NULL);
//                }
//                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }

//        // Set our material
//        setLightMaterialInternal(mCurrentLightMaterial);
//
//        // Set our lights
//        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
//        {
//            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
//        }

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
//                case GFXTDT_Cube :
//                {
//                    mCurrentCubemap[i] = mNewCubemap[i];
//                    if (mCurrentCubemap[i])
//                        mCurrentCubemap[i]->setToTexUnit(i);
//                    else
//                        setTextureInternal(i, NULL);
//                }
//                    break;
                default:
                    AssertFatal(false, "Unknown texture type!");
                    break;
            }
        }
    }

//    // Set light material
//    if(mLightMaterialDirty)
//    {
//        setLightMaterialInternal(mCurrentLightMaterial);
//        mLightMaterialDirty = false;
//    }
//
//    // Set our lights
//    if(mLightsDirty)
//    {
//        mLightsDirty = false;
//        for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
//        {
//            if(!mLightDirty[i])
//                continue;
//
//            mLightDirty[i] = false;
//            setLightInternal(i, mCurrentLight[i], mCurrentLightEnable[i]);
//        }
//    }

    _updateRenderTargets();

#ifdef TORQUE_DEBUG_RENDER
    doParanoidStateCheck();
#endif
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

