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
                    U32 renderLayerMask,
                    U32 renderGroupMask,
                    const Vector2& renderScale,
                    DebugStats* pDebugStats,
                    SimObject* pRenderHost );
   
   ~SceneRenderState();
   
    CameraView      mRenderCamera;
    b2AABB          mRenderAABB;
    U32             mRenderLayerMask;
    U32             mRenderGroupMask;
    Vector2         mRenderScale;
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
};

#endif // _SCENE_RENDER_STATE_H_
