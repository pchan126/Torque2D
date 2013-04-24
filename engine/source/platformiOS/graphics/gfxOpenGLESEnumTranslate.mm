//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "platformiOS/graphics/gfxOpenGLESEnumTranslate.h"
#include "String/StringTable.h"
 
GLenum GFXGLPrimType[GFXPT_COUNT];
GLenum GFXGLBlend[GFXBlend_COUNT];
GLenum GFXGLBlendOp[GFXBlendOp_COUNT];
GLenum GFXGLSamplerState[GFXSAMP_COUNT];
GLenum GFXGLTextureFilter[GFXTextureFilter_COUNT];
GLenum GFXGLTextureAddress[GFXAddress_COUNT];
GLenum GFXGLCmpFunc[GFXCmp_COUNT];
GLenum GFXGLStencilOp[GFXStencilOp_COUNT];
GLenum GFXGLTextureInternalFormat[GFXFormat_COUNT];
GLenum GFXGLTextureFormat[GFXFormat_COUNT];
GLenum GFXGLTextureType[GFXFormat_COUNT];
GLenum GFXGLBufferType[GFXBufferType_COUNT];
GLenum GFXGLCullMode[GFXCull_COUNT];
GLenum GFXGLFillMode[GFXFill_COUNT];
StringTableEntry GFXGLShaderAttributes[NUM_ATTRIBUTES];

