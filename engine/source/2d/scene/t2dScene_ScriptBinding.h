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

ConsoleMethodGroupBeginWithDocs(t2dScene, BehaviorComponent)

/*! Gets the system-wide scene count.
    @return The system-wide scene count.
*/
ConsoleFunctionWithDocs( getGlobalSceneCount, ConsoleInt, 1, 1, ())
{
    return t2dScene::getGlobalSceneCount();
}

//-----------------------------------------------------------------------------

/*! The gravity force to apply to all objects in the scene.
    @param forceX/forceY The direction and magnitude of the force in each direction. Formatted as either (\forceX forceY\ or (forceX, forceY)
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setGravity, ConsoleVoid, 3, 4, (forceX / forceY))
{
   // The force.
   Vector2 force;

   // Grab the element count.
   U32 elementCount = Utility::mGetStringElementCount(argv[2]);

   // ("forceX forceY")
   if (elementCount == 1)
   {
      force = Vector2(dAtof(argv[2]), dAtof(argv[3]));
   }
   else if (elementCount == 2)
   {
      force = Utility::mGetStringElementVector(argv[2]);
   }
   // Invalid
   else
   {
      Con::warnf("SceneObject::setGravity() - Invalid number of parameters!");
      return;
   }

    // Set gravity.
   object->setGravity(force);
}

//-----------------------------------------------------------------------------

ConsoleMethod(t2dScene, setAmbientLight, void, 5, 6, "(float red, float green, float blue, [float alpha = 1.0]) or ( stockColorName ) - Sets the scene ambient light color."
              "@param red The red value.\n"
              "@param green The green value.\n"
              "@param blue The blue value.\n"
              "@param alpha The alpha value.\n"
              "@return No return Value.")
{
    // The colors.
    F32 red;
    F32 green;
    F32 blue;
    F32 alpha = 1.0f;
    
    // (R, G, B)
    if (argc >= 5)
    {
        red   = dAtof(argv[2]);
        green = dAtof(argv[3]);
        blue  = dAtof(argv[4]);
        
        // Grab the alpha if it's there.
        if (argc > 5)
            alpha = dAtof(argv[5]);
    }
    
    // Invalid.
    else
    {
        Con::warnf("t2dScene::setAmbientLight() - Invalid Number of parameters!");
        return;
    }
    
    // Set blend color.
    object->setSceneLight(ColorF(red, green, blue, alpha));
}

//-----------------------------------------------------------------------------

ConsoleMethod(t2dScene, setLayerLight, void, 6, 7, "int layer, (float red, float green, float blue, [float alpha = 1.0]) or ( stockColorName ) - Sets the sprite blend color."
              "@param layer The layer to set an ambient Light for.\n"
              "@param red The red value.\n"
              "@param green The green value.\n"
              "@param blue The blue value.\n"
              "@param alpha The alpha value.\n"
              "@return No return Value.")
{
   // The Layer.
   S32 layer;
   // The colors.
   F32 red;
   F32 green;
   F32 blue;
   F32 alpha = 1.0f;
   
   layer = dAtoi(argv[2]);
   
   // (R, G, B)
   if (argc >= 6)
   {
      red   = dAtof(argv[3]);
      green = dAtof(argv[4]);
      blue  = dAtof(argv[5]);
      
      // Grab the alpha if it's there.
      if (argc > 6)
         alpha = dAtof(argv[6]);
   }
   
   // Invalid.
   else
   {
      Con::warnf("t2dScene::setAmbientLight() - Invalid Number of parameters!");
      return;
   }
   
   // Set blend color.
   object->setLayerLight(layer, ColorF(red, green, blue, alpha));
}


//-----------------------------------------------------------------------------

ConsoleMethod(t2dScene, setCameraPositionScale, void, 5, 5, "int layer, (float x, float y) - Sets a scale value for camera translation for this layer."
        "@param layer The layer to set an position scale for.\n"
        "@param x The x value.\n"
        "@param y The y value.\n"
        "@return No return Value.")
{
    object->getLayer(dAtoi(argv[2]))->setCameraTranslationScale(Point3F(dAtof(argv[3]), dAtof(argv[4]), 1.0f));
}

/*! Gets the gravity force applied to all objects in the scene.
    @return The gravity force applied to all objects in the scene.
*/
ConsoleMethodWithDocs(t2dScene, getGravity, ConsoleString, 2, 2, ())
{
    return Vector2(object->getGravity()).scriptThis();
}

//-----------------------------------------------------------------------------

/*! Sets the number of velocity iterations the physics solver uses.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setVelocityIterations, ConsoleVoid, 3, 3, (int iterations))
{
    object->setVelocityIterations( dAtoi(argv[2]) );
}

//-----------------------------------------------------------------------------

/*! Gets the number of velocity iterations the physics solver uses.
    (@return The number of velocity iterations the physics solver uses.
*/
ConsoleMethodWithDocs(t2dScene, getVelocityIterations, ConsoleInt, 2, 2, ())
{
    return object->getVelocityIterations();
}

//-----------------------------------------------------------------------------

/*! Sets the number of position iterations the physics solver uses.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setPositionIterations, ConsoleVoid, 3, 3, (int iterations))
{
    object->setPositionIterations( dAtoi(argv[2]) );
}

//-----------------------------------------------------------------------------

/*! Gets the number of position iterations the physics solver uses.
    (@return The number of position iterations the physics solver uses.
*/
ConsoleMethodWithDocs(t2dScene, getPositionIterations, ConsoleInt, 2, 2, ())
{
    return object->getPositionIterations();
}

//-----------------------------------------------------------------------------

/*! Add the SceneObject to the scene.
    @param sceneObject The SceneObject to add to the scene.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, add, ConsoleVoid, 3, 3, (sceneObject))
{
    // Find the specified object.
    SceneObject* pSceneObject = dynamic_cast<SceneObject*>(Sim::findObject(argv[2]));

    // Did we find the object?
    if ( !pSceneObject )
    {
        // No, so warn.
        Con::warnf("t2dScene::addToScene() - Could not find the specified object '%s'.", argv[2]);
        return;
    }

    // Add to t2dScene.
    object->addToScene( pSceneObject );
}

//-----------------------------------------------------------------------------

/*! Remove the SceneObject from the scene.
    @param sceneObject The SceneObject to remove from the scene.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, remove, ConsoleVoid, 3, 3, (sceneObject))
{
    // Find the specified object.
    SceneObject* pSceneObject = dynamic_cast<SceneObject*>(Sim::findObject(argv[2]));

    // Did we find the object?
    if ( !pSceneObject )
    {
        // No, so warn.
        Con::warnf("t2dScene::removeFromScene() - Could not find the specified object '%s'.", argv[2]);
        return;
    }


    // Remove from t2dScene.
    object->removeFromScene( pSceneObject );
} 

//-----------------------------------------------------------------------------

/*! Clear the scene of all scene objects.
    @param deleteObjects A boolean flag that sets whether to delete the objects as well as remove them from the scene (default is true).
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, clear, ConsoleVoid, 2, 3, ([deleteObjects]))
{
    // Calculate 'Delete Objects' flag.
    bool deleteObjects;
    if ( argc >= 3 )
        deleteObjects = dAtob( argv[2] );
    else
        deleteObjects = true;

    // Clear t2dScene.
    object->clearScene( deleteObjects );
}

//-----------------------------------------------------------------------------

/*! Gets the count of scene objects in the scnee.
    @return Returns the number of scene objects in current scene as an integer.
*/
ConsoleMethodWithDocs(t2dScene, getCount, ConsoleInt, 2, 2, ())
{
    // Get t2dScene Object-Count.
    return (S32)object->getSceneObjectCount();
}  


//-----------------------------------------------------------------------------

/*! Gets the scene object at the selected index.
    @param sceneObjectIndex The index of the desired object
    @return The scene object at the specified index.
*/
ConsoleMethodWithDocs(t2dScene, getObject, ConsoleInt, 3, 3, (sceneObjectIndex))
{
    // Fetch Object Index.
    const U32 objectIndex = dAtoi(argv[2]);

    // Fetch scene object count.
    const size_t sceneObjectCount = object->getSceneObjectCount();

    // Sanity!
    if ( objectIndex >= sceneObjectCount )
    {
        // Error so warn.
        Con::warnf("t2dScene::getObject() - Cannot retrieve specified object index (%d) as there are only (%d) object(s) in the scene!", objectIndex, sceneObjectCount );
        // Return no object.
        return 0;
    }

    // Fetch t2dScene Object.
    const SceneObject* pSceneObject = object->getSceneObject( objectIndex );

    // Check Object.
    if ( pSceneObject != nullptr )
    {
        // No error so return object id.
        return pSceneObject->getId();
    }
    else
    {
        // Error so warn.
        Con::warnf("t2dScene::getObject() - Cannot retrieve specified object index (%d)!", objectIndex);
        // Return no object.
        return 0;
    }
}

//-----------------------------------------------------------------------------

