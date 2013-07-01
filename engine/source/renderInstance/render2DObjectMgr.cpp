//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------
#include "./render2dObjectMgr.h"
#include "console/consoleTypes.h"
#include "2d/sceneobject/sceneObject.h"

IMPLEMENT_CONOBJECT(Render2DObjectManager);

//ConsoleDocClass( RenderObjectManager,
//   "@brief A render bin which uses object callbacks for rendering.\n\n"
//   "This render bin gathers object render instances and calls its delegate "
//   "method to perform rendering.  It is used infrequently for specialized "
//   "scene objects which perform custom rendering.\n\n"
//   "@ingroup RenderBin\n" );


Render2DObjectManager::Render2DObjectManager()
: RenderBinManager(RenderPassManager::RIT_Object, 1.0f, 1.0f)
{
   mOverrideMat = NULL;
}

Render2DObjectManager::Render2DObjectManager(RenderInstType riType, F32 renderOrder, F32 processAddOrder)
 : RenderBinManager(riType, renderOrder, processAddOrder)
{  
   mOverrideMat = NULL;
}

void Render2DObjectManager::initPersistFields()
{
   Parent::initPersistFields();
}

void Render2DObjectManager::setOverrideMaterial(BaseMatInstance* overrideMat)
{ 
   mOverrideMat = overrideMat;
}

//-----------------------------------------------------------------------------
// render objects
//-----------------------------------------------------------------------------
void Render2DObjectManager::render( SceneRenderState *state )
{
   PROFILE_SCOPE(RenderObjectMgr_render);

   // Early out if nothing to draw.
   if(!mElementList.size())
      return;

   for( U32 i=0; i<mElementList.size(); i++ )
   {
      ObjectRenderInst *ri = static_cast<ObjectRenderInst*>(mElementList[i].inst);
      if ( ri->renderDelegate )
         ri->renderDelegate( ri, state, mOverrideMat );      
   }
}