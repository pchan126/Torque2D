#include "./gfxEnums.h"

EnumTable srcBlendFactorTable(sizeof(srcBlendFactorLookup) / sizeof(EnumTable::Enums), &srcBlendFactorLookup[0]);
EnumTable dstBlendFactorTable(sizeof(dstBlendFactorLookup) / sizeof(EnumTable::Enums), &dstBlendFactorLookup[0]);
EnumTable blendOpFactorTable(sizeof(blendOpFactorLookup) / sizeof(EnumTable::Enums), &blendOpFactorLookup[0]);
EnumTable cmpFactorTable(sizeof(CmpFactorLookup)/ sizeof(EnumTable::Enums), &CmpFactorLookup[0]);