/*! Gets the Scene Object-List.
    @return Returns a string with a list of object IDs
*/
ConsoleMethodWithDocs(t2dScene, getSceneObjectList, ConsoleString, 2, 2, ())
{
    // t2dScene Object-List.
    Vector<SceneObject*> objList;

    // Finish here if there are no scene objects.
    size_t objCount = object->getSceneObjects( objList );
    if( objCount == 0 )
        return nullptr;

    // Our return buffer will be 6 times the size of our object list (4 for Id (+1 for future size?) + 1 for space).
    size_t maxBufferSize = objCount * 12;

    // Create Returnable Buffer.
    char *pBuffer = Con::getReturnBuffer( maxBufferSize ); 

    // Set Buffer Counter.
    U32 bufferCount = 0;

    // Iterate through the list and generate an id string list to return
    for ( S32 n = 0; n < objList.size(); n++ )
    {
        // Output Object ID.
        bufferCount += dSprintf( pBuffer + bufferCount, maxBufferSize-bufferCount, "%d ", objList[n]->getId() );

        // Finish early if we run out of buffer space.
        if ( bufferCount >= maxBufferSize )
        {
            // Warn.
            Con::warnf("t2dScene::getSceneObjectList() - Not enough space to return all %d objects!", objList.size());
            break;
        }
    }

    // Return buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Gets the number of assets set to preload for this scene.
    @return The number of assets set to preload for this scene.
*/
ConsoleMethodWithDocs(t2dScene, getAssetPreloadCount, ConsoleInt, 2, 2, ())
{
    return (S32)object->getAssetPreloadCount();
}

//-----------------------------------------------------------------------------

/*! Gets the asset to be preloaded at the specified index.
    @param index The index of the preloaded asset.
    @return The asset to be preloaded at the specified index.
*/
ConsoleMethodWithDocs(t2dScene, getAssetPreload, ConsoleString, 3, 3, (index))
{
    // Fetch preload index.
    const S32 index = dAtoi(argv[2]);

    // Fetch the asset pointer.
    const AssetPtr<AssetBase>* pAssetPtr = object->getAssetPreload( index );

    return pAssetPtr == nullptr ? nullptr : pAssetPtr->getAssetId();
}

//-----------------------------------------------------------------------------

/*! Adds the asset Id so that it is preloaded when the scene is loaded.
    The asset loaded immediately by this operation.  Duplicate assets are ignored.
    @param assetId The asset Id to be added.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, addAssetPreload, ConsoleVoid, 3, 3, (assetId))
{
    // Fetch asset Id.
    const char* pAssetId = argv[2];

    // Add asset preload.
    object->addAssetPreload( pAssetId );
}

//-----------------------------------------------------------------------------

/*! Removes the asset Id from being preloaded when the scene is loaded.
    The asset may be unloaded immediately by this operation if it has no other references.
    @param assetId The asset Id to be removed.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, removeAssetPreload, ConsoleVoid, 3, 3, (assetId))
{
    // Fetch asset Id.
    const char* pAssetId = argv[2];

    // Remove asset preload.
    object->removeAssetPreload( pAssetId );
}

//-----------------------------------------------------------------------------

/*! Clears all assets added as a preload.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, clearAssetPreloads, ConsoleVoid, 2, 2, ())
{
    // Clear asset preloads.
    object->clearAssetPreloads();
}

//-----------------------------------------------------------------------------

/*! Merges the specified scene into this scene by cloning the scenes contents.
*/
ConsoleMethodWithDocs(t2dScene, mergeScene, ConsoleVoid, 3, 3, (scene))
{
    // Find the specified scene.
    t2dScene * pScene = Sim::findObject<t2dScene>( argv[2] );

    // Did we find the scene?
    if ( pScene == nullptr )
    {
        // No, so warn.
        Con::warnf( "t2dScene::mergeScene() - Could not find the specified scene '%s'.", argv[2] );
        return;
    }

    object->mergeScene( pScene );
}

//-----------------------------------------------------------------------------

/*! Gets the Scene Controllers.
    @return Gets the scene controllers.
*/
ConsoleMethodWithDocs(t2dScene, getControllers, ConsoleString, 2, 2, ())
{
    // Fetch the scene controllers.
    SimSet* pControllerSet = object->getControllers();

    return ( pControllerSet == nullptr ) ? StringTable->EmptyString : pControllerSet->getIdString();
}

//-----------------------------------------------------------------------------

/*! Gets the Scene Time.
    @return Returns the time as a floating point number
*/
ConsoleMethodWithDocs(t2dScene, getSceneTime, ConsoleFloat, 2, 2, ())
{
    // Get t2dScene Time.
    return object->getSceneTime();
}   

//-----------------------------------------------------------------------------

/*! Sets scene pause status.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setScenePause, ConsoleVoid, 3, 3, (status))
{
    // Set t2dScene Pause.
    object->setScenePause( dAtob(argv[2]) );
}

//-----------------------------------------------------------------------------

/*! Gets scene pause status.
    @return Returns a boolean value. True if pause status, false otherwise.
*/
ConsoleMethodWithDocs(t2dScene, getScenePause, ConsoleBool, 2, 2, ())
{
    // Get t2dScene Pause.
    return object->getScenePause();
}

//-----------------------------------------------------------------------------

/*! Gets the joint count.
    @return Returns no value
*/
ConsoleMethodWithDocs(t2dScene, getJointCount, ConsoleInt, 2, 2, ())
{
    return (S32)object->getJointCount();
}

//-----------------------------------------------------------------------------

/*! Gets whether the joint Id is valid or not.
    @param jointId The Id of the joint.
    @return whether the joint Id is valid or not.
*/
ConsoleMethodWithDocs(t2dScene, isJoint, ConsoleBool, 3, 3, (int jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi( argv[2] );

    return object->findJoint( jointId ) != nullptr;
}                                                                  


//-----------------------------------------------------------------------------

/*! Gets the joint type of the specified joint Id.
    @param jointId The Id of the joint.
    @return The type of joint of the specified joint Id.
*/
ConsoleMethodWithDocs(t2dScene, getJointType, ConsoleString, 3, 3, (int jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi( argv[2] );

    // Fetch joint type.
    const b2JointType jointType = object->getJointType( jointId );

    // Ignore if invalid joint.
    if ( jointType == e_unknownJoint )
        return StringTable->EmptyString;

    return t2dScene::getJointTypeDescription( jointType ).c_str();
}                                                                  

//-----------------------------------------------------------------------------

/*! Deletes the specified joint Id.
    @param jointId The Id of the joint.
    @return Whether the joint was successfully deleted or not.
*/
ConsoleMethodWithDocs(t2dScene, deleteJoint, ConsoleBool, 3, 3, (int jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi( argv[2] );

    return object->deleteJoint( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a distance joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param distance The distance the joint should maintain between scene objects.  The default is the distance currently between the scene objects.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness (default).
    @param dampingRatio The damping ratio. 0 = no damping (default), 1 = critical damping.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createDistanceJoint, ConsoleInt, 4, 12, (sceneObjectA, sceneObjectB, [localAnchorA X/Y], [localAnchorB X/Y], [distance], [frequency], [dampingRatio], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createDistanceJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if (*sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createDistanceJoint() - Could not find scene object %d.", sceneObjectB);
    }

    if ( argc == 4 )
    {
        return object->createDistanceJoint( pSceneObjectA, pSceneObjectB );
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createDistanceJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA );
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createDistanceJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB );
    }

    // Fetch length.
    const F32 length = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, length );
    }

    // Fetch frequency (Hertz).
    const F32 frequency = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, length, frequency );
    }

    // Fetch damping ratio.
    const F32 dampingRatio = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, length, frequency, dampingRatio );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createDistanceJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, length, frequency, dampingRatio, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets the distance the joint should maintain between scene objects.
    @param jointId The Id of the joint to use.
    @param length The length the joint should maintain between scene objects.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setDistanceJointLength, ConsoleVoid, 4, 4, (jointId, length))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 length = dAtof(argv[3]);

    // Access joint.
    object->setDistanceJointLength( jointId, length );
}

//-----------------------------------------------------------------------------

/*! Gets the distance the joint should maintain between scene objects.
    @param jointId The Id of the joint to use.
    @return Returns the distance the joint should maintain between scene objects (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getDistanceJointLength, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getDistanceJointLength( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setDistanceJointFrequency, ConsoleVoid, 4, 4, (jointId, frequency))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 frequency = dAtof(argv[3]);

    // Access joint.
    object->setDistanceJointFrequency( jointId, frequency );
}

//-----------------------------------------------------------------------------

/*! Gets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @return Returns the mass-spring-damper frequency in Hertz (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getDistanceJointFrequency, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getDistanceJointFrequency( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the damping ratio.
    @param jointId The Id of the joint to use.
    @param dampingRatio The damping ratio. 0 = no damping, 1 = critical damping.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setDistanceJointDampingRatio, ConsoleVoid, 4, 4, (jointId, dampingRatio))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 dampingRatio = dAtof(argv[3]);

    // Access joint.
    object->setDistanceJointDampingRatio( jointId, dampingRatio );
}

//-----------------------------------------------------------------------------

/*! Gets the damping ratio.
    @param jointId The Id of the joint to use.
    @return Returns the damping ratio (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getDistanceJointDampingRatio, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getDistanceJointDampingRatio( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a rope joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param maxLength The maximum rigid length of the rope.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createRopeJoint, ConsoleInt, 4, 10, (sceneObjectA, sceneObjectB, [localAnchorA X/Y], [localAnchorB X/Y], [maxLength], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createRopeJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if (*sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createRopeJoint() - Could not find scene object %d.", sceneObjectB);
    }

    if ( argc == 4 )
    {
        return object->createRopeJoint( pSceneObjectA, pSceneObjectB );
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createRopeJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createRopeJoint( pSceneObjectA, pSceneObjectB, localAnchorA );
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createRopeJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createRopeJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB );
    }

    // Fetch maximum length.
    const F32 maxLength = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createRopeJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxLength );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createRopeJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxLength, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets the maximum rigid length of the rope.
    @param jointId The Id of the joint to use.
    @param maxLength The maximum rigid length of the rope.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setRopeJointMaxLength, ConsoleVoid, 4, 4, (jointId, maxLength))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 maxLength = dAtof(argv[3]);

    // Access joint.
    object->setRopeJointMaxLength( jointId, maxLength );
}

//-----------------------------------------------------------------------------

/*! Gets the maximum rigid length of the rope.
    @param jointId The Id of the joint to use.
    @return Returns the maximum rigid length of the rope (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getRopeJointMaxLength, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getRopeJointMaxLength( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a revolute joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createRevoluteJoint, ConsoleInt, 4, 9, (sceneObjectA, sceneObjectB, [localAnchorA X/Y], [localAnchorB X/Y], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createRevoluteJoint() - Could not find scene object %s.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createRevoluteJoint() - Could not find scene object %s.", sceneObjectB);
    }

    if ( sceneObjectA == sceneObjectB )
    {
       Con::warnf("t2dScene::createRevoluteJoint() - Cannot join scene object %s %s to itself!", sceneObjectA, sceneObjectB);
       return -1;
    }
   
   
    if ( argc == 4 )
    {
        return object->createRevoluteJoint( pSceneObjectA, pSceneObjectB );
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createRevoluteJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createRevoluteJoint( pSceneObjectA, pSceneObjectB, localAnchorA );
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createRevoluteJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createRevoluteJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createRevoluteJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets whether the joint has angular limits or not and the limits themselves.
    @param jointId The Id of the joint to use.
    @param enableLimit Whether the joint has angular limits or not.
    @param lowerAngle The lower angle of the angular limit.
    @param upperAngle The upper angle of the angular limit.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setRevoluteJointLimit, ConsoleVoid, 4, 6, (jointId, enableLimit, [lowerAngle], [upperAngle]))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const bool enableLimit = dAtob(argv[3]);
    const F32 lowerAngle = argc > 4 ? mDegToRad( dAtof(argv[4]) ) : 0.0f;
    const F32 upperAngle = argc > 5 ? mDegToRad( dAtof(argv[5]) ) : lowerAngle;

    // Access joint.
    object->setRevoluteJointLimit( jointId, enableLimit, lowerAngle, upperAngle );
}

//-----------------------------------------------------------------------------

/*! Gets whether the joint has angular limits or not and the limits themselves.
    @param jointId The Id of the joint to use.
    @return Returns whether the joint has angular limits or not and the limits themselves (empty string indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getRevoluteJointLimit, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Args.
    bool enableLimit;
    F32 lowerAngle;
    F32 upperAngle;

    // Access joint.
    if ( !object->getRevoluteJointLimit( jointId, enableLimit, lowerAngle, upperAngle ) )
    {
        return nullptr;
    }

    // Format output.
    char* pBuffer = Con::getReturnBuffer(64);
    dSprintf( pBuffer, 64, "%d %g %g", enableLimit, mRadToDeg(lowerAngle), mRadToDeg(upperAngle) );
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Sets whether the joint has a motor or not and the motor settings.
    @param jointId The Id of the joint to use.
    @param enableMotor Whether the joint has a motor or not.
    @param motorSpeed The motor speed (degrees per/sec).
    @param maxMotorTorque The maximum motor torque used to achieve the specified motor speed (N-m).
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setRevoluteJointMotor, ConsoleVoid, 4, 6, (jointId, enableMotor, [motorSpeed], [maxMotorTorque]))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const bool enableMotor = dAtob(argv[3]);
    const F32 motorSpeed = argc > 4 ? mDegToRad( dAtof(argv[4]) ) : 0.0f;
    const F32 maxMotorTorque = argc > 5 ? dAtof(argv[5]) : 0.0f;

    // Access joint.
    object->setRevoluteJointMotor( jointId, enableMotor, motorSpeed, maxMotorTorque );
}

//-----------------------------------------------------------------------------

/*! Gets whether the joint has a motor or not and the motor settings.
    @param jointId The Id of the joint to use.
    @return Returns whether the joint has a motor or not and the motor settings (empty string indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getRevoluteJointMotor, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Args.
    bool enableMotor;
    F32 motorSpeed;
    F32 maxMotorTorque;

    // Access joint.
    if ( !object->getRevoluteJointMotor( jointId, enableMotor, motorSpeed, maxMotorTorque ) )
    {
        return nullptr;
    }

    // Format output.
    char* pBuffer = Con::getReturnBuffer(64);
    dSprintf( pBuffer, 64, "%d %g %g", enableMotor, mRadToDeg(motorSpeed), maxMotorTorque );
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Gets the current angle of a revolute joint.
    @param jointId The Id of the joint to use.
    @return Returns the joint angle.
*/
ConsoleMethodWithDocs(t2dScene, getRevoluteJointAngle, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
	return object->getRevoluteJointAngle( jointId );
}

