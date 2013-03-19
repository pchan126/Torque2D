//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "gfx/gles/gfxGLESAppleFence.h"

GFXGLESAppleFence::GFXGLESAppleFence(GFXDevice* device) : GFXFence(device), mIssued(false)
{
//   glGenFencesAPPLE(1, &mHandle);
}

GFXGLESAppleFence::~GFXGLESAppleFence()
{
//   glDeleteFencesAPPLE(1, &mHandle);
}

void GFXGLESAppleFence::issue()
{
//   glSetFenceAPPLE(mHandle);
   mIssued = true;
}

GFXFence::FenceStatus GFXGLESAppleFence::getStatus() const
{
   if(!mIssued)
      return GFXFence::Unset;
      
//   GLboolean res = glTestFenceAPPLE(mHandle);
//   return res ? GFXFence::Processed : GFXFence::Pending;
    return GFXFence::Unsupported;
}

void GFXGLESAppleFence::block()
{
   if(!mIssued)
      return;
      
//   glFinishFenceAPPLE(mHandle);
}

void GFXGLESAppleFence::zombify()
{
//   glDeleteFencesAPPLE(1, &mHandle);
}

void GFXGLESAppleFence::resurrect()
{
//   glGenFencesAPPLE(1, &mHandle);
}

const String GFXGLESAppleFence::describeSelf() const
{
   return String::ToString("   GL Handle: %i", mHandle);
}
