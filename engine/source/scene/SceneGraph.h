//
// Created by Paul L Jan on 2013-12-05.
//

#ifndef __SceneGraph_H_
#define __SceneGraph_H_

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

#include "2d/scene/WorldQuery.h"
#include "2d/scene/SceneRenderObject.h"
#include "component/behaviors/behaviorComponent.h"
#include "lighting/lightManager.h"


class SceneRenderState;

class SceneGraph : public BehaviorComponent {

public:
    SceneGraph():
        mAmbientLightColor(1.0, 1.0, 1.0, 1.0),
        mLightManager("Basic2D","LM")
    {};

/// A signal used to notify of render passes.
    typedef Signal< void( SceneGraph *, const SceneRenderState* ) > RenderSignal;

protected:
    ColorF                      mAmbientLightColor;   // global ambient lighting
    LightManager                mLightManager;

    RenderSignal                mPreRenderSignal;
    RenderSignal                mPostRenderSignal;


private:
    typedef BehaviorComponent   Parent;

};


#endif //__SceneGraph_H_
