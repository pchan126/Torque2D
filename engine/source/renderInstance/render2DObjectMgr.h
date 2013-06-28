//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------
#ifndef _RENDER2DOBJECTMGR_H_
#define _RENDER2DOBJECTMGR_H_

#ifndef _RENDERBINMANAGER_H_
#include "renderInstance/renderBinManager.h"
#endif

//**************************************************************************
// RenderObjectManager
//**************************************************************************
class Render2DObjectManager : public RenderBinManager
{
   typedef RenderBinManager Parent;
public:
   Render2DObjectManager();
   Render2DObjectManager(RenderInstType riType, F32 renderOrder, F32 processAddOrder);

   virtual void setOverrideMaterial(BaseMatInstance* overrideMat); 

   // RenderBinMgr
   virtual void render(SceneRenderState * state);

   // ConsoleObject
   static void initPersistFields();
   DECLARE_CONOBJECT(Render2DObjectManager);
protected:
   BaseMatInstance* mOverrideMat;
};

#endif // _RENDER2DOBJECTMGR_H_