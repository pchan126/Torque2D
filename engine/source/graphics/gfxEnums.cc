#include "./gfxEnums.h"
#include "./gfxStringEnumTranslate.h"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

static std::array<EnumTable::Enums, 9> srcBlendFactorEntries =
        {{
                { GFXBlendZero,                  "ZERO"                  },
                { GFXBlendOne,                   "ONE"                   },
                { GFXBlendDestColor,             "DST_COLOR"             },
                { GFXBlendInvDestAlpha,   "ONE_MINUS_DST_COLOR"   },
                { GFXBlendSrcAlpha,             "SRC_ALPHA"             },
                { GFXBlendInvSrcColor,   "ONE_MINUS_SRC_ALPHA"   },
                { GFXBlendDestAlpha,             "DST_ALPHA"             },
                { GFXBlendInvDestAlpha,   "ONE_MINUS_DST_ALPHA"   },
                { GFXBlendSrcAlphaSat,    "SRC_ALPHA_SATURATE"    },
        }};

EnumTable srcBlendFactorTable = EnumTable(srcBlendFactorEntries.begin(), srcBlendFactorEntries.end());

//-----------------------------------------------------------------------------

static std::array<EnumTable::Enums, 8> dstBlendFactorEntries =
        {{
                { GFXBlendZero,                  "ZERO" },
                { GFXBlendOne,                   "ONE" },
                { GFXBlendSrcColor,             "SRC_COLOR" },
                { GFXBlendInvSrcColor,   "ONE_MINUS_SRC_COLOR" },
                { GFXBlendSrcAlpha,             "SRC_ALPHA" },
                { GFXBlendInvSrcAlpha,   "ONE_MINUS_SRC_ALPHA" },
                { GFXBlendDestAlpha,             "DST_ALPHA" },
                { GFXBlendInvDestAlpha,   "ONE_MINUS_DST_ALPHA" },
        }};

EnumTable dstBlendFactorTable = EnumTable(dstBlendFactorEntries.begin(), dstBlendFactorEntries.end());

static std::array<EnumTable::Enums, 5> blendOpFactorEntries =
        {{
   { GFXBlendOpAdd,                  "ADD" },
   { GFXBlendOpSubtract,                   "SUBTRACT" },
   { GFXBlendOpRevSubtract,             "REV_SUBTRACT" },
   { GFXBlendOpMin,   "MIN" },
   { GFXBlendOpMax,             "MAX" },
}};
EnumTable blendOpFactorTable = EnumTable(blendOpFactorEntries.begin(), blendOpFactorEntries.end());

static std::array<EnumTable::Enums, 8> cmpFactorEntries =
        {{
   { GFXCmpNever,                  "NEVER" },
   { GFXCmpLess,                   "LESS" },
   { GFXCmpEqual,             "EQUAL" },
   { GFXCmpLessEqual,   "LESS_EQUAL" },
   { GFXCmpGreater,             "GREATER" },
   { GFXCmpNotEqual,             "NOT_EQUAL" },
   { GFXCmpGreaterEqual,             "GREATER_EQUAL" },
   { GFXCmpAlways,             "ALWAYS" },
}};
EnumTable cmpFactorTable = EnumTable(cmpFactorEntries.begin(), cmpFactorEntries.end());

static std::array<EnumTable::Enums, 3> cullModeEntries =
        {{
   { GFXCullNone,                 "NONE" },
   { GFXCullCW,                   "CW" },
   { GFXCullCCW,                  "CWW" },
}};
EnumTable cullModeTable = EnumTable(cullModeEntries.begin(), cullModeEntries.end());

static std::array<EnumTable::Enums, 8> stencilOpEntries =
{{
   { GFXStencilOpKeep,              "KEEP" },
   { GFXStencilOpZero,              "ZERO" },
   { GFXStencilOpReplace,           "REPLACE" },
   { GFXStencilOpIncrSat,           "INCRSAT" },
   { GFXStencilOpDecrSat,           "DECRSAT" },
   { GFXStencilOpInvert,            "INVERT" },
   { GFXStencilOpIncr,              "INCR" },
   { GFXStencilOpDecr,              "DECR" },
}};
EnumTable stencilOpTable = EnumTable(stencilOpEntries.begin(), stencilOpEntries.end());


static std::array<EnumTable::Enums, 25> GFXTextureOpEntries =
{{
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
}};
EnumTable GFXTextureOpTable = EnumTable(GFXTextureOpEntries.begin(), GFXTextureOpEntries.end());

static std::array<EnumTable::Enums, 7> GFXTextureArgumentEntries =
        {{
   { GFXTADiffuse,                  "DIFFUSE" },
   { GFXTACurrent,                  "CURRENT" },
   { GFXTATexture,                  "TEXTURE" },
   { GFXTATFactor,                  "FACTOR" },
   { GFXTASpecular,                 "SPECULAR" },
   { GFXTATemp,                     "TEMP" },
   { GFXTAConstant,                 "CONSTANT" },
}};
EnumTable GFXTextureArgumentTable = EnumTable(GFXTextureArgumentEntries.begin(), GFXTextureArgumentEntries.end());

static std::array<EnumTable::Enums, 5> GFXTextureAddressModeEntries =
        {{
   { GFXAddressWrap,                "WRAP" },
   { GFXAddressMirror,              "MIRROR" },
   { GFXAddressClamp,               "CLAMP" },
   { GFXAddressBorder,              "BORDER" },
   { GFXAddressMirrorOnce,          "MIRRORONCE" },
}};
EnumTable GFXTextureAddressModeTable = EnumTable(GFXTextureAddressModeEntries.begin(), GFXTextureAddressModeEntries.end());

static std::array<EnumTable::Enums, 6> GFXTextureFilterTypeEntries =
{{
   { GFXTextureFilterNone,          "NONE" },
   { GFXTextureFilterPoint,         "POINT" },
   { GFXTextureFilterLinear,        "LINEAR" },
   { GFXTextureFilterAnisotropic,   "ANISOTROPIC" },
   { GFXTextureFilterPyramidalQuad, "PYRAMIDALQUAD" },
   { GFXTextureFilterGaussianQuad,  "GAUSSIANQUAD" },
}};
EnumTable GFXTextureFilterTypeTable = EnumTable(GFXTextureFilterTypeEntries.begin(), GFXTextureFilterTypeEntries.end());

static std::array<EnumTable::Enums, 6> GFXTextureTransformFlagsEntries =
{{
   { GFXTTFFDisable,                "DISABLE" },
   { GFXTTFFCoord1D,                "COORD1D" },
   { GFXTTFFCoord2D,                "COORD2D" },
   { GFXTTFFCoord3D,                "COORD3D" },
   { GFXTTFFCoord4D,                "COORD4D" },
   { GFXTTFFProjected,              "PROJECTED" },
}};
EnumTable GFXTextureTransformFlagsTable = EnumTable(GFXTextureTransformFlagsEntries.begin(), GFXTextureTransformFlagsEntries.end());

