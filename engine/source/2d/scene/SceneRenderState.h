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

#ifndef _SCENE_RENDER_STATE_H_
#define _SCENE_RENDER_STATE_H_

#ifndef _COLOR_H_
#include "graphics/color.h"
#endif
#ifndef _VECTOR2_H_
#include "2d/core/vector2.h"
#endif

#include "2d/gui/SceneWindow.h"

//-----------------------------------------------------------------------------

class GuiControl;
class RectF;
class DebugStats;

struct b2AABB;

class RenderPassManager;
//-----------------------------------------------------------------------------

/// The type of scene pass.
/// @see SceneManager
/// @see SceneRenderState
enum ScenePassType
{
   /// The regular diffuse scene pass.
   SPT_Diffuse,
   
   /// The scene pass made for reflection rendering.
   SPT_Reflect,
   
   /// The scene pass made for shadow map rendering.
   SPT_Shadow,
   
   /// A scene pass that isn't one of the other
   /// predefined scene pass types.
   SPT_Other,
};

class SceneRenderState
{
//public:
//   
//   /// The delegate used for material overrides.
//   /// @see getOverrideMaterial
//   typedef Delegate< BaseMatInstance*( BaseMatInstance* ) > MatDelegate;

protected:
   /// The type of scene render pass we're doing.
   ScenePassType mScenePassType;

   /// The render pass which we are setting up with this scene state.
   RenderPassManager* mRenderPass;
   ///
   MatrixF mDiffuseCameraTransform;

public:
   
   /// Construct a new SceneRenderState.
   ///
   /// @param sceneManager SceneManager rendered in this SceneRenderState.
   /// @param passType Type of rendering pass that the SceneRenderState is for.
   /// @param view The view that is being rendered
   /// @param renderPass The render pass which is being set up by this SceneRenderState.  If NULL,
   ///   then Scene::getDefaultRenderPass() is used.
   /// @param usePostEffect Whether PostFX are enabled in the rendering pass.
   SceneRenderState( const CameraView renderCamera,
                    U32 renderGroupMask,
                    DebugStats* pDebugStats,
                    SimObject* pRenderHost );
   
   ~SceneRenderState();
   
    CameraView      mRenderCamera;
   
    U32             mRenderGroupMask;
    DebugStats*     mpDebugStats;
    SimObject*      mpRenderHost;

   /// @name Passes
   /// @{
   
   /// Return the RenderPassManager that manages rendering objects batched
   /// for this SceneRenderState.
   RenderPassManager* getRenderPass() const { return mRenderPass; }
   
   /// Returns the type of scene rendering pass that we're doing.
   ScenePassType getScenePassType() const { return mScenePassType; }
   
   /// Returns true if this is a diffuse scene rendering pass.
   bool isDiffusePass() const { return mScenePassType == SPT_Diffuse; }
   
   /// Returns true if this is a reflection scene rendering pass.
   bool isReflectPass() const { return mScenePassType == SPT_Reflect; }
   
   /// Returns true if this is a shadow scene rendering pass.
   bool isShadowPass() const { return mScenePassType == SPT_Shadow; }
   
   /// Returns true if this is not one of the other rendering passes.
   bool isOtherPass() const { return mScenePassType >= SPT_Other; }
   
   /// @}

   /// @name Transforms, projections, and viewports.
   /// @{
   
//   /// Return the screen-space viewport rectangle.
//   const RectI& getViewport() const { return getCullingState().getCameraState().getViewport(); }
   
   /// Return the world->view transform matrix.
   const MatrixF& getWorldViewMatrix() const;
   
   /// Return the project transform matrix.
   const MatrixF& getProjectionMatrix() const;
   
//   /// Returns the actual camera position.
//   /// @see getDiffuseCameraPosition
//   const Point3F& getCameraPosition() const { return getCullingState().getCameraState().getViewPosition(); }
//   
//   /// Returns the camera transform (view->world) this SceneRenderState is using.
//   const MatrixF& getCameraTransform() const { return getCullingState().getCameraState().getViewWorldMatrix(); }
   
//   /// Returns the minimum distance something must be from the camera to not be culled.
//   F32 getNearPlane() const { return getFrustum().getNearDist();   }
//   
//   /// Returns the maximum distance something can be from the camera to not be culled.
//   F32 getFarPlane() const { return getFrustum().getFarDist();    }
   
//   /// Returns the camera vector normalized to 1 / far distance.
//   const Point3F& getVectorEye() const { return mVectorEye; }
   
//   /// Returns the possibly overloaded world to screen scale.
//   /// @see projectRadius
//   const Point2F& getWorldToScreenScale() const { return mWorldToScreenScale; }
   
//   /// Set a new world to screen scale to overload
//   /// future screen metrics operations.
//   void setWorldToScreenScale( const Point2F& scale ) { mWorldToScreenScale = scale; }
   
   /// Returns the pixel size of the radius projected to the screen at a desired distance.
   ///
   /// Internally this uses the stored world to screen scale and viewport extents.  This
   /// allows the projection to be overloaded in special cases like when rendering shadows
   /// or reflections.
   ///
   /// @see getWorldToScreenScale
   /// @see getViewportExtent
//   F32 projectRadius( F32 dist, F32 radius ) const
//   {
//      // We fixup any negative or zero distance
//      // so we don't get a divide by zero.
//      dist = dist > 0.0f ? dist : 0.001f;
//      return ( radius / dist ) * mWorldToScreenScale.y;
//   }
   
   /// Returns the camera position used during the diffuse rendering pass which may be different
   /// from the actual camera position.
   ///
   /// This is useful when doing level of detail calculations that need to be relative to the
   /// diffuse pass.
   ///
   /// @see getCameraPosition
   Point3F getDiffuseCameraPosition() const { return mDiffuseCameraTransform.getPosition(); }
   const MatrixF& getDiffuseCameraTransform() const { return mDiffuseCameraTransform; }
   
   /// Set a new diffuse camera transform.
   /// @see getDiffuseCameraTransform
   void setDiffuseCameraTransform( const MatrixF &mat ) { mDiffuseCameraTransform = mat; }
   
   /// @}
};

#endif // _SCENE_RENDER_STATE_H_
