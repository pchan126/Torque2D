#include "./gfxEnums.h"
#include "./gfxStringEnumTranslate.h"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

EnumTable::Enums srcBlendFactorLookup[] =
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

EnumTable::Enums dstBlendFactorLookup[] =
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

static EnumTable::Enums blendOpFactorLookup[] =
{
   { GFXBlendOpAdd,                  "ADD" },
   { GFXBlendOpSubtract,                   "SUBTRACT" },
   { GFXBlendOpRevSubtract,             "REV_SUBTRACT" },
   { GFXBlendOpMin,   "MIN" },
   { GFXBlendOpMax,             "MAX" },
};

static EnumTable::Enums CmpFactorLookup[] =
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

static EnumTable::Enums cullModeLookup[] =
{
   { GFXCullNone,                 "NONE" },
   { GFXCullCW,                   "CW" },
   { GFXCullCCW,                  "CWW" },
};

static EnumTable::Enums stencilOpLookup[] =
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


static EnumTable::Enums textureOpLookup[] =
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

static EnumTable::Enums GFXTextureArgumentLookup[] =
{
   { GFXTADiffuse,                  "DIFFUSE" },
   { GFXTACurrent,                  "CURRENT" },
   { GFXTATexture,                  "TEXTURE" },
   { GFXTATFactor,                  "FACTOR" },
   { GFXTASpecular,                 "SPECULAR" },
   { GFXTATemp,                     "TEMP" },
   { GFXTAConstant,                 "CONSTANT" },
};

static EnumTable::Enums GFXTextureAddressModeLookup[] =
{
   { GFXAddressWrap,                "WRAP" },
   { GFXAddressMirror,              "MIRROR" },
   { GFXAddressClamp,               "CLAMP" },
   { GFXAddressBorder,              "BORDER" },
   { GFXAddressMirrorOnce,          "MIRRORONCE" },
};

static EnumTable::Enums GFXTextureFilterTypeLookup[] =
{
   { GFXTextureFilterNone,          "NONE" },
   { GFXTextureFilterPoint,         "POINT" },
   { GFXTextureFilterLinear,        "LINEAR" },
   { GFXTextureFilterAnisotropic,   "ANISOTROPIC" },
   { GFXTextureFilterPyramidalQuad, "PYRAMIDALQUAD" },
   { GFXTextureFilterGaussianQuad,  "GAUSSIANQUAD" },
};

static EnumTable::Enums GFXTextureTransformFlagsLookup[] =
{
   { GFXTTFFDisable,                "DISABLE" },
   { GFXTTFFCoord1D,                "COORD1D" },
   { GFXTTFFCoord2D,                "COORD2D" },
   { GFXTTFFCoord3D,                "COORD3D" },
   { GFXTTFFCoord4D,                "COORD4D" },
   { GFXTTFFProjected,              "PROJECTED" },
};

EnumTable srcBlendFactorTable(sizeof(srcBlendFactorLookup) / sizeof(EnumTable::Enums), &srcBlendFactorLookup[0]);
EnumTable dstBlendFactorTable(sizeof(dstBlendFactorLookup) / sizeof(EnumTable::Enums), &dstBlendFactorLookup[0]);
EnumTable blendOpFactorTable(sizeof(blendOpFactorLookup) / sizeof(EnumTable::Enums), &blendOpFactorLookup[0]);
EnumTable cmpFactorTable(sizeof(CmpFactorLookup)/ sizeof(EnumTable::Enums), &CmpFactorLookup[0]);
EnumTable cullModeTable(sizeof(cullModeLookup)/ sizeof(EnumTable::Enums), &cullModeLookup[0]);
EnumTable stencilOpTable(sizeof(stencilOpLookup)/ sizeof(EnumTable::Enums), &stencilOpLookup[0]);
EnumTable GFXTextureOpTable(sizeof(textureOpLookup)/ sizeof(EnumTable::Enums), &textureOpLookup[0]);
EnumTable GFXTextureArgumentTable(sizeof(GFXTextureArgumentLookup)/ sizeof(EnumTable::Enums), &GFXTextureArgumentLookup[0]);
EnumTable GFXTextureAddressModeTable(sizeof(GFXTextureAddressModeLookup)/ sizeof(EnumTable::Enums), &GFXTextureAddressModeLookup[0]);
EnumTable GFXTextureFilterTypeTable(sizeof(GFXTextureFilterTypeLookup)/ sizeof(EnumTable::Enums), &GFXTextureFilterTypeLookup[0]);
EnumTable GFXTextureTransformFlagsTable(sizeof(GFXTextureTransformFlagsLookup)/ sizeof(EnumTable::Enums), &GFXTextureTransformFlagsLookup[0]);

