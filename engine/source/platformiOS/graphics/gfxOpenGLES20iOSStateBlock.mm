//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformiOS/graphics/GFXOpenGLES20iOSStateBlock.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSDevice.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSEnumTranslate.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSUtils.h"
#include "platformiOS/graphics/gfxOpenGLES20iOSTextureObject.h"


GFXOpenGLES20iOSStateBlock::GFXOpenGLES20iOSStateBlock(const GFXStateBlockDesc& desc) : GFXOpenGLStateBlock( desc ),
   mDesc(desc),
   mCachedHashValue(desc.getHashValue())
{
}

GFXOpenGLES20iOSStateBlock::~GFXOpenGLES20iOSStateBlock()
{
}

/// Returns the hash value of the desc that created this block
U32 GFXOpenGLES20iOSStateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXOpenGLES20iOSStateBlock::getDesc() const
{
   return mDesc;   
}

/// Called by OpenGL device to active this state block.
/// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
void GFXOpenGLES20iOSStateBlock::activate(const GFXOpenGLES20iOSStateBlock* oldState)
{
    GFXOpenGLES20iOSDevice* device = dynamic_cast<GFXOpenGLES20iOSDevice*>(GFX);

    // Blending
   device->setBlending(mDesc.blendEnable);
   device->setBlendFunc(mDesc.blendSrc, mDesc.blendDest);
   device->setBlendEquation(mDesc.blendOp);
   device->setColorMask(mDesc.colorWriteRed, mDesc.colorWriteBlue, mDesc.colorWriteGreen, mDesc.colorWriteAlpha);
   device->setCullMode(mDesc.cullMode);

//   // Depth
//   CHECK_TOGGLE_STATE(zEnable, GL_DEPTH_TEST);
//
//   if(STATE_CHANGE(zFunc))
//      glDepthFunc(GFXGLCmpFunc[mDesc.zFunc]);
//
//   if(STATE_CHANGE(zBias))
//   {
//      if (mDesc.zBias == 0)
//      {
//         glDisable(GL_POLYGON_OFFSET_FILL);
//      } else {
//         F32 bias = mDesc.zBias * 10000.0f;
//         glEnable(GL_POLYGON_OFFSET_FILL);
//         glPolygonOffset(bias, bias);
//      }
//   }
//
//   if(STATE_CHANGE(zWriteEnable))
//      glDepthMask(mDesc.zWriteEnable);
//
//   // Stencil
//   CHECK_TOGGLE_STATE(stencilEnable, GL_STENCIL_TEST);
//   if(STATE_CHANGE(stencilFunc) || STATE_CHANGE(stencilRef) || STATE_CHANGE(stencilMask))
//      glStencilFunc(GFXGLCmpFunc[mDesc.stencilFunc], mDesc.stencilRef, mDesc.stencilMask);
//   if(STATE_CHANGE(stencilFailOp) || STATE_CHANGE(stencilZFailOp) || STATE_CHANGE(stencilPassOp))
//      glStencilOp(GFXGLStencilOp[mDesc.stencilFailOp], GFXGLStencilOp[mDesc.stencilZFailOp], GFXGLStencilOp[mDesc.stencilPassOp]);
//   if(STATE_CHANGE(stencilWriteMask))
//      glStencilMask(mDesc.stencilWriteMask);
}
