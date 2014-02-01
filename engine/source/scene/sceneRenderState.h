//
// Created by Paul L Jan on 2013-12-06.
// Copyright (c) 2013 Michael Perry. All rights reserved.
//



#include "graphics/color.h"
#include "Vector2.h"
#include "SceneGraph.h"
#include "renderPassManager.h"

#ifndef __SceneRenderState_H_
#define __SceneRenderState_H_

class BaseMatInstance;

class SceneRenderState {

public:

    /// The delegate used for material overrides.
    /// @see getOverrideMaterial
    typedef Delegate< BaseMatInstance*( BaseMatInstance* ) > MatDelegate;

protected:
    /// Global ambient light color.
    ColorF mAmbientLightColor;
    /// The optional material override delegate.
    MatDelegate mMatDelegate;
    /// The type of scene render pass we're doing.
    ScenePassType mScenePassType;
    /// The render pass which we are setting up with this scene state.
    RenderPassManager* mRenderPass;
    /// Forces bin based post effects to be disabled
    /// during rendering with this scene state.
    bool mUsePostEffects;

public:
    SceneRenderState( ScenePassType passType,
                    RenderPassManager* renderPass = nullptr,
                    bool usePostEffects = true)
                    :mScenePassType( passType ),
                    mRenderPass(renderPass),
                    mUsePostEffects( usePostEffects )
    {     };

    ~SceneRenderState();

    ///
    MatrixF mDiffuseCameraTransform;

        /// Return the ambient light color to use for rendering the scene.
    ///
    /// At the moment, we only support a single global ambient color with which
    /// all objects in the scene are rendered.  This is because when using
    /// Advanced Lighting, we are not resolving light contribution on a per-surface
    /// or per-object basis but rather do it globally by gathering light
    /// contribution to the whole scene and since the ambient factor is decided
    /// by the sun/vector light, it simply becomes a base light level onto which
    /// shadowing/lighting is blended based on the shadow maps of the sun/vector
    /// light.
    ///
    /// @return The ambient light color for rendering.
    ColorF getAmbientLightColor() const { return mAmbientLightColor; }

    /// Set the global ambient light color to render with.
    void setAmbientLightColor( const ColorF& color ) { mAmbientLightColor = color; }

    /// Returns the optional material override delegate which is
    /// used during some special render passes.
    /// @see getOverrideMaterial
    MatDelegate& getMaterialDelegate() { return mMatDelegate; }

    const MatDelegate& getMaterialDelegate() const { return mMatDelegate; }

    /// @name Passes
    /// @{

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

    /// If true then bin based post effects are disabled
    /// during rendering with this scene state.
    bool usePostEffects() const { return mUsePostEffects; }

    void usePostEffects( bool value ) { mUsePostEffects = value; }

    /// Return the RenderPassManager that manages rendering objects batched
    /// for this t2dSceneRenderState.
    RenderPassManager* getRenderPass() const { return mRenderPass; }

//    /// @name Render Style
//    /// @{
//
//    /// Get the rendering style used for the scene
//    SceneRenderStyle getSceneRenderStyle() const { return mSceneRenderStyle; }
//
//    /// Set the rendering style used for the scene
//    void setSceneRenderStyle(SceneRenderStyle style) { mSceneRenderStyle = style; }
//
//    /// Get the stereo field being rendered
//    S32 getSceneRenderField() const { return mRenderField; }
//
//    /// Set the stereo field being rendered
//    void setSceneRenderField(S32 field) { mRenderField = field; }
//
//    /// @}

    /// @name Transforms, projections, and viewports.
    /// @{

    /// Return the screen-space viewport rectangle.
//    const RectI& getViewport() const { return getCullingState().getCameraState().getViewport(); }

    /// Return the world->view transform matrix.
    const MatrixF& getWorldViewMatrix() const;

    /// Return the project transform matrix.
    const MatrixF& getProjectionMatrix() const;

    /// Returns the actual camera position.
    /// @see getDiffuseCameraPosition
    const Point3F& getCameraPosition() const; // { return getCullingState().getCameraState().getViewPosition(); }

    /// Returns the camera transform (view->world) this SceneRenderState is using.
//    const MatrixF& getCameraTransform() const { return getCullingState().getCameraState().getViewWorldMatrix(); }
//
//    /// Returns the minimum distance something must be from the camera to not be culled.
//    F32 getNearPlane() const { return getCullingFrustum().getNearDist();   }
//
//    /// Returns the maximum distance something can be from the camera to not be culled.
//    F32 getFarPlane() const { return getCullingFrustum().getFarDist();    }
//
//    /// Returns the camera vector normalized to 1 / far distance.
//    const Point3F& getVectorEye() const { return mVectorEye; }
//
//    /// Returns the possibly overloaded world to screen scale.
//    /// @see projectRadius
//    const Point2F& getWorldToScreenScale() const { return mWorldToScreenScale; }
//
//    /// Set a new world to screen scale to overload
//    /// future screen metrics operations.
//    void setWorldToScreenScale( const Point2F& scale ) { mWorldToScreenScale = scale; }

    /// Returns the pixel size of the radius projected to the screen at a desired distance.
    ///
    /// Internally this uses the stored world to screen scale and viewport extents.  This
    /// allows the projection to be overloaded in special cases like when rendering shadows
    /// or reflections.
    ///
    /// @see getWorldToScreenScale
    /// @see getViewportExtent
//    F32 projectRadius( F32 dist, F32 radius ) const
//    {
//        // We fixup any negative or zero distance
//        // so we don't get a divide by zero.
//        dist = dist > 0.0f ? dist : 0.001f;
//        return ( radius / dist ) * mWorldToScreenScale.y;
//    }

    /// Returns the camera position used during the diffuse rendering pass which may be different
    /// from the actual camera position.
    ///
    /// This is useful when doing level of detail calculations that need to be relative to the
    /// diffuse pass.
    ///
    /// @see getCameraPosition
    Point3F getDiffuseCameraPosition() const  { return mDiffuseCameraTransform.getPosition(); }
    const MatrixF& getDiffuseCameraTransform() const { return mDiffuseCameraTransform; }

    /// Set a new diffuse camera transform.
    /// @see getDiffuseCameraTransform
    void setDiffuseCameraTransform( const MatrixF &mat ) { mDiffuseCameraTransform = mat; }

    /// @}
    //
    //



};


#endif //__SceneRenderState_H_