void GFXGLESEnumTranslate::init()
{
   // Buffer types
   GFXGLBufferType[GFXBufferTypeStatic] = GL_STATIC_DRAW;
   GFXGLBufferType[GFXBufferTypeDynamic] = GL_DYNAMIC_DRAW;
   GFXGLBufferType[GFXBufferTypeVolatile] = GL_STREAM_DRAW;

   // Primitives
   GFXGLPrimType[GFXPointList] = GL_POINTS;
   GFXGLPrimType[GFXLineList] = GL_LINES;
   GFXGLPrimType[GFXLineStrip] = GL_LINE_STRIP;
   GFXGLPrimType[GFXTriangleList] = GL_TRIANGLES;
   GFXGLPrimType[GFXTriangleStrip] = GL_TRIANGLE_STRIP;
   GFXGLPrimType[GFXTriangleFan] = GL_TRIANGLE_FAN;

   // Blend
   GFXGLBlend[GFXBlendZero] = GL_ZERO;
   GFXGLBlend[GFXBlendOne] = GL_ONE;
   GFXGLBlend[GFXBlendSrcColor] = GL_SRC_COLOR;
   GFXGLBlend[GFXBlendInvSrcColor] = GL_ONE_MINUS_SRC_COLOR;
   GFXGLBlend[GFXBlendSrcAlpha] = GL_SRC_ALPHA;
   GFXGLBlend[GFXBlendInvSrcAlpha] = GL_ONE_MINUS_SRC_ALPHA;
   GFXGLBlend[GFXBlendDestAlpha] = GL_DST_ALPHA;
   GFXGLBlend[GFXBlendInvDestAlpha] = GL_ONE_MINUS_DST_ALPHA;
   GFXGLBlend[GFXBlendDestColor] = GL_DST_COLOR;
   GFXGLBlend[GFXBlendInvDestColor] = GL_ONE_MINUS_DST_COLOR;
   GFXGLBlend[GFXBlendSrcAlphaSat] = GL_SRC_ALPHA_SATURATE;
   
   // Blend op
   GFXGLBlendOp[GFXBlendOpAdd] = GL_FUNC_ADD;
   GFXGLBlendOp[GFXBlendOpSubtract] = GL_FUNC_SUBTRACT;
   GFXGLBlendOp[GFXBlendOpRevSubtract] = GL_FUNC_REVERSE_SUBTRACT;
    GFXGLBlendOp[GFXBlendOpMin] = GL_NONE;
    GFXGLBlendOp[GFXBlendOpMax] = GL_NONE;

   // Sampler
   GFXGLSamplerState[GFXSAMPMagFilter] = GL_TEXTURE_MAG_FILTER;
   GFXGLSamplerState[GFXSAMPMinFilter] = GL_TEXTURE_MIN_FILTER;
   GFXGLSamplerState[GFXSAMPAddressU] = GL_TEXTURE_WRAP_S;
   GFXGLSamplerState[GFXSAMPAddressV] = GL_TEXTURE_WRAP_T;
   GFXGLSamplerState[GFXSAMPAddressW] = GL_ZERO;
   GFXGLSamplerState[GFXSAMPMipMapLODBias] = GL_ZERO;
   
   // Comparison
   GFXGLCmpFunc[GFXCmpNever] = GL_NEVER;
   GFXGLCmpFunc[GFXCmpLess] = GL_LESS;
   GFXGLCmpFunc[GFXCmpEqual] = GL_EQUAL;
   GFXGLCmpFunc[GFXCmpLessEqual] = GL_LEQUAL;
   GFXGLCmpFunc[GFXCmpGreater] = GL_GREATER;
   GFXGLCmpFunc[GFXCmpNotEqual] = GL_NOTEQUAL;
   GFXGLCmpFunc[GFXCmpGreaterEqual] = GL_GEQUAL;
   GFXGLCmpFunc[GFXCmpAlways] = GL_ALWAYS;

   GFXGLTextureFilter[GFXTextureFilterNone] = GL_NEAREST;
   GFXGLTextureFilter[GFXTextureFilterPoint] = GL_NEAREST;
   GFXGLTextureFilter[GFXTextureFilterLinear] = GL_LINEAR;

   GFXGLTextureFilter[GFXTextureFilterAnisotropic] = GL_LINEAR;
   GFXGLTextureFilter[GFXTextureFilterPyramidalQuad] = GL_LINEAR; 
   GFXGLTextureFilter[GFXTextureFilterGaussianQuad] = GL_LINEAR;

   GFXGLTextureAddress[GFXAddressWrap] = GL_REPEAT;
   GFXGLTextureAddress[GFXAddressMirror] = GL_REPEAT;
   GFXGLTextureAddress[GFXAddressClamp] = GL_CLAMP_TO_EDGE;
   GFXGLTextureAddress[GFXAddressBorder] = GL_REPEAT;
   GFXGLTextureAddress[GFXAddressMirrorOnce] = GL_REPEAT;
   
   // Stencil ops
   GFXGLStencilOp[GFXStencilOpKeep] = GL_KEEP;
   GFXGLStencilOp[GFXStencilOpZero] = GL_ZERO;
   GFXGLStencilOp[GFXStencilOpReplace] = GL_REPLACE;
   GFXGLStencilOp[GFXStencilOpIncrSat] = GL_INCR;
   GFXGLStencilOp[GFXStencilOpDecrSat] = GL_DECR;
   GFXGLStencilOp[GFXStencilOpInvert] = GL_INVERT;
   
   GFXGLStencilOp[GFXStencilOpIncr] = GL_INCR_WRAP;
   GFXGLStencilOp[GFXStencilOpDecr] = GL_DECR_WRAP;
   
   
   // Texture formats
   GFXGLTextureInternalFormat[GFXFormatA8] = GL_ALPHA;
   GFXGLTextureInternalFormat[GFXFormatL8] = GL_LUMINANCE;
   GFXGLTextureInternalFormat[GFXFormatR8] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR5G6B5] = GL_RGB;
   GFXGLTextureInternalFormat[GFXFormatR5G5B5A1] = GL_RGBA;
   GFXGLTextureInternalFormat[GFXFormatR5G5B5X1] = GL_RGBA;
   GFXGLTextureInternalFormat[GFXFormatL16] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR16F] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatD16] = GL_DEPTH_COMPONENT;
   GFXGLTextureInternalFormat[GFXFormatR8G8] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR8G8B8] = GL_RGB;
   GFXGLTextureInternalFormat[GFXFormatL8A8] = GL_LUMINANCE_ALPHA;
   GFXGLTextureInternalFormat[GFXFormatR8G8B8A8] = GL_RGBA;
   GFXGLTextureInternalFormat[GFXFormatR8G8B8X8] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR32F] = GL_ZERO; 
   GFXGLTextureInternalFormat[GFXFormatR16G16] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR16G16F] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR10G10B10A2] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatD32] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatD24X8] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatD24S8] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR16G16B16A16] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR16G16B16A16F] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR32G32F] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatR32G32B32A32F] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatDXT1] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatDXT2] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatDXT3] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatDXT4] = GL_ZERO;
   GFXGLTextureInternalFormat[GFXFormatDXT5] = GL_ZERO;
   
   GFXGLTextureFormat[GFXFormatA8] = GL_ALPHA;
   GFXGLTextureFormat[GFXFormatL8] = GL_LUMINANCE;
    GFXGLTextureFormat[GFXFormatR8] = GL_ZERO;
   GFXGLTextureFormat[GFXFormatR5G6B5] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatR5G5B5A1] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatR5G5B5X1] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatL16] = GL_LUMINANCE;
   GFXGLTextureFormat[GFXFormatR16F] = GL_ZERO;
   GFXGLTextureFormat[GFXFormatD16] = GL_DEPTH_COMPONENT;
    GFXGLTextureFormat[GFXFormatR8G8] = GL_ZERO;
   GFXGLTextureFormat[GFXFormatR8G8B8] = GL_RGB;
   GFXGLTextureFormat[GFXFormatR8G8B8A8] = GL_BGRA;
   GFXGLTextureFormat[GFXFormatR8G8B8X8] = GL_BGRA;
   GFXGLTextureFormat[GFXFormatR32F] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatR16G16] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatR16G16F] = GL_ZERO;
   GFXGLTextureFormat[GFXFormatR10G10B10A2] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatD32] = GL_DEPTH_COMPONENT;
   GFXGLTextureFormat[GFXFormatD24X8] = GL_DEPTH_COMPONENT;
   GFXGLTextureFormat[GFXFormatD24S8] = GL_DEPTH_COMPONENT;
   GFXGLTextureFormat[GFXFormatR16G16B16A16] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatR16G16B16A16F] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatR32G32B32A32F] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatDXT1] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatDXT2] = GL_ZERO;
   GFXGLTextureFormat[GFXFormatDXT3] = GL_RGBA;
   GFXGLTextureFormat[GFXFormatDXT4] = GL_ZERO;
   GFXGLTextureFormat[GFXFormatDXT5] = GL_RGBA;
   
   GFXGLTextureType[GFXFormatA8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatL8] = GL_UNSIGNED_BYTE;
    GFXGLTextureType[GFXFormatR8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR5G6B5] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR5G5B5A1] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR5G5B5X1] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatL16] = GL_UNSIGNED_SHORT;
   GFXGLTextureType[GFXFormatR16F] = GL_ZERO;
   GFXGLTextureType[GFXFormatD16] = GL_UNSIGNED_SHORT;
    GFXGLTextureType[GFXFormatR8G8] = GL_ZERO;
   GFXGLTextureType[GFXFormatR8G8B8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR8G8B8A8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR8G8B8X8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR32F] = GL_ZERO;
   GFXGLTextureType[GFXFormatR16G16] = GL_UNSIGNED_SHORT;
   GFXGLTextureType[GFXFormatR16G16F] = GL_ZERO;
   GFXGLTextureType[GFXFormatR10G10B10A2] = GL_UNSIGNED_SHORT;
   GFXGLTextureType[GFXFormatD32] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatD24X8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatD24S8] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatR16G16B16A16] = GL_UNSIGNED_SHORT;
   GFXGLTextureType[GFXFormatR16G16B16A16F] = GL_ZERO;
   GFXGLTextureType[GFXFormatR32G32B32A32F] = GL_ZERO;
   GFXGLTextureType[GFXFormatDXT1] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatDXT2] = GL_ZERO;
   GFXGLTextureType[GFXFormatDXT3] = GL_UNSIGNED_BYTE;
   GFXGLTextureType[GFXFormatDXT4] = GL_ZERO;
   GFXGLTextureType[GFXFormatDXT5] = GL_UNSIGNED_BYTE;

   // Cull
   GFXGLCullMode[GFXCullNone] = GL_BACK;
   GFXGLCullMode[GFXCullCW] = GL_BACK;
   GFXGLCullMode[GFXCullCCW] = GL_FRONT;
   
   // Fill
   GFXGLFillMode[GFXFillPoint] = GL_ZERO;
   GFXGLFillMode[GFXFillWireframe] = GL_ZERO;
   GFXGLFillMode[GFXFillSolid] = GL_ZERO;
    
    GFXGLShaderAttributes[ATTRIB_POSITION] = StringTable->insert("Position");
    GFXGLShaderAttributes[ATTRIB_NORMAL] = StringTable->insert("Normal");
    GFXGLShaderAttributes[ATTRIB_COLOR] = StringTable->insert("SourceColor");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD0] = StringTable->insert("inTexCoord");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD1] = StringTable->insert("inTexCoord2");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD2] = StringTable->insert("inTexCoord3");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD3] = StringTable->insert("inTexCoord4");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD4] = StringTable->insert("inTexCoord5");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD5] = StringTable->insert("inTexCoord6");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD6] = StringTable->insert("inTexCoord7");
    GFXGLShaderAttributes[ATTRIB_TEXCOORD7] = StringTable->insert("inTexCoord8");
}