//-----------------------------------------------------------------------------

/*! Gets the current speed of a revolute joint.
    @param jointId The Id of the joint to use.
    @return Returns the joint speed as Angular Velocity
*/
ConsoleMethodWithDocs(t2dScene, getRevoluteJointSpeed, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
	return object->getRevoluteJointSpeed( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a weld joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness (default).
    @param dampingRatio The damping ratio. 0 = no damping (default), 1 = critical damping.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createWeldJoint, ConsoleInt, 4, 11, (sceneObjectA, sceneObjectB, [localAnchorA X/Y], [localAnchorB X/Y], [frequency], [dampingRatio], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createWeldJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createWeldJoint() - Could not find scene object %d.", sceneObjectB);
    }

    if ( argc == 4 )
    {
        return object->createWeldJoint( pSceneObjectA, pSceneObjectB );
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createWeldJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createWeldJoint( pSceneObjectA, pSceneObjectB, localAnchorA );
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createWeldJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createWeldJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB );
    }

    // Fetch frequency (Hertz).
    const F32 frequency = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createWeldJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, frequency );
    }

    // Fetch damping ratio.
    const F32 dampingRatio = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createWeldJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, frequency, dampingRatio );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createWeldJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, frequency, dampingRatio, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setWeldJointFrequency, ConsoleVoid, 4, 4, (jointId, frequency))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 frequency = dAtof(argv[3]);

    // Access joint.
    object->setWeldJointFrequency( jointId, frequency );
}

//-----------------------------------------------------------------------------

