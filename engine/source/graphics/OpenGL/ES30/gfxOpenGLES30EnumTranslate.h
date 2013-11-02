//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLES30EnumTranslate_H_
#define _GFXGLES30EnumTranslate_H_

#include "platform/platformGL.h"
#include "graphics/gfxEnums.h"

namespace GFXGLES30EnumTranslate
{
   void init();
};

extern GLenum GFXGLPrimType[GFXPT_COUNT];
extern GLenum GFXGLBlend[GFXBlend_COUNT];
extern GLenum GFXGLBlendOp[GFXBlendOp_COUNT];
extern GLenum GFXGLSamplerState[GFXSAMP_COUNT];
extern GLenum GFXGLTextureFilter[GFXTextureFilter_COUNT];
extern GLenum GFXGLTextureAddress[GFXAddress_COUNT];
extern GLenum GFXGLCmpFunc[GFXCmp_COUNT];
extern GLenum GFXGLStencilOp[GFXStencilOp_COUNT];

extern GLenum GFXGLTextureInternalFormat[GFXFormat_COUNT];
extern GLenum GFXGLTextureFormat[GFXFormat_COUNT];
extern GLenum GFXGLTextureType[GFXFormat_COUNT];

extern GLenum GFXGLBufferType[GFXBufferType_COUNT];
extern GLenum GFXGLCullMode[GFXCull_COUNT];

extern GLenum GFXGLFillMode[GFXFill_COUNT];
extern StringTableEntry GFXGLShaderAttributes[GFXAttribute_Count];

#endif
