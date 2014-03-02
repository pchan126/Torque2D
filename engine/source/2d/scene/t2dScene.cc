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

#include "t2dScene.h"
#include "graphics/gfxDevice.h"
#include "console/consoleTypes.h"
#include "2d/sceneobject/SceneObject.h"
#include "ContactFilter.h"
#include "2d/scene/SceneRenderObject.h"
#include "2d/controllers/core/SceneController.h"
#include "2d/core/particleSystem.h"
#include "Layer.h"
#include "t2dSceneRenderState.h"
#include "t2dScene_ScriptBinding.h"
#include "debug/profiler.h"
#include "renderInstance/renderPassManager.h"

//------------------------------------------------------------------------------

SimObjectPtr<t2dScene> t2dScene::LoadingScene = nullptr;

//------------------------------------------------------------------------------

static ContactFilter mContactFilter;

// t2dScene counter.
static U32 sSceneCount = 0;
static U32 sSceneMasterIndex = 0;

// Joint custom node names.
static StringTableEntry jointCustomNodeName               = StringTable->insert( "Joints" );
static StringTableEntry jointCollideConnectedName         = StringTable->insert( "CollideConnected" );
static StringTableEntry jointLocalAnchorAName             = StringTable->insert( "LocalAnchorA" );
static StringTableEntry jointLocalAnchorBName             = StringTable->insert( "LocalAnchorB" );

static StringTableEntry jointDistanceNodeName             = StringTable->insert( "Distance" );
static StringTableEntry jointDistanceLengthName           = StringTable->insert( "Length" );
static StringTableEntry jointDistanceFrequencyName        = StringTable->insert( "Frequency" );
static StringTableEntry jointDistanceDampingRatioName     = StringTable->insert( "DampingRatio" );

static StringTableEntry jointRopeNodeName                 = StringTable->insert( "Rope" );
static StringTableEntry jointRopeMaxLengthName            = StringTable->insert( "MaxLength" );

static StringTableEntry jointRevoluteNodeName             = StringTable->insert( "Revolute" );
static StringTableEntry jointRevoluteLimitLowerAngleName  = StringTable->insert( "LowerAngle" );
static StringTableEntry jointRevoluteLimitUpperAngleName  = StringTable->insert( "UpperAngle" );
static StringTableEntry jointRevoluteMotorSpeedName       = StringTable->insert( "MotorSpeed" );
static StringTableEntry jointRevoluteMotorMaxTorqueName   = StringTable->insert( "MaxTorque" );

static StringTableEntry jointWeldNodeName                 = StringTable->insert( "Weld" );
static StringTableEntry jointWeldFrequencyName            = jointDistanceFrequencyName;
static StringTableEntry jointWeldDampingRatioName         = jointDistanceDampingRatioName;

static StringTableEntry jointWheelNodeName                = StringTable->insert( "Wheel" );
static StringTableEntry jointWheelWorldAxisName           = StringTable->insert( "WorldAxis" );
static StringTableEntry jointWheelMotorSpeedName          = StringTable->insert( "MotorSpeed" );
static StringTableEntry jointWheelMotorMaxTorqueName      = jointRevoluteMotorMaxTorqueName;
static StringTableEntry jointWheelFrequencyName           = jointDistanceFrequencyName;
static StringTableEntry jointWheelDampingRatioName        = jointDistanceDampingRatioName;

static StringTableEntry jointFrictionNodeName             = StringTable->insert( "Friction" );
static StringTableEntry jointFrictionMaxForceName         = StringTable->insert( "MaxForce" );
static StringTableEntry jointFrictionMaxTorqueName        = jointRevoluteMotorMaxTorqueName;

static StringTableEntry jointPrismaticNodeName            = StringTable->insert( "Prismatic" );
static StringTableEntry jointPrismaticWorldAxisName       = jointWheelWorldAxisName;
static StringTableEntry jointPrismaticLimitLowerTransName = StringTable->insert( "LowerTranslation" );
static StringTableEntry jointPrismaticLimitUpperTransName = StringTable->insert( "UpperTranslation" );
static StringTableEntry jointPrismaticMotorSpeedName      = jointRevoluteMotorSpeedName;
static StringTableEntry jointPrismaticMotorMaxForceName   = jointFrictionMaxForceName;

static StringTableEntry jointPulleyNodeName               = StringTable->insert( "Pulley" );
static StringTableEntry jointPulleyGroundAnchorAName      = StringTable->insert( "GroundAnchorA" );
static StringTableEntry jointPulleyGroundAnchorBName      = StringTable->insert( "GroundAnchorB" );
static StringTableEntry jointPulleyLengthAName            = StringTable->insert( "LengthA" );
static StringTableEntry jointPulleyLengthBName            = StringTable->insert( "LengthB" );
static StringTableEntry jointPulleyRatioName              = StringTable->insert( "Ratio" );

static StringTableEntry jointTargetNodeName               = StringTable->insert( "Target" );
static StringTableEntry jointTargetWorldTargetName        = StringTable->insert( "WorldTarget" );
static StringTableEntry jointTargetMaxForceName           = StringTable->insert( jointFrictionMaxForceName );
static StringTableEntry jointTargetFrequencyName          = jointDistanceFrequencyName;
static StringTableEntry jointTargetDampingRatioName       = jointDistanceDampingRatioName;

static StringTableEntry jointMotorNodeName                = StringTable->insert( "Motor" );
static StringTableEntry jointMotorLinearOffsetName        = StringTable->insert( "LinearOffset" );
static StringTableEntry jointMotorAngularOffsetName       = StringTable->insert( "AngularOffset" );

static StringTableEntry jointMotorMaxForceName            = jointFrictionMaxForceName;
static StringTableEntry jointMotorMaxTorqueName           = jointRevoluteMotorMaxTorqueName;
static StringTableEntry jointMotorCorrectionFactorName    = StringTable->insert( "CorrectionFactor" );

// Controller custom node names.
static StringTableEntry controllerCustomNodeName	      = StringTable->insert( "Controllers" );

// Asset preload custom node names.
static StringTableEntry assetPreloadNodeName              = StringTable->insert( "AssetPreloads" );
static StringTableEntry assetNodeName                     = StringTable->insert( "Asset" );

//-----------------------------------------------------------------------------

t2dScene::t2dScene() :
    /// World.
    mpWorld(nullptr),
    mWorldGravity(0.0f, 0.0f),
    mVelocityIterations(8),
    mPositionIterations(3),

    /// Joint access.
    mJointMasterId(1),

    /// t2dScene time.
    mSceneTime(0.0f),
    mScenePause(false),

    /// Debug and metrics.
    mDebugMask(0X00000000),
    mpDebugSceneObject(nullptr),

    /// Window rendering.
    mpCurrentRenderWindow(nullptr),
    
    /// Miscellaneous.
    mIsEditorScene(0),
    mUpdateCallback(false),
    mRenderCallback(false),
    mSceneIndex(0)
{
    // Set Vector Associations.
    VECTOR_SET_ASSOCIATION( mSceneObjects );
    VECTOR_SET_ASSOCIATION( mDeleteRequests );
    VECTOR_SET_ASSOCIATION( mDeleteRequestsTemp );
    VECTOR_SET_ASSOCIATION( mEndContacts );
    VECTOR_SET_ASSOCIATION( mAssetPreloads );
     
    // Set debug stats for batch renderer.
    mBatchRenderer.setDebugStats( &mDebugStats );

    // Register the scene controllers set.
    mControllers = new SimSet();
    mControllers->registerObject();

    mDefaultRenderPass = new RenderPassManager();
    mBaseRenderPass = mDefaultRenderPass;

    // Assign scene index.    
    mSceneIndex = ++sSceneMasterIndex;
    sSceneCount++;
}

//-----------------------------------------------------------------------------

t2dScene::~t2dScene()
{
    // Unregister the scene controllers set.
    if ( mControllers.notNull() )
        mControllers->deleteObject();

   for (int i = 0; i < mLayers.size(); i++) {
      delete mLayers[i];
   }

    delete mBaseRenderPass;

    // Decrease scene count.
    --sSceneCount;
}

//-----------------------------------------------------------------------------

bool t2dScene::onAdd()
{
    // Call Parent.
    if(!Parent::onAdd())
        return false;

    // Create physics world.
    mpWorld = new b2World( mWorldGravity );

    // Set contact filter.
    mpWorld->SetContactFilter( &mContactFilter );

    // Set contact listener.
    mpWorld->SetContactListener( this );

    // Set destruction listener.
    mpWorld->SetDestructionListener( this );

    // Create ground body.
    b2BodyDef groundBodyDef;
    groundBodyDef.userData = static_cast<PhysicsProxy*>(this);
    mpGroundBody = mpWorld->CreateBody(&groundBodyDef);
    mpGroundBody->SetAwake( false );

    // Create world query.
    mpWorldQuery = new WorldQuery(this);

    // Set loading scene.
    t2dScene::LoadingScene = this;

    // Turn-on tick processing.
    setProcessTicks( true );

    // Return Okay.
    return true;
}

//-----------------------------------------------------------------------------

void t2dScene::onRemove()
{
    // Turn-off tick processing.
    setProcessTicks( false );

    // Clear t2dScene.
    clearScene();

    // Process Delete Requests.
    processDeleteRequests(true);

    // Delete ground body.
    mpWorld->DestroyBody( mpGroundBody );
    mpGroundBody = nullptr;

    // Delete physics world and world query.
    delete mpWorldQuery;
    delete mpWorld;
    mpWorldQuery = nullptr;
    mpWorld = nullptr;

    // Detach All t2dScene Windows.
    detachAllSceneWindows();

    // Call Parent. Clear scene handles all the object removal, so we can skip
    // that part and just do the sim-object stuff.
    SimObject::onRemove();
}

//-----------------------------------------------------------------------------

void t2dScene::onDeleteNotify( SimObject* object )
{
    // Ignore if we're not monitoring a debug banner scene object.
    if ( mpDebugSceneObject == nullptr )
        return;

    // Ignore if it's not the one we're monitoring.
    SceneObject* pSceneObject = dynamic_cast<SceneObject*>( object );
    if ( pSceneObject != mpDebugSceneObject )
        return;

    // Reset the monitored scene object.
    mpDebugSceneObject = nullptr;
}

//-----------------------------------------------------------------------------

void t2dScene::initPersistFields()
{
    // Call Parent.
    Parent::initPersistFields();

    // Physics.
    addProtectedField("Gravity", TypeVector2, Offset(mWorldGravity, t2dScene), &setGravity, &getGravity, &writeGravity, "" );
    addField("VelocityIterations", TypeS32, Offset(mVelocityIterations, t2dScene), &writeVelocityIterations, "" );
    addField("PositionIterations", TypeS32, Offset(mPositionIterations, t2dScene), &writePositionIterations, "" );

    // Lighting
    addField("AmbientLighting", TypeColorF, Offset(mAmbientLightColor, t2dScene), &writeSceneLight, "");
    
    addProtectedField("Controllers", TypeSimObjectPtr, Offset(mControllers, t2dScene), &defaultProtectedNotSetFn, &defaultProtectedGetFn, &defaultProtectedNotWriteFn, "The scene controllers to use.");
   dsize_t offset = Offset(mControllers, t2dScene);
    // Callbacks.
    addField("UpdateCallback", TypeBool, Offset(mUpdateCallback, t2dScene), &writeUpdateCallback, "");
    addField("RenderCallback", TypeBool, Offset(mRenderCallback, t2dScene), &writeRenderCallback, "");
}

//-----------------------------------------------------------------------------

void t2dScene::BeginContact( b2Contact* pContact )
{
    // Ignore contact if it's not a touching contact.
    if ( !pContact->IsTouching() )
        return;

    PROFILE_SCOPE(Scene_BeginContact);

    // Fetch fixtures.
    b2Fixture* pFixtureA = pContact->GetFixtureA();
    b2Fixture* pFixtureB = pContact->GetFixtureB();

    // Fetch physics proxies.
    PhysicsProxy* pPhysicsProxyA = static_cast<PhysicsProxy*>(pFixtureA->GetBody()->GetUserData());
    PhysicsProxy* pPhysicsProxyB = static_cast<PhysicsProxy*>(pFixtureB->GetBody()->GetUserData());

    // Ignore stuff that's not a scene object.
    if (    pPhysicsProxyA->getPhysicsProxyType() != PhysicsProxy::PHYSIC_PROXY_SCENEOBJECT ||
            pPhysicsProxyB->getPhysicsProxyType() != PhysicsProxy::PHYSIC_PROXY_SCENEOBJECT )
    {
            return;
    }

    // Fetch scene objects.
    SceneObject* pSceneObjectA = static_cast<SceneObject*>(pPhysicsProxyA);
    SceneObject* pSceneObjectB = static_cast<SceneObject*>(pPhysicsProxyB);

    // Initialize the contact.
    TickContact tickContact;
    tickContact.initialize( pContact, pSceneObjectA, pSceneObjectB, pFixtureA, pFixtureB );

    // Add contact.
    mBeginContacts.insert( pContact, tickContact );
}

//-----------------------------------------------------------------------------

void t2dScene::EndContact( b2Contact* pContact )
{
    PROFILE_SCOPE(Scene_EndContact);

    // Fetch fixtures.
    b2Fixture* pFixtureA = pContact->GetFixtureA();
    b2Fixture* pFixtureB = pContact->GetFixtureB();

    // Fetch physics proxies.
    PhysicsProxy* pPhysicsProxyA = static_cast<PhysicsProxy*>(pFixtureA->GetBody()->GetUserData());
    PhysicsProxy* pPhysicsProxyB = static_cast<PhysicsProxy*>(pFixtureB->GetBody()->GetUserData());

    // Ignore stuff that's not a scene object.
    if (    pPhysicsProxyA->getPhysicsProxyType() != PhysicsProxy::PHYSIC_PROXY_SCENEOBJECT ||
            pPhysicsProxyB->getPhysicsProxyType() != PhysicsProxy::PHYSIC_PROXY_SCENEOBJECT )
    {
            return;
    }

    // Fetch scene objects.
    SceneObject* pSceneObjectA = static_cast<SceneObject*>(pPhysicsProxyA);
    SceneObject* pSceneObjectB = static_cast<SceneObject*>(pPhysicsProxyB);

    // Initialize the contact.
    TickContact tickContact;
    tickContact.initialize( pContact, pSceneObjectA, pSceneObjectB, pFixtureA, pFixtureB );

    // Add contact.
    mEndContacts.push_back( tickContact );
}

//-----------------------------------------------------------------------------

void t2dScene::PostSolve( b2Contact* pContact, const b2ContactImpulse* pImpulse )
{
    // Find contact mapping.
    typeContactHash::iterator contactItr = mBeginContacts.find( pContact );

    // Finish if we didn't find the contact.
    if ( contactItr == mBeginContacts.end() )
        return;

    // Fetch contact.
    TickContact& tickContact = contactItr->second;

    // Add the impulse.
    for ( U32 index = 0; index < b2_maxManifoldPoints; ++index )
    {
        tickContact.mNormalImpulses[index] += pImpulse->normalImpulses[index];
        tickContact.mTangentImpulses[index] += pImpulse->tangentImpulses[index];
    }
}

//-----------------------------------------------------------------------------

void t2dScene::forwardContacts( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(Scene_ForwardContacts);

    // Iterate end contacts.
    for( TickContact tickContact:mEndContacts )
    {
        // Inform the scene objects.
        tickContact.mpSceneObjectA->onEndCollision( tickContact );
        tickContact.mpSceneObjectB->onEndCollision( tickContact );
    }

    // Iterate begin contacts.
    for( auto contactItr:mBeginContacts )
    {
        // Fetch tick contact.
        TickContact& tickContact = contactItr.second;

        // Inform the scene objects.
        tickContact.mpSceneObjectA->onBeginCollision( tickContact );
        tickContact.mpSceneObjectB->onBeginCollision( tickContact );
    }
}

//-----------------------------------------------------------------------------