/*! Gets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @return Returns the mass-spring-damper frequency in Hertz (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getWeldJointFrequency, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getWeldJointFrequency( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the damping ratio.
    @param jointId The Id of the joint to use.
    @param dampingRatio The damping ratio. 0 = no damping, 1 = critical damping.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setWeldJointDampingRatio, ConsoleVoid, 4, 4, (jointId, dampingRatio))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 dampingRatio = dAtof(argv[3]);

    // Access joint.
    object->setWeldJointDampingRatio( jointId, dampingRatio );
}

//-----------------------------------------------------------------------------

/*! Gets the damping ratio.
    @param jointId The Id of the joint to use.
    @return Returns the damping ratio (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getWeldJointDampingRatio, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getWeldJointDampingRatio( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a wheel joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param worldAxis The world axis of the wheel suspension spring.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createWheelJoint, ConsoleInt, 7, 11, (sceneObjectA, sceneObjectB, localAnchorA X/Y, localAnchorB X/Y, worldAxis X/Y, [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createWheelJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createWheelJoint() - Could not find scene object %d.", sceneObjectB);
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createWheelJoint() - Invalid number of parameters!");
        return -1;
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createWheelJoint() - Invalid number of parameters!");
        return -1;
    }

    // World axis.
    const U32 worldAxisElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 worldAxis;

    if ( worldAxisElementCount == 1 && argc > (nextArg+1) )
    {
        worldAxis.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( worldAxisElementCount == 2 )
    {
        worldAxis = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createWheelJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createWheelJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldAxis );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createWheelJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldAxis, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets whether the joint has a motor or not and the motor settings.
    @param jointId The Id of the joint to use.
    @param enableMotor Whether the joint has a motor or not.
    @param motorSpeed The motor speed (degrees per/sec).
    @param maxMotorTorque The maximum motor torque used to achieve the specified motor speed (N-m).
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setWheelJointMotor, ConsoleVoid, 4, 6, (jointId, enableMotor, [motorSpeed], [maxMotorTorque]))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const bool enableMotor = dAtob(argv[3]);
    const F32 motorSpeed = argc > 4 ? mDegToRad( dAtof(argv[4]) ) : 0.0f;
    const F32 maxMotorTorque = argc > 5 ? dAtof(argv[5]) : 0.0f;

    // Access joint.
    object->setWheelJointMotor( jointId, enableMotor, motorSpeed, maxMotorTorque );
}

//-----------------------------------------------------------------------------

/*! Gets whether the joint has a motor or not and the motor settings.
    @param jointId The Id of the joint to use.
    @return Returns whether the joint has a motor or not and the motor settings (empty string indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getWheelJointMotor, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Args.
    bool enableMotor;
    F32 motorSpeed;
    F32 maxMotorTorque;

    // Access joint.
    if ( !object->getWheelJointMotor( jointId, enableMotor, motorSpeed, maxMotorTorque ) )
    {
        return nullptr;
    }

    // Format output.
    char* pBuffer = Con::getReturnBuffer(64);
    dSprintf( pBuffer, 64, "%d %g %g", enableMotor, mRadToDeg(motorSpeed), maxMotorTorque );
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Sets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setWheelJointFrequency, ConsoleVoid, 4, 4, (jointId, frequency))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 frequency = dAtof(argv[3]);

    // Access joint.
    object->setWheelJointFrequency( jointId, frequency );
}

//-----------------------------------------------------------------------------

/*! Gets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @return Returns the mass-spring-damper frequency in Hertz (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getWheelJointFrequency, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getWheelJointFrequency( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the damping ratio.
    @param jointId The Id of the joint to use.
    @param dampingRatio The damping ratio. 0 = no damping, 1 = critical damping.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setWheelJointDampingRatio, ConsoleVoid, 4, 4, (jointId, dampingRatio))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 dampingRatio = dAtof(argv[3]);

    // Access joint.
    object->setWheelJointDampingRatio( jointId, dampingRatio );
}

//-----------------------------------------------------------------------------

/*! Gets the damping ratio.
    @param jointId The Id of the joint to use.
    @return Returns the damping ratio (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getWheelJointDampingRatio, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getWheelJointDampingRatio( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a friction joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param maxForce The maximum friction force (N).
    @param maxTorque The maximum torque force (N-m).
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createFrictionJoint, ConsoleInt, 4, 11, (sceneObjectA, sceneObjectB, [localAnchorA X/Y], [localAnchorB X/Y], [maxForce], [maxTorque], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createFrictionJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createFrictionJoint() - Could not find scene object %d.", sceneObjectB);
    }

    if ( argc == 4 )
    {
        return object->createFrictionJoint( pSceneObjectA, pSceneObjectB );
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createFrictionJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createFrictionJoint( pSceneObjectA, pSceneObjectB, localAnchorA );
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createFrictionJoint() - Invalid number of parameters!");
        return -1;
    }

    // Fetch maximum force.
    const F32 maxForce = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createFrictionJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxForce );
    }

    // Fetch maximum torque.
    const F32 maxTorque = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createFrictionJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxForce, maxTorque );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createFrictionJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, maxForce, maxTorque, collideConnected );
}

//-----------------------------------------------------------------------------

ConsoleMethod(t2dScene, setFrictionJointMaxForce, void, 4, 4,  "(jointId, maxForce) Sets the maximum friction force.\n"
                                                                    "@param jointId The Id of the joint to use.\n"
                                                                    "@param maxForce The maximum friction force (N).\n"
                                                                    "@return Returns no value." )
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 maxForce = dAtof(argv[3]);

    // Access joint.
    object->setFrictionJointMaxForce( jointId, maxForce );
}

//-----------------------------------------------------------------------------

/*! Sets the maximum friction force.
    @param jointId The Id of the joint to use.
    @return Returns the maximum friction force (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getFrictionJointMaxForce, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getFrictionJointMaxForce( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the maximum torque force.
    @param jointId The Id of the joint to use.
    @param maxTorque The maximum torque force (N).
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setFrictionJointMaxTorque, ConsoleVoid, 4, 4, (jointId, maxTorque))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 maxTorque = dAtof(argv[3]);

    // Access joint.
    object->setFrictionJointMaxTorque( jointId, maxTorque );
}

//-----------------------------------------------------------------------------

/*! Gets the maximum torque force.
    @param jointId The Id of the joint to use.
    @return Returns the maximum torque force (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getFrictionJointMaxTorque, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getFrictionJointMaxTorque( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a prismatic joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param worldAxis The world axis defining the translational degree of freedom.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createPrismaticJoint, ConsoleInt, 7, 11, (sceneObjectA, sceneObjectB, localAnchorA X/Y, localAnchorB X/Y, worldAxis X/Y, [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createPrismaticJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createPrismaticJoint() - Could not find scene object %d.", sceneObjectB);
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPrismaticJoint() - Invalid number of parameters!");
        return -1;
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPrismaticJoint() - Invalid number of parameters!");
        return -1;
    }

    // World axis.
    const U32 worldAxisElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 worldAxis;

    if ( worldAxisElementCount == 1 && argc > (nextArg+1) )
    {
        worldAxis.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( worldAxisElementCount == 2 )
    {
        worldAxis = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPrismaticJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createPrismaticJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldAxis );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createPrismaticJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldAxis, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets whether the joint has translational limits or not and the limits themselves.
    @param jointId The Id of the joint to use.
    @param enableLimit Whether the joint has angular limits or not.
    @param lowerTranslation The lower translation limit.
    @param upperTranslation The upper translation limit.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setPrismaticJointLimit, ConsoleVoid, 4, 6, (jointId, enableLimit, [lowerTranslation], [upperTranslation]))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const bool enableLimit = dAtob(argv[3]);
    const F32 lowerTranslation = argc > 4 ? dAtof(argv[4]) : 0.0f;
    const F32 upperTranslation = argc > 5 ? dAtof(argv[5]) : lowerTranslation;

    // Access joint.
    object->setPrismaticJointLimit( jointId, enableLimit, lowerTranslation, upperTranslation );
}

//-----------------------------------------------------------------------------

/*! Gets whether the joint has translational limits or not and the limits themselves.
    @return Returns whether the joint has translational limits or not and the limits themselves (empty string indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getPrismaticJointLimit, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Args.
    bool enableLimit;
    F32 lowerTranslation;
    F32 upperTranslation;

    // Access joint.
    if ( !object->getPrismaticJointLimit( jointId, enableLimit, lowerTranslation, upperTranslation ) )
    {
        return nullptr;
    }

    // Format output.
    char* pBuffer = Con::getReturnBuffer(64);
    dSprintf( pBuffer, 64, "%d %g %g", enableLimit, lowerTranslation, upperTranslation );
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Sets whether the joint has a motor or not and the motor settings.
    @param jointId The Id of the joint to use.
    @param enableMotor Whether the joint has a motor or not.
    @param motorSpeed The motor speed (degrees per/sec).
    @param maxMotorForce The maximum motor force used to achieve the specified motor speed (N-m).
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setPrismaticJointMotor, ConsoleVoid, 4, 6, (jointId, enableMotor, [motorSpeed], [maxMotorForce]))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const bool enableMotor = dAtob(argv[3]);
    const F32 motorSpeed = argc > 4 ? mDegToRad( dAtof(argv[4]) ) : 0.0f;
    const F32 maxMotorForce = argc > 5 ? dAtof(argv[5]) : 0.0f;

    // Access joint.
    object->setPrismaticJointMotor( jointId, enableMotor, motorSpeed, maxMotorForce );
}

//-----------------------------------------------------------------------------

/*! Gets whether the joint has a motor or not and the motor settings.
    @return Returns whether the joint has a motor or not and the motor settings (empty string indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getPrismaticJointMotor, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Args.
    bool enableMotor;
    F32 motorSpeed;
    F32 maxMotorForce;

    // Access joint.
    if ( !object->getPrismaticJointMotor( jointId, enableMotor, motorSpeed, maxMotorForce ) )
    {
        return nullptr;
    }

    // Format output.
    char* pBuffer = Con::getReturnBuffer(64);
    dSprintf( pBuffer, 64, "%d %g %g", enableMotor, mRadToDeg(motorSpeed), maxMotorForce );
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Creates a prismatic joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param localAnchorA The local point of the first scene object where the joint connects.
    @param localAnchorB The local point of the second scene object where the joint connects.
    @param worldGroundAnchorA The world point of the first ground anchor.  This point never moves.
    @param worldGroundAnchorB The world point of the second ground anchor.  This point never moves.
    @param ratio The pulley ratio used to simulate a block-and-tackle pulley.
    @param lengthA The reference pulley length for the segment attached to scene object A.  Defaults to the distance between the first scene object and the first ground anchor.
    @param lengthB The reference pulley length for the segment attached to scene object B.  Defaults to the distance between the second scene object and the second ground anchor.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createPulleyJoint, ConsoleInt, 9, 16, (sceneObjectA, sceneObjectB, localAnchorA X/Y, localAnchorB X/Y, worldGroundAnchorA X/Y, worldGroundAnchorB X/Y, ratio, [lengthA], [lengthB], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createPulleyJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createPulleyJoint() - Could not find scene object %d.", sceneObjectB);
    }

    // Local anchor A.
    const U32 anchorAElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 localAnchorA;

    S32 nextArg = 5;
    if ( anchorAElementCount == 1 && argc > 5 )
    {
        localAnchorA.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( anchorAElementCount == 2 )
    {
        localAnchorA = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPulleyJoint() - Invalid number of parameters!");
        return -1;
    }

    // Local anchor B.
    const U32 anchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 localAnchorB;

    if ( anchorBElementCount == 1 && argc > (nextArg+1) )
    {
        localAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( anchorBElementCount == 2 )
    {
        localAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPulleyJoint() - Invalid number of parameters!");
        return -1;
    }

    // World ground anchor A.
    const U32 worldGroundAnchorAElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 worldGroundAnchorA;

    if ( worldGroundAnchorAElementCount == 1 && argc > 5 )
    {
        worldGroundAnchorA.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( worldGroundAnchorAElementCount == 2 )
    {
        worldGroundAnchorA = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPulleyJoint() - Invalid number of parameters!");
        return -1;
    }

    // World ground anchor B.
    const U32 worldGroundAnchorBElementCount = Utility::mGetStringElementCount(argv[nextArg]);

    b2Vec2 worldGroundAnchorB;

    if ( worldGroundAnchorBElementCount == 1 && argc > (nextArg+1) )
    {
        worldGroundAnchorB.Set( dAtof(argv[nextArg]), dAtof(argv[nextArg+1]) );
        nextArg += 2;
    }
    else if ( worldGroundAnchorBElementCount == 2 )
    {
        worldGroundAnchorB = Utility::mGetStringElementVector(argv[nextArg++]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createPulleyJoint() - Invalid number of parameters!");
        return -1;
    }

    // Fetch maximum ratio.
    const F32 ratio = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createPulleyJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldGroundAnchorA, worldGroundAnchorB, ratio );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createPulleyJoint( pSceneObjectA, pSceneObjectB, localAnchorA, localAnchorB, worldGroundAnchorA, worldGroundAnchorB, ratio, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Creates a target joint.
    @param sceneObject The scene object to connect to the joint.
    @param worldTarget The world point target to move the scene object to.
    @param maxForce The maximum force the joint should use to position the scene object at the target.
    @param useCenterOfMass Whether to use the center of mass as the point which the joint is attached or not.  Defaults to false.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness (default=0.7).
    @param dampingRatio The damping ratio. 0 = no damping (default), 1 = critical damping.
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createTargetJoint, ConsoleInt, 5, 10, (sceneObject, worldTarget X/Y, maxForce, [useCenterOfMass?], [frequency], [dampingRatio], [collideConnected]))
{
    // Fetch scene object.
    SceneObject* pSceneObject = Sim::findObject<SceneObject>(argv[2]);

    // Check scene object.
    if ( !pSceneObject )
    {
        Con::warnf("t2dScene::createTargetJoint() - Could not find scene object %d.", argv[2]);
        return -1;
    }

    // World target.
    const U32 worldTargetElementCount = Utility::mGetStringElementCount(argv[3]);

    b2Vec2 worldTarget;

    S32 nextArg = 4;
    if ( worldTargetElementCount == 1 && argc > 5 )
    {
        worldTarget.Set( dAtof(argv[3]), dAtof(argv[4]) );
        nextArg = 5;
    }
    else if ( worldTargetElementCount == 2 )
    {
        worldTarget = Utility::mGetStringElementVector(argv[3]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createTargetJoint() - Invalid number of parameters!");
        return -1;
    }

    // Fetch maximum force.
    const F32 maxForce = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createTargetJoint( pSceneObject, worldTarget, maxForce );
    }

    // Fetch the center-of-mass flag.
    const bool centerOfMass = dAtob(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createTargetJoint( pSceneObject, worldTarget, maxForce, centerOfMass );
    }

    // Fetch frequency (Hertz).
    const F32 frequency = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createTargetJoint( pSceneObject, worldTarget, maxForce, centerOfMass, frequency );
    }

    // Fetch damping ratio.
    const F32 dampingRatio = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createTargetJoint( pSceneObject, worldTarget, maxForce, centerOfMass, frequency, dampingRatio );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createTargetJoint( pSceneObject, worldTarget, maxForce, centerOfMass, frequency, dampingRatio, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets the target world point for the scene object.
    @param jointId The Id of the joint to use.
    @param worldTarget The target world point to move the scene object to.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setTargetJointTarget, ConsoleVoid, 4, 5, (jointId, worldTarget X/Y))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // World target.
    const U32 worldTargetElementCount = Utility::mGetStringElementCount(argv[3]);

    b2Vec2 worldTarget;

    if ( worldTargetElementCount == 1 && argc > 5 )
    {
        worldTarget.Set( dAtof(argv[3]), dAtof(argv[4]) );
    }
    else if ( worldTargetElementCount == 2 )
    {
        worldTarget = Utility::mGetStringElementVector(argv[3]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::setTargetJointTarget() - Invalid number of parameters!");
    }

    // Access joint.
    object->setTargetJointTarget( jointId, worldTarget );
}

//-----------------------------------------------------------------------------

/*! Gets the target world point for the scene object.
    @param jointId The Id of the joint to use.
    @return Returns the target world point for the scene object (always 0,0 if error).
*/
ConsoleMethodWithDocs(t2dScene, getTargetJointTarget, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    const Vector2 worldTarget = object->getTargetJointTarget( jointId );

    return worldTarget.scriptThis();
}

//-----------------------------------------------------------------------------

