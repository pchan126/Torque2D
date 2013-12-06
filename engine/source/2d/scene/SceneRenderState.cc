#include "platform/platform.h"
#include "./SceneRenderState.h"
#include "renderInstance/renderPassManager.h"
#include "Layer.h"

//-----------------------------------------------------------------------------

SceneRenderState::SceneRenderState(
                 t2dScene * sceneManager,
                ScenePassType passType,
                 const CameraView& view,
                 U32 renderGroupMask,
                 RenderPassManager* renderPass, // = NULL
                 bool usePostEffects )
                  : mSceneManager(sceneManager),
                  mRenderCamera(view),
                  mScenePassType( passType ),
                  mRenderGroupMask(renderGroupMask),
                  mRenderPass(renderPass),
                  mUsePostEffects( usePostEffects )
{
    if (sceneManager != NULL)
    {
        mRenderPass = sceneManager->getDefaultRenderPass();
        mRenderPass->assignSharedXform( RenderPassManager::View, view.getCameraViewMatrix() );
        mRenderPass->assignSharedXform( RenderPassManager::Projection, view.getCameraProjOrtho() );
        mpDebugStats = &sceneManager->getDebugStats();
    }
}

SceneRenderState::~SceneRenderState()
{
   
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getWorldViewMatrix() const
{
//   return getRenderPass()->getMatrixSet().getWorldToCamera();
	return MatrixF::Identity;
}

//-----------------------------------------------------------------------------

const MatrixF& SceneRenderState::getProjectionMatrix() const
{
//   return getRenderPass()->getMatrixSet().getCameraToScreen();
	return MatrixF::Identity;
}

//-----------------------------------------------------------------------------

void SceneRenderState::renderObjects( SceneObject** objects, U32 numObjects, SceneRenderQueue* pSceneRenderQueue )
{
//    // Let the objects batch their stuff.
//
//    PROFILE_START( SceneRenderState_prepRenderImages );
    for( U32 i = 0; i < numObjects; ++ i )
    {
        SceneObject* object = objects[ i ];
        object->scenePrepareRender( this, pSceneRenderQueue );
    }
//    PROFILE_END();
//
//    // Render what the objects have batched.
//
    getRenderPass()->renderPass( this );
}

void SceneRenderState::renderObjects( t2dScene * pScene, SceneRenderQueue* pSceneRenderQueue )
{
    // Yes so step through layers.
    for ( S32 layer = pScene->getLayerCount()-1; layer >= 0 ; layer-- )
    {
        GFX->pushViewMatrix();
        MatrixF vM = GFX->getViewMatrix();
        Layer* layerRef = pScene->getLayer(layer);
        Point3F tScale = layerRef->getCameraTranslationScale();
        vM[3] *= tScale.x;
        vM[7] *= tScale.y;
        vM[11] *= tScale.z;
        GFX->setViewMatrix(vM);

        b2AABB cameraAABB = this->mRenderCamera.getRotatedAABB();
        b2Vec2 centerPosition = cameraAABB.GetCenter();
        cameraAABB.lowerBound -= centerPosition;
        cameraAABB.upperBound -= centerPosition;
        centerPosition.x *= tScale.x;
        centerPosition.y *= tScale.y;
        cameraAABB.lowerBound += centerPosition;
        cameraAABB.upperBound += centerPosition;
        pScene->getWorldQuery()->clearQuery();
        pScene->getWorldQuery()->aabbQueryAABB( cameraAABB );

        // Fetch layer.
        typeWorldQueryResultVector& layerResults = pScene->getWorldQuery()->getLayeredQueryResults( layer );

        // Fetch layer object count.
        const U32 layerObjectCount = layerResults.size();

        // Are there any objects to render in this layer?
        if ( layerObjectCount > 0 )
        {
            // Yes, so increase render picked.
            mpDebugStats->renderPicked += layerObjectCount;

            // Iterate query results.
            for( WorldQueryResult worldQueryItr:layerResults )
            {
                // Fetch scene object.
                SceneObject* pSceneObject = worldQueryItr.mpSceneObject;

                // Skip if the object should not render.
                if ( !pSceneObject->shouldRender() )
                    continue;

                // Can the scene object prepare a render?
                if ( pSceneObject->canPrepareRender() )
                {
                    // Yes. so is it batch isolated.
                    if ( pSceneObject->getBatchIsolated() )
                    {
                        // Yes, so create a default render request  on the primary queue.
                        SceneRenderRequest* pIsolatedSceneRenderRequest = t2dScene::createDefaultRenderRequest( pSceneRenderQueue, pSceneObject );

                        // Create a new isolated render queue.
                        pIsolatedSceneRenderRequest->mpIsolatedRenderQueue = SceneRenderQueueFactory.createObject();

                        // Prepare in the isolated queue.
                        pSceneObject->scenePrepareRender(this, pIsolatedSceneRenderRequest->mpIsolatedRenderQueue );

                        // Increase render request count.
                        mpDebugStats->renderRequests += (U32)pIsolatedSceneRenderRequest->mpIsolatedRenderQueue->getRenderRequests().size();

                        // Adjust for the extra private render request.
                        mpDebugStats->renderRequests -= 1;
                    }
                    else
                    {
                        // No, so prepare in primary queue.
                        pSceneObject->scenePrepareRender(this, pSceneRenderQueue );
                    }
                }
                else
                {
                    // No, so create a default render request for it.
                    t2dScene::createDefaultRenderRequest( pSceneRenderQueue, pSceneObject );
                }
            }

            // Fetch render requests.
            SceneRenderQueue::typeRenderRequestVector& sceneRenderRequests = pSceneRenderQueue->getRenderRequests();

            // Fetch render request count.
            const U32 renderRequestCount = (U32)sceneRenderRequests.size();

            // Increase render request count.
            mpDebugStats->renderRequests += renderRequestCount;

            // Do we have more than a single render request?
            if ( renderRequestCount > 1 )
            {
                // Debug Profiling.
                PROFILE_SCOPE(Scene_RenderSceneLayerSorting);

                // Yes, so fetch layer sort mode.
                SceneRenderQueue::RenderSort mode = pScene->getLayerSortMode(layer);

                // Temporarily switch to normal sort if batch sort but batcher disabled.
                if ( !pScene->getBatchingEnabled() && mode == SceneRenderQueue::RENDER_SORT_BATCH )
                    mode = SceneRenderQueue::RENDER_SORT_NEWEST;

                // Set render queue mode.
                pSceneRenderQueue->setSortMode( mode );

                // Sort the render requests.
                pSceneRenderQueue->sort();
            }

            // Iterate render requests.
            for( SceneRenderRequest* pSceneRenderRequest:sceneRenderRequests )
            {
                // Debug Profiling.
                PROFILE_SCOPE(Scene_RenderSceneRequests);

                // Fetch scene render object.
                SceneRenderObject* pSceneRenderObject = pSceneRenderRequest->mpSceneRenderObject;

                // Flush if the object is not render batched and we're in strict order mode.
                if ( !pSceneRenderObject->isBatchRendered() && pScene->mBatchRenderer.getStrictOrderMode() )
                {
                    pScene->mBatchRenderer.flush( mpDebugStats->batchNoBatchFlush );
                }
                        // Flush if the object is batch isolated.
                else if ( pSceneRenderObject->getBatchIsolated() )
                {
                    pScene->mBatchRenderer.flush( mpDebugStats->batchIsolatedFlush );
                }

                // Yes, so is the object batch rendered?
                if ( pSceneRenderObject->isBatchRendered() )
                {
                    // Yes, so set the blend mode.
                    pScene->mBatchRenderer.setBlendMode( pSceneRenderRequest );

                    // Set the alpha test mode.
                    pScene->mBatchRenderer.setAlphaTestMode( pSceneRenderRequest );
                }

                // Set batch strict order mode.
                // NOTE:    We keep reasserting this because an object is free to change it during rendering.
                pScene->mBatchRenderer.setStrictOrderMode( pSceneRenderQueue->getStrictOrderMode() );

                // Is the object batch isolated?
                if ( pSceneRenderObject->getBatchIsolated() )
                {
                    // Yes, so fetch isolated render queue.
                    SceneRenderQueue* pIsolatedRenderQueue = pSceneRenderRequest->mpIsolatedRenderQueue;

                    // Sanity!
                    AssertFatal( pIsolatedRenderQueue != NULL, "Cannot render batch isolated with an isolated render queue." );

                    // Sort the isolated render requests.
                    pIsolatedRenderQueue->sort();

                    // Can the object render?
                    if ( pSceneRenderObject->validRender() )
                    {
                        // Yes, so iterate isolated render requests.
                        for( SceneRenderRequest* isolatedRenderRequest:pIsolatedRenderQueue->getRenderRequests() )
                        {
                            pSceneRenderObject->sceneRender(this, isolatedRenderRequest, &pScene->mBatchRenderer );
                        }
                    }
                    else
                    {
                        // No, so iterate isolated render requests.
                        for( SceneRenderRequest* isolatedRenderRequest:pIsolatedRenderQueue->getRenderRequests() )
                        {
                            pSceneRenderObject->sceneRenderFallback(this, isolatedRenderRequest, &pScene->mBatchRenderer );
                        }

                        // Increase render fallbacks.
                        mpDebugStats->renderFallbacks++;
                    }

                    // Flush isolated batch.
                    pScene->mBatchRenderer.flush( mpDebugStats->batchIsolatedFlush );
                }
                else
                {
                    // No, so can the object render?
                    if ( pSceneRenderObject->validRender() )
                    {
                        // Yes, so render object.
                        pSceneRenderObject->sceneRender(this, pSceneRenderRequest, &pScene->mBatchRenderer );
                    }
                    else
                    {
                        // No, so render using fallback.
                        pSceneRenderObject->sceneRenderFallback(this, pSceneRenderRequest, &pScene->mBatchRenderer );

                        // Increase render fallbacks.
                        mpDebugStats->renderFallbacks++;
                    }
                }
            }

            // Flush.
            // NOTE:    We cannot batch between layers as we adhere to a strict layer render order.
            pScene->mBatchRenderer.flush( mpDebugStats->batchLayerFlush );

            // Iterate query results.
            for( WorldQueryResult worldQuery:layerResults )
            {
                // Debug Profiling.
                PROFILE_SCOPE(Scene_RenderObjectOverlays);

                // Fetch scene object.
                SceneObject* pSceneObject = worldQuery.mpSceneObject;

                // Render object overlay.
                pSceneObject->sceneRenderOverlay(this);
            }

//               GFX->setActiveRenderTarget(oldTarget);
        }

        // Reset render queue.
        pSceneRenderQueue->resetState();
        GFX->popViewMatrix();
    }
    getRenderPass()->renderPass( this );
}





