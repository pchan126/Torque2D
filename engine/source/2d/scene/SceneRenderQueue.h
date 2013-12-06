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
#define _SCENE_RENDER_QUEUE_H_

#ifndef _SCENE_RENDER_REQUEST_H_
#include "2d/scene/SceneRenderRequest.h"
#endif

// Debug Profiling.
#include "debug/profiler.h"

//-----------------------------------------------------------------------------

class SceneRenderQueue : public IFactoryObjectReset
{
public:
    typedef Vector<SceneRenderRequest*> typeRenderRequestVector;

    // t2dScene Render Request Sort.
    enum RenderSort
    {
        RENDER_SORT_INVALID,
        ///---
        RENDER_SORT_OFF,
        RENDER_SORT_NEWEST,
        RENDER_SORT_OLDEST,
        RENDER_SORT_BATCH,
        RENDER_SORT_GROUP,
        RENDER_SORT_XAXIS,
        RENDER_SORT_YAXIS,
        RENDER_SORT_ZAXIS,
        RENDER_SORT_INVERSE_XAXIS,
        RENDER_SORT_INVERSE_YAXIS,
        RENDER_SORT_INVERSE_ZAXIS,
    };

private: 
    typeRenderRequestVector mRenderRequests;
    RenderSort              mSortMode;
    bool                    mStrictOrderMode;

private:
    static bool layeredNewFrontSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredOldFrontSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredDepthSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredInverseDepthSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layerBatchOrderSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layerGroupOrderSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredXSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredYSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredInverseXSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b);
    static bool layeredInverseYSortPointSort(const SceneRenderRequest *a, const SceneRenderRequest *b);

public:
    SceneRenderQueue()
    {
        resetState();
    }
    virtual ~SceneRenderQueue()
    {
        resetState();
    }

    virtual void resetState( void )
    {
        // Debug Profiling.
        PROFILE_SCOPE(SceneRenderQueue_ResetState);

        // Cache request.
        for( SceneRenderRequest* itr:mRenderRequests )
        {
            SceneRenderRequestFactory.cacheObject( itr );
        }
        mRenderRequests.clear();

        // Reset sort mode.
        mSortMode = RENDER_SORT_NEWEST;

        // Set strict order mode.
        mStrictOrderMode = true;
    }

    inline SceneRenderRequest* createRenderRequest( void )
    {
        // Debug Profiling.
        PROFILE_SCOPE(SceneRenderQueue_CreateRenderRequest);

        // Create scene render request.
        SceneRenderRequest* pSceneRenderRequest = SceneRenderRequestFactory.createObject();

        // Queue render request.
        mRenderRequests.push_back( pSceneRenderRequest );

        return pSceneRenderRequest;
    }

    inline typeRenderRequestVector& getRenderRequests( void ) { return mRenderRequests; }

    inline void setSortMode( RenderSort sortMode ) { mSortMode = sortMode; }
    inline RenderSort getSortMode( void ) const { return mSortMode; }

    inline void setStrictOrderMode( const bool strictOrderMode ) { mStrictOrderMode = strictOrderMode; }
    inline bool getStrictOrderMode( void ) const { return mStrictOrderMode; }

    void sort( void )
    {
        // Sort layer appropriately.
        switch( mSortMode )
        {
            case RENDER_SORT_NEWEST:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortNewest);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredNewFrontSort);
                    return;
                }

            case RENDER_SORT_OLDEST:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortOldest);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredOldFrontSort);
                    return;
                }

            case RENDER_SORT_BATCH:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortBatch);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layerBatchOrderSort);

                    // Batching means we don't need strict order.
                    mStrictOrderMode = false;
                    return;
                }

            case RENDER_SORT_GROUP:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortGroup);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layerGroupOrderSort);
                    return;
                }

            case RENDER_SORT_XAXIS:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortXAxis);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredXSortPointSort);
                    return;
                }

            case RENDER_SORT_YAXIS:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortYAxis);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredYSortPointSort);
                    return;
                }

            case RENDER_SORT_ZAXIS:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortZAxis);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredDepthSort);
                    return;
                }

            case RENDER_SORT_INVERSE_XAXIS:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortInverseXAxis);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredInverseXSortPointSort);
                    return;
                }

            case RENDER_SORT_INVERSE_YAXIS:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortInverseYAxis);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredInverseYSortPointSort);
                    return;
                }

            case RENDER_SORT_INVERSE_ZAXIS:
                {
                    // Debug Profiling.
                    PROFILE_SCOPE(SceneRenderQueue_SortInverseZAxis);
                    std::sort( mRenderRequests.begin(), mRenderRequests.end(), layeredInverseDepthSort);
                    return;
                }

            case RENDER_SORT_OFF:
                {
                    return;
                }

            default:
                break;
        };
    }

    static RenderSort getRenderSortEnum(const char* label);
    static const char* getRenderSortDescription( const RenderSort& sortMode );
    static EnumTable::Enums renderSorts[11];
    static EnumTable renderSortTable;
};

#endif // _SCENE_RENDER_QUEUE_H_
