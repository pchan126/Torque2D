//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "materials/customMaterialDefinition.h"

#include "materials/materialManager.h"
#include "console/consoleTypes.h"
#include "materials/shaderData.h"
#include "graphics/sim/cubemapAsset.h"
#include "graphics/gfxCubemap.h"
#include "graphics/sim/GFXStateBlockAsset.h"


//****************************************************************************
// Custom Material
//****************************************************************************
IMPLEMENT_CONOBJECT(CustomMaterial);

//ConsoleDocClass( CustomMaterial,
//   "@brief Material object which provides more control over surface properties.\n\n"
//
//   "CustomMaterials allow the user to specify their own shaders via the ShaderData datablock. "
//   "Because CustomMaterials are derived from Materials, they can hold a lot of the same properties. "
//   "It is up to the user to code how these properties are used.\n\n"
//
//   "@tsexample\n"
//   "singleton CustomMaterial( WaterBasicMat )\n"
//   "{\n"
//   "   sampler[\"reflectMap\"] = \"$reflectbuff\";\n"
//   "   sampler[\"refractBuff\"] = \"$backbuff\";\n\n"
//   "   cubemap = NewLevelSkyCubemap;\n"
//   "   shader = WaterBasicShader;\n"
//   "   stateBlock = WaterBasicStateBlock;\n"
//   "   version = 2.0;\n"
//   "};\n"
//   "@endtsexample\n\n"
//
//   "@see Material, GFXStateBlockAsset, ShaderData\n\n"
//
//   "@ingroup Materials\n"
//);

//----------------------------------------------------------------------------
// Constructor
//----------------------------------------------------------------------------
CustomMaterial::CustomMaterial()
{  
   mFallback = nullptr;
   mMaxTex = 0;
   mVersion = 1.1f;
   mTranslucent = false;
   dMemset( mFlags, 0, sizeof( mFlags ) );   
   mShaderData = nullptr;
   mRefract = false;
   mStateBlockData = nullptr;
   mForwardLit = false;
}

//--------------------------------------------------------------------------
// Init fields
//--------------------------------------------------------------------------
void CustomMaterial::initPersistFields()
{
   addField("version",     TypeF32,             Offset(mVersion, CustomMaterial), 
      "@brief Specifies pixel shader version for hardware.\n\n"
      "Valid pixel shader versions include 2.0, 3.0, etc. "
      "@note All features aren't compatible with all pixel shader versions.");
   addField("fallback",    TypeMaterialAssetPtr,    Offset(mFallback,  CustomMaterial),
      "@brief Alternate material for targeting lower end hardware.\n\n"
      "If the CustomMaterial requires a higher pixel shader version than the one "
      "it's using, it's fallback Material will be processed instead. "
      "If the fallback material wasn't defined, Torque 3D will assert and attempt to use a very "
      "basic material in it's place.\n\n");
   addField("shader",      TypeRealString,      Offset(mShaderDataName, CustomMaterial), 
      "@brief Name of the ShaderData to use for this effect.\n\n");
   addField("stateBlock",  TypeGFXStateBlockAssetPtr,    Offset(mStateBlockData,  CustomMaterial),
      "@brief Name of a GFXStateBlockAsset for this effect.\n\n");
   addField("target",      TypeRealString,      Offset(mOutputTarget, CustomMaterial), 
      "@brief String identifier of this material's target texture.");
   addField("forwardLit",  TypeBool,      Offset(mForwardLit, CustomMaterial), 
      "@brief Determines if the material should recieve lights in Basic Lighting. "
      "Has no effect in Advanced Lighting.\n\n");

   Parent::initPersistFields();
}

//--------------------------------------------------------------------------
// On add - verify data settings
//--------------------------------------------------------------------------
bool CustomMaterial::onAdd()
{
   if (Parent::onAdd() == false)
      return false;

   mShaderData = dynamic_cast<ShaderData*>(Sim::findObject( mShaderDataName ) );
   if(mShaderDataName.isNotEmpty() && mShaderData == nullptr)
   {
      Con::errorf("Failed to find ShaderData %s", mShaderDataName.c_str());
      return false;
   }
   
   const char* samplerDecl = "sampler";
   S32 i = 0;
   for (SimFieldDictionary::Iterator itr = getFieldDictionary()->begin(); itr != getFieldDictionary()->end(); ++itr)
   {
   	SimFieldDictionary::Entry* entry = itr->second;
      if (dStrStartsWith(entry->slotName, samplerDecl))
      {
      	if (i >= MAX_TEX_PER_PASS)
         {
            Con::errorf("Too many sampler declarations, you may only have %i", MAX_TEX_PER_PASS);
            return false;
         }
         
         if (dStrlen(entry->slotName) == dStrlen(samplerDecl))
         {
         	Con::errorf("sampler declarations must have a sampler name, e.g. sampler[\"diffuseMap\"]");
            return false;
         }
         
      	mSamplerNames[i] = entry->slotName + dStrlen(samplerDecl);
         mSamplerNames[i].insert(0, '$');
         mTexFilename[i] = entry->value;
         ++i;
      }
   }

   return true;
}

//--------------------------------------------------------------------------
// On remove
//--------------------------------------------------------------------------
void CustomMaterial::onRemove()
{
   Parent::onRemove();
}

//--------------------------------------------------------------------------
// Map this material to the texture specified in the "mapTo" data variable
//--------------------------------------------------------------------------
void CustomMaterial::_mapMaterial()
{
   if( String(getName()).isEmpty() )
   {
      Con::warnf( "Unnamed Material!  Could not map to: %s", mMapTo.c_str() );
      return;
   }

   if( mMapTo.isEmpty() )
      return;

   MATMGR->mapMaterial(mMapTo, getName());
}

const GFXStateBlockAsset* CustomMaterial::getStateBlockData() const
{
   return mStateBlockData;
}
