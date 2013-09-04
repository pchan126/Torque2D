#include "./gfxEnums.h"
#include "./gfxStringEnumTranslate.h"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

EnumTable srcBlendFactorTable =
        {
                { GFXBlendZero,                  "ZERO"                  },
                { GFXBlendOne,                   "ONE"                   },
                { GFXBlendDestColor,             "DST_COLOR"             },
                { GFXBlendInvDestAlpha,   "ONE_MINUS_DST_COLOR"   },
                { GFXBlendSrcAlpha,             "SRC_ALPHA"             },
                { GFXBlendInvSrcColor,   "ONE_MINUS_SRC_ALPHA"   },
                { GFXBlendDestAlpha,             "DST_ALPHA"             },
                { GFXBlendInvDestAlpha,   "ONE_MINUS_DST_ALPHA"   },
                { GFXBlendSrcAlphaSat,    "SRC_ALPHA_SATURATE"    },
        };

//-----------------------------------------------------------------------------

EnumTable dstBlendFactorTable =
        {
                { GFXBlendZero,                  "ZERO" },
                { GFXBlendOne,                   "ONE" },
                { GFXBlendSrcColor,             "SRC_COLOR" },
                { GFXBlendInvSrcColor,   "ONE_MINUS_SRC_COLOR" },
                { GFXBlendSrcAlpha,             "SRC_ALPHA" },
                { GFXBlendInvSrcAlpha,   "ONE_MINUS_SRC_ALPHA" },
                { GFXBlendDestAlpha,             "DST_ALPHA" },
                { GFXBlendInvDestAlpha,   "ONE_MINUS_DST_ALPHA" },
        };

EnumTable blendOpFactorTable =
{
   { GFXBlendOpAdd,                  "ADD" },
   { GFXBlendOpSubtract,                   "SUBTRACT" },
   { GFXBlendOpRevSubtract,             "REV_SUBTRACT" },
   { GFXBlendOpMin,   "MIN" },
   { GFXBlendOpMax,             "MAX" },
};

EnumTable cmpFactorTable =
{
   { GFXCmpNever,                  "NEVER" },
   { GFXCmpLess,                   "LESS" },
   { GFXCmpEqual,             "EQUAL" },
   { GFXCmpLessEqual,   "LESS_EQUAL" },
   { GFXCmpGreater,             "GREATER" },
   { GFXCmpNotEqual,             "NOT_EQUAL" },
   { GFXCmpGreaterEqual,             "GREATER_EQUAL" },
   { GFXCmpAlways,             "ALWAYS" },
};

EnumTable cullModeTable =
{
   { GFXCullNone,                 "NONE" },
   { GFXCullCW,                   "CW" },
   { GFXCullCCW,                  "CWW" },
};

EnumTable stencilOpTable =
{
   { GFXStencilOpKeep,              "KEEP" },
   { GFXStencilOpZero,              "ZERO" },
   { GFXStencilOpReplace,           "REPLACE" },
   { GFXStencilOpIncrSat,           "INCRSAT" },
   { GFXStencilOpDecrSat,           "DECRSAT" },
   { GFXStencilOpInvert,            "INVERT" },
   { GFXStencilOpIncr,              "INCR" },
   { GFXStencilOpDecr,              "DECR" },
};


EnumTable GFXTextureOpTable =
{
   { GFXTOPDisable,                 "DISABLE" },
   { GFXTOPSelectARG1,              "SELECTARG1" },
   { GFXTOPSelectARG2,              "SELECTARG2" },
   { GFXTOPModulate,                "MODULATE" },
   { GFXTOPModulate2X,              "MODULATE2X" },
   { GFXTOPModulate4X,              "MODULATE4X" },
   { GFXTOPAdd,                     "ADD" },
   { GFXTOPAddSigned,               "ADDSIGNED" },
   { GFXTOPAddSigned2X,             "ADDSIGNED2X" },
   { GFXTOPSubtract,                "SUBTRACT" },
   { GFXTOPAddSmooth,               "ADDSMOOTH" },
   { GFXTOPBlendDiffuseAlpha,       "BLENDDIFFUSEALPHA" },
   { GFXTOPBlendTextureAlpha,       "BLENDTEXTUREALPHA" },
   { GFXTOPBlendFactorAlpha,        "BLENDFACTORALPHA" },
   { GFXTOPBlendTextureAlphaPM,     "BLENDTEXTUREALPHAPM" },
   { GFXTOPBlendCURRENTALPHA,       "BLENDCURRENTALPHA" },
   { GFXTOPPreModulate,             "PREMODULATE" },
   { GFXTOPModulateAlphaAddColor,   "MODULATEALPHAADDCOLOR" },
   { GFXTOPModulateColorAddAlpha,   "MODULATECOLORADDALPHA" },
   { GFXTOPModulateInvAlphaAddColor,"MODULATEINVALPHAADDCOLOR" },
   { GFXTOPModulateInvColorAddAlpha,"MODULATEINVCOLORADDALPHA" },
   { GFXTOPBumpEnvMap,              "BUMPENVMAP" },
   { GFXTOPBumpEnvMapLuminance,     "BUMPENVMAPLUMINANCE" },
   { GFXTOPDotProduct3,             "DOTPRODUCT3" },
   { GFXTOPLERP,                    "LERP" },
};

EnumTable GFXTextureArgumentTable =
{
   { GFXTADiffuse,                  "DIFFUSE" },
   { GFXTACurrent,                  "CURRENT" },
   { GFXTATexture,                  "TEXTURE" },
   { GFXTATFactor,                  "FACTOR" },
   { GFXTASpecular,                 "SPECULAR" },
   { GFXTATemp,                     "TEMP" },
   { GFXTAConstant,                 "CONSTANT" },
};

EnumTable GFXTextureAddressModeTable =
{
   { GFXAddressWrap,                "WRAP" },
   { GFXAddressMirror,              "MIRROR" },
   { GFXAddressClamp,               "CLAMP" },
   { GFXAddressBorder,              "BORDER" },
   { GFXAddressMirrorOnce,          "MIRRORONCE" },
};

EnumTable GFXTextureFilterTypeTable =
{
   { GFXTextureFilterNone,          "NONE" },
   { GFXTextureFilterPoint,         "POINT" },
   { GFXTextureFilterLinear,        "LINEAR" },
   { GFXTextureFilterAnisotropic,   "ANISOTROPIC" },
   { GFXTextureFilterPyramidalQuad, "PYRAMIDALQUAD" },
   { GFXTextureFilterGaussianQuad,  "GAUSSIANQUAD" },
};

EnumTable GFXTextureTransformFlagsTable =
{
   { GFXTTFFDisable,                "DISABLE" },
   { GFXTTFFCoord1D,                "COORD1D" },
   { GFXTTFFCoord2D,                "COORD2D" },
   { GFXTTFFCoord3D,                "COORD3D" },
   { GFXTTFFCoord4D,                "COORD4D" },
   { GFXTTFFProjected,              "PROJECTED" },
};

