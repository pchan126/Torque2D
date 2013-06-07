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

#ifndef _VECTOR2_H_
#include "2d/core/vector2.h"
#endif

#include "2d/gui/SceneWindow.h"

//-----------------------------------------------------------------------------

class GuiControl;
class RectF;
class DebugStats;

struct b2AABB;

//-----------------------------------------------------------------------------

struct SceneRenderState
{
    SceneRenderState(
        const CameraView renderCamera,
        U32 renderLayerMask,
        U32 renderGroupMask,
        const Vector2& renderScale,
        DebugStats* pDebugStats,
        SimObject* pRenderHost )
   : mRenderCamera(renderCamera),
     mRenderScale(renderScale),
     mRenderLayerMask(renderLayerMask),
     mRenderGroupMask(renderGroupMask),
     mpDebugStats(pDebugStats),
     mpRenderHost(pRenderHost)
    {
        mRenderAABB       = CoreMath::mRectFtoAABB( renderCamera.mSourceArea );
    }

    CameraView      mRenderCamera;
    b2AABB          mRenderAABB;
    U32             mRenderLayerMask;
    U32             mRenderGroupMask;
    Vector2         mRenderScale;
    DebugStats*     mpDebugStats;
    SimObject*      mpRenderHost;


};

#endif // _SCENE_RENDER_STATE_H_
