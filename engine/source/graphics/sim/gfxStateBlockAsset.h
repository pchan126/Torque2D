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
#ifndef __GFXStateBlockAsset_H_
#define __GFXStateBlockAsset_H_

#ifndef _GFXSTATEBLOCK_H_
#include "graphics/gfxStateBlock.h"
#endif
#ifndef _ASSET_BASE_H_
#include "assets/assetBase.h"
#endif
#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

class GFXSamplerStateAsset;

extern EnumTable srcBlendFactorTable;
extern EnumTable dstBlendFactorTable;
extern EnumTable blendOpFactorTable;

//-----------------------------------------------------------------------------

DefineConsoleType( TypeGFXStateBlockAssetPtr )
DefineConsoleType( TypeGFXSamplerStateAssetPtr )

//-----------------------------------------------------------------------------


/// Allows definition of render state via script, basically wraps a GFXStateBlockDesc
class GFXStateBlockAsset : public AssetBase
{
   typedef SimObject Parent;

   GFXStateBlockDesc mState;
   AssetPtr<GFXSamplerStateAsset> mSamplerStates[TEXTURE_STAGE_COUNT];
public:
   GFXStateBlockAsset();

   // SimObject
   virtual bool onAdd();
   static void initPersistFields();  

   // GFXStateBlockAsset
   const GFXStateBlockDesc getState() const { return mState; }
   
   static bool             writeSrcBlendFactor( void* obj, StringTableEntry pFieldName ) { return static_cast<GFXStateBlockAsset*>(obj)->getState().blendSrc != GFXBlendOne; }
   static bool             writeDstBlendFactor( void* obj, StringTableEntry pFieldName ) { return static_cast<GFXStateBlockAsset*>(obj)->getState().blendDest != GFXBlendZero; }
   static bool             writeBlendOpFactor( void* obj, StringTableEntry pFieldName ) { return static_cast<GFXStateBlockAsset*>(obj)->getState().blendOp != GFXBlendOpAdd; }

   static bool             writeSepSrcBlendFactor( void* obj, StringTableEntry pFieldName ) { return static_cast<GFXStateBlockAsset*>(obj)->getState().separateAlphaBlendSrc != GFXBlendOne; }
   static bool             writeSepDstBlendFactor( void* obj, StringTableEntry pFieldName ) { return static_cast<GFXStateBlockAsset*>(obj)->getState().separateAlphaBlendDest != GFXBlendZero; }
   static bool             writeSepBlendOpFactor( void* obj, StringTableEntry pFieldName ) { return static_cast<GFXStateBlockAsset*>(obj)->getState().separateAlphaBlendOp != GFXBlendOpAdd; }

   
   DECLARE_CONOBJECT(GFXStateBlockAsset);
};

/// Allows definition of sampler state via script, basically wraps a GFXSamplerStateDesc
class GFXSamplerStateAsset : public AssetBase
{
   typedef SimObject Parent;
   GFXSamplerStateDesc mState;
public:
   // SimObject
   static void initPersistFields();  

   /// Copies the data of this object into desc
   void setSamplerState(GFXSamplerStateDesc& desc);

   DECLARE_CONOBJECT(GFXSamplerStateAsset);
};


#endif // __GFXStateBlockAsset_H_
