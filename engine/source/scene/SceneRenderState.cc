//
// Created by Paul L Jan on 2013-12-06.
// Copyright (c) 2013 Michael Perry. All rights reserved.
//

#include "SceneRenderState.h"

SceneRenderState::~SceneRenderState()
{

}

const MatrixF& SceneRenderState::getWorldViewMatrix() const
{
//   return getRenderPass()->getMatrixSet().getWorldToCamera();
	return MatrixF::Identity;
}

const MatrixF& SceneRenderState::getProjectionMatrix() const
{
//   return getRenderPass()->getMatrixSet().getCameraToScreen();
	return MatrixF::Identity;
}