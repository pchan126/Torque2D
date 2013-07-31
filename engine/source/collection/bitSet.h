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

#ifndef _BITSET_H_
#define _BITSET_H_

#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#include <bitset>

/// A convenience class to manipulate a set of bits.
///
/// Notice that bits are accessed directly, ie, by passing
/// a variable with the relevant bit set or not, instead of
/// passing the index of the relevant bit.
class BitSet32
{
private:
   /// Internal representation of bitset.
   std::bitset<32> mbits;

public:
   BitSet32()                         { mbits = 0; }
   BitSet32(std::bitset<32> in_bits)  { mbits = in_bits; }
   BitSet32(const BitSet32& in_rCopy) { mbits = in_rCopy.mbits; }
   BitSet32(U32 i)                    { mbits = std::bitset<32>(i);}

   operator U32() const               { return (U32)mbits.to_ulong(); }

   /// Set all bits to true.
   void set()                         { mbits.set(); }
   void set(const U32 m)              { mbits.set(m); }
   void set(const U32 m, bool b)       { mbits.set(m, b); }

   /// Clear all bits.
   void reset()                       { mbits.reset(); }
   void reset(const U32 m)            { mbits.reset(m); }

   /// Toggle the specified bit(s).
   void flip()                      { mbits.flip(); }
   void flip(const U32 m)           { mbits.flip(m); }

   /// Are any of the specified bit(s) set?
   bool test(const U32 m) const       { return mbits.test(m); }

   bool any() const                   { return mbits.any(); }
   bool none() const                  { return mbits.none(); }

   size_t size() const                {return mbits.size(); }

   /// @name Operator Overloads
   /// @{
   BitSet32& operator =(const U32 m)  { mbits  = m;  return *this; }
   BitSet32& operator|=(const U32 m)  { mbits |= m; return *this; }
   BitSet32& operator&=(const U32 m)  { mbits &= m; return *this; }
   BitSet32& operator^=(const U32 m)  { mbits ^= m; return *this; }

   BitSet32 operator|(const BitSet32 m) const { return BitSet32(mbits | m.mbits); }
   BitSet32 operator&(const BitSet32 m) const { return BitSet32(mbits & m.mbits); }
   BitSet32 operator^(const BitSet32 m) const { return BitSet32(mbits ^ m.mbits); }
   /// @}
};


#endif //_NBITSET_H_
