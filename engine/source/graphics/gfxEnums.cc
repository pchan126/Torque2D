#include "./gfxEnums.h"
#include "./gfxStringEnumTranslate.h"

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

