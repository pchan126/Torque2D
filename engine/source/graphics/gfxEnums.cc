#include "./gfxEnums.h"
#include "./gfxStringEnumTranslate.h"

//-----------------------------------------------------------------------------

//-----------------------------------------------------------------------------

static std::array<EnumTable::Enums, 9> srcBlendFactorEntries =
        {
          EnumTable::Enums( GFXBlendZero,                  "ZERO"                  ),
                EnumTable::Enums( GFXBlendOne,                   "ONE"                   ),
                EnumTable::Enums( GFXBlendDestColor,             "DST_COLOR"             ),
                EnumTable::Enums( GFXBlendInvDestAlpha,   "ONE_MINUS_DST_COLOR"   ),
                EnumTable::Enums( GFXBlendSrcAlpha,             "SRC_ALPHA"             ),
                EnumTable::Enums( GFXBlendInvSrcColor,   "ONE_MINUS_SRC_ALPHA"   ),
                EnumTable::Enums( GFXBlendDestAlpha,             "DST_ALPHA"             ),
                EnumTable::Enums( GFXBlendInvDestAlpha,   "ONE_MINUS_DST_ALPHA"   ),
                EnumTable::Enums( GFXBlendSrcAlphaSat,    "SRC_ALPHA_SATURATE"    ),
        };

EnumTable srcBlendFactorTable = EnumTable(srcBlendFactorEntries.begin(), srcBlendFactorEntries.end());

//-----------------------------------------------------------------------------

static std::array<EnumTable::Enums, 8> dstBlendFactorEntries =
        {
          EnumTable::Enums( GFXBlendZero,                  "ZERO" ),
          EnumTable::Enums( GFXBlendOne,                   "ONE" ),
          EnumTable::Enums( GFXBlendSrcColor,             "SRC_COLOR" ),
          EnumTable::Enums( GFXBlendInvSrcColor,   "ONE_MINUS_SRC_COLOR" ),
          EnumTable::Enums( GFXBlendSrcAlpha,             "SRC_ALPHA" ),
          EnumTable::Enums( GFXBlendInvSrcAlpha,   "ONE_MINUS_SRC_ALPHA" ),
          EnumTable::Enums( GFXBlendDestAlpha,             "DST_ALPHA" ),
          EnumTable::Enums( GFXBlendInvDestAlpha,   "ONE_MINUS_DST_ALPHA" ),
        };

EnumTable dstBlendFactorTable = EnumTable(dstBlendFactorEntries.begin(), dstBlendFactorEntries.end());

static std::array<EnumTable::Enums, 5> blendOpFactorEntries =
        {
   EnumTable::Enums( GFXBlendOpAdd,                  "ADD" ),
   EnumTable::Enums( GFXBlendOpSubtract,                   "SUBTRACT" ),
   EnumTable::Enums( GFXBlendOpRevSubtract,             "REV_SUBTRACT" ),
   EnumTable::Enums( GFXBlendOpMin,   "MIN" ),
   EnumTable::Enums( GFXBlendOpMax,             "MAX" ),
};
EnumTable blendOpFactorTable = EnumTable(blendOpFactorEntries.begin(), blendOpFactorEntries.end());

static std::array<EnumTable::Enums, 8> cmpFactorEntries =
        {
   EnumTable::Enums( GFXCmpNever,                  "NEVER" ),
   EnumTable::Enums( GFXCmpLess,                   "LESS" ),
   EnumTable::Enums( GFXCmpEqual,             "EQUAL" ),
   EnumTable::Enums( GFXCmpLessEqual,   "LESS_EQUAL" ),
   EnumTable::Enums( GFXCmpGreater,             "GREATER" ),
   EnumTable::Enums( GFXCmpNotEqual,             "NOT_EQUAL" ),
   EnumTable::Enums( GFXCmpGreaterEqual,             "GREATER_EQUAL" ),
   EnumTable::Enums( GFXCmpAlways,             "ALWAYS" ),
};
EnumTable cmpFactorTable = EnumTable(cmpFactorEntries.begin(), cmpFactorEntries.end());

static std::array<EnumTable::Enums, 3> cullModeEntries =
        {
   EnumTable::Enums( GFXCullNone,                 "NONE" ),
   EnumTable::Enums( GFXCullCW,                   "CW" ),
   EnumTable::Enums( GFXCullCCW,                  "CWW" ),
};
EnumTable cullModeTable = EnumTable(cullModeEntries.begin(), cullModeEntries.end());

static std::array<EnumTable::Enums, 8> stencilOpEntries =
{
   EnumTable::Enums( GFXStencilOpKeep,              "KEEP" ),
   EnumTable::Enums( GFXStencilOpZero,              "ZERO" ),
   EnumTable::Enums( GFXStencilOpReplace,           "REPLACE" ),
   EnumTable::Enums( GFXStencilOpIncrSat,           "INCRSAT" ),
   EnumTable::Enums( GFXStencilOpDecrSat,           "DECRSAT" ),
   EnumTable::Enums( GFXStencilOpInvert,            "INVERT" ),
   EnumTable::Enums( GFXStencilOpIncr,              "INCR" ),
   EnumTable::Enums( GFXStencilOpDecr,              "DECR" ),
};
EnumTable stencilOpTable = EnumTable(stencilOpEntries.begin(), stencilOpEntries.end());


