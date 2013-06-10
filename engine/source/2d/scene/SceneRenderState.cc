#include "./SceneRenderState.h"

//-----------------------------------------------------------------------------

SceneRenderState::SceneRenderState(
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

SceneRenderState::~SceneRenderState()
{
   
}