void t2dScene::dispatchBeginContactCallbacks( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(Scene_DispatchBeginContactCallbacks);

    // Sanity!
    AssertFatal( b2_maxManifoldPoints == 2, "t2dScene::dispatchBeginContactCallbacks() - Invalid assumption about max manifold points." );

    // Fetch contact count.
    const size_t contactCount = mBeginContacts.size();

    // Finish if no contacts.
    if ( contactCount == 0 )
        return;

    // Iterate all contacts.
    for ( auto contactItr:mBeginContacts )
    {
        // Fetch contact.
        const TickContact& tickContact = contactItr.second;

        // Fetch scene objects.
        SceneObject* pSceneObjectA = tickContact.mpSceneObjectA;
        SceneObject* pSceneObjectB = tickContact.mpSceneObjectB;

        // Skip if either object is being deleted.
        if ( pSceneObjectA->isBeingDeleted() || pSceneObjectB->isBeingDeleted() )
            continue;

        // Skip if both objects don't have collision callback active.
        if ( !pSceneObjectA->getCollisionCallback() && !pSceneObjectB->getCollisionCallback() )
            continue;

        // Fetch normal and contact points.
        const U32& pointCount = tickContact.mPointCount;
        const b2Vec2& normal = tickContact.mWorldManifold.normal;
        const b2Vec2& point1 = tickContact.mWorldManifold.points[0];
        const b2Vec2& point2 = tickContact.mWorldManifold.points[1];
        const S32 shapeIndexA = pSceneObjectA->getCollisionShapeIndex( tickContact.mpFixtureA );
        const S32 shapeIndexB = pSceneObjectB->getCollisionShapeIndex( tickContact.mpFixtureB );

        // Sanity!
        AssertFatal( shapeIndexA >= 0, "t2dScene::dispatchBeginContactCallbacks() - Cannot find shape index reported on physics proxy of a fixture." );
        AssertFatal( shapeIndexB >= 0, "t2dScene::dispatchBeginContactCallbacks() - Cannot find shape index reported on physics proxy of a fixture." );

        // Fetch collision impulse information
        const F32 normalImpulse1 = tickContact.mNormalImpulses[0];
        const F32 normalImpulse2 = tickContact.mNormalImpulses[1];
        const F32 tangentImpulse1 = tickContact.mTangentImpulses[0];
        const F32 tangentImpulse2 = tickContact.mTangentImpulses[1];

        // Format objects.
        char sceneObjectABuffer[16];
        char sceneObjectBBuffer[16];
        dSprintf( sceneObjectABuffer, sizeof(sceneObjectABuffer), "%d", pSceneObjectA->getId() );
        dSprintf( sceneObjectBBuffer, sizeof(sceneObjectBBuffer), "%d", pSceneObjectB->getId() );

        // Format miscellaneous information.
        char miscInfoBuffer[128];
        if ( pointCount == 2 )
        {
            dSprintf(miscInfoBuffer, sizeof(miscInfoBuffer),
                "%d %d %0.4f %0.4f %0.4f %0.4f %0.4f %0.4f %0.4f %0.4f %0.4f %0.4f",
                shapeIndexA, shapeIndexB,
                normal.x, normal.y,
                point1.x, point1.y,
                normalImpulse1,
                tangentImpulse1,
                point2.x, point2.y,
                normalImpulse2,
                tangentImpulse2 );
        }
        else if ( pointCount == 1 )
        {
            dSprintf(miscInfoBuffer, sizeof(miscInfoBuffer),
                "%d %d %0.4f %0.4f %0.4f %0.4f %0.4f %0.4f",
                shapeIndexA, shapeIndexB,
                normal.x, normal.y,
                point1.x, point1.y,
                normalImpulse1,
                tangentImpulse1 );
        }
        else
        {
            dSprintf(miscInfoBuffer, sizeof(miscInfoBuffer),
                "%d %d",
                shapeIndexA, shapeIndexB );
        }

        // Does the scene handle the collision callback?
        Namespace* pNamespace = getNamespace();
        if ( pNamespace != nullptr && pNamespace->lookup( StringTable->insert( "onSceneCollision" ) ) != nullptr )
        {
            // Yes, so perform script callback on the t2dScene.
            Con::executef( this, 4, "onSceneCollision",
                sceneObjectABuffer,
                sceneObjectBBuffer,
                miscInfoBuffer );
        }
        else
        {
            // No, so call it on its behaviors.
            const char* args[5] = { "onSceneCollision", "", sceneObjectABuffer, sceneObjectBBuffer, miscInfoBuffer };
            callOnBehaviors( 5, args );
        }

        // Is object A allowed to collide with object B?
        if ((pSceneObjectA->mCollisionGroupMask & pSceneObjectB->mSceneGroupMask) != 0)
        {
            // Yes, so does it handle the collision callback?
            if ( pSceneObjectA->isMethod("onCollision") )            
            {
                // Yes, so perform the script callback on it.
                Con::executef( pSceneObjectA, 3, "onCollision",
                    sceneObjectBBuffer,
                    miscInfoBuffer );
            }
            else
            {
                // No, so call it on its behaviors.
                const char* args[4] = { "onCollision", "", sceneObjectBBuffer, miscInfoBuffer };
                pSceneObjectA->callOnBehaviors( 4, args );
            }
        }

        // Is object B allowed to collide with object A?
        if (    (pSceneObjectB->mCollisionGroupMask & pSceneObjectA->mSceneGroupMask) != 0)
        {
            // Yes, so does it handle the collision callback?
            if ( pSceneObjectB->isMethod("onCollision") )            
            {
                // Yes, so perform the script callback on it.
                Con::executef( pSceneObjectB, 3, "onCollision",
                    sceneObjectABuffer,
                    miscInfoBuffer );
            }
            else
            {
                // No, so call it on its behaviors.
                const char* args[4] = { "onCollision", "", sceneObjectABuffer, miscInfoBuffer };
                pSceneObjectB->callOnBehaviors( 4, args );
            }
        }
    }
}

//-----------------------------------------------------------------------------

void t2dScene::dispatchEndContactCallbacks( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(Scene_DispatchEndContactCallbacks);

    // Sanity!
    AssertFatal( b2_maxManifoldPoints == 2, "t2dScene::dispatchEndContactCallbacks() - Invalid assumption about max manifold points." );

    // Fetch contact count.
    const size_t contactCount = mEndContacts.size();

    // Finish if no contacts.
    if ( contactCount == 0 )
        return;

    // Iterate all contacts.
    for ( const TickContact tickContact:mEndContacts )
    {
        // Fetch scene objects.
        SceneObject* pSceneObjectA = tickContact.mpSceneObjectA;
        SceneObject* pSceneObjectB = tickContact.mpSceneObjectB;

        // Skip if either object is being deleted.
        if ( pSceneObjectA->isBeingDeleted() || pSceneObjectB->isBeingDeleted() )
            continue;

        // Skip if both objects don't have collision callback active.
        if ( !pSceneObjectA->getCollisionCallback() && !pSceneObjectB->getCollisionCallback() )
            continue;

        // Fetch shape index.
        const S32 shapeIndexA = pSceneObjectA->getCollisionShapeIndex( tickContact.mpFixtureA );
        const S32 shapeIndexB = pSceneObjectB->getCollisionShapeIndex( tickContact.mpFixtureB );

        // Sanity!
        AssertFatal( shapeIndexA >= 0, "t2dScene::dispatchEndContactCallbacks() - Cannot find shape index reported on physics proxy of a fixture." );
        AssertFatal( shapeIndexB >= 0, "t2dScene::dispatchEndContactCallbacks() - Cannot find shape index reported on physics proxy of a fixture." );

        // Format objects.
        char sceneObjectABuffer[16];
        char sceneObjectBBuffer[16];
        dSprintf( sceneObjectABuffer, sizeof(sceneObjectABuffer), "%d", pSceneObjectA->getId() );
        dSprintf( sceneObjectBBuffer, sizeof(sceneObjectBBuffer), "%d", pSceneObjectB->getId() );

        // Format miscellaneous information.
        char miscInfoBuffer[32];
        dSprintf(miscInfoBuffer, sizeof(miscInfoBuffer), "%d %d", shapeIndexA, shapeIndexB );

        // Does the scene handle the collision callback?
        Namespace* pNamespace = getNamespace();
        if ( pNamespace != nullptr && pNamespace->lookup( StringTable->insert( "onSceneEndCollision" ) ) != nullptr )
        {
            // Yes, so does the scene handle the collision callback?
            Con::executef( this, 4, "onSceneEndCollision",
                sceneObjectABuffer,
                sceneObjectBBuffer,
                miscInfoBuffer );
        }
        else
        {
            // No, so call it on its behaviors.
            const char* args[5] = { "onSceneEndCollision", "", sceneObjectABuffer, sceneObjectBBuffer, miscInfoBuffer };
            callOnBehaviors( 5, args );
        }

        // Is object A allowed to collide with object B?
        if (    (pSceneObjectA->mCollisionGroupMask & pSceneObjectB->mSceneGroupMask) != 0)
        {
            // Yes, so does it handle the collision callback?
            if ( pSceneObjectA->isMethod("onEndCollision") )            
            {
                // Yes, so perform the script callback on it.
                Con::executef( pSceneObjectA, 3, "onEndCollision",
                    sceneObjectBBuffer,
                    miscInfoBuffer );
            }
            else
            {
                // No, so call it on its behaviors.
                const char* args[4] = { "onEndCollision", "", sceneObjectBBuffer, miscInfoBuffer };
                pSceneObjectA->callOnBehaviors( 4, args );
            }
        }

        // Is object B allowed to collide with object A?
        if (    (pSceneObjectB->mCollisionGroupMask & pSceneObjectA->mSceneGroupMask) != 0 )
        {
            // Yes, so does it handle the collision callback?
            if ( pSceneObjectB->isMethod("onEndCollision") )            
            {
                // Yes, so perform the script callback on it.
                Con::executef( pSceneObjectB, 3, "onEndCollision",
                    sceneObjectABuffer,
                    miscInfoBuffer );
            }
            else
            {
                // No, so call it on its behaviors.
                const char* args[4] = { "onEndCollision", "", sceneObjectABuffer, miscInfoBuffer };
                pSceneObjectB->callOnBehaviors( 4, args );
            }
        }
    }
}

//-----------------------------------------------------------------------------

void t2dScene::processTick( void )
{
    // Debug Profiling.
    PROFILE_SCOPE(Scene_ProcessTick);

    // Finish if the t2dScene is not added to the simulation.
    if ( !isProperlyAdded() )
        return;

    // Process Delete Requests.
    processDeleteRequests(false);

    // Update debug stats.
    mDebugStats.fps           = Con::getFloatVariable("fps::framePeriod", 0.0f);
    mDebugStats.frameCount    = (U32)Con::getIntVariable("fps::frameCount", 0);
    mDebugStats.bodyCount     = (U32)mpWorld->GetBodyCount();
    mDebugStats.jointCount    = (U32)mpWorld->GetJointCount();
    mDebugStats.contactCount  = (U32)mpWorld->GetContactCount();
    mDebugStats.proxyCount    = (U32)mpWorld->GetProxyCount();
    mDebugStats.objectsCount  = (U32)mSceneObjects.size();
    mDebugStats.worldProfile  = mpWorld->GetProfile();

    // Set particle stats.
    mDebugStats.particlesAlloc = ParticleSystem::Instance->getAllocatedParticleCount();
    mDebugStats.particlesUsed = ParticleSystem::Instance->getActiveParticleCount();
    mDebugStats.particlesFree = mDebugStats.particlesAlloc - mDebugStats.particlesUsed;

    // Finish if scene is paused.
    if ( !getScenePause() )
    {
        // Reset object stats.
        U32 objectsEnabled = 0;
        U32 objectsVisible = 0;
        U32 objectsAwake   = 0;

        // Fetch if a "normal" i.e. non-editor scene.
        const bool isNormalScene = !getIsEditorScene();

        // Update scene time.
        mSceneTime += Tickable::smTickSec;

        // Clear ticked scene objects.
        mTickedSceneObjects.clear();

        // Iterate scene objects.
        for( S32 n = 0; n < mSceneObjects.size(); ++n )
        {
            // Fetch scene object.
            SceneObject* pSceneObject = mSceneObjects[n];

            // Update awake/asleep counts.
            if ( pSceneObject->getAwake() )
                objectsAwake++;

            // Update visible.
            if ( pSceneObject->getVisible() )
                objectsVisible++;

            // Push scene object if it's eligible for ticking.
            if ( pSceneObject->isEnabled() )
            {
                // Update enabled.
                objectsEnabled++;

                // Add to ticked objects if object is not being deleted and this is a "normal" scene or
                // the object is marked as allowing editor ticks.
                if ( !pSceneObject->isBeingDeleted() && (isNormalScene || pSceneObject->getIsEditorTickAllowed() )  )
                    mTickedSceneObjects.push_back( pSceneObject );
            }
        }

        // Update object stats.
        mDebugStats.objectsEnabled = objectsEnabled;
        mDebugStats.objectsVisible = objectsVisible;
        mDebugStats.objectsAwake   = objectsAwake;

        // Debug Status Reference.
        DebugStats* pDebugStats = &mDebugStats;

        // ****************************************************
        // Pre-integrate objects.
        // ****************************************************

        // Iterate ticked scene objects.
        for ( auto tickedSceneObject: mTickedSceneObjects )
        {
            // Debug Profiling.
            PROFILE_SCOPE(Scene_PreIntegrate);

            // Pre-integrate.
            tickedSceneObject->preIntegrate( mSceneTime, Tickable::smTickSec, pDebugStats );
        }

        // ****************************************************
        // Integrate controllers.
        // ****************************************************

        // Fetch the controller set.
        SimSet* pControllerSet = getControllers();

        // Do we have any scene controllers?
        if ( pControllerSet != nullptr )
        {
            // Debug Profiling.
            PROFILE_SCOPE(Scene_IntegrateSceneControllers);

            // Yes, so fetch scene controller count.
            const S32 sceneControllerCount = (S32)pControllerSet->size();

            // Iterate scene controllers.
            for( S32 i = 0; i < sceneControllerCount; i++ )
            {
                // Fetch the scene controller.
                SceneController* pController = dynamic_cast<SceneController*>((*pControllerSet)[i]);

                // Skip if not a controller.
                if ( pController == nullptr )
                    continue;

                // Integrate.
                pController->integrate( this, mSceneTime, Tickable::smTickSec, pDebugStats );
            }
        }

        // Debug Profiling.
        PROFILE_START(Scene_IntegratePhysicsSystem);

        // Reset contacts.
        mBeginContacts.clear();
        mEndContacts.clear();

        // Only step the physics if a "normal" scene.
        if ( isNormalScene )
        {
            // Step the physics.
            mpWorld->Step( Tickable::smTickSec, mVelocityIterations, mPositionIterations );
        }

        // Debug Profiling.
        PROFILE_END();   // Scene_IntegratePhysicsSystem

        // Forward the contacts.
        forwardContacts();

        // ****************************************************
        // Integrate objects.
        // ****************************************************

        // Iterate ticked scene objects.
        for ( auto tickedSceneObject: mTickedSceneObjects )
        {
            // Debug Profiling.
            PROFILE_SCOPE(Scene_IntegrateObject);

            // Integrate.
            tickedSceneObject->integrateObject( mSceneTime, Tickable::smTickSec, pDebugStats );
        }

        // ****************************************************
        // Post-Integrate Stage.
        // ****************************************************

        // Iterate ticked scene objects.
        for ( auto tickedSceneObject: mTickedSceneObjects )
        {
            // Debug Profiling.
            PROFILE_SCOPE(Scene_PostIntegrate);

            // Post-integrate.
            tickedSceneObject->postIntegrate( mSceneTime, Tickable::smTickSec, pDebugStats );
        }

        // t2dScene update callback.
        if( mUpdateCallback )
        {
            // Debug Profiling.
            PROFILE_SCOPE(Scene_OnSceneUpdatetCallback);

            Con::executef( this, 1, "onSceneUpdate" );
        }

        // Only dispatch contacts if a "normal" scene.
        if ( isNormalScene )
        {
            // Dispatch contacts callbacks.
            dispatchEndContactCallbacks();
            dispatchBeginContactCallbacks();
        }

        // Clear ticked scene objects.
        mTickedSceneObjects.clear();
    }

    // Update debug stat ranges.
    mDebugStats.updateRanges();
}

