//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLAPPLEFENCE_H_
#define _GFXGLAPPLEFENCE_H_

#include "gfx/gfxFence.h"
#import <OpenGLES/ES2/glext.h>

class GFXGLESAppleFence : public GFXFence
{
public:
   GFXGLESAppleFence(GFXDevice* device);
   virtual ~GFXGLESAppleFence();
   
   // GFXFence interface
   virtual void issue();
   virtual FenceStatus getStatus() const;
   virtual void block();
   
   // GFXResource interface
   virtual void zombify();
   virtual void resurrect();
   virtual const String describeSelf() const;
   
private:
   GLuint mHandle;
   bool mIssued;
};

#endif