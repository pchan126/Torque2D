//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformiOS/graphics/gfxOpenGLESStateBlock.h"
#include "platformiOS/graphics/gfxOpenGLESDevice.h"
#include "platformiOS/graphics/gfxOpenGLESEnumTranslate.h"
#include "platformiOS/graphics/gfxOpenGLESUtils.h"
#include "platformiOS/graphics/gfxOpenGLESTextureObject.h"


GFXOpenGLESStateBlock::GFXOpenGLESStateBlock(const GFXStateBlockDesc& desc) : GFXOpenGLStateBlock( desc ),
   mDesc(desc),
   mCachedHashValue(desc.getHashValue())
{
}

GFXOpenGLESStateBlock::~GFXOpenGLESStateBlock()
{
}

/// Returns the hash value of the desc that created this block
U32 GFXOpenGLESStateBlock::getHashValue() const
{
   return mCachedHashValue;
}

/// Returns a GFXStateBlockDesc that this block represents
const GFXStateBlockDesc& GFXOpenGLESStateBlock::getDesc() const
{
   return mDesc;   
}

/// Called by OpenGL device to active this state block.
/// @param oldState  The current state, used to make sure we don't set redundant states on the device.  Pass NULL to reset all states.
void GFXOpenGLESStateBlock::activate(const GFXOpenGLESStateBlock* oldState)
{
   // Big scary warning copied from Apple docs 
   // http://developer.apple.com/documentation/GraphicsImaging/Conceptual/OpenGL-MacProgGuide/opengl_performance/chapter_13_section_2.html#//apple_ref/doc/uid/TP40001987-CH213-SW12
   // Don't set a state that's already set. Once a feature is enabled, it does not need to be enabled again.
   // Calling an enable function more than once does nothing except waste time because OpenGL does not check 
   // the state of a feature when you call glEnable or glDisable. For instance, if you call glEnable(GL_LIGHTING) 
   // more than once, OpenGL does not check to see if the lighting state is already enabled. It simply updates 
   // the state value even if that value is identical to the current value.

    // or like a sane person use glIsEnabled to check the state first.
    
#define STATE_CHANGE(state) (!oldState || oldState->mDesc.state != mDesc.state)
#define TOGGLE_STATE(state, enum) if(mDesc.state) glEnable(enum); else glDisable(enum)
#define CHECK_TOGGLE_STATE(state, enum) if(!oldState || oldState->mDesc.state != mDesc.state) { if(mDesc.state) { glEnable(enum); } else { glDisable(enum); } }

    GFXOpenGLESDevice* device = dynamic_cast<GFXOpenGLESDevice*>(GFX);

    // Blending
//   CHECK_TOGGLE_STATE(blendEnable, GL_BLEND);
    if ((glIsEnabled(GL_BLEND) == GL_TRUE) && !mDesc.blendEnable) glDisable(GL_BLEND);
    if ((glIsEnabled(GL_BLEND) == GL_FALSE) && mDesc.blendEnable) glEnable(GL_BLEND);
    
   if(STATE_CHANGE(blendSrc) || STATE_CHANGE(blendDest))
      glBlendFunc(GFXGLBlend[mDesc.blendSrc], GFXGLBlend[mDesc.blendDest]);
   if(STATE_CHANGE(blendOp))
      glBlendEquation(GFXGLBlendOp[mDesc.blendOp]);

   // Alpha testing
//   CHECK_TOGGLE_STATE(alphaTestEnable, GL_ALPHA_TEST);      
//   if(STATE_CHANGE(alphaTestFunc) || STATE_CHANGE(alphaTestRef))
//      glAlphaFunc(GFXGLCmpFunc[mDesc.alphaTestFunc], (F32) mDesc.alphaTestRef * 1.0f/255.0f);

   // Color write masks
   if(STATE_CHANGE(colorWriteRed) || STATE_CHANGE(colorWriteBlue) || STATE_CHANGE(colorWriteGreen) || STATE_CHANGE(colorWriteAlpha))
      glColorMask(mDesc.colorWriteRed, mDesc.colorWriteBlue, mDesc.colorWriteGreen, mDesc.colorWriteAlpha);
   
    // Culling
    device->setCullMode(mDesc.cullMode);

   // Depth
   CHECK_TOGGLE_STATE(zEnable, GL_DEPTH_TEST);
   
   if(STATE_CHANGE(zFunc))
      glDepthFunc(GFXGLCmpFunc[mDesc.zFunc]);
   
   if(STATE_CHANGE(zBias))
   {
      if (mDesc.zBias == 0)
      {
         glDisable(GL_POLYGON_OFFSET_FILL);
      } else {
         F32 bias = mDesc.zBias * 10000.0f;
         glEnable(GL_POLYGON_OFFSET_FILL);
         glPolygonOffset(bias, bias);
      } 
   }
   
   if(STATE_CHANGE(zWriteEnable))
      glDepthMask(mDesc.zWriteEnable);

   // Stencil
   CHECK_TOGGLE_STATE(stencilEnable, GL_STENCIL_TEST);
   if(STATE_CHANGE(stencilFunc) || STATE_CHANGE(stencilRef) || STATE_CHANGE(stencilMask))
      glStencilFunc(GFXGLCmpFunc[mDesc.stencilFunc], mDesc.stencilRef, mDesc.stencilMask);
   if(STATE_CHANGE(stencilFailOp) || STATE_CHANGE(stencilZFailOp) || STATE_CHANGE(stencilPassOp))
      glStencilOp(GFXGLStencilOp[mDesc.stencilFailOp], GFXGLStencilOp[mDesc.stencilZFailOp], GFXGLStencilOp[mDesc.stencilPassOp]);
   if(STATE_CHANGE(stencilWriteMask))
      glStencilMask(mDesc.stencilWriteMask);

//   // "Misc"
//   CHECK_TOGGLE_STATE(ffLighting, GL_LIGHTING);
//
//   glColorMaterial(GL_FRONT_AND_BACK, GL_AMBIENT_AND_DIFFUSE);
//   CHECK_TOGGLE_STATE(vertexColorEnable, GL_COLOR_MATERIAL);

//   if(STATE_CHANGE(fillMode))
//      glPolygonMode(GL_FRONT_AND_BACK, GFXGLFillMode[mDesc.fillMode]);

#undef CHECK_STATE
#undef TOGGLE_STATE
#undef CHECK_TOGGLE_STATE

   // TODO: states added for detail blend

   // Non per object texture mode states
   for (U32 i = 0; i < getMin(getOwningDevice()->getNumSamplers(), (U32) TEXTURE_STAGE_COUNT); i++)
   {
      GFXOpenGLESTextureObject* tex = static_cast<GFXOpenGLESTextureObject*>(getOwningDevice()->getCurrentTexture(i));
      const GFXSamplerStateDesc &ssd = mDesc.samplers[i];
      bool updateTexParam = true;
      glActiveTexture(GL_TEXTURE0 + i);
      switch (ssd.textureColorOp)
      {
      case GFXTOPDisable :
         if(!tex)
            break;
//         glDisable(GL_TEXTURE_2D);
         updateTexParam = false;
         break;
      case GFXTOPModulate :
//         glEnable(GL_TEXTURE_2D);
//         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
         break;
      case GFXTOPAdd :
//         glEnable(GL_TEXTURE_2D);
//         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_ADD);
         break;
      default :
//         glEnable(GL_TEXTURE_2D);
//         glTexEnvi(GL_TEXTURE_ENV, GL_TEXTURE_ENV_MODE, GL_MODULATE);
         break;
      }

#define SSF(state, enum, value, tex) if(!oldState || oldState->mDesc.samplers[i].state != mDesc.samplers[i].state) glTexParameteri(tex->getBinding(), enum, value)
#define SSW(state, enum, value, tex) if(!oldState || oldState->mDesc.samplers[i].state != mDesc.samplers[i].state) glTexParameteri(tex->getBinding(), enum, !tex->mIsNPoT2 ? value : GL_CLAMP_TO_EDGE)
      // Per object texture mode states. 
      // TODO: Check dirty flag of samplers[i] and don't do this if it's dirty (it'll happen in the texture bind)
      if (updateTexParam && tex)
      {
         SSF(minFilter, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, tex->mMipLevels), tex);
         SSF(mipFilter, GL_TEXTURE_MIN_FILTER, minificationFilter(ssd.minFilter, ssd.mipFilter, tex->mMipLevels), tex);
//         SSF(magFilter, GL_TEXTURE_MAG_FILTER, GFXGLTextureFilter[ssd.magFilter], tex);
//         SSW(addressModeU, GL_TEXTURE_WRAP_S, GFXGLTextureAddress[ssd.addressModeU], tex);
//         SSW(addressModeV, GL_TEXTURE_WRAP_T, GFXGLTextureAddress[ssd.addressModeV], tex);

         if( ( !oldState || oldState->mDesc.samplers[i].maxAnisotropy != ssd.maxAnisotropy ) &&
             static_cast< GFXOpenGLESDevice* >( GFX )->supportsAnisotropic() )
            glTexParameterf(GL_TEXTURE_2D, GL_TEXTURE_MAX_ANISOTROPY_EXT, ssd.maxAnisotropy);

//         if( ( !oldState || oldState->mDesc.samplers[i].mipLODBias != ssd.mipLODBias ) )
//            glTexEnvf(GL_TEXTURE_FILTER_CONTROL_EXT, GL_TEXTURE_LOD_BIAS_EXT, ssd.mipLODBias);
      }
   }
#undef SSF
#undef SSW
}