//-----------------------------------------------------------------------------

void t2dScene::interpolateTick( F32 timeDelta )
{
    // Finish if scene is paused.
    if ( getScenePause() ) return;

    // Debug Profiling.
    PROFILE_SCOPE(Scene_InterpolateTick);

    // ****************************************************
    // Interpolate scene objects.
    // ****************************************************

    // Iterate scene objects.
    for( SceneObject* pSceneObject: mSceneObjects )
    {
        // Skip interpolation of scene object if it's not eligible.
        if ( !pSceneObject->isEnabled() || pSceneObject->isBeingDeleted() )
            continue;

        pSceneObject->interpolateObject( timeDelta );
    }
}

void t2dScene::renderScene( ScenePassType passType )
{
//    SceneCameraState cameraState = SceneCameraState::fromGFX();
}

//-----------------------------------------------------------------------------

void t2dScene::renderScene(t2dSceneRenderState *renderState)
{
    // Debug Profiling.
    PROFILE_SCOPE(Scene_RenderSceneTotal);

    // Fetch debug stats.
    DebugStats* pDebugStats = renderState->mpDebugStats;

    // Reset Render Stats.
    pDebugStats->renderPicked                   = 0;
    pDebugStats->renderRequests                 = 0;
    pDebugStats->renderFallbacks                = 0;
    pDebugStats->batchTrianglesSubmitted        = 0;
    pDebugStats->batchDrawCallsStrict           = 0;
    pDebugStats->batchDrawCallsSorted           = 0;
    pDebugStats->batchFlushes                   = 0;
    pDebugStats->batchBlendStateFlush           = 0;
    pDebugStats->batchColorStateFlush           = 0;
    pDebugStats->batchAlphaStateFlush           = 0;
    pDebugStats->batchTextureChangeFlush        = 0;
    pDebugStats->batchBufferFullFlush           = 0;
    pDebugStats->batchIsolatedFlush             = 0;
    pDebugStats->batchLayerFlush                = 0;
    pDebugStats->batchNoBatchFlush              = 0;
    pDebugStats->batchAnonymousFlush            = 0;

    if( renderState->isDiffusePass() )
    {
        renderState->setAmbientLightColor( mAmbientLightColor );
    }

    GFX->pushProjectionMatrix();

    // Set orthographic projection.
    GFX->pushWorldMatrix();
    GFX->setProjectionMatrix(renderState->mRenderCamera.getCameraProjOrtho());

    // Set batch renderer wireframe mode.
    mBatchRenderer.setWireframeMode( getDebugMask() & SCENE_DEBUG_WIREFRAME_RENDER );

    // Debug Profiling.
    PROFILE_START(Scene_RenderSceneVisibleQuery);

    // Rotate the render AABB by the camera angle.
    b2AABB cameraAABB = renderState->mRenderCamera.getRotatedAABB();

    GFX->setViewMatrix(renderState->mRenderCamera.getCameraViewMatrix());

    // Clear world query.
    mpWorldQuery->clearQuery();

    // Set filter.
    WorldQueryFilter queryFilter( renderState->mRenderGroupMask, true, true, false, false );
    mpWorldQuery->setQueryFilter( queryFilter );

    // Query render AABB.
    mpWorldQuery->aabbQueryAABB( cameraAABB );
    
    typeWorldQueryResultVector& globalResults = mpWorldQuery->getQueryResults();
    mLightManager.registerGlobalLights( globalResults );

    // Debug Profiling.
    PROFILE_END();  //Scene_RenderSceneVisibleQuery

    // Are there any query results?
    if ( mpWorldQuery->getQueryResultsCount() > 0 )
    {
        // Debug Profiling.
        PROFILE_SCOPE(Scene_RenderSceneCompileRenderRequests);

        // Fetch the primary scene render queue.
        SceneRenderQueue* pSceneRenderQueue = SceneRenderQueueFactory.createObject();

        renderState->renderObjects(this, pSceneRenderQueue);

        // Cache render queue..
        SceneRenderQueueFactory.cacheObject( pSceneRenderQueue );
    }

    mLightManager.unregisterAllLights();
    
    // Draw controllers.
    if ( getDebugMask() & t2dScene::SCENE_DEBUG_CONTROLLERS )
    {
        // Fetch the controller set.
        SimSet* pControllerSet = getControllers();

        // Do we have any scene controllers?
        if ( pControllerSet != nullptr )
        {
            // Debug Profiling.
            PROFILE_SCOPE(Scene_RenderControllers);

            // Yes, so fetch scene controller count.
            const S32 sceneControllerCount = (S32)pControllerSet->size();

            // Iterate scene controllers.
            for( S32 i = 0; i < sceneControllerCount; i++ )
            {
                // Fetch the scene controller.
                SceneController* pController = dynamic_cast<SceneController*>((*pControllerSet)[i]);

                // Skip if not a controller.
                if ( pController == nullptr )
                    continue;

                // Render the overlay.
                pController->renderOverlay( this, renderState, &mBatchRenderer );
            }

            // Flush isolated batch.
            mBatchRenderer.flush( pDebugStats->batchIsolatedFlush );
        }
    }

    // Draw Joints.
    if ( getDebugMask() & t2dScene::SCENE_DEBUG_JOINTS )
    {
        // Debug Profiling.
        PROFILE_SCOPE(Scene_RenderSceneJointOverlays);

        mDebugDraw.DrawJoints( mpWorld );
    }

    // Update debug stat ranges.
    mDebugStats.updateRanges();

    // Are we using the render callback?
    if( mRenderCallback )
    {
        // Debug Profiling.
        PROFILE_SCOPE(Scene_OnSceneRenderCallback);

        // Yes, so perform callback.
        Con::executef( this, 1, "onSceneRender" );
    }
    
    GFX->popProjectionMatrix();
}

//-----------------------------------------------------------------------------

void t2dScene::clearScene( bool deleteObjects )
{
    while( mSceneObjects.size() > 0 )
    {
        // Fetch first scene object.
        SceneObject* pSceneObject = mSceneObjects[0];

        // Remove Object from t2dScene.
        removeFromScene( pSceneObject );

        // Queue Object for deletion.
        if ( deleteObjects )
            pSceneObject->safeDelete();
    }

    // Fetch the controller set.
    SimSet* pControllerSet = getControllers();

    // Do we have any scene controllers?
    if ( pControllerSet != nullptr )
    {
        // Yes, so delete them all.
        while( pControllerSet->size() > 0 )
            pControllerSet->at(0)->deleteObject();
    }

    mLightManager.unregisterAllLights();
   
    // Clear asset preloads.
    clearAssetPreloads();
}

//-----------------------------------------------------------------------------

void t2dScene::addToScene( SceneObject* pSceneObject )
{
    if ( pSceneObject == nullptr )
        return;

    // Fetch current scene.
    t2dScene * pCurrentScene = pSceneObject->getScene();

    // Ignore if already in the scene.
    if ( pCurrentScene == this )
        return;

#if defined(TORQUE_DEBUG)
    // Sanity!
    for ( S32 n = 0; n < mSceneObjects.size(); ++n )
    {
        AssertFatal( mSceneObjects[n] != pSceneObject, "A scene object has become corrupt." );
    }
#endif

    // Check that object is not already in a scene.
    if ( pCurrentScene )
    {
        // Remove from scene.
        pCurrentScene->removeFromScene( pSceneObject );
    }

    // Add scene object.
    mSceneObjects.push_back( pSceneObject );

   U32 objLayer = pSceneObject->getSceneLayer();
    while (mLayers.size() <= objLayer )
    {
       Layer* temp = new Layer;
       temp->setLight(getSceneLight());
       mLayers.push_back(temp);
    }
   
   Layer* temp = mLayers[pSceneObject->getSceneLayer()];
   temp->addObject(pSceneObject);
   
    // Register with the scene.
    pSceneObject->OnRegisterScene( this );

   // Perform callback only if properly added to the simulation.
    if ( pSceneObject->isProperlyAdded() )
    {
        Con::executef(pSceneObject, 2, "onAddToScene", getIdString());
    }
    else
    {
        // Warning.
        Con::warnf("t2dScene object added to scene but the object is not registered with the simulation.  No 'onAddToScene' can be performed!  Use Target scene.");
    }
}

//-----------------------------------------------------------------------------

void t2dScene::removeFromScene( SceneObject* pSceneObject )
{
    if ( pSceneObject == nullptr )
        return;

    // Check if object is actually in a scene.
    if ( !pSceneObject->getScene() )
    {
        Con::warnf("t2dScene::removeFromScene() - Object '%s' is not in a scene!.", pSceneObject->getIdString());
        return;
    }

    // Remove as debug-object if set.
    if ( pSceneObject == getDebugSceneObject() )
        setDebugSceneObject( nullptr );

    // Process Destroy Notifications.
    pSceneObject->processDestroyNotifications();

    // Dismount Any Camera.
    pSceneObject->dismountCamera();

    AssertFatal(getLayerCount() > pSceneObject->getSceneLayer(), "object is not in a valid layer" );
    mLayers[pSceneObject->getSceneLayer()]->removeObject(pSceneObject);

   // Remove from the SceneWindow last pickers
    for( U32 i = 0; i < (U32)mAttachedSceneWindows.size(); ++i )
    {
        (dynamic_cast<SceneWindow*>(mAttachedSceneWindows[i]))->removeFromInputEventPick(pSceneObject);
    }

   if ( pSceneObject->mLight)
      pSceneObject->mpScene->mLightManager.unregisterGlobalLight(pSceneObject->mLight);

      // Unregister from scene.
    pSceneObject->OnUnregisterScene( this );

    // Find scene object and remove it quickly.
    for ( S32 n = 0; n < mSceneObjects.size(); ++n )
    {
        if ( mSceneObjects[n] == pSceneObject )
        {
            mSceneObjects.erase( n );
            break;
        }
    }
   
    // Perform callback.
    Con::executef( pSceneObject, 2, "onRemoveFromScene", getIdString() );
}

//-----------------------------------------------------------------------------

SceneObject*t2dScene::getSceneObject( const U32 objectIndex ) const
{
    // Sanity!
    AssertFatal( objectIndex < getSceneObjectCount(), "t2dScene::getSceneObject() - Invalid object index." );

    return mSceneObjects[objectIndex];
}

//-----------------------------------------------------------------------------

size_t t2dScene::getSceneObjects( typeSceneObjectVector& objects ) const
{
    // No objects if scene is empty!
    if ( getSceneObjectCount() == 0 )
        return 0;

    // Merge with objects.
    objects.merge( mSceneObjects );

    return getSceneObjectCount();
}

//-----------------------------------------------------------------------------

size_t t2dScene::getSceneObjects( typeSceneObjectVector& objects, const U32 sceneLayer ) const
{
    // No objects if scene is empty!
    if ( getSceneObjectCount() == 0 )
        return 0;

    // Reset object count.
    U32 count = 0;

    // Iterate scene objects.
    for( S32 n = 0; n < mSceneObjects.size(); ++n )
    {
        // Fetch scene object.
        SceneObject* pSceneObject = mSceneObjects[n];

        // Skip if not the correct layer.
        if ( pSceneObject->getSceneLayer() != sceneLayer )
            continue;

        // Add to objects.
        objects.push_back( pSceneObject );

        // Increase count.
        count++;
    }

    return count;
}

//-----------------------------------------------------------------------------

const AssetPtr<AssetBase>*t2dScene::getAssetPreload( const S32 index ) const
{
    // Is the index valid?
    if ( index < 0 || index >= mAssetPreloads.size() )
    {
        // Yes, so warn.
        Con::warnf( "t2dScene::getAssetPreload() - Out of range index '%d'.  There are only '%d' asset preloads.", index, mAssetPreloads.size() );
        return nullptr;
    }

    return mAssetPreloads[index];
}

//-----------------------------------------------------------------------------

