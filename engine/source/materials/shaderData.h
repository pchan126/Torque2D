//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------
#ifndef _SHADERTDATA_H_
#define _SHADERTDATA_H_

#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif
#ifndef _GFXSHADER_H_
#include "graphics/gfxShader.h"
#endif
#ifndef _HASHTABLE_H
#include "collection/hashTable.h"
#endif

class GFXShader;
class ShaderData;
struct GFXShaderMacro;


///
class ShaderData : public AssetBase
{
   typedef AssetBase Parent;

protected:

   ///
   static Vector<ShaderData*> smAllShaderData;

   typedef HashTable<String,GFXShaderRef> ShaderCache;

   ShaderCache mShaders;

   bool mUseDevicePixVersion;

   F32 mPixVersion;

//   FileName mDXVertexShaderName;
//
//   FileName mDXPixelShaderName;

   StringTableEntry mOGLVertexShaderName;

   StringTableEntry mOGLPixelShaderName;

   /// A semicolon, tab, or newline delimited string of case
   /// sensitive defines that are passed to the shader compiler.
   ///
   /// For example:
   ///
   /// SAMPLE_TAPS=10;USE_TEXKILL;USE_TORQUE_FOG=1
   ///
   String mDefines;

   /// The shader macros built from mDefines.
   /// @see _getMacros()
   Vector<GFXShaderMacro> mShaderMacros;

   /// Returns the shader macros taking care to rebuild
   /// them if the content has changed.
   const Vector<GFXShaderMacro>& _getMacros();

   /// Helper for converting an array of macros 
   /// into a formatted string.
   void _stringizeMacros(  const Vector<GFXShaderMacro> &macros, 
                           String *outString );

   /// Creates a new shader returning NULL on error.
   GFXShader* _createShader( const Vector<GFXShaderMacro> &macros );

   /// @see LightManager::smActivateSignal
   static void _onLMActivate( const char *lm, bool activate );

public:


   ShaderData();

   /// Returns an initialized shader instance or NULL 
   /// if the shader failed to be created.
   GFXShader* getShader( const Vector<GFXShaderMacro> &macros = Vector<GFXShaderMacro>() );

   /// Forces a reinitialization of all the instanced shaders.
   void reloadShaders();

   /// Forces a reinitialization of the instanced shaders for
   /// all loaded ShaderData objects in the system.
   static void reloadAllShaders();

   /// Returns the required pixel shader version for this shader.
   F32 getPixVersion() const { return mPixVersion; }
   
   // SimObject
   virtual bool onAdd();
   virtual void onRemove();

   // ConsoleObject
   static void initPersistFields();
   DECLARE_CONOBJECT(ShaderData);
};

#endif // _SHADERTDATA_H_