/*! Sets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @param frequency The mass-spring-damper frequency in Hertz. A value of 0 disables softness.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setTargetJointFrequency, ConsoleVoid, 4, 4, (jointId, frequency))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 frequency = dAtof(argv[3]);

    // Access joint.
    object->setTargetJointFrequency( jointId, frequency );
}

//-----------------------------------------------------------------------------

/*! Gets the mass-spring-damper frequency in Hertz.
    @param jointId The Id of the joint to use.
    @return Returns the mass-spring-damper frequency in Hertz (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getTargetJointFrequency, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getTargetJointFrequency( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the damping ratio.
    @param jointId The Id of the joint to use.
    @param dampingRatio The damping ratio. 0 = no damping, 1 = critical damping.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setTargetJointDampingRatio, ConsoleVoid, 4, 4, (jointId, dampingRatio))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 dampingRatio = dAtof(argv[3]);

    // Access joint.
    object->setTargetJointDampingRatio( jointId, dampingRatio );
}

//-----------------------------------------------------------------------------

/*! Sets the damping ratio.
    @param jointId The Id of the joint to use.
    @return Returns the damping ratio (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getTargetJointDampingRatio, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getTargetJointDampingRatio( jointId );
}

//-----------------------------------------------------------------------------

/*! Creates a motor joint.
    @param sceneObjectA The first scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param sceneObjectB The second scene object to connect to the joint.  Use an empty string to indicate the Scene ground body.
    @param linearOffsetX/Y The linear offset in sceneObjectA space.
    @param angularOffset The angularOffset between the bodies.
    @param maxForce The maximum friction force (N).
    @param maxTorque The maximum torque force (N-m).
    @param correctionFactor The correction factor (tolerance).
    @param collideConnected Whether the scene objects can collide with each other while connected with this joint.
    @return The joint Id (-1 if error).
*/
ConsoleMethodWithDocs(t2dScene, createMotorJoint, ConsoleInt, 4, 11, (sceneObjectA, sceneObjectB, [linearOffset X/Y], [angularOffset], [maxForce], [maxTorque], [correctionFactor], [collideConnected]))
{
    // Fetch scene object references.
    const char* sceneObjectA = argv[2];
    const char* sceneObjectB = argv[3];

    SceneObject* pSceneObjectA = nullptr;
    SceneObject* pSceneObjectB = nullptr;

    // Fetch scene object.
    if ( *sceneObjectA != 0 )
    {
        pSceneObjectA = Sim::findObject<SceneObject>(sceneObjectA);

        if ( !pSceneObjectA )
            Con::warnf("t2dScene::createMotorJoint() - Could not find scene object %d.", sceneObjectA);
    }

    // Fetch scene object.
    if ( *sceneObjectB != 0 )
    {
        pSceneObjectB = Sim::findObject<SceneObject>(sceneObjectB);

        if ( !pSceneObjectB )
            Con::warnf("t2dScene::createMotorJoint() - Could not find scene object %d.", sceneObjectB);
    }

    if ( argc == 4 )
    {
        return object->createMotorJoint( pSceneObjectA, pSceneObjectB );
    }

    // Linear offset.
    const U32 linearOffsetElementCount = Utility::mGetStringElementCount(argv[4]);

    b2Vec2 linearOffset;

    S32 nextArg = 5;
    if ( linearOffsetElementCount == 1 && argc > 5 )
    {
        linearOffset.Set( dAtof(argv[4]), dAtof(argv[5]) );
        nextArg = 6;
    }
    else if ( linearOffsetElementCount == 2 )
    {
        linearOffset = Utility::mGetStringElementVector(argv[4]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::createMotorJoint() - Invalid number of parameters!");
        return -1;
    }

    if ( argc <= nextArg )
    {
        return object->createMotorJoint( pSceneObjectA, pSceneObjectB, linearOffset );
    }

    // Fetch angular offset.
    const F32 angularOffset = mDegToRad( dAtof(argv[nextArg++]) );

    if ( argc <= nextArg )
    {
        return object->createMotorJoint( pSceneObjectA, pSceneObjectB, linearOffset, angularOffset );
    }

    // Fetch maximum force.
    const F32 maxForce = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createMotorJoint( pSceneObjectA, pSceneObjectB, linearOffset, angularOffset, maxForce );
    }

    // Fetch maximum torque.
    const F32 maxTorque = dAtof(argv[nextArg++]);

    if ( argc <= nextArg )
    {
        return object->createMotorJoint( pSceneObjectA, pSceneObjectB, linearOffset, angularOffset, maxForce, maxTorque );
    }

    // Fetch collide connected.
    const bool collideConnected = dAtob(argv[nextArg++]);

    return object->createMotorJoint( pSceneObjectA, pSceneObjectB, linearOffset, angularOffset, maxForce, maxTorque, collideConnected );
}

//-----------------------------------------------------------------------------

/*! Sets the linear offset in sceneObjectA space.
    @param jointId The Id of the joint to use.
    @param linearOffsetX/Y The linear offset in sceneObjectA space.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setMotorJointLinearOffset, ConsoleVoid, 4, 5, (jointId, linearOffset X/Y))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Linear offset.
    const U32 linearOffsetElementCount = Utility::mGetStringElementCount(argv[3]);

    b2Vec2 linearOffset;

    if ( linearOffsetElementCount == 1 && argc > 5 )
    {
        linearOffset.Set( dAtof(argv[3]), dAtof(argv[4]) );
    }
    else if ( linearOffsetElementCount == 2 )
    {
        linearOffset = Utility::mGetStringElementVector(argv[3]);
    }
    // Invalid
    else
    {
        Con::warnf("t2dScene::setMotorJointLinearOffset() - Invalid number of parameters!");
    }

    // Access joint.
    object->setMotorJointLinearOffset( jointId, linearOffset );
}

//-----------------------------------------------------------------------------

/*! Gets the linear offset in sceneObjectA space.
    @param jointId The Id of the joint to use.
    @return Returns the linear offset in sceneObjectA space (always 0,0 if error).
*/
ConsoleMethodWithDocs(t2dScene, getMotorJointLinearOffset, ConsoleString, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    const Vector2 linearOffset = object->getMotorJointLinearOffset( jointId );

    return linearOffset.scriptThis();
}

//-----------------------------------------------------------------------------

/*! Sets the angularOffset between the bodies.
    @param jointId The Id of the joint to use.
    @param angularOffset The angularOffset between the bodies.
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setMotorJointAngularOffset, ConsoleVoid, 4, 4, (jointId, angularOffset))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 angularOffset = mDegToRad(dAtof(argv[3]));

    // Access joint.
    object->setMotorJointAngularOffset( jointId, angularOffset );
}

//-----------------------------------------------------------------------------

/*! Gets angularOffset between the bodies.
    @param jointId The Id of the joint to use.
    @return Returns the angularOffset between the bodies (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getMotorJointAngularOffset, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return mRadToDeg( object->getMotorJointAngularOffset( jointId ) );
}

//-----------------------------------------------------------------------------

/*! Sets the maximum motor force.
    @param jointId The Id of the joint to use.
    @param maxForce The maximum motor force (N).
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setMotorJointMaxForce, ConsoleVoid, 4, 4, (jointId, maxForce))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 maxForce = dAtof(argv[3]);

    // Access joint.
    object->setMotorJointMaxForce( jointId, maxForce );
}

//-----------------------------------------------------------------------------

/*! Sets the maximum motor force.
    @param jointId The Id of the joint to use.
    @return Returns the maximum motor force (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getMotorJointMaxForce, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getMotorJointMaxForce( jointId );
}

//-----------------------------------------------------------------------------

/*! Sets the maximum motor torque force.
    @param jointId The Id of the joint to use.
    @param maxTorque The maximum motor torque force (N).
    @return Returns no value.
*/
ConsoleMethodWithDocs(t2dScene, setMotorJointMaxTorque, ConsoleVoid, 4, 4, (jointId, maxTorque))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Fetch args.
    const F32 maxTorque = dAtof(argv[3]);

    // Access joint.
    object->setMotorJointMaxTorque( jointId, maxTorque );
}

//-----------------------------------------------------------------------------

/*! Gets the maximum motor torque force.
    @param jointId The Id of the joint to use.
    @return Returns the maximum motor torque force (-1 indicates error).
*/
ConsoleMethodWithDocs(t2dScene, getMotorJointMaxTorque, ConsoleFloat, 3, 3, (jointId))
{
    // Fetch joint Id.
    const S32 jointId = dAtoi(argv[2]);

    // Access joint.
    return object->getMotorJointMaxTorque( jointId );
}

//-----------------------------------------------------------------------------

/*! Picks objects intersecting the specified area with optional group/layer masks.
    @param startx/y The coordinates of the start point as either (\x y\ or (x,y)
    @param endx/y The coordinates of the end point as either (\x y\ or (x,y)
    @param sceneGroupMask Optional scene group mask.  (-1) or empty string selects all groups.
    @param sceneLayerMask Optional scene layer mask.  (-1) or empty string selects all layers.
    @param pickMode Optional mode 'any', 'aabb', 'oobb' or 'collision' (default is 'oobb').
    @return Returns list of object IDs.
*/
ConsoleMethodWithDocs(t2dScene, pickArea, ConsoleString, 4, 9, (startx/y, endx/y, [sceneGroupMask], [sceneLayerMask], [pickMode] ))
{
    // Upper left and lower right bound.
    Vector2 v1, v2;

    // The index of the first optional parameter.
    U32 firstArg;

    // Grab the number of elements in the first two parameters.
    U32 elementCount1 = Utility::mGetStringElementCount(argv[2]);
    U32 elementCount2 = 1;
    if (argc > 3)
        elementCount2 = Utility::mGetStringElementCount(argv[3]);

    // ("x1 y1 x2 y2")
    if ((elementCount1 == 4) && (argc < 9))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[2], 2);
        firstArg = 3;
    }
   
    // ("x1 y1", "x2 y2")
    else if ((elementCount1 == 2) && (elementCount2 == 2) && (argc > 3) && (argc < 10))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[3]);
        firstArg = 4;
    }
   
    // (x1, y1, x2, y2)
    else if (argc > 5)
    {
        v1 = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        v2 = Vector2(dAtof(argv[4]), dAtof(argv[5]));
        firstArg = 6;
    }
   
    // Invalid
    else
    {
        Con::warnf("t2dScene::pickArea() - Invalid number of parameters!");
        return nullptr;
    }

    // Calculate scene group mask.
    U32 sceneGroupMask = MASK_ALL;
    if ( (U32)argc > firstArg )
    {
        if ( *argv[firstArg] != 0 )
            sceneGroupMask = dAtoi(argv[firstArg]);
    }