void t2dScene::addAssetPreload( const char* pAssetId )
{
    // Sanity!
    AssertFatal( pAssetId != nullptr, "t2dScene::addAssetPreload() - Cannot add a nullptr asset preload." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Ignore if asset already added.
    const size_t assetPreloadCount = mAssetPreloads.size();
    for( S32 index = 0; index < assetPreloadCount; ++index )
    {
        if ( mAssetPreloads[index]->getAssetId() == assetId )
            return;
    }

    // Create asset pointer.
    AssetPtr<AssetBase>* pAssetPtr = new AssetPtr<AssetBase>( pAssetId );

    // Was the asset acquired?
    if ( pAssetPtr->isNull() )
    {
        // No, so warn.
        Con::warnf( "t2dScene::addAssetPreload() - Failed to acquire asset '%s' so not added as a preload.", pAssetId );

        // No, so delete the asset pointer.
        delete pAssetPtr;
        return;
    }

    // Add asset.
    mAssetPreloads.push_back( pAssetPtr );
}

//-----------------------------------------------------------------------------

void t2dScene::removeAssetPreload( const char* pAssetId )
{
    // Sanity!
    AssertFatal( pAssetId != nullptr, "t2dScene::removeAssetPreload() - Cannot remove a nullptr asset preload." );

    // Fetch asset Id.
    StringTableEntry assetId = StringTable->insert( pAssetId );

    // Remove asset Id.
    const size_t assetPreloadCount = mAssetPreloads.size();
    for( S32 index = 0; index < assetPreloadCount; ++index )
    {
        if ( mAssetPreloads[index]->getAssetId() == assetId )
        {
            delete mAssetPreloads[index];
            mAssetPreloads.erase( index );
            return;
        }
    }
}

//-----------------------------------------------------------------------------

void t2dScene::clearAssetPreloads( void )
{
    // Delete all the asset preloads.
    while( mAssetPreloads.size() > 0 )
    {
        delete mAssetPreloads.back();
        mAssetPreloads.pop_back();
    }
}

//-----------------------------------------------------------------------------

void t2dScene::mergeScene( const t2dScene * pScene )
{
    // Fetch the scene object count.
    const size_t count = pScene->getSceneObjectCount();

    // Finish if there are not objects to copy.
    if ( count == 0 )
        return;


    for( U32 index = 0; index < count; ++index )
    {
        // Fetch a clone of the scene object.
        SceneObject* pSceneObject = (SceneObject*)pScene->getSceneObject( index )->clone( true );

        // Add the clone.
        addToScene( pSceneObject );
    }
}

//-----------------------------------------------------------------------------

b2Joint*t2dScene::findJoint( const S32 jointId )
{
    // Find joint.
    typeJointHash::iterator itr = mJoints.find( jointId );

    return itr == mJoints.end() ? nullptr : itr->second;
}

//-----------------------------------------------------------------------------

b2JointType t2dScene::getJointType( const S32 jointId )
{
    // Sanity!
    if ( jointId >= mJointMasterId )
    {
        Con::warnf("The joint Id of %d is invalid.", jointId);
        return e_unknownJoint;
    }

    return findJoint( jointId )->GetType();
}

//-----------------------------------------------------------------------------

S32 t2dScene::findJointId( b2Joint* pJoint )
{
    // Sanity!
    AssertFatal( pJoint != nullptr, "Joint cannot be nullptr." );

    // Find joint.
    typeReverseJointHash::iterator itr = mReverseJoints.find( pJoint );

    if ( itr == mReverseJoints.end() )
    {
        Con::warnf("The joint Id could not be found via a joint reference of %p", pJoint);
        return 0;
    }

    return itr->second;
}

//-----------------------------------------------------------------------------

S32 t2dScene::createJoint( b2JointDef* pJointDef )
{
    // Sanity!
    AssertFatal( pJointDef != nullptr, "Joint definition cannot be nullptr." );

    // Create Joint.
    b2Joint* pJoint = mpWorld->CreateJoint( pJointDef );

    // Allocate joint Id.
    const S32 jointId = mJointMasterId++;

    // Insert joint.
    typeJointHash::iterator itr = mJoints.insert( jointId, pJoint );

    // Sanity!
    AssertFatal( itr != mJoints.end(), "Joint already in hash table." );

    // Insert reverse joint.
    mReverseJoints.insert( pJoint, jointId );

    return jointId;
}

//-----------------------------------------------------------------------------

bool t2dScene::deleteJoint( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Finish if no joint.
    if ( pJoint == nullptr )
        return false;

    // Destroy joint.
    // This should result in the joint references also being destroyed
    // as the scene is a destruction listener.
    mpWorld->DestroyJoint( pJoint );

    return true;
}

//-----------------------------------------------------------------------------

bool t2dScene::hasJoints( SceneObject* pSceneObject )
{
    // Sanity!
    AssertFatal( pSceneObject != nullptr, "t2dScene object cannot be nullptr!" );
    AssertFatal( pSceneObject->getScene() != this, "t2dScene object is not in this scene" );

    // Fetch body.
    b2Body* pBody = pSceneObject->getBody();

    // Fetch joint edge.
    b2JointEdge* pJointEdge = pBody->GetJointList();

    if ( pJointEdge == nullptr || pJointEdge->joint == nullptr )
        return false;

    // Found at least one joint.
    return true;
}

//-----------------------------------------------------------------------------

S32 t2dScene::createDistanceJoint(
    const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
    const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
    const F32 length,
    const F32 frequency,
    const F32 dampingRatio,
    const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createDistanceJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2DistanceJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.length           = length < 0.0f ? (pBodyB->GetWorldPoint( localAnchorB ) - pBodyA->GetWorldPoint( localAnchorA )).Length() : length;
    jointDef.frequencyHz      = frequency;
    jointDef.dampingRatio     = dampingRatio;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setDistanceJointLength(
        const U32 jointId,
        const F32 length )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_distanceJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2DistanceJoint* pRealJoint = static_cast<b2DistanceJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetLength( length );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getDistanceJointLength( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_distanceJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2DistanceJoint* pRealJoint = static_cast<b2DistanceJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetLength();
}

//-----------------------------------------------------------------------------

void t2dScene::setDistanceJointFrequency(
        const U32 jointId,
        const F32 frequency )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_distanceJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2DistanceJoint* pRealJoint = static_cast<b2DistanceJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetFrequency( frequency );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getDistanceJointFrequency( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_distanceJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2DistanceJoint* pRealJoint = static_cast<b2DistanceJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetFrequency();
}

//-----------------------------------------------------------------------------

void t2dScene::setDistanceJointDampingRatio(
        const U32 jointId,
        const F32 dampingRatio )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_distanceJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2DistanceJoint* pRealJoint = static_cast<b2DistanceJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetDampingRatio( dampingRatio );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getDistanceJointDampingRatio( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_distanceJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str());
        return -1.0f;
    }

    // Cast joint.
    b2DistanceJoint* pRealJoint = static_cast<b2DistanceJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetDampingRatio();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createRopeJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const F32 maxLength,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createRopeJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2RopeJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.maxLength        = maxLength < 0.0f ? (pBodyB->GetWorldPoint( localAnchorB ) - pBodyA->GetWorldPoint( localAnchorA )).Length() : maxLength;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setRopeJointMaxLength(
        const U32 jointId,
        const F32 maxLength )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_ropeJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2RopeJoint* pRealJoint = static_cast<b2RopeJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMaxLength( maxLength );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getRopeJointMaxLength( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_ropeJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2RopeJoint* pRealJoint = static_cast<b2RopeJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetMaxLength();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createRevoluteJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createRevoluteJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2RevoluteJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.referenceAngle   = pBodyB->GetAngle() - pBodyA->GetAngle();
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setRevoluteJointLimit(
        const U32 jointId,
        const bool enableLimit,
        const F32 lowerAngle, const F32 upperAngle )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_revoluteJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2RevoluteJoint* pRealJoint = static_cast<b2RevoluteJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetLimits( lowerAngle, upperAngle );
    pRealJoint->EnableLimit( enableLimit );
}

//-----------------------------------------------------------------------------

bool t2dScene::getRevoluteJointLimit(
        const U32 jointId,
        bool& enableLimit,
        F32& lowerAngle, F32& upperAngle )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return false;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_revoluteJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return false;
    }

    // Cast joint.
    b2RevoluteJoint* pRealJoint = static_cast<b2RevoluteJoint*>( pJoint );

    // Access joint.
    enableLimit = pRealJoint->IsLimitEnabled();
    lowerAngle = pRealJoint->GetLowerLimit();
    upperAngle = pRealJoint->GetUpperLimit();

    return true;
}

//-----------------------------------------------------------------------------

void t2dScene::setRevoluteJointMotor(
        const U32 jointId,
        const bool enableMotor,
        const F32 motorSpeed,
        const F32 maxMotorTorque )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_revoluteJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2RevoluteJoint* pRealJoint = static_cast<b2RevoluteJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMotorSpeed( motorSpeed );
    pRealJoint->SetMaxMotorTorque( maxMotorTorque );
    pRealJoint->EnableMotor( enableMotor );    
}

//-----------------------------------------------------------------------------

bool t2dScene::getRevoluteJointMotor(
        const U32 jointId,
        bool& enableMotor,
        F32& motorSpeed,
        F32& maxMotorTorque )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return false;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_revoluteJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return false;
    }

    // Cast joint.
    b2RevoluteJoint* pRealJoint = static_cast<b2RevoluteJoint*>( pJoint );

    // Access joint.
    enableMotor = pRealJoint->IsMotorEnabled();
    motorSpeed = pRealJoint->GetMotorSpeed();
    maxMotorTorque = pRealJoint->GetMaxMotorTorque();

    return true;
}

//-----------------------------------------------------------------------------

F32 t2dScene::getRevoluteJointAngle( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return 0.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_revoluteJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return 0.0f;
    }

    // Cast joint.
    b2RevoluteJoint* pRealJoint = static_cast<b2RevoluteJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetJointAngle();
}

//-----------------------------------------------------------------------------

F32	t2dScene::getRevoluteJointSpeed( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return 0.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_revoluteJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return 0.0f;
    }

    // Cast joint.
    b2RevoluteJoint* pRealJoint = static_cast<b2RevoluteJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetJointSpeed();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createWeldJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const F32 frequency,
        const F32 dampingRatio,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createWeldJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2WeldJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.referenceAngle   = pBodyB->GetAngle() - pBodyA->GetAngle();
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.frequencyHz      = frequency;
    jointDef.dampingRatio     = dampingRatio;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setWeldJointFrequency(
        const U32 jointId,
        const F32 frequency )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_weldJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2WeldJoint* pRealJoint = static_cast<b2WeldJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetFrequency( frequency );
}


//-----------------------------------------------------------------------------

F32 t2dScene::getWeldJointFrequency( const U32 jointId  )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_weldJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2WeldJoint* pRealJoint = static_cast<b2WeldJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetFrequency();
}

//-----------------------------------------------------------------------------

void t2dScene::setWeldJointDampingRatio(
        const U32 jointId,
        const F32 dampingRatio )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_weldJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2WeldJoint* pRealJoint = static_cast<b2WeldJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetDampingRatio( dampingRatio );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getWeldJointDampingRatio( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_weldJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2WeldJoint* pRealJoint = static_cast<b2WeldJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetDampingRatio();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createWheelJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const b2Vec2& worldAxis,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createWheelJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2WheelJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.localAxisA       = pBodyA->GetLocalVector( worldAxis );
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setWheelJointMotor(
        const U32 jointId,
        const bool enableMotor,
        const F32 motorSpeed,
        const F32 maxMotorTorque )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_wheelJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2WheelJoint* pRealJoint = static_cast<b2WheelJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMotorSpeed( motorSpeed );
    pRealJoint->SetMaxMotorTorque( maxMotorTorque );
    pRealJoint->EnableMotor( enableMotor ); 
}

//-----------------------------------------------------------------------------

bool t2dScene::getWheelJointMotor(
        const U32 jointId,
        bool& enableMotor,
        F32& motorSpeed,
        F32& maxMotorTorque )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return false;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_wheelJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return false;
    }

    // Cast joint.
    b2WheelJoint* pRealJoint = static_cast<b2WheelJoint*>( pJoint );

    // Access joint.
    enableMotor = pRealJoint->IsMotorEnabled();
    motorSpeed = pRealJoint->GetMotorSpeed();
    maxMotorTorque = pRealJoint->GetMaxMotorTorque();

    return true;
}

//-----------------------------------------------------------------------------

void t2dScene::setWheelJointFrequency(
        const U32 jointId,
        const F32 frequency )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_wheelJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2WheelJoint* pRealJoint = static_cast<b2WheelJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetSpringFrequencyHz( frequency );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getWheelJointFrequency( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_wheelJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2WheelJoint* pRealJoint = static_cast<b2WheelJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetSpringFrequencyHz();
}

//-----------------------------------------------------------------------------

void t2dScene::setWheelJointDampingRatio(
        const U32 jointId,
        const F32 dampingRatio )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_wheelJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2WheelJoint* pRealJoint = static_cast<b2WheelJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetSpringDampingRatio( dampingRatio );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getWheelJointDampingRatio( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_wheelJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2WheelJoint* pRealJoint = static_cast<b2WheelJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetSpringDampingRatio();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createFrictionJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const F32 maxForce,
        const F32 maxTorque,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createFrictionJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2FrictionJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.maxForce         = maxForce;
    jointDef.maxTorque        = maxTorque;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setFrictionJointMaxForce(
        const U32 jointId,
        const F32 maxForce )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_frictionJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2FrictionJoint* pRealJoint = static_cast<b2FrictionJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMaxForce( maxForce );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getFrictionJointMaxForce( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_frictionJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2FrictionJoint* pRealJoint = static_cast<b2FrictionJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetMaxForce();
}

//-----------------------------------------------------------------------------

void t2dScene::setFrictionJointMaxTorque(
        const U32 jointId,
        const F32 maxTorque )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_frictionJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2FrictionJoint* pRealJoint = static_cast<b2FrictionJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMaxTorque( maxTorque );
}


//-----------------------------------------------------------------------------

F32 t2dScene::getFrictionJointMaxTorque( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_frictionJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2FrictionJoint* pRealJoint = static_cast<b2FrictionJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetMaxTorque();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createPrismaticJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const b2Vec2& worldAxis,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createPrismaticJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2PrismaticJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.referenceAngle   = pBodyB->GetAngle() - pBodyA->GetAngle();
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.localAxisA       = pBodyA->GetLocalVector( worldAxis );
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setPrismaticJointLimit(
        const U32 jointId,
        const bool enableLimit,
        const F32 lowerTranslation, const F32 upperTranslation )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_prismaticJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2PrismaticJoint* pRealJoint = static_cast<b2PrismaticJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetLimits( lowerTranslation, upperTranslation );
    pRealJoint->EnableLimit( enableLimit );
}

//-----------------------------------------------------------------------------

bool t2dScene::getPrismaticJointLimit(
        const U32 jointId,
        bool& enableLimit,
        F32& lowerTranslation, F32& upperTranslation )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return false;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_prismaticJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return false;
    }

    // Cast joint.
    b2PrismaticJoint* pRealJoint = static_cast<b2PrismaticJoint*>( pJoint );

    // Access joint.
    enableLimit = pRealJoint->IsLimitEnabled();
    lowerTranslation = pRealJoint->GetLowerLimit();
    upperTranslation = pRealJoint->GetUpperLimit();

    return true;
}

//-----------------------------------------------------------------------------

void t2dScene::setPrismaticJointMotor(
        const U32 jointId,
        const bool enableMotor,
        const F32 motorSpeed,
        const F32 maxMotorForce )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_prismaticJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2PrismaticJoint* pRealJoint = static_cast<b2PrismaticJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMotorSpeed( motorSpeed );
    pRealJoint->SetMaxMotorForce( maxMotorForce );
    pRealJoint->EnableMotor( enableMotor ); 
}

//-----------------------------------------------------------------------------

bool t2dScene::getPrismaticJointMotor(
        const U32 jointId,
        bool& enableMotor,
        F32& motorSpeed,
        F32& maxMotorForce )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return false;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_prismaticJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return false;
    }

    // Cast joint.
    b2PrismaticJoint* pRealJoint = static_cast<b2PrismaticJoint*>( pJoint );

    // Access joint.
    enableMotor = pRealJoint->IsMotorEnabled();
    motorSpeed = pRealJoint->GetMotorSpeed();
    maxMotorForce = pRealJoint->GetMaxMotorForce();

    return true;
}

//-----------------------------------------------------------------------------

