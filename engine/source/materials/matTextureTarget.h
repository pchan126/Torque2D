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

#ifndef _MATTEXTURETARGET_H_
#define _MATTEXTURETARGET_H_

#include <memory>
class NamedTexTarget;
typedef std::shared_ptr<NamedTexTarget> NamedTexTargetRef;

#include "collection/hashTable.h"
#include "sim/refBase.h"
#include "graphics/gfxTextureObject.h"
#include "math/mRect.h"
#include "graphics/gfxStateBlock.h"
#include "delegates/delegate.h"

struct GFXShaderMacro;
//class ConditionerFeature;

///
class NamedTexTarget: public std::enable_shared_from_this<NamedTexTarget>
{
public:
   
   ///
   static NamedTexTargetRef find(const String &name);

   ///
   NamedTexTarget();

   ///
   virtual ~NamedTexTarget();
   
   ///
   bool registerWithName( const String &name );

   ///
   void unregister();

   ///
   bool isRegistered() const { return mIsRegistered; }

   /// Returns the target name we were registered with.
   const String& getName() const { return mName; }

   // Register the passed texture with our name, unregistering "anyone"
   // priorly registered with that name.   
   // Pass NULL to only unregister.
   void setTexture(GFXTexHandle &tex) { setTexture( 0, tex ); }

   ///
   void setTexture(U32 index, GFXTexHandle &tex);

   ///
   GFXTexHandle getTexture( U32 index = 0 ) const;

   /// The delegate used to override the getTexture method.
   /// @see getTexture
   typedef Delegate<GFXTextureObject*(U32)> TexDelegate;

   /// 
   /// @see getTexture
   TexDelegate& getTextureDelegate() { return mTexDelegate; }
   const TexDelegate& getTextureDelegate() const { return mTexDelegate; }

   /// Release all the textures.
   void release();

   // NOTE:
   //
   // The following members are here to support the existing conditioner
   // and target system used for the deferred gbuffer and lighting.
   //
   // We will refactor that system as part of material2 removing the concept
   // of conditioners from C++ (moving them to HLSL/GLSL) and make the shader
   // features which use the texture responsible for setting the correct sampler
   // states.
   //
   // It could be that at this time this class could completely
   // be removed and instead these textures can be registered
   // with the TEXMGR and looked up there exclusively.
   //
   void setViewport( const RectI &viewport ) { mViewport = viewport; }
   const RectI& getViewport() const { return mViewport; }
   void setSamplerState( const GFXSamplerStateDesc &desc ) { mSamplerDesc = desc; }
   void setupSamplerState( GFXSamplerStateDesc *desc ) const  { *desc = mSamplerDesc; }
//   void setConditioner( ConditionerFeature *cond ) { mConditioner = cond; }
//   ConditionerFeature* getConditioner() const { return mConditioner; }
   void getShaderMacros( Vector<GFXShaderMacro> *outMacros );

protected:
   
   typedef HashMap<String,NamedTexTargetRef> TargetMap;

   ///
   static TargetMap smTargets;

   ///
   bool mIsRegistered;

   /// The target name we were registered with.
   String mName;

   /// The held textures.
   GFXTexHandle mTex[4];

   ///
   TexDelegate mTexDelegate;

   ///
   RectI mViewport; 

   ///
   GFXSamplerStateDesc mSamplerDesc;

   ///
//   ConditionerFeature *mConditioner;
};


inline GFXTexHandle NamedTexTarget::getTexture( U32 index ) const
{
   AssertFatal( index < 4, "NamedTexTarget::getTexture - Got invalid index!" );
   if ( mTexDelegate.empty() )
      return mTex[index];

   return GFXTexHandle(mTexDelegate( index ));
}

#endif // _MATTEXTURETARGET_H_
