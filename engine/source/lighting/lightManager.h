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

#ifndef _LIGHTMANAGER_H_
#define _LIGHTMANAGER_H_

#include "string/str.h"
#include "delegates/delegateSignal.h"
#include "lighting/lightInfo.h"
#include "lighting/lightQuery.h"

class SimObject;
class LightManager;
class Material;
class ProcessedMaterial;
struct SceneData;
class Point3F;
class AvailableSLInterfaces;
class SceneObject;
class GFXShaderConstBuffer;
class GFXShaderConstHandle;
class ShaderConstHandles;
class SceneRenderState;
class RenderPrePassMgr;
class Frustum;
class WorldQueryResult;
///
typedef HashMap<String,LightManager*> LightManagerMap;

typedef Vector<WorldQueryResult> typeWorldQueryResultVector;

class LightManager
{
public:

    LightManager( const char *name, const char *id );
    virtual ~LightManager();

   ///
   static void initLightFields();

   /// 
   static LightInfo* createLightInfo(LightInfo* light = nullptr);

//   ///
//   static LightManager* findByName( const char *name );

//   /// Returns a tab seperated list of available light managers.
//   static void getLightManagerNames( String *outString );

   /// The light manager activation signal.
   static Signal<void(const char*,bool)> smActivateSignal;

   /// Returns the active LM.
   static inline LightManager* getActiveLM() { return smActiveLM; }

   /// Return an id string used to load different versions of light manager
   /// specific assets.  It shoud be short, contain no spaces, and be safe 
   /// for filename use.
   const char* getName() const { return mName.c_str(); }

   /// Return an id string used to load different versions of light manager
   /// specific assets.  It shoud be short, contain no spaces, and be safe 
   /// for filename use.
   const char* getId() const { return mId.c_str(); }

//   // Should return true if this light manager is compatible
//   // on the current platform and GFX device.
//   virtual bool isCompatible() const = 0;

//   // Called when the lighting manager should become active
//   virtual void activate( );
//
//   // Called when we don't want the light manager active (should clean up)
//   virtual void deactivate();

//   // Returns the active scene lighting interface for this light manager.  
//   virtual AvailableSLInterfaces* getSceneLightingInterface();

//   // Returns a "default" light info that callers should not free.  Used for instances where we don't actually care about 
//   // the light (for example, setting default data for SceneData)
//   virtual LightInfo* getDefaultLight();
//
//   /// Returns the special light or the default light if useDefault is true.
//   /// @see getDefaultLight
//   virtual LightInfo* getSpecialLight( SpecialLightTypesEnum type, 
//                                       bool useDefault = true );
//
//   /// Set a special light type.
//   virtual void setSpecialLight( SpecialLightTypesEnum type, LightInfo *light );

//   GFXShaderRef mLastShader;

   // registered before scene traversal...
   virtual void registerGlobalLight( std::shared_ptr<LightInfo> light, SimObject *obj );
   virtual void unregisterGlobalLight( std::shared_ptr<LightInfo> light );

//   // registered per object...
//   virtual void registerLocalLight( LightInfo *light );
//   virtual void unregisterLocalLight( LightInfo *light );

   virtual void registerGlobalLights( typeWorldQueryResultVector queryVector );
   virtual void unregisterAllLights();

   /// Returns all unsorted and un-scored lights (both global and local).
   void getAllUnsortedLights( Vector<std::shared_ptr<LightInfo>> *list ) const;

   void getSortedLightsByDistance( Vector<std::shared_ptr<LightInfo>> *list, Point3F);

   /// Sets shader constants / textures for light infos
   virtual void setLightInfo(ProcessedMaterial *pmat,
           const Material *mat,
           const SceneData &sgData,
           SceneRenderState const *state,
           U32 pass,
           GFXShaderConstBuffer *shaderConsts);

   /// Allows us to set textures during the Material::setTextureStage call, return true if we've done work.
   virtual bool setTextureStage( const SceneData &sgData, 
                                 const U32 currTexFlag, 
                                 const U32 textureSlot, 
                                 GFXShaderConstBuffer *shaderConsts, 
                                 ShaderConstHandles *handles )  { return false; };

//   /// Called when the static scene lighting (aka lightmaps) should be computed.
//   virtual bool lightScene( const char* callback, const char* param );
//
//   /// Returns true if this light manager is active
//   virtual bool isActive() const { return mIsActive; }
   Point3F getSortPoint() { return sortPoint; };
protected:

   /// The current active light manager.
   static LightManager *smActiveLM;

//   /// Find the pre-pass render bin on the scene's default render pass.
//   RenderPrePassMgr* _findPrePassRenderBin();

//   /// This helper function sets the shader constansts
//   /// for the stock 4 light forward lighting code.
//   static void _update4LightConsts( const SceneData &sgData,
//                                    GFXShaderConstHandle *lightPositionSC,
//                                    GFXShaderConstHandle *lightDiffuseSC,
//                                    GFXShaderConstHandle *lightAmbientSC,
//                                    GFXShaderConstHandle *lightInvRadiusSqSC,
//                                    GFXShaderConstHandle *lightSpotDirSC,
//                                    GFXShaderConstHandle *lightSpotAngleSC,
//									GFXShaderConstHandle *lightSpotFalloffSC,
//                                    GFXShaderConstBuffer *shaderConsts );

   /// A dummy default light used when no lights
   /// happen to be registered with the manager.
   LightInfo *mDefaultLight;
  
   /// The list of global registered lights which is
   /// initialized before the scene is rendered.
   LightInfoList mRegisteredLights;

//   /// The registered special light list.
//   LightInfo *mSpecialLights[slSpecialLightTypesCount];

//   /// The root culling position used for
//   /// special sun light placement.
//   /// @see setSpecialLight
//   Point3F mCullPos;

//   /// The scene lighting interfaces for 
//   /// lightmap generation.
//   AvailableSLInterfaces *mAvailableSLInterfaces;

   /// Attaches any LightInfoEx data for this manager 
   /// to the light info object.
    virtual void _addLightInfoEx( LightInfo *lightInfo ) {};

   ///
    virtual void _initLightFields() {};

   /// Returns the static light manager map.
   static LightManagerMap& _getLightManagers();
  
   /// The constant light manager name initialized
   /// in the constructor.
   const String mName;

   /// The constant light manager identifier initialized
   /// in the constructor.
   const String mId;

   /// Is true if this light manager has been activated.
   bool mIsActive;

   Point3F sortPoint;
//   static S32 QSORT_CALLBACK lightDistance(const void* a, const void* b);
    static bool lightDistance(const std::shared_ptr<LightInfo> pLightInfoA, const std::shared_ptr<LightInfo> pLightInfoB);
};

/// Returns the current active light manager.
#define LIGHTMGR LightManager::getActiveLM()

#endif // _LIGHTMANAGER_H_
