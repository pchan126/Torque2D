//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GFXGLESEnumTranslate_H_
#define _GFXGLESEnumTranslate_H_

#import <OpenGLES/ES2/glext.h>
#include "graphics/gfxEnums.h"

namespace GFXGLESEnumTranslate
{
   void init();
};

enum SHADER_ATTRIBUTES{   // order is important
    ATTRIB_POSITION,
    ATTRIB_NORMAL,
    ATTRIB_COLOR,
    ATTRIB_TEXCOORD0,
    ATTRIB_TEXCOORD1,
    ATTRIB_TEXCOORD2,
    ATTRIB_TEXCOORD3,
    ATTRIB_TEXCOORD4,
    ATTRIB_TEXCOORD5,
    ATTRIB_TEXCOORD6,
    ATTRIB_TEXCOORD7,
    ATTRIB_POINTSIZE,
    NUM_ATTRIBUTES };

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
extern StringTableEntry GFXGLShaderAttributes[NUM_ATTRIBUTES];

#endif
