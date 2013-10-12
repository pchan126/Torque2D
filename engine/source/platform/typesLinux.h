//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#ifndef _TYPESLINUX_H_
#define _TYPESLINUX_H_

/* eek. */
#ifndef NULL
#define NULL 0
#endif

#define PLATFORM_LITTLE_ENDIAN

#define FN_CDECL

//------------------------------------------------------------------------------
//-------------------------------------- Basic Types...

typedef std::int8_t        S8;      ///< Compiler independent Signed Char
typedef std::uint8_t       U8;      ///< Compiler independent Unsigned Char

typedef std::int16_t       S16;     ///< Compiler independent Signed 16-bit short
typedef std::uint16_t      U16;     ///< Compiler independent Unsigned 16-bit short

typedef std::int32_t       S32;     ///< Compiler independent Signed 32-bit integer
typedef std::uint32_t      U32;     ///< Compiler independent Unsigned 32-bit integer

typedef std::int64_t       S64;     ///< Compiler independent Signed 64-bit integer
typedef std::uint64_t      U64;     ///< Compiler independent Unsigned 64-bit integer

typedef float              F32;     ///< Compiler independent 32-bit float
typedef double             F64;     ///< Compiler independent 64-bit float

typedef size_t             ST;  /// < Compiler dependant 32 or 64 bit>
typedef ptrdiff_t          PTR;       /// < Compiler dependant integer, same size as pointer>

typedef const char* StringTableEntry;

typedef S32 FileTime;

#define __EQUAL_CONST_F F32(0.000001)

static const F32 Float_One  = F32(1.0);
static const F32 Float_Half = F32(0.5);
static const F32 Float_Zero = F32(0.0);
static const F32 Float_Pi   = F32(3.14159265358979323846);
static const F32 Float_2Pi  = F32(2.0 * 3.14159265358979323846);

static const F32 F32_MAX = F32(3.402823466e+38F);
static const F32 F32_MIN = F32(1.175494351e-38F);


#endif
