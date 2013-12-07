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

#include "graphics/color.h"
#include "2d/core/vector2.h"
#include "2d/gui/SceneWindow.h"
#include "t2dScene.h"
#include "SceneRenderState.h"

//-----------------------------------------------------------------------------

class GuiControl;
class RectF;
class DebugStats;

struct b2AABB;

class RenderPassManager;

//-----------------------------------------------------------------------------

class t2dSceneRenderState
        : public SceneRenderState {

protected:
    /// SceneManager being rendered in this state.
    t2dScene * mSceneManager;

    ///
   MatrixF mDiffuseCameraTransform;

    /// Disables AdvancedLighting bin draws during rendering with this scene state.
    bool mDisableAdvancedLightingBins;

    /// If true (default) lightmapped meshes should be rendered.
    bool mRenderLightmappedMeshes;

    /// If true (default) non-lightmapped meshes should be rendered.
    bool mRenderNonLightmappedMeshes;

public:
    CameraView      mRenderCamera;

    U32             mRenderGroupMask;

    DebugStats*     mpDebugStats;

   /// Construct a new t2dSceneRenderState.
   ///
   /// @param sceneManager SceneManager rendered in this t2dSceneRenderState.
   /// @param passType Type of rendering pass that the t2dSceneRenderState is for.
   /// @param view The view that is being rendered
   /// @param renderPass The render pass which is being set up by this t2dSceneRenderState.  If NULL,
   ///   then t2dScene::getDefaultRenderPass() is used.
   /// @param usePostEffect Whether PostFX are enabled in the rendering pass.
   t2dSceneRenderState( t2dScene * scene,
                    ScenePassType passType,
                    const CameraView& view,
                    U32 renderGroupMask,
                    RenderPassManager* renderPass = nullptr,
                    bool usePostEffects = true);

    /// Return the SceneManager that is being rendered in this t2dSceneRenderState.
    t2dScene * getSceneManager() const { return mSceneManager; }

    /// @name Rendering
    /// @{

//    /// Get the AABB around the scene portion that we render.
//    const Box3F& getRenderArea() const { return mRenderArea; }
//
//    /// Set the AABB of the space that should be rendered.
//    void setRenderArea( const Box3F& area ) { mRenderArea = area; }

    /// Batch the given objects to the render pass manager and then
    /// render the batched instances.
    ///
    /// @param objects List of objects.
    /// @param numObjects Number of objects in @a objects.
    void renderObjects( SceneObject** objects, U32 numObjects, SceneRenderQueue* pSceneRenderQueue );

   void renderObjects( t2dScene * pScene, SceneRenderQueue* pSceneRenderQueue );
   
    /// @}

    /// @name Lighting
    /// @{

    /// If true then Advanced Lighting bin draws are disabled during rendering with
    /// this scene state.
//    bool disableAdvancedLightingBins() const { return mDisableAdvancedLightingBins; }
//    void disableAdvancedLightingBins(bool enabled) { mDisableAdvancedLightingBins = enabled; }
//
//    bool renderLightmappedMeshes() const { return mRenderLightmappedMeshes; }
//    void renderLightmappedMeshes( bool enabled ) { mRenderLightmappedMeshes = enabled; }
//
//    bool renderNonLightmappedMeshes() const { return mRenderNonLightmappedMeshes; }
//    void renderNonLightmappedMeshes( bool enabled ) { mRenderNonLightmappedMeshes = enabled; }

    /// @}

   /// @name Transforms, projections, and viewports.
   /// @{
   
//   /// Return the screen-space viewport rectangle.
//   const RectI& getViewport() const { return getCullingState().getCameraState().getViewport(); }

    //   /// Returns the actual camera position.
//   /// @see getDiffuseCameraPosition
//   const Point3F& getCameraPosition() const { return getCullingState().getCameraState().getViewPosition(); }
//   
//   /// Returns the camera transform (view->world) this t2dSceneRenderState is using.
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


    /// @name Material Overrides
    /// @{

    /// When performing a special render pass like shadows this
    /// returns a specialized override material.  It can return
    /// NULL if the override wants to disable rendering.  If
    /// there is no override in place then the input material is
    /// returned unaltered.
    BaseMatInstance* getOverrideMaterial( BaseMatInstance* matInst ) const
    {
        if ( !matInst || mMatDelegate.empty() )
            return matInst;

        return mMatDelegate( matInst );
    }

    /// @}

};

#endif // _SCENE_RENDER_STATE_H_
