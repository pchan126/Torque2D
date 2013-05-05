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


GFXOpenGLDevice::GFXOpenGLDevice( U32 adapterIndex ) :
            currentCullMode(GFXCullNone),
            mIsBlending( false )
{
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
    AssertFatal(mCurrentRT.isValid(), "GFXOpenGLESDevice::setClipRect - must have a render target set to do any rendering operations!");
    
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
    const F32 near = 0.0f;
    const F32 far = 1.0f;
    
    MatrixF projection(true);
    projection.setOrtho(left, right, bottom, top, near, far);
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

