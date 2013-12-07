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
    SceneRenderState( ScenePassType passType,
                    RenderPassManager* renderPass = nullptr,
                    bool usePostEffects = true)
                    :mScenePassType( passType ),
                    mRenderPass(renderPass),
                    mUsePostEffects( usePostEffects )
    {     };

    ~SceneRenderState();

    /// Return the world->view transform matrix.
    const MatrixF& getWorldViewMatrix() const;

    /// Return the project transform matrix.
    const MatrixF& getProjectionMatrix() const;

    /// The delegate used for material overrides.
    /// @see getOverrideMaterial
    typedef Delegate< BaseMatInstance*( BaseMatInstance* ) > MatDelegate;

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
};


#endif //__SceneRenderState_H_