//    // Calculate scene layer mask.
//    U32 sceneLayerMask = MASK_ALL;
//    if ( (U32)argc > (firstArg + 1) )
//    {
//        if ( *argv[firstArg + 1] != 0 )
//            sceneLayerMask = dAtoi(argv[firstArg + 1]);
//    }

    // Calculate pick mode.
    t2dScene::PickMode pickMode = t2dScene::PICK_OOBB;
    if ( (U32)argc > (firstArg + 2))
    {
        pickMode = t2dScene::getPickModeEnum(argv[firstArg + 2]);
    }
    if ( pickMode == t2dScene::PICK_INVALID )
    {
        Con::warnf("t2dScene::pickArea() - Invalid pick mode of %s", argv[firstArg + 2]);
        pickMode = t2dScene::PICK_OOBB;
    }


    // Fetch world query and clear results.
    WorldQuery* pWorldQuery = object->getWorldQuery( true );

    // Set filter.
    WorldQueryFilter queryFilter( sceneGroupMask, true, false, true, true );
    pWorldQuery->setQueryFilter( queryFilter );

    // Calculate normalized AABB.
    b2AABB aabb;
    aabb.lowerBound.x = std::min( v1.x, v2.x );
    aabb.lowerBound.y = std::min( v1.y, v2.y );
    aabb.upperBound.x = std::max( v1.x, v2.x );
    aabb.upperBound.y = std::max( v1.y, v2.y );

    // Perform query.
    if ( pickMode == t2dScene::PICK_ANY )
    {
        pWorldQuery->anyQueryAABB( aabb );    
    }
    else if ( pickMode == t2dScene::PICK_AABB )
    {
        pWorldQuery->aabbQueryAABB( aabb );    
    }
    else if ( pickMode == t2dScene::PICK_OOBB )
    {
        pWorldQuery->oobbQueryAABB( aabb );    
    }
    else if ( pickMode == t2dScene::PICK_COLLISION )
    {
        pWorldQuery->collisionQueryAABB( aabb );    
    }
    else
    {
        AssertFatal( false, "Unsupported pick mode." );
    }

    // Fetch result count.
    const size_t resultCount = pWorldQuery->getQueryResultsCount();

    // Finish if no results.
    if ( resultCount == 0 )
        return nullptr;

    // Fetch results.
    typeWorldQueryResultVector& queryResults = pWorldQuery->getQueryResults();

    // Set Max Buffer Size.
    const U32 maxBufferSize = 4096;

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(maxBufferSize);

    // Set Buffer Counter.
    U32 bufferCount = 0;

    // Add picked objects.
    for ( U32 n = 0; n < resultCount; n++ )
    {
        // Output Object ID.
        bufferCount += dSprintf( pBuffer + bufferCount, maxBufferSize-bufferCount, "%d ", queryResults[n].mpSceneObject->getId() );

        // Finish early if we run out of buffer space.
        if ( bufferCount >= maxBufferSize )
        {
            // Warn.
            Con::warnf("t2dScene::pickArea() - Too many items picked to return to scripts!");
            break;
        }
    }

    // Clear world query.
    pWorldQuery->clearQuery();

    // Return buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Picks objects intersecting the specified ray with optional group/layer masks.
    @param startx/y The coordinates of the start point as either (\x y\ or (x,y)
    @param endx/y The coordinates of the end point as either (\x y\ or (x,y)
    @param sceneGroupMask Optional scene group mask.  (-1) or empty string selects all groups.
    @param sceneLayerMask Optional scene layer mask.  (-1) or empty string selects all layers.
    @param pickMode Optional mode 'any', 'aabb', 'oobb' or 'collision' (default is 'oobb').
    @return Returns list of object IDs.
*/
ConsoleMethodWithDocs(t2dScene, pickRay, ConsoleString, 4, 9, (startx/y, endx/y, [sceneGroupMask], [sceneLayerMask], [pickMode] ))
{
    // Upper left and lower right bound.
    Vector2 v1, v2;

    // The index of the first optional parameter.
    U32 firstArg;

    // Grab the number of elements in the first two parameters.
    U32 elementCount1 = Utility::mGetStringElementCount(argv[2]);
    U32 elementCount2 = 1;
    if (argc > 3)
        elementCount2 = Utility::mGetStringElementCount(argv[3]);

    // ("x1 y1 x2 y2")
    if ((elementCount1 == 4) && (argc < 9))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[2], 2);
        firstArg = 3;
    }
   
    // ("x1 y1", "x2 y2")
    else if ((elementCount1 == 2) && (elementCount2 == 2) && (argc > 3) && (argc < 10))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[3]);
        firstArg = 4;
    }
   
    // (x1, y1, x2, y2)
    else if (argc > 5)
    {
        v1 = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        v2 = Vector2(dAtof(argv[4]), dAtof(argv[5]));
        firstArg = 6;
    }
   
    // Invalid
    else
    {
        Con::warnf("t2dScene::pickRay() - Invalid number of parameters!");
        return nullptr;
    }

    // Calculate scene group mask.
    U32 sceneGroupMask = MASK_ALL;
    if ( (U32)argc > firstArg )
    {
        if ( *argv[firstArg] != 0 )
            sceneGroupMask = dAtoi(argv[firstArg]);
    }

    // Calculate pick mode.
    t2dScene::PickMode pickMode = t2dScene::PICK_OOBB;
    if ( (U32)argc > (firstArg + 2))
    {
        pickMode = t2dScene::getPickModeEnum(argv[firstArg + 2]);
    }
    if ( pickMode == t2dScene::PICK_INVALID )
    {
        Con::warnf("t2dScene::pickRay() - Invalid pick mode of %s", argv[firstArg + 2]);
        pickMode = t2dScene::PICK_OOBB;
    }


    // Fetch world query and clear results.
    WorldQuery* pWorldQuery = object->getWorldQuery( true );

    // Set filter.
    WorldQueryFilter queryFilter( sceneGroupMask, true, false, true, true );
    pWorldQuery->setQueryFilter( queryFilter );

    // Perform query.
    if ( pickMode == t2dScene::PICK_ANY )
    {
        pWorldQuery->anyQueryRay( v1, v2 );    
    }
    else if ( pickMode == t2dScene::PICK_AABB )
    {
        pWorldQuery->aabbQueryRay( v1, v2 );    
    }
    else if ( pickMode == t2dScene::PICK_OOBB )
    {
        pWorldQuery->oobbQueryRay( v1, v2 );    
    }
    else if ( pickMode == t2dScene::PICK_COLLISION )
    {
        pWorldQuery->collisionQueryRay( v1, v2 );    
    }
    else
    {
        AssertFatal( false, "Unsupported pick mode." );
    }

    // Sanity!
    AssertFatal( pWorldQuery->getIsRaycastQueryResult(), "Invalid non-ray-cast query result returned." );

    // Fetch result count.
    const size_t resultCount = pWorldQuery->getQueryResultsCount();

    // Finish if no results.
    if ( resultCount == 0 )
        return nullptr;

    // Sort ray-cast result.
    pWorldQuery->sortRaycastQueryResult();

    // Fetch results.
    typeWorldQueryResultVector& queryResults = pWorldQuery->getQueryResults();

    // Set Max Buffer Size.
    const U32 maxBufferSize = 4096;

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(maxBufferSize);

    // Set Buffer Counter.
    U32 bufferCount = 0;

    // Add Picked Objects to List.
    for ( U32 n = 0; n < resultCount; n++ )
    {
        // Output Object ID.
        bufferCount += dSprintf( pBuffer + bufferCount, maxBufferSize-bufferCount, "%d ", queryResults[n].mpSceneObject->getId() );

        // Finish early if we run out of buffer space.
        if ( bufferCount >= maxBufferSize )
        {
            // Warn.
            Con::warnf("t2dScene::pickRay() - Too many items picked to return to scripts!");
            break;
        }
    }

    // Clear world query.
    pWorldQuery->clearQuery();

    // Return buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod(t2dScene, performBlastImpulse, void, 4, 7, "(x / y, radius, [blastPower], [sceneGroupMask], [numRays] ) applys force radiating from point with optional group/layer masks.\n"
        "@param startx/y The coordinates of the start point as either (\"x y\") or (x,y)\n"
        "@param radius maximum range of blast effect.\n"
        "@param blastPower Optional blast strength.  (1) or empty string selects all groups.\n"
        "@param sceneGroupMask Optional scene group mask.  (-1) or empty string selects all groups.\n"
        "@param numRays Optional number of rayCasts peformed (36).\n"
        "@returns void.")
{
    // The point.
    Vector2 point;

    // The index of the first optional parameter.
    U32 firstArg;

    // Grab the number of elements in the first parameter.
    U32 elementCount = Utility::mGetStringElementCount(argv[2]);

    // ("x y")
    if ((elementCount == 2) && (argc < 8))
    {
        point = Utility::mGetStringElementVector(argv[2]);
        firstArg = 3;
    }
            // (x, y)
    else if ((elementCount == 1) && (argc > 3))
    {
        point = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        firstArg = 4;
    }
            // Invalid
    else
    {
        Con::warnf("t2dScene::performBlastImpulse() - Invalid number of parameters!");
        return;
    }

    F32 radius = dAtof(argv[firstArg]);

    F32 blastPower = 1.0f;
    if ( (U32)argc > (firstArg + 1 ))
        blastPower = dAtof(argv[firstArg+1]);

    // Calculate scene group mask.
    U32 sceneGroupMask = MASK_ALL;
    if ( (U32)argc > firstArg+2 )
    {
        if ( *argv[firstArg+2] != 0 )
            sceneGroupMask = dAtoi(argv[firstArg+1]);
    }

    S32 numRays = 36.0f;
    if ( (U32)argc > (firstArg + 3 ))
        numRays = dAtoi(argv[firstArg+3]);

    object->performBlastImpulse(point, radius, blastPower, sceneGroupMask, numRays);
}



