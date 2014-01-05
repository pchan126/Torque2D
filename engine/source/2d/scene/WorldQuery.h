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

#ifndef _WORLD_QUERY_H_
#define _WORLD_QUERY_H_

#ifndef _WORLD_QUERY_FILTER_H_
#include "2d/scene/WorldQueryFilter.h"
#endif

#ifndef _WORLD_QUERY_RESULT_H_
#include "2d/scene/WorldQueryResult.h"
#endif

///-----------------------------------------------------------------------------

class t2dScene;

///-----------------------------------------------------------------------------

class WorldQuery :
    protected b2DynamicTree,
    public b2QueryCallback,
    public b2RayCastCallback,
    public SimObject
{
public:
    WorldQuery( t2dScene * pScene );
    virtual         ~WorldQuery() {}

    /// Standard scope.
    S32             add( SceneObject* pSceneObject );
    void            remove( SceneObject* pSceneObject );
    bool            update( SceneObject* pSceneObject, const b2AABB& aabb, const b2Vec2& displacement );

    /// Always in scope.
    void            addAlwaysInScope( SceneObject* pSceneObject );
    void            removeAlwaysInScope( SceneObject* pSceneObject );

    /// World collision-shape queries.
    size_t             collisionQueryAABB( const b2AABB& aabb );
    size_t             collisionQueryRay( const Vector2& point1, const Vector2& point2 );
    size_t             collisionQueryPoint( const Vector2& point );
    size_t             collisionQueryCircle( const Vector2& centroid, const F32 radius );

    /// AABB queries.
    size_t             aabbQueryAABB( const b2AABB& aabb );
    size_t             aabbQueryRay( const Vector2& point1, const Vector2& point2 );
    size_t             aabbQueryPoint( const Vector2& point );
    size_t             aabbQueryCircle( const Vector2& centroid, const F32 radius );

    /// OOBB queries.
    size_t             oobbQueryAABB( const b2AABB& aabb );
    size_t             oobbQueryRay( const Vector2& point1, const Vector2& point2 );
    size_t             oobbQueryPoint( const Vector2& point );
    size_t             oobbQueryCircle( const Vector2& centroid, const F32 radius );

    /// Any queries.
    size_t             anyQueryAABB( const b2AABB& aabb );
    size_t             anyQueryRay( const Vector2& point1, const Vector2& point2 );
    size_t             anyQueryPoint( const Vector2& point );
    size_t             anyQueryCircle( const Vector2& centroid, const F32 radius );

    /// Filtering.
    inline void     setQueryFilter( const WorldQueryFilter& queryFilter ) { mQueryFilter = queryFilter; }
   
    /// Results.
    void            clearQuery( void );
    typeWorldQueryResultVector& getLayeredQueryResults( const U32 layer );
    typeWorldQueryResultVector& getQueryResults( void ) { return mQueryResults; }
    inline size_t getQueryResultsCount( void ) const { return mQueryResults.size(); }
    inline bool     getIsRaycastQueryResult( void ) const { return mIsRaycastQueryResult; }
    void            sortRaycastQueryResult( void );

    /// Callbacks.
    virtual bool    ReportFixture( b2Fixture* fixture );
    virtual F32     ReportFixture( b2Fixture* fixture, const b2Vec2& point, const b2Vec2& normal, F32 fraction );
    bool            QueryCallback( S32 proxyId );
    F32             RayCastCallback( const b2RayCastInput& input, S32 proxyId );

private:
    void            injectAlwaysInScope( void );
    static bool            rayCastFractionSort(const WorldQueryResult a, const WorldQueryResult b);

private:
    t2dScene *                      mpScene;
    WorldQueryFilter            mQueryFilter;
    b2PolygonShape              mComparePolygonShape;
    b2CircleShape               mCompareCircleShape;
    b2RayCastInput              mCompareRay;
    b2Vec2                      mComparePoint;
    b2Transform                 mCompareTransform;
    bool                        mCheckPoint;
    bool                        mCheckAABB;
    bool                        mCheckOOBB;
    bool                        mCheckCircle;
    Vector<typeWorldQueryResultVector>  mLayeredQueryResults;
    typeWorldQueryResultVector  mQueryResults;
    bool                        mIsRaycastQueryResult;
    typeSceneObjectVector       mAlwaysInScopeSet;
    U32                         mMasterQueryKey;
};

#endif // _WORLD_QUERY_H_