//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLESStateBlock_H_
#define _GFXOpenGLESStateBlock_H_

#include "graphics/OpenGL/gfxOpenGLStateBlock.h"

class GFXOpenGLESStateBlock : public GFXOpenGLStateBlock
{   
public:
   // 
   // GFXOpenGLESStateBlock interface
   //
   GFXOpenGLESStateBlock(const GFXStateBlockDesc& desc);
   virtual ~GFXOpenGLESStateBlock();

   /// Called by OpenGL device to active this state block.
   /// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
   virtual void activate(const GFXOpenGLESStateBlock* oldState);


   // 
   // GFXStateBlock interface
   //

   /// Returns the hash value of the desc that created this block
   virtual U32 getHashValue() const;

   /// Returns a GFXStateBlockDesc that this block represents
   virtual const GFXStateBlockDesc& getDesc() const;   

   //
   // GFXResource
   //
   virtual void zombify() { }
   /// When called the resource should restore all device sensitive information destroyed by zombify()
   virtual void resurrect() { }
private:
   GFXStateBlockDesc mDesc;
   U32 mCachedHashValue;
};

typedef StrongRefPtr<GFXOpenGLESStateBlock> GFXOpenGLESStateBlockRef;

#endif