S32 t2dScene::createPulleyJoint(
        const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
        const b2Vec2& localAnchorA, const b2Vec2& localAnchorB,
        const b2Vec2& worldGroundAnchorA, const b2Vec2& worldGroundAnchorB,
        const F32 ratio,
        const F32 lengthA, const F32 lengthB,
        const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createPulleyJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2PulleyJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = pBodyA;
    jointDef.bodyB            = pBodyB;
    jointDef.groundAnchorA    = worldGroundAnchorA;
    jointDef.groundAnchorB    = worldGroundAnchorB;
    jointDef.localAnchorA     = localAnchorA;
    jointDef.localAnchorB     = localAnchorB;
    jointDef.lengthA          = lengthA < 0.0f ? (pBodyA->GetWorldPoint( localAnchorA ) - worldGroundAnchorA).Length() : lengthA;
    jointDef.lengthB          = lengthB < 0.0f ? (pBodyA->GetWorldPoint( localAnchorB ) - worldGroundAnchorB).Length() : lengthB;
    jointDef.ratio            = ratio > b2_epsilon ? ratio : b2_epsilon + b2_epsilon;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

S32 t2dScene::createTargetJoint(
        const SceneObject* pSceneObject,
        const b2Vec2& worldTarget,
        const F32 maxForce,
        const bool useCenterOfMass,
        const F32 frequency,
        const F32 dampingRatio,
        const bool collideConnected )
{
    // Sanity!
    if ( pSceneObject != nullptr && pSceneObject->getScene() == nullptr )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for invalid object.
    if ( pSceneObject == nullptr )
    {
        Con::warnf("t2dScene::createPulleyJoint() - Cannot create joint without a scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBody = pSceneObject->getBody();
    
    // Populate definition.
    b2MouseJointDef jointDef;
    jointDef.userData         = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected = collideConnected;
    jointDef.bodyA            = getGroundBody();
    jointDef.bodyB            = pBody;
    jointDef.target           = useCenterOfMass ? pBody->GetWorldCenter() : worldTarget;
    jointDef.maxForce         = maxForce;
    jointDef.frequencyHz      = frequency;
    jointDef.dampingRatio     = dampingRatio;

    // Create joint.
    const U32 jointId = createJoint( &jointDef );

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( findJoint( jointId ) );

    // Are we using the center of mass?
    if ( !useCenterOfMass )
    {
        // No, so set the target as the world target.
        // NOTE:-   This is done because initially the target (mouse) joint assumes the target 
        //          coincides with the body anchor.
        pRealJoint->SetTarget( worldTarget );
    }

    return jointId;
}

//-----------------------------------------------------------------------------

void t2dScene::setTargetJointTarget(
        const U32 jointId,
        const b2Vec2& worldTarget )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetTarget( worldTarget );
}

//-----------------------------------------------------------------------------
b2Vec2 t2dScene::getTargetJointTarget( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return b2Vec2_zero;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return b2Vec2_zero;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetTarget();
}

//-----------------------------------------------------------------------------

void t2dScene::setTargetJointMaxForce(
        const U32 jointId,
        const F32 maxForce )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMaxForce( maxForce );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getTargetJointMaxForce( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetMaxForce();
}

//-----------------------------------------------------------------------------

void t2dScene::setTargetJointFrequency(
        const U32 jointId,
        const F32 frequency )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetFrequency( frequency );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getTargetJointFrequency( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetFrequency();
}

//-----------------------------------------------------------------------------

void t2dScene::setTargetJointDampingRatio(
        const U32 jointId,
        const F32 dampingRatio )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetDampingRatio( dampingRatio );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getTargetJointDampingRatio( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_mouseJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2MouseJoint* pRealJoint = static_cast<b2MouseJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetDampingRatio();
}

//-----------------------------------------------------------------------------

S32 t2dScene::createMotorJoint(
            const SceneObject* pSceneObjectA, const SceneObject* pSceneObjectB,
            const b2Vec2 linearOffset,
            const F32 angularOffset,
            const F32 maxForce,
            const F32 maxTorque,
            const F32 correctionFactor,
            const bool collideConnected )
{
    // Sanity!
    if (    (pSceneObjectA != nullptr && pSceneObjectA->getScene() == nullptr) ||
            (pSceneObjectB != nullptr && pSceneObjectB->getScene() == nullptr) )
    {
        // Warn.
        Con::printf( "Cannot add a joint to a scene object that is not in a scene." );
        return -1;
    }

    // Check for two invalid objects.
    if ( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
    {
        Con::warnf("t2dScene::createMotorJoint() - Cannot create joint without at least a single scene object." );
        return -1;
    }

    // Fetch bodies.
    b2Body* pBodyA = pSceneObjectA != nullptr ? pSceneObjectA->getBody() : getGroundBody();
    b2Body* pBodyB = pSceneObjectB != nullptr ? pSceneObjectB->getBody() : getGroundBody();
    
    // Populate definition.
    b2MotorJointDef jointDef;
    jointDef.userData           = static_cast<PhysicsProxy*>(this);
    jointDef.collideConnected   = collideConnected;
    jointDef.bodyA              = pBodyA;
    jointDef.bodyB              = pBodyB;
    jointDef.linearOffset       = linearOffset;
    jointDef.angularOffset      = angularOffset;
    jointDef.correctionFactor   = correctionFactor;
    jointDef.maxForce           = maxForce;
    jointDef.maxTorque          = maxTorque;
    
    // Create joint.
    return createJoint( &jointDef );
}

//-----------------------------------------------------------------------------

void t2dScene::setMotorJointLinearOffset(
        const U32 jointId,
        const b2Vec2& linearOffset )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetLinearOffset( linearOffset );
}

//-----------------------------------------------------------------------------

b2Vec2 t2dScene::getMotorJointLinearOffset( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return b2Vec2_zero;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return b2Vec2_zero;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetLinearOffset();
}

//-----------------------------------------------------------------------------

void t2dScene::setMotorJointAngularOffset(
        const U32 jointId,
        const F32 angularOffset )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetAngularOffset( angularOffset );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getMotorJointAngularOffset( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetAngularOffset();
}

//-----------------------------------------------------------------------------

void t2dScene::setMotorJointMaxForce(
        const U32 jointId,
        const F32 maxForce )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMaxForce( maxForce );
}

//-----------------------------------------------------------------------------

F32 t2dScene::getMotorJointMaxForce( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetMaxForce();
}

//-----------------------------------------------------------------------------

void t2dScene::setMotorJointMaxTorque(
        const U32 jointId,
        const F32 maxTorque )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    pRealJoint->SetMaxTorque( maxTorque );
}


//-----------------------------------------------------------------------------

F32 t2dScene::getMotorJointMaxTorque( const U32 jointId )
{
    // Fetch joint.
    b2Joint* pJoint = findJoint( jointId );

    // Ignore invalid joint.
    if ( !pJoint )
        return -1.0f;

    // Fetch joint type.
    const b2JointType jointType = pJoint->GetType();

    if ( jointType != e_motorJoint )
    {
        Con::warnf( "Invalid joint type of %s.", getJointTypeDescription(jointType).c_str() );
        return -1.0f;
    }

    // Cast joint.
    b2MotorJoint* pRealJoint = static_cast<b2MotorJoint*>( pJoint );

    // Access joint.
    return pRealJoint->GetMaxTorque();
}

//-----------------------------------------------------------------------------

void t2dScene::setDebugSceneObject( SceneObject* pSceneObject )
{
    // Ignore no change.
    if ( mpDebugSceneObject == pSceneObject )
        return;

    // Remove delete notification for existing monitored object.
    if ( mpDebugSceneObject != nullptr )
        clearNotify( mpDebugSceneObject );

    // Set monitored scene object.
    mpDebugSceneObject = pSceneObject;

    // Finish if resetting monitored object.
    if ( pSceneObject == nullptr )
        return;

    // Add delete notification for new monitored object.
    deleteNotify( pSceneObject );
}

void t2dScene::setLayerLight(U32 layer, ColorF light)
{
   if (mLayers.size() > layer)
      mLayers[layer]->setLight(light);
};


//-----------------------------------------------------------------------------

void t2dScene::setLayerSortMode( const U32 layer, const SceneRenderQueue::RenderSort sortMode )
{
   if (layer > 64)
   {
      // No, so warn.
      Con::warnf( "t2dScene::setLayerSortMode() - Layer '%d' is bigger than 64, is this intended?", layer );
      //
   }
   
   // Is the layer valid?
    while ( layer >= mLayers.size() )
    {
       mLayers.push_back(new Layer());
    }

    // Is the sort mode valid?
    if ( sortMode == SceneRenderQueue::RENDER_SORT_INVALID )
    {
        // No, so warn.
        Con::warnf( "t2dScene::setLayerSortMode() - Sort mode is invalid for layer '%d'.", layer );

        return;
    }

    mLayers[layer]->setSortMode(sortMode);
}

//-----------------------------------------------------------------------------

SceneRenderQueue::RenderSort t2dScene::getLayerSortMode( const U32 layer )
{
    // Is the layer valid?
    if ( layer >= mLayers.size() )
    {
        // No, so warn.
        Con::warnf( "t2dScene::getLayerSortMode() - Layer '%d' is out of range.", layer );

        return SceneRenderQueue::RENDER_SORT_INVALID;
    }

    return mLayers[layer]->getSortMode();
}

//-----------------------------------------------------------------------------

void t2dScene::attachSceneWindow( SceneWindow* pSceneWindow2D )
{
    // Ignore if already attached.
    if ( isSceneWindowAttached( pSceneWindow2D ) )
        return;

    // Add to Attached List.
    mAttachedSceneWindows.addObject( pSceneWindow2D );
}

//-----------------------------------------------------------------------------

void t2dScene::detachSceneWindow( SceneWindow* pSceneWindow2D )
{
    // Ignore if not attached.
    if ( !isSceneWindowAttached( pSceneWindow2D ) )
        return;

    // Add to Attached List.
    mAttachedSceneWindows.removeObject( pSceneWindow2D );
}

//-----------------------------------------------------------------------------

void t2dScene::detachAllSceneWindows( void )
{
    // Detach All t2dScene Windows.
    while( mAttachedSceneWindows.size() > 0 )
        dynamic_cast<SceneWindow*>(mAttachedSceneWindows[mAttachedSceneWindows.size()-1])->resetScene();
}

//-----------------------------------------------------------------------------

bool t2dScene::isSceneWindowAttached( SceneWindow* pSceneWindow2D )
{
    for( SimObject* itr:mAttachedSceneWindows )
        if ( pSceneWindow2D == dynamic_cast<SceneWindow*>(itr) )
            // Found.
            return true;

    // Not Found.
    return false;
}

//-----------------------------------------------------------------------------

void t2dScene::addDeleteRequest( SceneObject* pSceneObject )
{
    // Ignore if it's already being safe-deleted.
    if ( pSceneObject->isBeingDeleted() )
        return;

    // Populate Delete Request.
    tDeleteRequest deleteRequest = { pSceneObject->getId(), nullptr, false };

    // Push Delete Request.
    mDeleteRequests.push_back( deleteRequest );

    // Flag Delete in Progress.
    pSceneObject->mBeingSafeDeleted = true;
}


//-----------------------------------------------------------------------------

void t2dScene::processDeleteRequests( const bool forceImmediate )
{
    // Ignore if there's no delete requests!
    if ( mDeleteRequests.size() == 0 )
        return;

    // Validate All Delete Requests.
    U32 safeDeleteReadyCount = 0;
    for ( U32 requestIndex = 0; requestIndex < (U32)mDeleteRequests.size(); )
    {
        // Fetch Reference to Delete Request.
        tDeleteRequest& deleteRequest = mDeleteRequests[requestIndex];

        // Fetch Object.
        // NOTE:- Let's be safer and check that it's definitely a scene-object.
        SceneObject* pSceneObject = dynamic_cast<SceneObject*>( Sim::findObject( deleteRequest.mObjectId ) );

        // Does this object exist?
        if ( pSceneObject )
        {
            // Yes, so write object.
            deleteRequest.mpSceneObject = pSceneObject;

            // Calculate Safe-Ready Flag.
            deleteRequest.mSafeDeleteReady = forceImmediate || pSceneObject->getSafeDelete();

            // Is it ready to safe-delete?
            if ( deleteRequest.mSafeDeleteReady )
            {
                // Yes, so increase safe-ready count.
                ++safeDeleteReadyCount;
            }         
        }
        else
        {
            // No, so it looks like the object got deleted prematurely; let's just remove
            // the request instead.
            mDeleteRequests.erase( requestIndex );
            
            // Repeat this item.
            continue;
        }

        // Skip to next request index.
        ++requestIndex;
    }

    // Stop if there's no delete requests!
    if ( mDeleteRequests.size() == 0 )
        return;

    // Transfer Delete-Requests to Temporary version.
    // NOTE:-   We do this because we may delete objects which have dependencies.  This would
    //          cause objects to be added to the safe-delete list.  We don't want to work on
    //          the list whilst this is happening so we'll transfer it to a temporary list.
    mDeleteRequestsTemp = mDeleteRequests;

    // Can we process all remaining delete-requests?
    if ( safeDeleteReadyCount == (U32)mDeleteRequestsTemp.size() )
    {
        // Yes, so process ALL safe-ready delete-requests.
        for ( U32 requestIndex = 0; requestIndex < (U32)mDeleteRequestsTemp.size(); ++requestIndex )
        {
            // Yes, so fetch object.
            SceneObject* pSceneObject = mDeleteRequestsTemp[requestIndex].mpSceneObject;

            // Do script callback.
            Con::executef(this, 2, "onSafeDelete", pSceneObject->getIdString() );

            // Destroy the object.
            pSceneObject->deleteObject();
        }

        // Remove All delete-requests.
        mDeleteRequestsTemp.clear();
    }
    else
    {
        // No, so process only safe-ready delete-requests.
        for ( U32 requestIndex = 0; requestIndex <(U32) mDeleteRequestsTemp.size(); )
        {
            // Fetch Reference to Delete Request.
            tDeleteRequest& deleteRequest = mDeleteRequestsTemp[requestIndex];

            // Is the Object Safe-Ready?
            if ( deleteRequest.mSafeDeleteReady )
            {
                // Yes, so fetch object.
                SceneObject* pSceneObject = deleteRequest.mpSceneObject;

                // Do script callback.
                Con::executef(this, 2, "onSafeDelete", pSceneObject->getIdString() );

                // Destroy the object.
                pSceneObject->deleteObject();

                // Quickly remove delete-request.
                mDeleteRequestsTemp.erase( requestIndex );

                // Repeat this item.
                continue;
            }

            // Skip to next request index.
            ++requestIndex;
        }
    }
}

//-----------------------------------------------------------------------------

void t2dScene::SayGoodbye( b2Joint* pJoint )
{
    // Find the joint id.
    const U32 jointId = findJointId( pJoint );

    // Ignore a bad joint.
    if ( jointId == 0 )
        return;

    // Remove joint references.
    mJoints.erase( jointId );
    mReverseJoints.erase( pJoint );
}

//-----------------------------------------------------------------------------

SceneObject*t2dScene::create( const char* pType )
{
    // Sanity!
    AssertFatal( pType != nullptr, "t2dScene::create() - Cannot create a nullptr type." );

    // Find the class rep.
    AbstractClassRep* pClassRep = AbstractClassRep::findClassRep( pType ); 

    // Did we find the type?
    if ( pClassRep == nullptr )
    {
        // No, so warn.
        Con::warnf( "t2dScene::create() - Could not find type '%s' to create.", pType );
        return nullptr;
    }

    // Find the scene object rep.
    AbstractClassRep* pSceneObjectRep = AbstractClassRep::findClassRep( "SceneObject" ); 

    // Sanity!
    AssertFatal( pSceneObjectRep != nullptr,  "t2dScene::create() - Could not find SceneObject class rep." );

    // Is the type derived from scene object?
    if ( !pClassRep->isClass( pSceneObjectRep ) )
    {
        // No, so warn.
        Con::warnf( "t2dScene::create() - Type '%s' is not derived from SceneObject.", pType );
        return nullptr;
    }
    
    // Create the type.
    SceneObject* pSceneObject = dynamic_cast<SceneObject*>( pClassRep->create() );

    // Sanity!
    AssertFatal( pSceneObject != nullptr, "t2dScene::create() - Failed to create type via class rep." );

    // Attempt to register the object.
    if ( !pSceneObject->registerObject() )
    {
        // No, so warn.
        Con::warnf( "t2dScene::create() - Failed to register type '%s'.", pType );
        delete pSceneObject;
        return nullptr;
    }

    // Add to the scene.
    addToScene( pSceneObject );

    return pSceneObject;
}

//-----------------------------------------------------------------------------

void t2dScene::onTamlPreRead( void )
{
    // Call parent.
    Parent::onTamlPreRead();
}

//-----------------------------------------------------------------------------

