//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#include "platform/platform.h"
#include "lighting/lightManager.h"

#include "console/console.h"
#include "console/consoleTypes.h"
#include "memory/safeDelete.h"
//#include "console/sim.h"
#include "sim/simSet.h"
#include "2d/scene/Scene.h"
#include "2d/sceneobject/SceneObject.h"
//#include "materials/materialManager.h"
//#include "materials/sceneData.h"
#include "lighting/lightInfo.h"
//#include "lighting/lightingInterfaces.h"
//#include "T3D/gameBase/gameConnection.h"
#include "graphics/gfxStringEnumTranslate.h"
//#include "console/engineAPI.h"
//#include "renderInstance/renderPrePassMgr.h"


//Signal<void(const char*,bool)> LightManager::smActivateSignal;
LightManager *LightManager::smActiveLM = NULL;


LightManager::LightManager( const char *name, const char *id )
   :  mName( name ),
      mId( id ),
      mIsActive( false ),      
      mDefaultLight( NULL )
{ 
   _getLightManagers().insert( mName, this );
    if (smActiveLM == NULL)
        smActiveLM = this;

//   dMemset( &mSpecialLights, 0, sizeof( mSpecialLights ) );
}

LightManager::~LightManager() 
{
   _getLightManagers().erase( mName );
//   SAFE_DELETE( mAvailableSLInterfaces );
   SAFE_DELETE( mDefaultLight );
}

LightManagerMap& LightManager::_getLightManagers()
{
   static LightManagerMap lightManagerMap;
   return lightManagerMap;
}

//LightManager* LightManager::findByName( const char *name )
//{
//   LightManagerMap &lightManagers = _getLightManagers();
//
//   LightManagerMap::iterator iter = lightManagers.find( name );
//   if ( iter != lightManagers.end() )
//      return iter->value;
//
//   return NULL;
//}
//
//void LightManager::getLightManagerNames( String *outString )
//{
//   LightManagerMap &lightManagers = _getLightManagers();
//   LightManagerMap::iterator iter = lightManagers.begin();
//   for ( ; iter != lightManagers.end(); iter++ )
//      *outString += iter->key + "\t";
//
//   // TODO!
//   //outString->rtrim();
//}

LightInfo* LightManager::createLightInfo(LightInfo* light /* = NULL */)
{
   LightInfo *outLight = (light != NULL) ? light : new LightInfo;

   LightManagerMap &lightManagers = _getLightManagers();
   LightManagerMap::iterator iter = lightManagers.begin();
   for ( ; iter != lightManagers.end(); iter++ )
   {
      LightManager *lm = iter->value;
      lm->_addLightInfoEx( outLight );
   }

   return outLight;
}

void LightManager::initLightFields()
{
   LightManagerMap &lightManagers = _getLightManagers();

   LightManagerMap::iterator iter = lightManagers.begin();
   for ( ; iter != lightManagers.end(); iter++ )
   {
      LightManager *lm = iter->value;
      lm->_initLightFields();
   }
}


void LightManager::registerGlobalLights( typeWorldQueryResultVector queryVector )
{
    // Let the lights register themselves.
    for( typeWorldQueryResultVector::iterator worldQueryItr = queryVector.begin(); worldQueryItr != queryVector.end(); ++worldQueryItr )
    {
        // Fetch scene object.
        SceneObject* pSceneObject = worldQueryItr->mpSceneObject;
        ISceneLight *lightInterface = dynamic_cast<ISceneLight*>( pSceneObject );
        if ( lightInterface )
            lightInterface->submitLights( this, false );
    }
}

void LightManager::registerGlobalLight( LightInfo *light, SimObject *obj )
{
   AssertFatal( !mRegisteredLights.contains( light ), 
      "LightManager::registerGlobalLight - This light is already registered!" );

   mRegisteredLights.push_back( light );
}

void LightManager::unregisterGlobalLight( LightInfo *light )
{
   mRegisteredLights.unregisterLight( light );

//   // If this is the sun... clear the special light too.
//   if ( light == mSpecialLights[slSunLightType] )
//      dMemset( mSpecialLights, 0, sizeof( mSpecialLights ) );
}

void LightManager::unregisterAllLights()
{
//   dMemset( mSpecialLights, 0, sizeof( mSpecialLights ) );
   mRegisteredLights.clear();
}

void LightManager::getAllUnsortedLights( Vector<LightInfo*> *list ) const
{
    list->merge( mRegisteredLights );
}
