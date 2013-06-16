#include "./SceneRenderState.h"

//-----------------------------------------------------------------------------

SceneRenderState::SceneRenderState(
                 const CameraView renderCamera,
                 U32 renderGroupMask,
                 DebugStats* pDebugStats,
                 SimObject* pRenderHost )
                  : mRenderCamera(renderCamera),
                  mRenderGroupMask(renderGroupMask),
                  mpDebugStats(pDebugStats),
                  mpRenderHost(pRenderHost)
{
}

SceneRenderState::~SceneRenderState()
{
   
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getWorldViewMatrix() const
{
//   return getRenderPass()->getMatrixSet().getWorldToCamera();
	return MatrixF(true);
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getProjectionMatrix() const
{
//   return getRenderPass()->getMatrixSet().getCameraToScreen();
	return MatrixF(true);
}

