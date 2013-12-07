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

#ifndef _SCENE_RENDER_REQUEST_H_
#define _SCENE_RENDER_REQUEST_H_

#include "2d/scene/SceneRenderFactories.h"
#include "2d/core/Vector2.h"
#include "graphics/color.h"
#include "graphics/gfxEnums.h"

// Debug Profiling.
#include "debug/profiler.h"

//-----------------------------------------------------------------------------

class SceneRenderObject;
class SceneRenderQueue;

//-----------------------------------------------------------------------------

class SceneRenderRequest : public IFactoryObjectReset
{
public:
    SceneRenderRequest() : mpIsolatedRenderQueue(nullptr)
    {
        resetState();
    }

    ~SceneRenderRequest() {}

    /// Sets mandatory configuration.
    inline SceneRenderRequest* set(
        SceneRenderObject* pSceneRenderObject,
        const Vector2& worldPosition,
        const F32 depth,
        const Vector2& sortPoint = Vector2::getZero(),
        const S32 serialId = 0,
        StringTableEntry renderGroup = StringTable->EmptyString,
        void* pCustomData1 = nullptr,
        void* pCustomData2 = nullptr,
        S32 customDataKey1 = 0,
        S32 customDataKey2 = 0)
    {
        // Sanity!
        AssertFatal( pSceneRenderObject != nullptr, "Cannot submit a nullptr scene render object." );

        mpSceneRenderObject = pSceneRenderObject;
        mWorldPosition = worldPosition;
        mDepth = depth;
        mSortPoint = sortPoint;
        mSerialId = serialId;
        mRenderGroup = renderGroup;
        mpCustomData1 = pCustomData1;
        mpCustomData2 = pCustomData2;
        mCustomDataKey1 = customDataKey1;
        mCustomDataKey2 = customDataKey2;

        return this;
    }

    /// Reset request state.
    virtual void resetState( void )
    {
        // Debug Profiling.
        PROFILE_SCOPE(SceneRenderRequest_ResetState);

        mpSceneRenderObject = nullptr;
        mWorldPosition.SetZero();
        mDepth = 0.0f;
        mSortPoint.SetZero();
        mSerialId = 0;
        mRenderGroup = StringTable->EmptyString;

        mBlendMode = true;
        mSrcBlendFactor = GFXBlendSrcAlpha;
        mDstBlendFactor = GFXBlendInvSrcAlpha;
        mBlendColor = ColorF(1.0f,1.0f,1.0f,1.0f);
        mAlphaTest = -1.0f;

        mpCustomData1 = nullptr;
        mpCustomData2 = nullptr;
        mCustomDataKey1 = 0;
        mCustomDataKey2 = 0;

        if ( mpIsolatedRenderQueue != nullptr )
        {
            SceneRenderQueueFactory.cacheObject( mpIsolatedRenderQueue );
            mpIsolatedRenderQueue = nullptr;
        }
    }

public:
    SceneRenderObject*  mpSceneRenderObject;
    Vector2             mWorldPosition;
    F32                 mDepth;
    Vector2             mSortPoint;
    S32                 mSerialId;
    StringTableEntry    mRenderGroup;

    bool                mBlendMode;
    GFXBlend              mSrcBlendFactor;
    GFXBlend              mDstBlendFactor;
    ColorF              mBlendColor;
    F32                 mAlphaTest;

    void*               mpCustomData1;
    void*               mpCustomData2;
    S32                 mCustomDataKey1;
    S32                 mCustomDataKey2;

    SceneRenderQueue*   mpIsolatedRenderQueue;
};

#endif // _SCENE_RENDER_REQUEST_H_