/*! Picks objects intersecting the specified point with optional group/layer masks.
    @param x/y The coordinate of the point as either (\x y\ or (x,y)
    @param sceneGroupMask Optional scene group mask.  (-1) or empty string selects all groups.
    @param sceneLayerMask Optional scene layer mask.  (-1) or empty string selects all layers.
    @param pickMode Optional mode 'any', 'aabb', 'oobb' or 'collision' (default is 'ooabb').
    @return Returns list of object IDs.
*/
ConsoleMethodWithDocs(t2dScene, pickPoint, ConsoleString, 3, 7, (x / y, [sceneGroupMask], [sceneLayerMask], [pickMode] ))
{
    // The point.
    Vector2 point;

    // The index of the first optional parameter.
    U32 firstArg;

    // Grab the number of elements in the first parameter.
    U32 elementCount = Utility::mGetStringElementCount(argv[2]);

    // ("x y")
    if ((elementCount == 2) && (argc < 8))
    {
        point = Utility::mGetStringElementVector(argv[2]);
        firstArg = 3;
    }
   
    // (x, y)
    else if ((elementCount == 1) && (argc > 3))
    {
        point = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        firstArg = 4;
    }
   
    // Invalid
    else
    {
        Con::warnf("t2dScene::pickPoint() - Invalid number of parameters!");
        return nullptr;
    }

    // Calculate scene group mask.
    U32 sceneGroupMask = MASK_ALL;
    if ( (U32)argc > firstArg )
    {
        if ( *argv[firstArg] != 0 )
            sceneGroupMask = dAtoi(argv[firstArg]);
    }

    // Calculate pick mode.
    t2dScene::PickMode pickMode = t2dScene::PICK_OOBB;
    if ( (U32)argc > (firstArg + 2 ))
    {
        pickMode = t2dScene::getPickModeEnum(argv[firstArg + 2]);
    }
    if ( pickMode == t2dScene::PICK_INVALID )
    {
        Con::warnf("t2dScene::pickPoint() - Invalid pick mode of %s", argv[firstArg + 2]);
        pickMode = t2dScene::PICK_OOBB;
    }


    // Fetch world query and clear results.
    WorldQuery* pWorldQuery = object->getWorldQuery( true );

    // Set filter.
    WorldQueryFilter queryFilter( sceneGroupMask, true, false, true, true );
    pWorldQuery->setQueryFilter( queryFilter );

    // Perform query.
    if ( pickMode == t2dScene::PICK_ANY )
    {
        pWorldQuery->anyQueryPoint( point );    
    }
    else if ( pickMode == t2dScene::PICK_AABB )
    {
        pWorldQuery->aabbQueryPoint( point );    
    }
    else if ( pickMode == t2dScene::PICK_OOBB )
    {
        pWorldQuery->oobbQueryPoint( point );    
    }
    else if ( pickMode == t2dScene::PICK_COLLISION )
    {
        pWorldQuery->collisionQueryPoint( point );    
    }
    else
    {
        AssertFatal( false, "Unsupported pick mode." );
    }

    // Fetch result count.
    const size_t resultCount = pWorldQuery->getQueryResultsCount();

    // Finish if no results.
    if ( resultCount == 0 )
        return nullptr;

    // Fetch results.
    typeWorldQueryResultVector& queryResults = pWorldQuery->getQueryResults();

    // Set Max Buffer Size.
    const U32 maxBufferSize = 4096;

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(maxBufferSize);

    // Set Buffer Counter.
    U32 bufferCount = 0;

    // Add Picked Objects to List.
    for ( U32 n = 0; n < resultCount; n++ )
    {
        // Output Object ID.
        bufferCount += dSprintf( pBuffer + bufferCount, maxBufferSize-bufferCount, "%d ", queryResults[n].mpSceneObject->getId() );

        // Finish early if we run out of buffer space.
        if ( bufferCount >= maxBufferSize )
        {
            // Warn.
            Con::warnf("t2dScene::pickPoint() - Too many items picked to return to scripts!");
            break;
        }
    }

    // Clear world query.
    pWorldQuery->clearQuery();

    // Return buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Picks objects intersecting the specified circle with optional group/layer masks.
    @param x/y The coordinate of the point as either (\x y\ or (x,y)
    @param radius The radius of the circle.
    @param sceneGroupMask Optional scene group mask.  (-1) or empty string selects all groups.
    @param sceneLayerMask Optional scene layer mask.  (-1) or empty string selects all layers.
    @param pickMode Optional mode 'any', 'aabb', 'oobb' or 'collision' (default is 'ooabb').
    @return Returns list of object IDs.
*/
ConsoleMethodWithDocs(t2dScene, pickCircle, ConsoleString, 4, 8, (x / y, radius, [sceneGroupMask], [sceneLayerMask], [pickMode] ))
{
    // The point.
    Vector2 point;

    // The index of the first optional parameter.
    U32 firstArg;

    // Grab the number of elements in the first parameter.
    U32 elementCount = Utility::mGetStringElementCount(argv[2]);

    // ("x y")
    if ((elementCount == 2) && (argc < 8))
    {
        point = Utility::mGetStringElementVector(argv[2]);
        firstArg = 3;
    }
   
    // (x, y)
    else if ((elementCount == 1) && (argc > 3))
    {
        point = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        firstArg = 4;
    }
   
    // Invalid
    else
    {
        Con::warnf("t2dScene::pickPoint() - Invalid number of parameters!");
        return nullptr;
    }

    // Fetch radius.
    const F32 radius = dAtof(argv[firstArg++]);

    // Check radius.
    if ( radius <= 0.0f )
    {
        Con::warnf( "t2dScene::pickCircle()  Radius must be greater than zero." );
        return StringTable->EmptyString;
    }

    // Calculate scene group mask.
    U32 sceneGroupMask = MASK_ALL;
    if ( (U32)argc > firstArg )
    {
        if ( *argv[firstArg] != 0 )
            sceneGroupMask = dAtoi(argv[firstArg]);
    }

//    // Calculate scene layer mask.
//    U32 sceneLayerMask = MASK_ALL;
//    if ( (U32)argc > (firstArg + 1) )
//    {
//        if ( *argv[firstArg + 1] != 0 )
//            sceneLayerMask = dAtoi(argv[firstArg + 1]);
//    }

    // Calculate pick mode.
    t2dScene::PickMode pickMode = t2dScene::PICK_OOBB;
    if ( (U32)argc > (firstArg + 2 ))
    {
        pickMode = t2dScene::getPickModeEnum(argv[firstArg + 2]);
    }
    if ( pickMode == t2dScene::PICK_INVALID )
    {
        Con::warnf("t2dScene::pickPoint() - Invalid pick mode of %s", argv[firstArg + 2]);
        pickMode = t2dScene::PICK_OOBB;
    }


    // Fetch world query and clear results.
    WorldQuery* pWorldQuery = object->getWorldQuery( true );

    // Set filter.
    WorldQueryFilter queryFilter( sceneGroupMask, true, false, true, true );
    pWorldQuery->setQueryFilter( queryFilter );

    // Perform query.
    if ( pickMode == t2dScene::PICK_ANY )
    {
        pWorldQuery->anyQueryCircle( point, radius );    
    }
    else if ( pickMode == t2dScene::PICK_AABB )
    {
        pWorldQuery->aabbQueryCircle( point, radius );    
    }
    else if ( pickMode == t2dScene::PICK_OOBB )
    {
        pWorldQuery->oobbQueryCircle( point, radius );    
    }
    else if ( pickMode == t2dScene::PICK_COLLISION )
    {
        pWorldQuery->collisionQueryCircle( point, radius );    
    }
    else
    {
        AssertFatal( false, "Unsupported pick mode." );
    }

    // Fetch result count.
    const size_t resultCount = pWorldQuery->getQueryResultsCount();

    // Finish if no results.
    if ( resultCount == 0 )
        return nullptr;

    // Fetch results.
    typeWorldQueryResultVector& queryResults = pWorldQuery->getQueryResults();

    // Set Max Buffer Size.
    const U32 maxBufferSize = 4096;

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(maxBufferSize);

    // Set Buffer Counter.
    U32 bufferCount = 0;

    // Add Picked Objects to List.
    for ( U32 n = 0; n < resultCount; n++ )
    {
        // Output Object ID.
        bufferCount += dSprintf( pBuffer + bufferCount, maxBufferSize-bufferCount, "%d ", queryResults[n].mpSceneObject->getId() );

        // Finish early if we run out of buffer space.
        if ( bufferCount >= maxBufferSize )
        {
            // Warn.
            Con::warnf("t2dScene::pickPoint() - Too many items picked to return to scripts!");
            break;
        }
    }

    // Clear world query.
    pWorldQuery->clearQuery();

    // Return buffer.
    return pBuffer;
}


//-----------------------------------------------------------------------------

/*! Picks objects with collision shapes intersecting the specified ray with optional group/layer masks.
    Unlike other pick methods, this returns the complete detail for each object encountered, returning the collision point, normal and fraction of the ray intersection.
    @param startx/y The coordinates of the start point as either (\x y\ or (x,y)
    @param endx/y The coordinates of the end point as either (\x y\ or (x,y)
    @param sceneGroupMask Optional scene group mask.  (-1) or empty string selects all groups.
    @param sceneLayerMask Optional scene layer mask.  (-1) or empty string selects all layers.
    @return Returns a list of objects in blocks of detail items where each block represents a single object and its collision detail in the format:
    <ObjectId PointX PointY NormalX NormalY RayFraction ShapeIndex> <ObjectId PointX PointY NormalX NormalY RayFraction ShapeIndex> <ObjectId PointX PointY NormalX NormalY RayFraction ShapeIndex> etc.
*/
ConsoleMethodWithDocs(t2dScene, pickRayCollision, ConsoleString, 4, 8, (startx/y, endx/y, [sceneGroupMask], [sceneLayerMask] ))
{
    // Upper left and lower right bound.
    Vector2 v1, v2;

    // The index of the first optional parameter.
    U32 firstArg;

    // Grab the number of elements in the first two parameters.
    U32 elementCount1 = Utility::mGetStringElementCount(argv[2]);
    U32 elementCount2 = 1;
    if (argc > 3)
        elementCount2 = Utility::mGetStringElementCount(argv[3]);

    // ("x1 y1 x2 y2")
    if ((elementCount1 == 4) && (argc < 9))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[2], 2);
        firstArg = 3;
    }
   
    // ("x1 y1", "x2 y2")
    else if ((elementCount1 == 2) && (elementCount2 == 2) && (argc > 3) && (argc < 9))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[3]);
        firstArg = 4;
    }
   
    // (x1, y1, x2, y2)
    else if (argc > 5)
    {
        v1 = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        v2 = Vector2(dAtof(argv[4]), dAtof(argv[5]));
        firstArg = 6;
    }
   
    // Invalid
    else
    {
        Con::warnf("t2dScene::pickRayCollision() - Invalid number of parameters!");
        return nullptr;
    }

    // Calculate scene group mask.
    U32 sceneGroupMask = MASK_ALL;
    if ( (U32)argc > firstArg )
    {
        if ( *argv[firstArg] != 0 )
            sceneGroupMask = dAtoi(argv[firstArg]);
    }

//    // Calculate scene layer mask.
//    U32 sceneLayerMask = MASK_ALL;
//    if ( (U32)argc > (firstArg + 1) )
//    {
//        if ( *argv[firstArg + 1] != 0 )
//            sceneLayerMask = dAtoi(argv[firstArg + 1]);
//    }

    // Fetch world query and clear results.
    WorldQuery* pWorldQuery = object->getWorldQuery( true );

    // Set filter.
    WorldQueryFilter queryFilter( sceneGroupMask, true, false, true, true );
    pWorldQuery->setQueryFilter( queryFilter );

    // Perform query.
    pWorldQuery->collisionQueryRay( v1, v2 );    

    // Sanity!
    AssertFatal( pWorldQuery->getIsRaycastQueryResult(), "Invalid non-ray-cast query result returned." );

    // Fetch result count.
    const size_t resultCount = pWorldQuery->getQueryResultsCount();

    // Finish if no results.
    if ( resultCount == 0 )
        return nullptr;

    // Sort ray-cast result.
    pWorldQuery->sortRaycastQueryResult();

    // Fetch results.
    typeWorldQueryResultVector& queryResults = pWorldQuery->getQueryResults();

    // Set Max Buffer Size.
    const U32 maxBufferSize = 4096;

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(maxBufferSize);

    // Set Buffer Counter.
    U32 bufferCount = 0;

    // Add Picked Objects to List.
    for ( U32 n = 0; n < resultCount; n++ )
    {
        // Fetch query result.
        const WorldQueryResult& queryResult = queryResults[n];

        bufferCount += dSprintf( pBuffer + bufferCount, maxBufferSize-bufferCount, "%d %g %g %g %g %g %d ",
            queryResult.mpSceneObject->getId(),
            queryResult.mPoint.x, queryResult.mPoint.y,
            queryResult.mNormal.x, queryResult.mNormal.y,
            queryResult.mFraction,
            queryResult.mShapeIndex );

        // Finish early if we run out of buffer space.
        if ( bufferCount >= maxBufferSize )
        {
            // Warn.
            Con::warnf("t2dScene::pickRayCollision() - Too many items picked to return to scripts!");
            break;
        }
    }

    // Clear world query.
    pWorldQuery->clearQuery();

    // Return buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

