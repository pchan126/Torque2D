//
//  Layer.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-06-13.
//

#ifndef __Torque2D__Layer__
#define __Torque2D__Layer__

#include "2d/sceneobject/SceneObject.h"

//---------------------------------------------------------------------------
/// A group of SceneObjects.
///
/// A Layer is a stricter form of SimSet. SceneObjects may only be a member
/// of a single Layer at a time.
///
/// The Layer will automatically enforce the single-group-membership rule.
///
/// @code
///      // From engine/sim/simPath.cc - getting a pointer to a Layer
///      Layer* pMissionGroup = dynamic_cast<Layer*>(Sim::findObject("MissionGroup"));
///
///      // From game/trigger.cc:46 - iterating over a SceneObject's group.
///      SceneObject* trigger = ...;
///      Layer* pGroup = trigger->getGroup();
///      for (Layer::iterator itr = pGroup->begin(); itr != pGroup->end(); itr++)
///      {
///         // do something with *itr
///      }
/// @endcode
class Layer: public SimSet
{
private:
   friend class SceneObject;
   
   typedef SimSet Parent;
   SimNameDictionary nameDictionary;
   
   SceneRenderQueue::RenderSort mSortMode;
   
   Point3F mCameraTranslationScale;
   
public:
   Layer(): mSortMode(SceneRenderQueue::RENDER_SORT_NEWEST) {};
   ~Layer();
   
   /// Add an object to the group.
   virtual void addObject(SceneObject*);
   void addObject(SceneObject*, SimObjectId);
   void addObject(SceneObject*, const char *name);
   
//   /// Remove an object from the group.
//   virtual void removeObject(SceneObject*);
//   
//   virtual void onRemove();
   
//   /// Find an object in the group.
//   virtual SceneObject* findObject(const char* name);
   
   void setSortMode( const SceneRenderQueue::RenderSort sortMode ) { mSortMode = sortMode; };
   SceneRenderQueue::RenderSort getSortMode(void) { return mSortMode; };
   
//   bool processArguments(S32 argc, const char **argv);
   
   DECLARE_CONOBJECT(Layer);
};

inline void Layer::addObject(SceneObject* obj)
{
   Parent::addObject( obj );
}

inline void Layer::addObject(SceneObject* obj, SimObjectId id)
{
   obj->mId = id;
   addObject( obj );
}

inline void Layer::addObject(SceneObject *obj, const char *name)
{
   addObject( obj );
   obj->assignName(name);
}

class Layeriterator: public SimSetiterator
{
public:
   Layeriterator(Layer* grp): SimSetiterator(grp) {}
   SceneObject* operator++();
};

#endif /* defined(__Torque2D__Layer__) */
