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
#include "t2dScene.h"
#include "2d/sceneobject/SceneObject.h"
//#include "materials/materialManager.h"
//#include "materials/sceneData.h"
#include "lighting/lightInfo.h"
//#include "lighting/lightingInterfaces.h"
//#include "T3D/gameBase/gameConnection.h"
#include "graphics/gfxStringEnumTranslate.h"
//#include "console/engineAPI.h"
//#include "renderInstance/renderPrePassMgr.h"


Signal<void(const char*,bool)> LightManager::smActivateSignal;
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

   mRegisteredLights.setSize(0);
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
   for ( LightManagerMap::pair iter:lightManagers )
   {
      LightManager *lm = iter.second;
      lm->_addLightInfoEx( outLight );
   }

   return outLight;
}

void LightManager::initLightFields()
{
   LightManagerMap &lightManagers = _getLightManagers();

   for ( LightManagerMap::pair iter:lightManagers )
   {
      LightManager *lm = iter.second;
      lm->_initLightFields();
   }
}


void LightManager::registerGlobalLights( typeWorldQueryResultVector queryVector )
{
//   Con::printf("LightManager::registerGlobalLights");
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

//   Con::printf("registerGlobalLight #%i %s", mRegisteredLights.size(), light->getColor().scriptThis());
   
   mRegisteredLights.push_back( light );
}

void LightManager::setLightInfo(  ProcessedMaterial* pmat,
                                     const Material* mat,
                                     const SceneData& sgData,
                                     const SceneRenderState *state,
                                     U32 pass,
                                     GFXShaderConstBuffer* shaderConsts )
{
//   PROFILE_SCOPE( LightManager_SetLightInfo );
//   
//   GFXShader *shader = shaderConsts->getShader();
//   
//   // Check to see if this is the same shader.  Since we
//   // sort by material we should get hit repeatedly by the
//   // same one.  This optimization should save us many
//   // hash table lookups.
//   if ( mLastShader.getPointer() != shader )
//   {
//      LightConstantMap::Iterator iter = mConstantLookup.find(shader);
//      if ( iter != mConstantLookup.end() )
//      {
//         mLastConstants = iter->value;
//      }
//      else
//      {
//         LightingShaderConstants* lsc = new LightingShaderConstants();
//         mConstantLookup[shader] = lsc;
//         
//         mLastConstants = lsc;
//      }
//      
//      // Set our new shader
//      mLastShader = shader;
//   }
//   
//   // Make sure that our current lighting constants are initialized
//   if (!mLastConstants->mInit)
//      mLastConstants->init(shader);
//   
//   // NOTE: If you encounter a crash from this point forward
//   // while setting a shader constant its probably because the
//   // mConstantLookup has bad shaders/constants in it.
//   //
//   // This is a known crash bug that can occur if materials/shaders
//   // are reloaded and the light manager is not reset.
//   //
//   // We should look to fix this by clearing the table.
//   
//   _update4LightConsts( sgData,
//                       mLastConstants->mLightPosition,
//                       mLastConstants->mLightDiffuse,
//                       mLastConstants->mLightAmbient,
//                       mLastConstants->mLightInvRadiusSq,
//                       mLastConstants->mLightSpotDir,
//                       mLastConstants->mLightSpotAngle,
//                       mLastConstants->mLightSpotFalloff,
//                       shaderConsts );
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

//S32 QSORT_CALLBACK LightManager::lightDistance(const void* a, const void* b)
//{
//   // Fetch scene render requests.
//   LightInfo* pLightInfoA  = *((LightInfo**)a);
//   LightInfo* pLightInfoB  = *((LightInfo**)b);
//
//   Point3F point = LIGHTMGR->getSortPoint();
//
//   return (((pLightInfoA->getPosition()-point).lenSquared() - (pLightInfoB->getPosition()-point).lenSquared()));
//}

bool LightManager::lightDistance(const LightInfo* pLightInfoA, const LightInfo* pLightInfoB)
{
    // Fetch scene render requests.
    Point3F point = LIGHTMGR->getSortPoint();

    return (((pLightInfoA->getPosition()-point).lenSquared() < (pLightInfoB->getPosition()-point).lenSquared()));
}

void LightManager::getSortedLightsByDistance( Vector<LightInfo*> *list, Point3F point)
{
   list->merge(mRegisteredLights);
   // Sort asset definitions.
   sortPoint = point;
//   dQsort( list->address(), list->size(), sizeof(const LightInfo*), lightDistance );
    std::sort( list->begin(), list->end(), lightDistance );
}