static std::array<EnumTable::Enums, 25> GFXTextureOpEntries =
{
   EnumTable::Enums( GFXTOPDisable,                 "DISABLE" ),
   EnumTable::Enums( GFXTOPSelectARG1,              "SELECTARG1" ),
   EnumTable::Enums( GFXTOPSelectARG2,              "SELECTARG2" ),
   EnumTable::Enums( GFXTOPModulate,                "MODULATE" ),
   EnumTable::Enums( GFXTOPModulate2X,              "MODULATE2X" ),
   EnumTable::Enums( GFXTOPModulate4X,              "MODULATE4X" ),
   EnumTable::Enums( GFXTOPAdd,                     "ADD" ),
   EnumTable::Enums( GFXTOPAddSigned,               "ADDSIGNED" ),
   EnumTable::Enums( GFXTOPAddSigned2X,             "ADDSIGNED2X" ),
   EnumTable::Enums( GFXTOPSubtract,                "SUBTRACT" ),
   EnumTable::Enums( GFXTOPAddSmooth,               "ADDSMOOTH" ),
   EnumTable::Enums( GFXTOPBlendDiffuseAlpha,       "BLENDDIFFUSEALPHA" ),
   EnumTable::Enums( GFXTOPBlendTextureAlpha,       "BLENDTEXTUREALPHA" ),
   EnumTable::Enums( GFXTOPBlendFactorAlpha,        "BLENDFACTORALPHA" ),
   EnumTable::Enums( GFXTOPBlendTextureAlphaPM,     "BLENDTEXTUREALPHAPM" ),
   EnumTable::Enums( GFXTOPBlendCURRENTALPHA,       "BLENDCURRENTALPHA" ),
   EnumTable::Enums( GFXTOPPreModulate,             "PREMODULATE" ),
   EnumTable::Enums( GFXTOPModulateAlphaAddColor,   "MODULATEALPHAADDCOLOR" ),
   EnumTable::Enums( GFXTOPModulateColorAddAlpha,   "MODULATECOLORADDALPHA" ),
   EnumTable::Enums( GFXTOPModulateInvAlphaAddColor,"MODULATEINVALPHAADDCOLOR" ),
   EnumTable::Enums( GFXTOPModulateInvColorAddAlpha,"MODULATEINVCOLORADDALPHA" ),
   EnumTable::Enums( GFXTOPBumpEnvMap,              "BUMPENVMAP" ),
   EnumTable::Enums( GFXTOPBumpEnvMapLuminance,     "BUMPENVMAPLUMINANCE" ),
   EnumTable::Enums( GFXTOPDotProduct3,             "DOTPRODUCT3" ),
   EnumTable::Enums( GFXTOPLERP,                    "LERP" ),
};
EnumTable GFXTextureOpTable = EnumTable(GFXTextureOpEntries.begin(), GFXTextureOpEntries.end());

static std::array<EnumTable::Enums, 7> GFXTextureArgumentEntries =
        {
   EnumTable::Enums( GFXTADiffuse,                  "DIFFUSE" ),
   EnumTable::Enums( GFXTACurrent,                  "CURRENT" ),
   EnumTable::Enums( GFXTATexture,                  "TEXTURE" ),
   EnumTable::Enums( GFXTATFactor,                  "FACTOR" ),
   EnumTable::Enums( GFXTASpecular,                 "SPECULAR" ),
   EnumTable::Enums( GFXTATemp,                     "TEMP" ),
   EnumTable::Enums( GFXTAConstant,                 "CONSTANT" ),
};
EnumTable GFXTextureArgumentTable = EnumTable(GFXTextureArgumentEntries.begin(), GFXTextureArgumentEntries.end());

static std::array<EnumTable::Enums, 5> GFXTextureAddressModeEntries =
        {
   EnumTable::Enums( GFXAddressWrap,                "WRAP" ),
   EnumTable::Enums( GFXAddressMirror,              "MIRROR" ),
   EnumTable::Enums( GFXAddressClamp,               "CLAMP" ),
   EnumTable::Enums( GFXAddressBorder,              "BORDER" ),
   EnumTable::Enums( GFXAddressMirrorOnce,          "MIRRORONCE" ),
};
EnumTable GFXTextureAddressModeTable = EnumTable(GFXTextureAddressModeEntries.begin(), GFXTextureAddressModeEntries.end());

static std::array<EnumTable::Enums, 6> GFXTextureFilterTypeEntries =
{
   EnumTable::Enums( GFXTextureFilterNone,          "NONE" ),
   EnumTable::Enums( GFXTextureFilterPoint,         "POINT" ),
   EnumTable::Enums( GFXTextureFilterLinear,        "LINEAR" ),
   EnumTable::Enums( GFXTextureFilterAnisotropic,   "ANISOTROPIC" ),
   EnumTable::Enums( GFXTextureFilterPyramidalQuad, "PYRAMIDALQUAD" ),
   EnumTable::Enums( GFXTextureFilterGaussianQuad,  "GAUSSIANQUAD" ),
};
EnumTable GFXTextureFilterTypeTable = EnumTable(GFXTextureFilterTypeEntries.begin(), GFXTextureFilterTypeEntries.end());

static std::array<EnumTable::Enums, 6> GFXTextureTransformFlagsEntries =
{
   EnumTable::Enums( GFXTTFFDisable,                "DISABLE" ),
   EnumTable::Enums( GFXTTFFCoord1D,                "COORD1D" ),
   EnumTable::Enums( GFXTTFFCoord2D,                "COORD2D" ),
   EnumTable::Enums( GFXTTFFCoord3D,                "COORD3D" ),
   EnumTable::Enums( GFXTTFFCoord4D,                "COORD4D" ),
   EnumTable::Enums( GFXTTFFProjected,              "PROJECTED" ),
};
EnumTable GFXTextureTransformFlagsTable = EnumTable(GFXTextureTransformFlagsEntries.begin(), GFXTextureTransformFlagsEntries.end());