void t2dScene::onTamlPostRead( const TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlPostRead( customNodes );

    // Reset the loading scene.
    t2dScene::LoadingScene = nullptr;

    // Find joint custom node.
    const TamlCustomNode* pJointNode = customNodes.findNode( jointCustomNodeName );

    // Do we have any joints?
    if ( pJointNode != nullptr )
    {
        // Yes, so fetch children joint nodes.
        // Iterate joints.
        for( TamlCustomNode* pJointNode:pJointNode->getChildren() )
        {
            // Fetch node name.
            StringTableEntry nodeName = pJointNode->getNodeName();

            // Is this a distance joint?
            if ( nodeName == jointDistanceNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                F32 length = -1.0f;
                F32 frequency = 0.0f;
                F32 dampingRatio = 0.0f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointDistanceLengthName )
                    {
                        pField->getFieldValue( length );
                    }
                    else if ( fieldName == jointDistanceFrequencyName )
                    {
                        pField->getFieldValue( frequency );
                    }
                    else if ( fieldName == jointDistanceDampingRatioName )
                    {
                        pField->getFieldValue( dampingRatio );
                    }
                }

                // Create joint.
                createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, length, frequency, dampingRatio, collideConnected );
            }
            // is this a rope joint?
            else if ( nodeName == jointRopeNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                F32 maxLength = -1.0f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField: pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointRopeMaxLengthName )
                    {
                        pField->getFieldValue( maxLength );
                    }
                }

                // Create joint.
                createRopeJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxLength, collideConnected );
            }
            // Is this a revolute joint?
            else if ( nodeName == jointRevoluteNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                bool enableLimit = false;
                F32 lowerAngle = 0.0f;
                F32 upperAngle = 0.0f;

                bool enableMotor = false;
                F32 motorSpeed = b2_pi;
                F32 maxMotorTorque = 0.0f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointRevoluteLimitLowerAngleName )
                    {
                        pField->getFieldValue( lowerAngle );
                        lowerAngle = mDegToRad( lowerAngle );
                        enableLimit = true;
                    }
                    else if ( fieldName == jointRevoluteLimitUpperAngleName )
                    {
                        pField->getFieldValue( upperAngle );
                        upperAngle = mDegToRad( upperAngle );
                        enableLimit = true;
                    }
                    else if ( fieldName == jointRevoluteMotorSpeedName )
                    {
                        pField->getFieldValue( motorSpeed );
                        motorSpeed = mDegToRad( motorSpeed );
                        enableMotor = true;
                    }
                    else if ( fieldName == jointRevoluteMotorMaxTorqueName )
                    {
                        pField->getFieldValue( maxMotorTorque );
                        enableMotor = true;
                    }
                }

                // Create joint.
                const U32 jointId = createRevoluteJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, collideConnected );
            
                if ( enableLimit )
                    setRevoluteJointLimit( jointId, true, lowerAngle, upperAngle );

                if ( enableMotor )
                    setRevoluteJointMotor( jointId, true, motorSpeed, maxMotorTorque );
            }
            // is this a weld joint?
            else if ( nodeName == jointWeldNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                F32 frequency = 0.0f;
                F32 dampingRatio = 0.0f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointWeldFrequencyName )
                    {
                        pField->getFieldValue( frequency );
                    }
                    else if ( fieldName == jointWeldDampingRatioName )
                    {
                        pField->getFieldValue( dampingRatio );
                    }
                }

                // Create joint.
                createWeldJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, frequency, dampingRatio, collideConnected );
            }
            // Is this a wheel joint?
            else if ( nodeName == jointWheelNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                bool enableMotor = false;
                F32 motorSpeed = b2_pi;
                F32 maxMotorTorque = 0.0f;

                F32 frequency = 0.0f;
                F32 dampingRatio = 0.0f;
                b2Vec2 worldAxis( 0.0f, 1.0f );

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields())
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointWheelMotorSpeedName )
                    {
                        pField->getFieldValue( motorSpeed );
                        motorSpeed = mDegToRad( motorSpeed );
                        enableMotor = true;
                    }
                    else if ( fieldName == jointWheelMotorMaxTorqueName )
                    {
                        pField->getFieldValue( maxMotorTorque );
                        enableMotor = true;
                    }
                    else if ( fieldName == jointWheelFrequencyName )
                    {
                        pField->getFieldValue( frequency );
                    }
                    else if ( fieldName == jointWheelDampingRatioName )
                    {
                        pField->getFieldValue( dampingRatio );
                    }
                    else if ( fieldName == jointWheelWorldAxisName )
                    {
                        pField->getFieldValue( worldAxis );                    
                    }
                }

                // Create joint.
                const U32 jointId = createWheelJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldAxis, collideConnected );

                if ( enableMotor )
                    setWheelJointMotor( jointId, true, motorSpeed, maxMotorTorque );

                setWheelJointFrequency( jointId, frequency );
                setWheelJointDampingRatio( jointId, dampingRatio );
            }
            // Is this a friction joint?
            else if ( nodeName == jointFrictionNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                F32 maxForce = 0.0f;
                F32 maxTorque = 0.0f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointFrictionMaxForceName )
                    {
                        pField->getFieldValue( maxForce );
                    }
                    else if ( fieldName == jointFrictionMaxTorqueName )
                    {
                        pField->getFieldValue( maxTorque );
                    }
                }

                // Create joint.
                createFrictionJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxForce, maxTorque, collideConnected );
            }
            // Is this a prismatic joint?
            else if ( nodeName == jointPrismaticNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                bool enableLimit;
                F32 lowerTransLimit = 0.0f;
                F32 upperTransLimit = 1.0f;

                bool enableMotor = false;
                F32 motorSpeed = b2_pi;
                F32 maxMotorForce = 0.0f;

                b2Vec2 worldAxis( 0.0f, 1.0f );

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointPrismaticLimitLowerTransName )
                    {
                        pField->getFieldValue( lowerTransLimit );
                        enableLimit = true;
                    }
                    else if ( fieldName == jointPrismaticLimitUpperTransName )
                    {
                        pField->getFieldValue( upperTransLimit );
                        enableLimit = true;
                    }
                    else if ( fieldName == jointPrismaticMotorSpeedName )
                    {
                        pField->getFieldValue( motorSpeed );
                        motorSpeed = mDegToRad( motorSpeed );
                        enableMotor = true;
                    }
                    else if ( fieldName == jointPrismaticMotorMaxForceName )
                    {
                        pField->getFieldValue( maxMotorForce );
                        enableMotor = true;
                    }
                    else if ( fieldName == jointPrismaticWorldAxisName )
                    {
                        pField->getFieldValue( worldAxis );
                    }
                }

                // Create joint.
                const U32 jointId = createPrismaticJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldAxis, collideConnected );

                if ( enableLimit )
                    setPrismaticJointLimit( jointId, true, lowerTransLimit, upperTransLimit );

                if ( enableMotor )
                    setPrismaticJointMotor( jointId, true, motorSpeed, maxMotorForce );
            }
            // Is this a pulley joint?
            else if ( nodeName == jointPulleyNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                b2Vec2 localAnchorA = b2Vec2_zero;
                b2Vec2 localAnchorB = b2Vec2_zero;
                bool collideConnected = false;

                F32 lengthA = -1.0f;
                F32 lengthB = -1.0f;
                F32 ratio = 0.5f;
                b2Vec2 worldGroundAnchorA = b2Vec2_zero;
                b2Vec2 worldGroundAnchorB = b2Vec2_zero;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointLocalAnchorAName )
                    {
                        pField->getFieldValue( localAnchorA );
                    }
                    else if ( fieldName == jointLocalAnchorBName )
                    {
                        pField->getFieldValue( localAnchorB );
                    }
                    else if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointPulleyLengthAName )
                    {
                        pField->getFieldValue( lengthA );
                    }
                    else if ( fieldName == jointPulleyLengthBName )
                    {
                        pField->getFieldValue( lengthB );
                    }
                    else if ( fieldName == jointPulleyRatioName )
                    {
                        pField->getFieldValue( ratio );
                    }
                    else if ( fieldName == jointPulleyGroundAnchorAName )
                    {
                        pField->getFieldValue( worldGroundAnchorA );
                    }
                    else if ( fieldName == jointPulleyGroundAnchorBName )
                    {
                        pField->getFieldValue( worldGroundAnchorB );
                    }
                }

                // Create joint.
                createPulleyJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldGroundAnchorA, worldGroundAnchorB, ratio, lengthA, lengthB, collideConnected );

            }
            // Is this a target joint?
            else if ( nodeName == jointTargetNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObject = jointChildren.size() == 1 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObject == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has an invalid connected object.", nodeName );
                    continue;
                }

                bool collideConnected = false;
                b2Vec2 worldTarget = b2Vec2_zero;
                F32 maxForce = 1.0f;
                F32 frequency = 5.0f;
                F32 dampingRatio = 0.7f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointTargetWorldTargetName )
                    {
                        pField->getFieldValue( worldTarget );
                    }
                    else if ( fieldName == jointTargetMaxForceName )
                    {
                        pField->getFieldValue( maxForce );
                    }
                    else if ( fieldName == jointTargetFrequencyName )
                    {
                        pField->getFieldValue( frequency );
                    }
                    else if ( fieldName == jointTargetDampingRatioName )
                    {
                        pField->getFieldValue( dampingRatio );
                    }
                }

                // Create joint.
                createTargetJoint( pSceneObject, worldTarget, maxForce, frequency, dampingRatio, collideConnected );
            }
            // Is this a motor joint?
            else if ( nodeName == jointMotorNodeName )
            {
                // Fetch joint children.
                const TamlCustomNodeVector& jointChildren = pJointNode->getChildren();

                // Fetch joint objects.
                SceneObject* pSceneObjectA = jointChildren.size() > 0 ? jointChildren[0]->getProxyObject<SceneObject>(true) : nullptr;
                SceneObject* pSceneObjectB = jointChildren.size() == 2 ? jointChildren[1]->getProxyObject<SceneObject>(true) : nullptr;

                // Did we get any connected objects?
                if( pSceneObjectA == nullptr && pSceneObjectB == nullptr )
                {
                    // No, so warn.
                    Con::warnf( "t2dScene::onTamlPostRead() - Encountered a joint '%s' but it has invalid connected objects.", nodeName );
                    continue;
                }

                bool collideConnected = false;

                b2Vec2 linearOffset = b2Vec2_zero;
                F32 angularOffset = 0.0f;
                F32 maxForce = 1.0f;
                F32 maxTorque = 1.0f;
                F32 correctionFactor = 0.3f;

                // Fetch joint fields.
                // Iterate property fields.
                for ( TamlCustomField* pField:pJointNode->getFields() )
                {
                    // Fetch property field name.
                    StringTableEntry fieldName = pField->getFieldName();

                    // Fetch fields.
                    if ( fieldName == jointCollideConnectedName )
                    {
                        pField->getFieldValue( collideConnected );
                    }
                    else if ( fieldName == jointMotorLinearOffsetName )
                    {
                        pField->getFieldValue( linearOffset );
                    }
                    else if ( fieldName == jointMotorAngularOffsetName )
                    {
                        pField->getFieldValue( angularOffset );
                        angularOffset = mDegToRad( angularOffset );
                    }
                    else if ( fieldName == jointMotorMaxForceName )
                    {
                        pField->getFieldValue( maxForce );
                    }
                    else if ( fieldName == jointMotorMaxTorqueName )
                    {
                        pField->getFieldValue( maxTorque );
                    }
                    else if ( fieldName == jointMotorCorrectionFactorName )
                    {
                        pField->getFieldValue( correctionFactor );
                    }
                }

                // Create joint.
                createMotorJoint( pSceneObjectA, pSceneObjectB, linearOffset, angularOffset, maxForce, maxTorque, correctionFactor, collideConnected );

            }
            // Unknown joint type!
            else
            {
                // Warn.
                Con::warnf( "Unknown joint type of '%s' encountered.", nodeName );

                // Sanity!
                AssertFatal( false, "t2dScene::onTamlCustomRead() - Unknown joint type detected." );

                continue;
            }
        }
    }

    // Find controller custom node.
    const TamlCustomNode* pControllerNode = customNodes.findNode( controllerCustomNodeName );

    // Do we have any controllers?
    if ( pControllerNode != nullptr )
    {
        // Yes, so fetch the scene controllers.
        SimSet* pControllerSet = getControllers();

        // Fetch children controller nodes.
        // Iterate controllers.
       for( auto itr = pControllerNode->getChildren().begin(); itr != pControllerNode->getChildren().end(); ++itr)
        {
            // Is the node a proxy object?
            if ( !pControllerNode->isProxyObject() )
            {
                // No, so warn.
                Con::warnf("t2dScene::onTamlPostRead() - Reading scene controllers but node '%s'is not an object.", pControllerNode->getNodeName() );

                continue;
            }

            // Add the proxy object.
            SimObject* pProxyObject = pControllerNode->getProxyObject<SimObject>(false);

            // Is it a scene controller?
            if ( dynamic_cast<SceneController*>( pProxyObject ) == nullptr )
            {
                // No, so warn.
                Con::warnf("t2dScene::onTamlPostRead() - Reading scene controllers but node '%s'is not a scene controller.", pControllerNode->getNodeName() );

                // Delete the object.
                pProxyObject->deleteObject();

                continue;
            }

            // Add to scene controllers.
            pControllerSet->addObject( pProxyObject );
        }
    }

    // Find asset preload custom node.
    const TamlCustomNode* pAssetPreloadNode = customNodes.findNode( assetPreloadNodeName );

    // Do we have any asset preloads?
    if ( pAssetPreloadNode != nullptr )
    {
        // Yes, so clear any existing asset preloads.
        clearAssetPreloads();

        // Yes, so fetch asset Id type prefix.
        StringTableEntry assetIdTypePrefix = ConsoleBaseType::getType( TypeAssetId )->getTypePrefix();

        // Fetch the prefix length.
        const dsize_t assetIdPrefixLength = dStrlen( assetIdTypePrefix );

        // Fetch the preload children nodes.
        // Iterate asset preloads.
        for( TamlCustomNode* pAssetNode:pAssetPreloadNode->getChildren() )
        {
            // Ignore non-asset nodes.
            if ( pAssetNode->getNodeName() != assetNodeName )
                continue;

            // Find the asset Id field.
            const TamlCustomField* pAssetIdField = pAssetNode->findField( "Id" );

            // Did we find the field?
            if ( pAssetIdField == nullptr )
            {
                // No, so warn.
                Con::warnf("t2dScene::onTamlPostRead() - Found asset preload but failed to find asset Id field." );
                continue;
            }

            // Fetch field value.
            const char* pFieldValue = pAssetIdField->getFieldValue();

            // Calculate the field value start (skip any type prefix).
            const dsize_t prefixOffset = dStrnicmp( pFieldValue, assetIdTypePrefix, assetIdPrefixLength ) == 0 ? assetIdPrefixLength : 0;

            // Add asset preload.
            addAssetPreload( pFieldValue + prefixOffset );
        }
    }   
}

//-----------------------------------------------------------------------------

