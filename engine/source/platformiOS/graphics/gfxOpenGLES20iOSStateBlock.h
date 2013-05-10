//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXOpenGLES20iOSStateBlock_H_
#define _GFXOpenGLES20iOSStateBlock_H_

#include "graphics/OpenGL/gfxOpenGLStateBlock.h"

class GFXOpenGLES20iOSStateBlock : public GFXOpenGLStateBlock
{   
public:
   // 
   // GFXOpenGLES20iOSStateBlock interface
   //
   GFXOpenGLES20iOSStateBlock(const GFXStateBlockDesc& desc);
   virtual ~GFXOpenGLES20iOSStateBlock();

   /// Called by OpenGL device to active this state block.
   /// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
   virtual void activate(const GFXOpenGLES20iOSStateBlock* oldState);


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

typedef StrongRefPtr<GFXOpenGLES20iOSStateBlock> GFXOpenGLES20iOSStateBlockRef;

#endif