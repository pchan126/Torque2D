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

#ifndef _SCENE_RENDER_QUEUE_H_
#include "2d/scene/SceneRenderQueue.h"
#endif

#ifndef _SCENE_RENDER_OBJECT_H_
#include "2d/scene/SceneRenderObject.h"
#endif

//-----------------------------------------------------------------------------

EnumTable SceneRenderQueue::renderSortTable =
                {
                { SceneRenderQueue::RENDER_SORT_OFF,            "Off" },
                { SceneRenderQueue::RENDER_SORT_NEWEST,         "New" },
                { SceneRenderQueue::RENDER_SORT_OLDEST,         "Old" },
                { SceneRenderQueue::RENDER_SORT_BATCH,          "Batch" },
                { SceneRenderQueue::RENDER_SORT_GROUP,          "Group" },
                { SceneRenderQueue::RENDER_SORT_XAXIS,          "X" },
                { SceneRenderQueue::RENDER_SORT_YAXIS,          "Y" },
                { SceneRenderQueue::RENDER_SORT_ZAXIS,          "Z" },
                { SceneRenderQueue::RENDER_SORT_INVERSE_XAXIS,  "-X" },
                { SceneRenderQueue::RENDER_SORT_INVERSE_YAXIS,  "-Y" },
                { SceneRenderQueue::RENDER_SORT_INVERSE_ZAXIS,  "-Z" },
                };

//-----------------------------------------------------------------------------

SceneRenderQueue::RenderSort SceneRenderQueue::getRenderSortEnum(const char* label)
{
    if (renderSortTable.isLabel(label))
        return (RenderSort)renderSortTable[label];

    // Warn.
    Con::warnf( "SceneRenderQueue::getRenderSortEnum() - Invalid sort enum of '%s'", label );

    return SceneRenderQueue::RENDER_SORT_INVALID;
}

//-----------------------------------------------------------------------------

const char* SceneRenderQueue::getRenderSortDescription( const RenderSort& sortMode )
{
    if (renderSortTable.isIndex(sortMode))
        return renderSortTable[sortMode].c_str();

    // Warn.
    Con::warnf( "SceneRenderQueue::getRenderSortDescription() - Invalid sort enum." );

    return StringTable->EmptyString;
}


//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredNewFrontSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    return a->mSerialId < b->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredOldFrontSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    return b->mSerialId < a->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredDepthSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    // Fetch depths.
    const F32 depthA = a->mDepth;
    const F32 depthB = b->mDepth;

    return depthA < depthB ? false : depthA > depthB ? true : a->mSerialId < b->mSerialId;
}


//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredInverseDepthSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    // Fetch depths.
    const F32 depthA = a->mDepth;
    const F32 depthB = b->mDepth;

    return depthA < depthB ? true : depthA > depthB ? true : a->mSerialId < b->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layerBatchOrderSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    // Fetch scene render objects.
    SceneRenderObject* pSceneRenderObjectA = a->mpSceneRenderObject;
    SceneRenderObject* pSceneRenderObjectB = b->mpSceneRenderObject;

    // Fetch batch render isolation.
    const bool renderIsolatedA = pSceneRenderObjectA->getBatchIsolated();
    const bool renderIsolatedB = pSceneRenderObjectB->getBatchIsolated();

    // A not render isolated?
    if ( !renderIsolatedA )
    {
        // Use the serial Id if neither are render isolated.
        if ( !renderIsolatedB )
            return a->mSerialId < b->mSerialId;

        // A not render isolated but B is.
        return false;
    }

    // B not render isolated?
    if ( !renderIsolatedB )
    {
        // A is render isolated.
        return true;
    }

    // A and B are render isolated so use serial Id,
    return a->mSerialId < b->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layerGroupOrderSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    // Fetch the groups.
    StringTableEntry renderGroupA = a->mRenderGroup;
    StringTableEntry renderGroupB = b->mRenderGroup;

    // Sort by render group (address, arbitrary but static) and use age if render groups are identical.
    return renderGroupA == renderGroupB ? a->mSerialId < b->mSerialId : renderGroupA < renderGroupB ? true : false;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredXSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    const F32 x1 = a->mWorldPosition.x + a->mSortPoint.x;
    const F32 x2 = b->mWorldPosition.x + b->mSortPoint.x;

    // We sort lower x values before higher values.
    return x1 < x2 ? true : x1 > x2 ? false : a->mSerialId < b->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredYSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    // Fetch scene render requests.
    const F32 y1 = a->mWorldPosition.y + a->mSortPoint.y;
    const F32 y2 = b->mWorldPosition.y + b->mSortPoint.y;

    // We sort lower y values before higher values.
    return y1 < y2 ? true : y1 > y2 ? false : a->mSerialId < b->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredInverseXSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    // Fetch scene render requests.
    const F32 x1 = a->mWorldPosition.x + a->mSortPoint.x;
    const F32 x2 = b->mWorldPosition.x + b->mSortPoint.x;

    // We sort higher x values before lower values.
    return x1 < x2 ? false : x1 > x2 ? true : a->mSerialId < b->mSerialId;
}

//-----------------------------------------------------------------------------

bool SceneRenderQueue::layeredInverseYSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b)
{
    const F32 y1 = a->mWorldPosition.y + a->mSortPoint.y;
    const F32 y2 = b->mWorldPosition.y + b->mSortPoint.y;

    // We sort higher y values before lower values.
    return y1 < y2 ? false : y1 > y2 ? true : a->mSerialId < b->mSerialId;
}