void t2dScene::onTamlCustomWrite( TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlCustomWrite( customNodes );

    // Fetch joint count.
    const dsize_t jointCount = getJointCount();

    // Do we have any joints?
    if ( jointCount > 0 )
    {
        // Yes, so add joint custom node.
        TamlCustomNode* pJointCustomNode = customNodes.addNode( jointCustomNodeName );

        // Iterate joints.
        for( auto jointItr:mJoints )
        {
            // Fetch base joint.
            b2Joint* pBaseJoint = jointItr.second;

            // Add joint node.
            // NOTE:    The name of the node will get updated shortly.
            TamlCustomNode* pJointNode = pJointCustomNode->addNode( StringTable->EmptyString );

            // Fetch common details.
            b2Body* pBodyA = pBaseJoint->GetBodyA();
            b2Body* pBodyB = pBaseJoint->GetBodyB();

            // Fetch physics proxies.
            PhysicsProxy* pPhysicsProxyA = static_cast<PhysicsProxy*>(pBodyA->GetUserData());
            PhysicsProxy* pPhysicsProxyB = static_cast<PhysicsProxy*>(pBodyB->GetUserData());

            // Fetch physics proxy type.
            PhysicsProxy::ePhysicsProxyType proxyTypeA = static_cast<PhysicsProxy*>(pBodyA->GetUserData())->getPhysicsProxyType();
            PhysicsProxy::ePhysicsProxyType proxyTypeB = static_cast<PhysicsProxy*>(pBodyB->GetUserData())->getPhysicsProxyType();

            // Fetch scene objects.
            SceneObject* pSceneObjectA = proxyTypeA == PhysicsProxy::PHYSIC_PROXY_SCENEOBJECT ? static_cast<SceneObject*>(pPhysicsProxyA) : nullptr;
            SceneObject* pSceneObjectB = proxyTypeB == PhysicsProxy::PHYSIC_PROXY_SCENEOBJECT ? static_cast<SceneObject*>(pPhysicsProxyB) : nullptr;

            // Populate joint appropriately.
            switch( pBaseJoint->GetType() )
            {
                case e_distanceJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointDistanceNodeName );

                        // Fetch joint.
                        const b2DistanceJoint* pJoint = dynamic_cast<const b2DistanceJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid distance joint type returned." );

                        // Add length.
                        pJointNode->addField( jointDistanceLengthName, pJoint->GetLength() );

                        // Add frequency.
                        if ( mNotZero( pJoint->GetFrequency() ) )
                            pJointNode->addField( jointDistanceFrequencyName, pJoint->GetFrequency() );

                        // Add damping ratio.
                        if ( mNotZero( pJoint->GetDampingRatio() ) )
                            pJointNode->addField( jointDistanceDampingRatioName, pJoint->GetDampingRatio() );

                        // Add local anchors.
                        if ( mNotZero( pJoint->GetLocalAnchorA().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        if ( mNotZero( pJoint->GetLocalAnchorB().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_ropeJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointRopeNodeName );

                        // Fetch joint.
                        const b2RopeJoint* pJoint = dynamic_cast<const b2RopeJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid rope joint type returned." );

                        // Add max length.
                        if ( mNotZero( pJoint->GetMaxLength() ) )
                            pJointNode->addField( jointRopeMaxLengthName, pJoint->GetMaxLength() );

                        // Add local anchors.
                        if ( mNotZero( pJoint->GetLocalAnchorA().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        if ( mNotZero( pJoint->GetLocalAnchorB().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_revoluteJoint:
                    {
                        // Set join name.
                        pJointNode->setNodeName( jointRevoluteNodeName );

                        // Fetch joint.
                        const b2RevoluteJoint* pJoint = dynamic_cast<const b2RevoluteJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid revolute joint type returned." );

                        // Add limit.
                        if ( pJoint->IsLimitEnabled() )
                        {
                            // Add limits.
                            pJointNode->addField( jointRevoluteLimitLowerAngleName, mRadToDeg(pJoint->GetLowerLimit()) );
                            pJointNode->addField( jointRevoluteLimitUpperAngleName, mRadToDeg(pJoint->GetUpperLimit()) );
                        }

                        // Add motor.
                        if ( pJoint->IsMotorEnabled() )
                        {
                            // Add motor.
                            pJointNode->addField( jointRevoluteMotorSpeedName, mRadToDeg(pJoint->GetMotorSpeed()) );
                            pJointNode->addField( jointRevoluteMotorMaxTorqueName, pJoint->GetMaxMotorTorque() );
                        }

                        // Add local anchors.
                        if ( mNotZero( pJoint->GetLocalAnchorA().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        if ( mNotZero( pJoint->GetLocalAnchorB().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_weldJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointWeldNodeName );

                        // Fetch joint.
                        const b2WeldJoint* pJoint = dynamic_cast<const b2WeldJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid weld joint type returned." );

                        // Add frequency.
                        if ( mNotZero( pJoint->GetFrequency() ) )
                            pJointNode->addField( jointWeldFrequencyName, pJoint->GetFrequency() );

                        // Add damping ratio.
                        if ( mNotZero( pJoint->GetDampingRatio() ) )
                            pJointNode->addField( jointWeldDampingRatioName, pJoint->GetDampingRatio() );

                        // Add local anchors.
                        if ( mNotZero( pJoint->GetLocalAnchorA().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        if ( mNotZero( pJoint->GetLocalAnchorB().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_wheelJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointWheelNodeName );

                        // Fetch joint.
                        b2WheelJoint* pJoint = dynamic_cast<b2WheelJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid wheel joint type returned." );

                        // Add motor.
                        if ( pJoint->IsMotorEnabled() )
                        {
                            // Add motor.
                            pJointNode->addField( jointWheelMotorSpeedName, mRadToDeg(pJoint->GetMotorSpeed()) );
                            pJointNode->addField( jointWheelMotorMaxTorqueName, pJoint->GetMaxMotorTorque() );
                        }

                        // Add frequency.
                        if ( mNotZero( pJoint->GetSpringFrequencyHz() ) )
                            pJointNode->addField( jointWheelFrequencyName, pJoint->GetSpringFrequencyHz() );

                        // Add damping ratio.
                        if ( mNotZero( pJoint->GetSpringDampingRatio() ) )
                            pJointNode->addField( jointWheelDampingRatioName, pJoint->GetSpringDampingRatio() );

                        // Add world axis.
                        pJointNode->addField( jointWheelWorldAxisName, pJoint->GetBodyA()->GetWorldVector( pJoint->GetLocalAxisA() ) );

                        // Add local anchors.
                        pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_frictionJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointFrictionNodeName );

                        // Fetch joint.
                        const b2FrictionJoint* pJoint = dynamic_cast<const b2FrictionJoint*>( pBaseJoint );

                        // Add max force.
                        if ( mNotZero( pJoint->GetMaxForce() ) )
                            pJointNode->addField( jointFrictionMaxForceName, pJoint->GetMaxForce() );

                        // Add max torque.
                        if ( mNotZero( pJoint->GetMaxTorque() ) )
                            pJointNode->addField( jointFrictionMaxTorqueName, pJoint->GetMaxTorque() );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid friction joint type returned." );

                        // Add local anchors.
                        if ( mNotZero( pJoint->GetLocalAnchorA().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        if ( mNotZero( pJoint->GetLocalAnchorB().LengthSquared() ) )
                            pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_prismaticJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointPrismaticNodeName );

                        // Fetch joint.
                        b2PrismaticJoint* pJoint = dynamic_cast<b2PrismaticJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid prismatic joint type returned." );

                        // Add limit.
                        if ( pJoint->IsLimitEnabled() )
                        {
                            // Add limits.
                            pJointNode->addField( jointPrismaticLimitLowerTransName, pJoint->GetLowerLimit() );
                            pJointNode->addField( jointPrismaticLimitUpperTransName, pJoint->GetUpperLimit() );
                        }

                        // Add motor.
                        if ( pJoint->IsMotorEnabled() )
                        {
                            // Add motor.
                            pJointNode->addField( jointPrismaticMotorSpeedName, mRadToDeg(pJoint->GetMotorSpeed()) );
                            pJointNode->addField( jointPrismaticMotorMaxForceName, pJoint->GetMaxMotorForce() );
                        }

                        // Add world axis.
                        pJointNode->addField( jointPrismaticWorldAxisName, pJoint->GetBodyA()->GetWorldVector( pJoint->GetLocalAxisA() ) );

                        // Add local anchors.
                        pJointNode->addField( jointLocalAnchorAName, pJoint->GetLocalAnchorA() );
                        pJointNode->addField( jointLocalAnchorBName, pJoint->GetLocalAnchorB() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_pulleyJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointPulleyNodeName );

                        // Fetch joint.
                        b2PulleyJoint* pJoint = dynamic_cast<b2PulleyJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid pulley joint type returned." );

                        // Add lengths.
                        pJointNode->addField( jointPulleyLengthAName, pJoint->GetLengthA() );
                        pJointNode->addField( jointPulleyLengthBName, pJoint->GetLengthB() );

                        // Add ratio,
                        pJointNode->addField( jointPulleyRatioName, pJoint->GetRatio() );

                        // Add ground anchors.
                        pJointNode->addField( jointPulleyGroundAnchorAName, pJoint->GetGroundAnchorA() );
                        pJointNode->addField( jointPulleyGroundAnchorBName, pJoint->GetGroundAnchorB() );

                        // Add local anchors.
                        pJointNode->addField( jointLocalAnchorAName, pJoint->GetBodyA()->GetLocalPoint( pJoint->GetAnchorA() ) );
                        pJointNode->addField( jointLocalAnchorBName, pJoint->GetBodyB()->GetLocalPoint( pJoint->GetAnchorB() ) );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_mouseJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointTargetNodeName );

                        // Fetch joint.
                        const b2MouseJoint* pJoint = dynamic_cast<const b2MouseJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid target joint type returned." );

                        // Add target.
                        pJointNode->addField( jointTargetWorldTargetName, pJoint->GetTarget() );

                        // Add max force.
                        pJointNode->addField( jointTargetMaxForceName, pJoint->GetMaxForce() );

                        // Add frequency
                        pJointNode->addField( jointTargetFrequencyName, pJoint->GetFrequency() );

                        // Add damping ratio.
                        pJointNode->addField( jointTargetDampingRatioName, pJoint->GetDampingRatio() );

                        // Add body.
                        // NOTE: This joint uses BODYB as the object, BODYA is the ground-body however for easy of use
                        // we'll refer to this as OBJECTA in the persisted format.
                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

                case e_motorJoint:
                    {
                        // Set joint name.
                        pJointNode->setNodeName( jointMotorNodeName );

                        // Fetch joint.
                        const b2MotorJoint* pJoint = dynamic_cast<const b2MotorJoint*>( pBaseJoint );

                        // Sanity!
                        AssertFatal( pJoint != nullptr, "t2dScene::onTamlCustomWrite() - Invalid motor joint type returned." );

                        // Add linear offset.
                        if ( mNotZero( pJoint->GetLinearOffset().LengthSquared() ) )
                            pJointNode->addField( jointMotorLinearOffsetName, pJoint->GetLinearOffset() );

                        // Add angular offset.
                        if ( mNotZero( pJoint->GetAngularOffset() ) )
                            pJointNode->addField( jointMotorAngularOffsetName, mRadToDeg( pJoint->GetAngularOffset() ) );

                        // Add max force.
                        pJointNode->addField( jointMotorMaxForceName, pJoint->GetMaxForce() );

                        // Add max torque.
                        pJointNode->addField( jointMotorMaxTorqueName, pJoint->GetMaxTorque() );

                        // Add correction factor.
                        pJointNode->addField( jointMotorCorrectionFactorName, pJoint->GetCorrectionFactor() );

                        // Add scene object bodies.
                        if ( pSceneObjectA != nullptr )
                            pJointNode->addNode( pSceneObjectA );

                        if ( pSceneObjectB != nullptr )
                            pJointNode->addNode( pSceneObjectB );
                    }
                    break;

            default:
                // Sanity!
                AssertFatal( false, "t2dScene::onTamlCustomWrite() - Unknown joint type detected." );
            }

            // Add collide connected flag.
            if ( pBaseJoint->GetCollideConnected() )
                pJointNode->addField( jointCollideConnectedName, pBaseJoint->GetCollideConnected() );
        }
    }

    // Fetch controller count.
    const size_t sceneControllerCount = getControllers() ? getControllers()->size() : 0;
    
    // Do we have any scene controllers?
    if ( sceneControllerCount > 0 )
    {
        // Yes, so add controller node.
        TamlCustomNode* pControllerCustomNode = customNodes.addNode( controllerCustomNodeName );

        // Fetch the scene controllers.
        SimSet* pControllerSet = getControllers();

        // Iterate scene controllers.
        for( S32 i = 0; i < sceneControllerCount; i++ )
        {
            // Fetch the set object.
            SimObject* pSetObject = pControllerSet->at(i);

            // Skip if not a controller.
            if ( !pSetObject->isType<SceneController*>() )
                continue;

            // Add controller node.
            pControllerCustomNode->addNode( pSetObject );
        }
    }

    // Fetch asset preload count.
    const size_t assetPreloadCount = getAssetPreloadCount();

    // Do we have any asset preloads?
    if ( assetPreloadCount > 0 )
    {
        // Yes, so fetch asset Id type prefix.
        StringTableEntry assetIdTypePrefix = ConsoleBaseType::getType( TypeAssetId )->getTypePrefix();

        // Add asset preload node.
        TamlCustomNode* pAssetPreloadCustomNode = customNodes.addNode( assetPreloadNodeName );

        // Iterate asset preloads.
        for( auto assetItr:mAssetPreloads )
        {
            // Add node.
            TamlCustomNode* pAssetNode = pAssetPreloadCustomNode->addNode( assetNodeName );

            char valueBuffer[1024];
            dSprintf( valueBuffer, sizeof(valueBuffer), "%s%s", assetIdTypePrefix, assetItr->getAssetId() );

            // Add asset Id.
            pAssetNode->addField( "Id", valueBuffer );
        }        
    }
}

//-----------------------------------------------------------------------------

void t2dScene::onTamlCustomRead( const TamlCustomNodes& customNodes )
{
    // Call parent.
    Parent::onTamlCustomRead( customNodes );
}

//-----------------------------------------------------------------------------

U32 t2dScene::getGlobalSceneCount( void )
{
    return sSceneCount;
}

//-----------------------------------------------------------------------------

SceneRenderRequest*t2dScene::createDefaultRenderRequest( SceneRenderQueue* pSceneRenderQueue, SceneObject* pSceneObject )
{
    // Create a render request and populate it with the default details.
    SceneRenderRequest* pSceneRenderRequest = pSceneRenderQueue->createRenderRequest()->set(
        pSceneObject,
        pSceneObject->getRenderPosition(),
        pSceneObject->getSceneLayerDepth(),
        pSceneObject->getSortPoint(),
        pSceneObject->getSerialId(),
        pSceneObject->getRenderGroup() );

    // Prepare the blending for it.
    pSceneRenderRequest->mBlendMode = pSceneObject->getBlendMode();
    pSceneRenderRequest->mBlendColor = pSceneObject->getBlendColor();
    pSceneRenderRequest->mSrcBlendFactor = pSceneObject->getSrcBlendFactor();
    pSceneRenderRequest->mDstBlendFactor = pSceneObject->getDstBlendFactor();
    pSceneRenderRequest->mAlphaTest = pSceneObject->getAlphaTest();

    return pSceneRenderRequest;
}

//-----------------------------------------------------------------------------

SimObject*t2dScene::getTamlChild( const U32 childIndex ) const
{
    // Sanity!
    AssertFatal( childIndex < (U32)mSceneObjects.size(), "t2dScene::getTamlChild() - Child index is out of range." );

    // For when the assert is not used.
    if ( childIndex >= (U32)mSceneObjects.size() )
        return nullptr;

    return mSceneObjects[ childIndex ];
}

//-----------------------------------------------------------------------------

void t2dScene::addTamlChild( SimObject* pSimObject )
{
    // Sanity!
    AssertFatal( pSimObject != nullptr, "t2dScene::addTamlChild() - Cannot add a nullptr child object." );

    // Fetch as a scene object.
    SceneObject* pSceneObject = dynamic_cast<SceneObject*>( pSimObject );

    // Is it a scene object?
    if ( pSceneObject == nullptr )
    {
        // No, so warn.
        Con::warnf( "t2dScene::addTamlChild() - Cannot add a child object that isn't a scene object." );
        return;
    }

    // Add the scene object.
    addToScene( pSceneObject );
}

//-----------------------------------------------------------------------------

static EnumTable::Enums DebugOptionsLookupEntries[11] =
                {
                EnumTable::Enums( t2dScene::SCENE_DEBUG_METRICS,           "metrics" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_FPS_METRICS,       "fps" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_CONTROLLERS,       "controllers" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_JOINTS,            "joints" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_WIREFRAME_RENDER,  "wireframe" ),
                ///
                EnumTable::Enums( t2dScene::SCENE_DEBUG_AABB,              "aabb" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_OOBB,              "oobb" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_SLEEP,             "sleep" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_COLLISION_SHAPES,  "collision" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_POSITION_AND_COM,  "position" ),
                EnumTable::Enums( t2dScene::SCENE_DEBUG_SORT_POINTS,       "sort" ),
                };

static EnumTable DebugOptionsLookupTable = EnumTable(11, DebugOptionsLookupEntries);

//-----------------------------------------------------------------------------

t2dScene::DebugOption t2dScene::getDebugOptionEnum(const char* label)
{
    if (DebugOptionsLookupTable.isLabel(label))
        return ((t2dScene::DebugOption)DebugOptionsLookupTable[label]);

    // Warn.
    Con::warnf( "t2dScene::getDebugOptionEnum() - Invalid debug option '%s'.", label );

    return t2dScene::SCENE_DEBUG_INVALID;
}

//-----------------------------------------------------------------------------

const char*t2dScene::getDebugOptionDescription( t2dScene::DebugOption debugOption )
{
    if (DebugOptionsLookupTable.isIndex(debugOption))
        return DebugOptionsLookupTable[debugOption].c_str();

    // Warn.
    Con::warnf( "t2dScene::getDebugOptionDescription() - Invalid debug option." );

    return StringTable->EmptyString;
}

//-----------------------------------------------------------------------------

static EnumTable::Enums jointTypeEntries[10] =
                {
                EnumTable::Enums( e_distanceJoint,  "distance"  ),
                EnumTable::Enums( e_ropeJoint,      "rope"      ),
                EnumTable::Enums( e_revoluteJoint,  "revolute"  ),
                EnumTable::Enums( e_weldJoint,      "weld"      ),
                EnumTable::Enums( e_wheelJoint,     "wheel"     ),
                EnumTable::Enums( e_frictionJoint,  "friction"  ),
                EnumTable::Enums( e_prismaticJoint, "prismatic" ),
                EnumTable::Enums( e_pulleyJoint,    "pulley"    ),
                EnumTable::Enums( e_mouseJoint,     "target"    ),
                EnumTable::Enums( e_motorJoint,     "motor"     ),
                };

EnumTable jointTypeTable = EnumTable(10, jointTypeEntries);
//-----------------------------------------------------------------------------

const std::string t2dScene::getJointTypeDescription(b2JointType jointType)
{
    if (jointTypeTable.isIndex(jointType))
        return jointTypeTable[jointType];

    // Warn.
    Con::warnf( "t2dScene::getJointTypeDescription() - Invalid joint type." );

    return StringTable->EmptyString;
}

//-----------------------------------------------------------------------------

b2JointType t2dScene::getJointTypeEnum(const char* label)
{
    if (jointTypeTable.isLabel(label))
        return (b2JointType)jointTypeTable[label];

    // Warn.
    Con::warnf( "t2dScene::getJointTypeEnum() - Invalid joint of '%s'", label );

    return e_unknownJoint;
}

//-----------------------------------------------------------------------------

static EnumTable::Enums pickModeLookupEntries[4] =
                {
                EnumTable::Enums( t2dScene::PICK_ANY,          "Any" ),
                EnumTable::Enums( t2dScene::PICK_AABB,         "AABB" ),
                EnumTable::Enums( t2dScene::PICK_OOBB,         "OOBB" ),
                EnumTable::Enums( t2dScene::PICK_COLLISION,    "Collision" ),
                };

static EnumTable pickModeLookupTable = EnumTable(4, pickModeLookupEntries);
//-----------------------------------------------------------------------------

t2dScene::PickMode t2dScene::getPickModeEnum(const char* label)
{
    if (pickModeLookupTable.isLabel(label))
        return (t2dScene::PickMode)pickModeLookupTable[label];

    // Warn.
    Con::warnf( "t2dScene::getPickModeEnum() - Invalid pick mode '%s'.", label );

    return t2dScene::PICK_INVALID;
}

//-----------------------------------------------------------------------------

const char*t2dScene::getPickModeDescription( t2dScene::PickMode pickMode )
{
    if (pickModeLookupTable.isIndex(pickMode))
        return pickModeLookupTable[pickMode].c_str();

    // Warn.
    Con::warnf( "t2dScene::getPickModeDescription() - Invalid pick mode.");

    return StringTable->EmptyString;
}

//-----------------------------------------------------------------------------

static void WriteJointsCustomTamlScehema( const AbstractClassRep* pClassRep, TiXmlElement* pParentElement )
{
    // Sanity!
    AssertFatal( pClassRep != nullptr,  "Taml::WriteJointsCustomTamlScehema() - ClassRep cannot be nullptr." );
    AssertFatal( pParentElement != nullptr,  "Taml::WriteJointsCustomTamlScehema() - Parent Element cannot be nullptr." );
   assert(pClassRep != nullptr);
   assert(pParentElement != nullptr);

    char buffer[1024];

    // Create joints node element.
    TiXmlElement* pJointsNodeElement = new TiXmlElement( "xs:element" );
    dSprintf( buffer, sizeof(buffer), "%s.%s", pClassRep->getClassName(), jointCustomNodeName );
    pJointsNodeElement->SetAttribute( "name", buffer );
    pJointsNodeElement->SetAttribute( "minOccurs", 0 );
    pJointsNodeElement->SetAttribute( "maxOccurs", 1 );
    pParentElement->LinkEndChild( pJointsNodeElement );
    
    // Create complex type.
    TiXmlElement* pJointsNodeComplexTypeElement = new TiXmlElement( "xs:complexType" );
    pJointsNodeElement->LinkEndChild( pJointsNodeComplexTypeElement );
    
    // Create choice element.
    TiXmlElement* pJointsNodeChoiceElement = new TiXmlElement( "xs:choice" );
    pJointsNodeChoiceElement->SetAttribute( "minOccurs", 0 );
    pJointsNodeChoiceElement->SetAttribute( "maxOccurs", "unbounded" );
    pJointsNodeComplexTypeElement->LinkEndChild( pJointsNodeChoiceElement );

    // ********************************************************************************
    // Create Distance Joint Element.
    // ********************************************************************************
    TiXmlElement* pDistanceJointElement = new TiXmlElement( "xs:element" );
    pDistanceJointElement->SetAttribute( "name", jointDistanceNodeName );
    pDistanceJointElement->SetAttribute( "minOccurs", 0 );
    pDistanceJointElement->SetAttribute( "maxOccurs", 1 );
    pJointsNodeChoiceElement->LinkEndChild( pDistanceJointElement );

    // Create complex type Element.
    TiXmlElement* pDistanceJointComplexTypeElement = new TiXmlElement( "xs:complexType" );
    pDistanceJointElement->LinkEndChild( pDistanceJointComplexTypeElement );

    // Create "Length" attribute.
    TiXmlElement* pDistanceJointElementA = new TiXmlElement( "xs:attribute" );
    pDistanceJointElementA->SetAttribute( "name", jointDistanceLengthName );
    pDistanceJointComplexTypeElement->LinkEndChild( pDistanceJointElementA );
    TiXmlElement* pDistanceJointElementB = new TiXmlElement( "xs:simpleType" );
    pDistanceJointElementA->LinkEndChild( pDistanceJointElementB );
    TiXmlElement* pDistanceJointElementC = new TiXmlElement( "xs:restriction" );
    pDistanceJointElementC->SetAttribute( "base", "xs:float" );
    pDistanceJointElementB->LinkEndChild( pDistanceJointElementC );
    TiXmlElement* pDistanceJointElementD = new TiXmlElement( "xs:minInclusive" );
    pDistanceJointElementD->SetAttribute( "value", "0" );
    pDistanceJointElementC->LinkEndChild( pDistanceJointElementD );

    // Create "Frequency" attribute.
    pDistanceJointElementA = new TiXmlElement( "xs:attribute" );
    pDistanceJointElementA->SetAttribute( "name", jointDistanceFrequencyName );
    pDistanceJointComplexTypeElement->LinkEndChild( pDistanceJointElementA );
    pDistanceJointElementB = new TiXmlElement( "xs:simpleType" );
    pDistanceJointElementA->LinkEndChild( pDistanceJointElementB );
    pDistanceJointElementC = new TiXmlElement( "xs:restriction" );
    pDistanceJointElementC->SetAttribute( "base", "xs:float" );
    pDistanceJointElementB->LinkEndChild( pDistanceJointElementC );
    pDistanceJointElementD = new TiXmlElement( "xs:minInclusive" );
    pDistanceJointElementD->SetAttribute( "value", "0" );
    pDistanceJointElementC->LinkEndChild( pDistanceJointElementD );

    // Create "Damping Ratio" attribute.
    pDistanceJointElementA = new TiXmlElement( "xs:attribute" );
    pDistanceJointElementA->SetAttribute( "name", jointDistanceDampingRatioName );
    pDistanceJointComplexTypeElement->LinkEndChild( pDistanceJointElementA );
    pDistanceJointElementB = new TiXmlElement( "xs:simpleType" );
    pDistanceJointElementA->LinkEndChild( pDistanceJointElementB );
    pDistanceJointElementC = new TiXmlElement( "xs:restriction" );
    pDistanceJointElementC->SetAttribute( "base", "xs:float" );
    pDistanceJointElementB->LinkEndChild( pDistanceJointElementC );
    pDistanceJointElementD = new TiXmlElement( "xs:minInclusive" );
    pDistanceJointElementD->SetAttribute( "value", "0" );
    pDistanceJointElementC->LinkEndChild( pDistanceJointElementD );
}

//-----------------------------------------------------------------------------

static void WriteControllersCustomTamlScehema( const AbstractClassRep* pClassRep, TiXmlElement* pParentElement )
{
    // Sanity!
    AssertFatal( pClassRep != nullptr,  "Taml::WriteControllersCustomTamlScehema() - ClassRep cannot be nullptr." );
    AssertFatal( pParentElement != nullptr,  "Taml::WriteControllersCustomTamlScehema() - Parent Element cannot be nullptr." );

    char buffer[1024];

    // Create controllers node element.
    TiXmlElement* pControllersNodeElement = new TiXmlElement( "xs:element" );
    dSprintf( buffer, sizeof(buffer), "%s.%s", pClassRep->getClassName(), controllerCustomNodeName );
    pControllersNodeElement->SetAttribute( "name", buffer );
    pControllersNodeElement->SetAttribute( "minOccurs", 0 );
    pControllersNodeElement->SetAttribute( "maxOccurs", 1 );
    pParentElement->LinkEndChild( pControllersNodeElement );
    
    // Create complex type.
    TiXmlElement* pControllersNodeComplexTypeElement = new TiXmlElement( "xs:complexType" );
    pControllersNodeElement->LinkEndChild( pControllersNodeComplexTypeElement );
    
    // Create choice element.
    TiXmlElement* pControllersNodeChoiceElement = new TiXmlElement( "xs:choice" );
    pControllersNodeChoiceElement->SetAttribute( "minOccurs", 0 );
    pControllersNodeChoiceElement->SetAttribute( "maxOccurs", "unbounded" );
    pControllersNodeComplexTypeElement->LinkEndChild( pControllersNodeChoiceElement );

    // Fetch the scene controller type.
    AbstractClassRep* pPickingSceneControllerType = AbstractClassRep::findClassRep( "PickingSceneController" );
    AbstractClassRep* pGroupedSceneControllerType = AbstractClassRep::findClassRep( "GroupedSceneController" );

    // Sanity!
    AssertFatal( pPickingSceneControllerType != nullptr, "t2dScene::WriteControllersCustomTamlScehema() - Cannot find the PickingSceneController type." );
    AssertFatal( pGroupedSceneControllerType != nullptr, "t2dScene::WriteControllersCustomTamlScehema() - Cannot find the GroupedSceneController type." );

    // Add choice members.
    for ( AbstractClassRep* pChoiceType = AbstractClassRep::getClassList(); pChoiceType != nullptr; pChoiceType = pChoiceType->getNextClass() )
    {
        // Skip if not derived from either of the scene controllers.
        if ( !pChoiceType->isClass( pPickingSceneControllerType ) && !pChoiceType->isClass( pGroupedSceneControllerType ) )
            continue;

        // Add choice member.
        TiXmlElement* pChoiceMemberElement = new TiXmlElement( "xs:element" );
        pChoiceMemberElement->SetAttribute( "name", pChoiceType->getClassName() );
        dSprintf( buffer, sizeof(buffer), "%s_Type", pChoiceType->getClassName() );
        pChoiceMemberElement->SetAttribute( "type", buffer );
        pControllersNodeChoiceElement->LinkEndChild( pChoiceMemberElement );
    }
}

//-----------------------------------------------------------------------------

static void WritePreloadsCustomTamlScehema( const AbstractClassRep* pClassRep, TiXmlElement* pParentElement )
{
    // Sanity!
    AssertFatal( pClassRep != nullptr,  "Taml::WritePreloadsCustomTamlScehema() - ClassRep cannot be nullptr." );
    AssertFatal( pParentElement != nullptr,  "Taml::WritePreloadsCustomTamlScehema() - Parent Element cannot be nullptr." );

    char buffer[1024];

    // Create preloads node element.
    TiXmlElement* pPreloadsNodeElement = new TiXmlElement( "xs:element" );
    dSprintf( buffer, sizeof(buffer), "%s.%s", pClassRep->getClassName(), assetPreloadNodeName );
    pPreloadsNodeElement->SetAttribute( "name", buffer );
    pPreloadsNodeElement->SetAttribute( "minOccurs", 0 );
    pPreloadsNodeElement->SetAttribute( "maxOccurs", 1 );
    pParentElement->LinkEndChild( pPreloadsNodeElement );
    
    // Create complex type.
    TiXmlElement* pPreloadsNodeComplexTypeElement = new TiXmlElement( "xs:complexType" );
    pPreloadsNodeElement->LinkEndChild( pPreloadsNodeComplexTypeElement );
    
    // Create choice element.
    TiXmlElement* pPreloadsNodeChoiceElement = new TiXmlElement( "xs:choice" );
    pPreloadsNodeChoiceElement->SetAttribute( "minOccurs", 0 );
    pPreloadsNodeChoiceElement->SetAttribute( "maxOccurs", "unbounded" );
    pPreloadsNodeComplexTypeElement->LinkEndChild( pPreloadsNodeChoiceElement );

    // Add choice member element.
    TiXmlElement* pChoiceMemberElement = new TiXmlElement( "xs:element" );
    pChoiceMemberElement->SetAttribute( "name", assetNodeName );
    pPreloadsNodeChoiceElement->LinkEndChild( pChoiceMemberElement );

    // Add choice member complex type element.
    TiXmlElement* pChoiceMemberComplexTypeElement = new TiXmlElement( "xs:complexType" );
    pChoiceMemberElement->LinkEndChild( pChoiceMemberComplexTypeElement );

    // Add choice member attribute element.
    TiXmlElement* pChoiceAttributeElement = new TiXmlElement( "xs:attribute" );
    pChoiceAttributeElement->SetAttribute( "name", "Id" );
    dSprintf( buffer, sizeof(buffer), "%s", "AssetId_ConsoleType" );
    pChoiceAttributeElement->SetAttribute( "type", buffer );
    pChoiceMemberComplexTypeElement->LinkEndChild( pChoiceAttributeElement );
}

//-----------------------------------------------------------------------------

static void WriteCustomTamlSchema( const AbstractClassRep* pClassRep, TiXmlElement* pParentElement )
{
    // Sanity!
    AssertFatal( pClassRep != nullptr,  "Taml::WriteCustomTamlSchema() - ClassRep cannot be nullptr." );
    AssertFatal( pParentElement != nullptr,  "Taml::WriteCustomTamlSchema() - Parent Element cannot be nullptr." );

    WriteJointsCustomTamlScehema( pClassRep, pParentElement );
    WriteControllersCustomTamlScehema( pClassRep, pParentElement );
    WritePreloadsCustomTamlScehema( pClassRep, pParentElement );
}

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT_CHILDREN_SCHEMA(t2dScene, WriteCustomTamlSchema);

RenderPassManager *t2dScene::getDefaultRenderPass() const {
    if (mDefaultRenderPass)
        return mDefaultRenderPass;
    return nullptr;
}


// http://www.iforce2d.net/b2dtut/explosions
void t2dScene::performBlastImpulse(b2Vec2 center, F32 radius, F32 blastPower, U32 sceneGroupMask, S32 numRays) {
    for (S32 i = 0; i < numRays; i++) {
        F32 angle = mDegToRad((i / (F32)numRays)*360.0);
        b2Vec2 rayDir( mSin(angle), mCos(angle));
        b2Vec2 rayEnd = center + radius * rayDir;

        // Fetch world query and clear results.
        WorldQuery* pWorldQuery = getWorldQuery( true );

        // Set filter.
        WorldQueryFilter queryFilter( sceneGroupMask, true, false, true, true );
        pWorldQuery->setQueryFilter( queryFilter );

        // Perform query.
        pWorldQuery->collisionQueryRay( center, rayEnd );

        // Sanity!
        AssertFatal( pWorldQuery->getIsRaycastQueryResult(), "Invalid non-ray-cast query result returned." );

        // Fetch result count.
        const size_t resultCount = pWorldQuery->getQueryResultsCount();

        // Finish if no results.
        if ( resultCount == 0 )
            continue;

        // Fetch results.
        typeWorldQueryResultVector& queryResults = pWorldQuery->getQueryResults();

        // Add Picked Objects to List.
        for ( WorldQueryResult n: queryResults )
        {
            b2Vec2 blastDir = n.mPoint - center;
            float distance = blastDir.Normalize();
            //ignore bodies exactly at the blast point - blast direction is undefined
            if ( distance == 0 )
                continue;
            float invDistance = 1 / distance;
            float impulseMag = blastPower * invDistance * invDistance;
            n.mpSceneObject->applyLinearImpulse( impulseMag*blastDir, n.mPoint);
        }

        // Clear world query.
        pWorldQuery->clearQuery();
    }
}


void t2dScene::setAllRenderLayers(bool flag)
{
   for (auto itr:mLayers)
   { itr->setRenderFlag(flag); }
};