/*! Sets Debug option(s) on.
    @param debugOptions Either a list of debug modes (comma-separated), or a string with the modes (space-separated)
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setDebugOn, ConsoleVoid, 3, 2 + DEBUG_MODE_COUNT, (debugOptions))
{
    // Reset the mask.
    U32 mask = 0;

    // Grab the element count of the first parameter.
    const U32 elementCount = Utility::mGetStringElementCount(argv[2]);

    // Make sure we get at least one number.
    if (elementCount < 1)
    {
        Con::warnf( "t2dScene::setDebugOn() - Invalid number of parameters!" );
        return;
    }

    // Space-separated list.
    if (argc == 3)
    {
        // Convert the string to a mask.
        for (U32 i = 0; i < elementCount; i++)
        {
            // Fetch the debug option.
            const char* pDebugOption = Utility::mGetStringElement( argv[2], i );
            t2dScene::DebugOption debugOption = t2dScene::getDebugOptionEnum( pDebugOption );
        
            // Is the option valid?
            if ( debugOption == t2dScene::SCENE_DEBUG_INVALID )
            {
                // No, so warn.
                Con::warnf( "t2dScene::setDebugOn() - Invalid debug option '%s' specified.", pDebugOption );
                continue;
            }
         
            // Merge into mask.
            mask |= debugOption;
        }
    }
    // Comma-separated list.
    else
    {
        // Convert the list to a mask.
        for (U32 i = 2; i < (U32)argc; i++)
        {
            // Fetch the debug option.
            const char* pDebugOption = argv[i];
            t2dScene::DebugOption debugOption = t2dScene::getDebugOptionEnum( argv[i] );

            // Is the option valid?
            if ( debugOption == t2dScene::SCENE_DEBUG_INVALID )
            {
                // No, so warn.
                Con::warnf( "t2dScene::setDebugOn() - Invalid debug option '%s' specified.", pDebugOption );
                continue;
            }

            // Merge into mask.
            mask |= debugOption;
        }
    }

    // Set debug mask.
    object->setDebugOn(mask);
} 

//-----------------------------------------------------------------------------

/*! Sets Debug options(s) off.
    @param debugOptions Either a list of debug modes to turn off (comma-separated) or a string (space-separated)
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setDebugOff, ConsoleVoid, 3, 2 + DEBUG_MODE_COUNT, (debugOptions))
{
    // Reset the mask.
    U32 mask = 0;

    // Grab the element count of the first parameter.
    const U32 elementCount = Utility::mGetStringElementCount(argv[2]);

    // Make sure we get at least one number.
    if (elementCount < 1)
    {
        Con::warnf( "t2dScene::setDebugOff() - Invalid number of parameters!" );
        return;
    }

    // Space-separated list.
    if (argc == 3)
    {
        // Convert the string to a mask.
        for (U32 i = 0; i < elementCount; i++)
        {
            // Fetch the debug option.
            const char* pDebugOption = Utility::mGetStringElement( argv[2], i );
            t2dScene::DebugOption debugOption = t2dScene::getDebugOptionEnum( pDebugOption );
        
            // Is the option valid?
            if ( debugOption == t2dScene::SCENE_DEBUG_INVALID )
            {
                // No, so warn.
                Con::warnf( "t2dScene::setDebugOff() - Invalid debug option '%s' specified.", pDebugOption );
                continue;
            }
         
            // Merge into mask.
            mask |= debugOption;
        }
    }
    // Comma-separated list.
    else
    {
        // Convert the list to a mask.
        for (U32 i = 2; i < (U32)argc; i++)
        {
            // Fetch the debug option.
            const char* pDebugOption = argv[i];
            t2dScene::DebugOption debugOption = t2dScene::getDebugOptionEnum( argv[i] );

            // Is the option valid?
            if ( debugOption == t2dScene::SCENE_DEBUG_INVALID )
            {
                // No, so warn.
                Con::warnf( "t2dScene::setDebugOff() - Invalid debug option '%s' specified.", pDebugOption );
                continue;
            }

            // Merge into mask.
            mask |= debugOption;
        }
    }

    // Set debug mask.
    object->setDebugOff(mask);
} 

//-----------------------------------------------------------------------------

/*! Gets the state of the debug modes.
    @return Returns a space separated list of debug modes that are active.
*/
ConsoleMethodWithDocs(t2dScene, getDebugOn, ConsoleString, 2, 2, ())
{
    // Fetch debug mask,.
    const U32 debugMask = object->getDebugMask();

    // Fetch a return buffer.
    S32 bufferSize = 1024;
    char* pReturnBuffer = Con::getReturnBuffer(bufferSize);
    *pReturnBuffer = 0;
    char* pWriteCursor = pReturnBuffer;

    // Iterate debug mask.
    for( U32 bit = 0; bit < 32; ++bit )
    {
        // Calculate debug mask bit.
        const S32 debugBit = 1 << bit;
        if ( (debugMask & debugBit) == 0 )
            continue;

        // Format option.
        const S32 size = dSprintf( pWriteCursor, bufferSize, "%s ", object->getDebugOptionDescription( (t2dScene::DebugOption)debugBit ) );
        bufferSize -= size;
        pWriteCursor += size;
    }

    return pReturnBuffer;
}

//-----------------------------------------------------------------------------

/*! Sets the scene object to monitor in the debug metrics.
    @param SceneObject The scene object to monitor in the debug metrics.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setDebugSceneObject, ConsoleVoid, 3, 3, (sceneObject))
{
    // Fetch scene object Id.
    const SimObjectId sceneObjectId = dAtoi(argv[2]);

    // Find scene object.
    SceneObject* pSceneObject = dynamic_cast<SceneObject*>( Sim::findObject(sceneObjectId) );

    // Sanity!
    if ( !pSceneObject )
    {
        Con::warnf("t2dScene::setDebugSceneObject() - Could not find scene object %d.", sceneObjectId);
        return;
    }

    // Set debug object.
    object->setDebugSceneObject( pSceneObject );
}

//-----------------------------------------------------------------------------

/*! ( Gets the scene object being monitored in the debug metrics.
    @return The scene object being monitored in the debug metrics (zero if none being monitored).
*/
ConsoleMethodWithDocs(t2dScene, getDebugSceneObject, ConsoleInt, 2, 2, ())
{
    // Fetch scene object.
    SceneObject* pSceneObject = object->getDebugSceneObject();

    if ( pSceneObject == nullptr )
        return 0;

    return pSceneObject->getId();
}

//-----------------------------------------------------------------------------

/*! Sets the layer to use the specified render sort mode.
    @param layer The layer to modify.
    @param sortMode The sort mode to use on the specified layer.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setLayerSortMode, ConsoleVoid, 4, 4, (layer, sortMode))
{
    // Fetch the layer.
    const U32 layer = dAtoi(argv[2]);

    // Fetch the sort mode.
    const SceneRenderQueue::RenderSort sortMode = SceneRenderQueue::getRenderSortEnum( argv[3] );

    object->setLayerSortMode( layer, sortMode );
}

//-----------------------------------------------------------------------------

/*! Gets the render sort mode for the specified layer.
    @param layer The layer to retrieve.
    @return The render sort mode for the specified layer.
*/
ConsoleMethodWithDocs(t2dScene, getLayerSortMode, ConsoleString, 3, 3, (layer))
{
    // Fetch the layer.
    const U32 layer = dAtoi(argv[2]);

    // Fetch the sort mode.
    return SceneRenderQueue::getRenderSortDescription( object->getLayerSortMode( layer ) );
}

//-----------------------------------------------------------------------------

/*! Resets the debug statistics.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, resetDebugStats, ConsoleVoid, 2, 2, ())
{
    object->resetDebugStats();
}

//-----------------------------------------------------------------------------

/*! Gets the current average frames-per-second.
    @return The current average frames-per-second.
*/
ConsoleMethodWithDocs(t2dScene, getFPS, ConsoleFloat, 2, 2, ())
{
    return object->getDebugStats().fps;
}

//-----------------------------------------------------------------------------

/*! Gets the minimum average frames-per-second.
    @return The minimum Faverage frames-per-second.
*/
ConsoleMethodWithDocs(t2dScene, getMinFPS, ConsoleFloat, 2, 2, ())
{
    return object->getDebugStats().minFPS;
}

//-----------------------------------------------------------------------------

/*! Gets the maximum average frames-per-second.
    @return The average frames-per-second.
*/
ConsoleMethodWithDocs(t2dScene, getMaxFPS, ConsoleFloat, 2, 2, ())
{
    return object->getDebugStats().maxFPS;
}

//-----------------------------------------------------------------------------

/*! Gets the current rendered frame count.
    @return The current rendered frame count.
*/
ConsoleMethodWithDocs(t2dScene, getFrameCount, ConsoleInt, 2, 2, ())
{
    return (S32)object->getDebugStats().frameCount;
}

//-----------------------------------------------------------------------------

/*! Sets whether render batching is enabled or not.
    @param enabled Whether render batching is enabled or not.
    return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setBatchingEnabled, ConsoleVoid, 3, 3, ( bool enabled ))
{
    // Fetch args.
    const bool enabled = dAtob(argv[2]);

    // Sets batching enabled.
    object->setBatchingEnabled( enabled );
}

//-----------------------------------------------------------------------------

/*! Gets whether render batching is enabled or not.
    return Whether render batching is enabled or not.
*/
ConsoleMethodWithDocs(t2dScene, getBatchingEnabled, ConsoleBool, 2, 2, ())
{
    // Gets batching enabled.
    return object->getBatchingEnabled();
}

//-----------------------------------------------------------------------------

/*! Sets whether this is an editor scene.
    @return No return value.
*/
ConsoleMethodWithDocs(t2dScene, setIsEditorScene, ConsoleVoid, 3, 3, ())
{
   object->setIsEditorScene(dAtob(argv[2]));
}

//-----------------------------------------------------------------------------

/*! Creates the specified scene-object derived type and adds it to the scene.
    @return The scene-object or NULL if not created.
*/
ConsoleMethodWithDocs(t2dScene, create, ConsoleString, 3, 3, (type))
{
    // Create the scene object.
    SceneObject* pSceneObject = object->create( argv[2] );

    return pSceneObject == nullptr ? nullptr : pSceneObject->getIdString();
}

ConsoleMethodGroupEndWithDocs(Scene)
