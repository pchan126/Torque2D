//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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

#include "ts/tsShapeAlloc.h"

#define readOnly()  AssertFatal(mMode==TSShapeAlloc::ReadMode, "TSShapeAlloc: write-only function called when reading")
#define writeOnly() AssertFatal(mMode==TSShapeAlloc::WriteMode,"TSShapeAlloc: read-only function called when writing")

void TSShapeAlloc::setRead(S32 * memBuffer32, S16 * memBuffer16, S8 * memBuffer8, bool clear)
{
   mMemBuffer32 = memBuffer32;
   mMemBuffer16 = memBuffer16;
   mMemBuffer8  = memBuffer8 ;

   mMemGuard32  = 0;
   mMemGuard16  = 0;
   mMemGuard8   = 0;

   mSaveGuard32  = 0;
   mSaveGuard16  = 0;
   mSaveGuard8   = 0;

   if (clear)
   {
      mDest = NULL;
      mSize = 0;
//      s32_ptr_index.clear();
//      s16_ptr_index.clear();
//      s8_ptr_index.clear();
   }

   setSkipMode(false);
   mMode = TSShapeAlloc::ReadMode;
}

void TSShapeAlloc::setWrite()
{
   mMemBuffer32 = 0;
   mMemBuffer16 = 0;
   mMemBuffer8  = 0;

   mSize32 = mFullSize32 = 0;
   mSize16 = mFullSize16 = 0;
   mSize8  = mFullSize8  = 0;

//    s32_ptr_index.clear();
//    s16_ptr_index.clear();
//    s8_ptr_index.clear();

   mMemGuard32  = 0;
   mMemGuard16  = 0;
   mMemGuard8   = 0;

   setSkipMode(false); // doesn't really do anything here...
   mMode = TSShapeAlloc::WriteMode;
}

void TSShapeAlloc::doAlloc()
{
   readOnly();

   mDest = new S8[mSize];
   mSize = 0;
}

void TSShapeAlloc::align32()
{
   readOnly();

   S32 aligned = mSize+3 & (~0x3);
   allocShape8(aligned-mSize);
}


//IMPLEMENT_ALLOC(32,S32)

S32 TSShapeAlloc::get32()
{
   readOnly();
   return *(mMemBuffer32++);
}

void TSShapeAlloc::get32(S32 * dest, S32 num)
{
   readOnly();
   dMemcpy(dest,mMemBuffer32,sizeof(S32)*num);
   mMemBuffer32 += num;
}

S32 * TSShapeAlloc::allocShape32(S32 num)
{
   readOnly();
   S32 * ret = (S32*) mDest;
//   auto itr = std::find(m_ptr_index.begin(), m_ptr_index.end(), mDest);
//   U32 ret;
//
//   if (itr != m_ptr_index.end())
//       ret = m_ptr_index-m_ptr_index.begin();
//    else
//   {
//       ret = m_ptr_index.size();
//       m_ptr_index.push_back(mDest);
//   }

   if (mDest)
      mDest += mMult*num*sizeof(S32);
   mSize += sizeof(S32)*mMult*num;
   return ret;
}

S32 * TSShapeAlloc::getPointer32(S32 num)
{
   readOnly();
   S32 * ret = (S32*)mMemBuffer32;
   mMemBuffer32 += num;
   return ret;
}

S32 * TSShapeAlloc::copyToShape32(S32 num, bool returnSomething)
{
   readOnly();
   S32 * ret = (!returnSomething || mDest) ? (S32*)mDest : mMemBuffer32;
   if (mDest)
   {
      dMemcpy((S8*)mDest,(S8*)mMemBuffer32,
              mMult*num*sizeof(S32));
      mDest += mMult*num*sizeof(S32);
   }
   mMemBuffer32 += num;
   mSize += sizeof(S32)*mMult*num;
   return ret;
}

bool TSShapeAlloc::checkGuard32()
{
   readOnly();
   mSaveGuard32=get32();
   bool ret = (mSaveGuard32==mMemGuard32);
   mMemGuard32 += 1;
   return ret;
}

S32 TSShapeAlloc::getPrevGuard32()
{
   readOnly();
   return mMemGuard32 - 1;
}

S32 TSShapeAlloc::getSaveGuard32()
{
   readOnly();
   return mSaveGuard32;
}

S32 * TSShapeAlloc::getBuffer32()
{
   writeOnly();
   return mMemBuffer32;
}

S32 TSShapeAlloc::getBufferSize32()
{
   writeOnly();
   return mSize32;
}

S32 * TSShapeAlloc::extend32(S32 add)
{
   writeOnly();
   if (mSize32+add>mFullSize32)
   {
      S32 numPages = 1+(mFullSize32+add)/TSShapeAlloc::PageSize;
      mFullSize32 = numPages*TSShapeAlloc::PageSize;
      S32 * temp = new S32[mFullSize32];
      dMemcpy(temp,mMemBuffer32, mSize32 * sizeof(S32));
      delete [] mMemBuffer32;
      mMemBuffer32 = temp;
   }
   S32 * ret = mMemBuffer32 + mSize32;
   mSize32 += add;
   return ret;
}

S32 TSShapeAlloc::set32(S32 entry)
{
   writeOnly();
   *extend32(1) = entry;
   return entry;
}

void TSShapeAlloc::copyToBuffer32(S32 * entries, S32 count)
{
   writeOnly();
   if (entries)
      dMemcpy((U8*)extend32(count),(U8*)entries,count*sizeof(S32));
   else
      dMemset((U8*)extend32(count),0,count*sizeof(S32));
}

void TSShapeAlloc::setGuard32()
{
   writeOnly();
   set32(mMemGuard32);
   mMemGuard32 += 1;
}


//IMPLEMENT_ALLOC(16,S16)
S16 TSShapeAlloc::get16()
{
    readOnly();
    return *(mMemBuffer16++);
}

void TSShapeAlloc::get16(S16 * dest, S32 num)
{
    readOnly();
    dMemcpy(dest,mMemBuffer16,sizeof(S16)*num);
    mMemBuffer16 += num;
}

S16 * TSShapeAlloc::allocShape16(S32 num)
{
    readOnly();
    S16 * ret = (S16*) mDest;
//    U32 ret = mSize;
//    auto itr = std::find(m_ptr_index.begin(), m_ptr_index.end(), mDest);
//    U32 ret;
//
//    if (itr != m_ptr_index.end())
//        ret = m_ptr_index-m_ptr_index.begin();
//    else
//    {
//        ret = m_ptr_index.size();
//        m_ptr_index.push_back(mDest);
//    }

    if (mDest)
        mDest += mMult*num*sizeof(S16);
    mSize += sizeof(S16)*mMult*num;
    return ret;
}

S16 * TSShapeAlloc::getPointer16(S32 num)
{
    readOnly();
    S16 * ret = (S16*)mMemBuffer16;
    mMemBuffer16 += num;
    return ret;
}

S16 * TSShapeAlloc::copyToShape16(S32 num, bool returnSomething)
{
    readOnly();
    S16 * ret = (!returnSomething || mDest) ? (S16*)mDest : mMemBuffer16;
    if (mDest)
    {
        dMemcpy((S8*)mDest,(S8*)mMemBuffer16,
                mMult*num*sizeof(S16));
        mDest += mMult*num*sizeof(S16);
    }
    mMemBuffer16 += num;
    mSize += sizeof(S16)*mMult*num;
    return ret;
}

bool TSShapeAlloc::checkGuard16()
{
    readOnly();
    mSaveGuard16=get16();
    bool ret = (mSaveGuard16==mMemGuard16);
    mMemGuard16 += 1;
    return ret;
}

S16 TSShapeAlloc::getPrevGuard16()
{
    readOnly();
    return mMemGuard16 - 1;
}

S16 TSShapeAlloc::getSaveGuard16()
{
    readOnly();
    return mSaveGuard16;
}

S16 * TSShapeAlloc::getBuffer16()
{
    writeOnly();
    return mMemBuffer16;
}

S32 TSShapeAlloc::getBufferSize16()
{
    writeOnly();
    return mSize16;
}

S16 * TSShapeAlloc::extend16(S32 add)
{
    writeOnly();
    if (mSize16+add>mFullSize16)
    {
        S32 numPages = 1+(mFullSize16+add)/TSShapeAlloc::PageSize;
        mFullSize16 = numPages*TSShapeAlloc::PageSize;
        S16 * temp = new S16[mFullSize16];
        dMemcpy(temp,mMemBuffer16, mSize16 * sizeof(S16));
        delete [] mMemBuffer16;
        mMemBuffer16 = temp;
    }
    S16 * ret = mMemBuffer16 + mSize16;
    mSize16 += add;
    return ret;
}

S16 TSShapeAlloc::set16(S16 entry)
{
    writeOnly();
    *extend16(1) = entry;
    return entry;
}

void TSShapeAlloc::copyToBuffer16(S16 * entries, S32 count)
{
    writeOnly();
    if (entries)
        dMemcpy((U8*)extend16(count),(U8*)entries,count*sizeof(S16));
    else
        dMemset((U8*)extend16(count),0,count*sizeof(S16));
}

void TSShapeAlloc::setGuard16()
{
    writeOnly();
    set16(mMemGuard16);
    mMemGuard16 += 1;
}

//IMPLEMENT_ALLOC(8,S8)
S8 TSShapeAlloc::get8()
{
    readOnly();
    return *(mMemBuffer8++);
}

void TSShapeAlloc::get8(S8 * dest, S32 num)
{
    readOnly();
    dMemcpy(dest,mMemBuffer8,sizeof(S8)*num);
    mMemBuffer8 += num;
}

S8 * TSShapeAlloc::allocShape8(S32 num)
{
    readOnly();
    S8 * ret = (S8*) mDest;
//    U32 ret = mSize;
//    auto itr = std::find(m_ptr_index.begin(), m_ptr_index.end(), mDest);
//    U32 ret;
//
//    if (itr != m_ptr_index.end())
//        ret = m_ptr_index-m_ptr_index.begin();
//    else
//    {
//        ret = m_ptr_index.size();
//        m_ptr_index.push_back(mDest);
//    }

    if (mDest)
        mDest += mMult*num*sizeof(S8);
    mSize += sizeof(S8)*mMult*num;
    return ret;
}

S8 * TSShapeAlloc::getPointer8(S32 num)
{
    readOnly();
    S8 * ret = (S8*)mMemBuffer8;
    mMemBuffer8 += num;
    return ret;
}

S8 * TSShapeAlloc::copyToShape8(S32 num, bool returnSomething)
{
    readOnly();
    S8 * ret = (!returnSomething || mDest) ? (S8*)mDest : mMemBuffer8;
    if (mDest)
    {
        dMemcpy((S8*)mDest,(S8*)mMemBuffer8,
                mMult*num*sizeof(S8));
        mDest += mMult*num*sizeof(S8);
    }
    mMemBuffer8 += num;
    mSize += sizeof(S8)*mMult*num;
    return ret;
}

bool TSShapeAlloc::checkGuard8()
{
    readOnly();
    mSaveGuard8=get8();
    bool ret = (mSaveGuard8==mMemGuard8);
    mMemGuard8 += 1;
    return ret;
}

S8 TSShapeAlloc::getPrevGuard8()
{
    readOnly();
    return mMemGuard8 - 1;
}

S8 TSShapeAlloc::getSaveGuard8()
{
    readOnly();
    return mSaveGuard8;
}

S8 * TSShapeAlloc::getBuffer8()
{
    writeOnly();
    return mMemBuffer8;
}

S32 TSShapeAlloc::getBufferSize8()
{
    writeOnly();
    return mSize8;
}

S8 * TSShapeAlloc::extend8(S32 add)
{
    writeOnly();
    if (mSize8+add>mFullSize8)
    {
        S32 numPages = 1+(mFullSize8+add)/TSShapeAlloc::PageSize;
        mFullSize8 = numPages*TSShapeAlloc::PageSize;
        S8 * temp = new S8[mFullSize8];
        dMemcpy(temp,mMemBuffer8, mSize8 * sizeof(S8));
        delete [] mMemBuffer8;
        mMemBuffer8 = temp;
    }
    S8 * ret = mMemBuffer8 + mSize8;
    mSize8 += add;
    return ret;
}

S8 TSShapeAlloc::set8(S8 entry)
{
    writeOnly();
    *extend8(1) = entry;
    return entry;
}

void TSShapeAlloc::copyToBuffer8(S8 * entries, S32 count)
{
    writeOnly();
    if (entries)
        dMemcpy((U8*)extend8(count),(U8*)entries,count*sizeof(S8));
    else
        dMemset((U8*)extend8(count),0,count*sizeof(S8));
}

void TSShapeAlloc::setGuard8()
{
    writeOnly();
    set8(mMemGuard8);
    mMemGuard8 += 1;
}

void TSShapeAlloc::checkGuard()
{
   bool check32 = checkGuard32();
   bool check16 = checkGuard16();
   bool check8  = checkGuard8();

   AssertFatal(check32,avar("TSShapeAlloc::checkGuard32: found %i, wanted %i",getSaveGuard32(),getPrevGuard32()));
   AssertFatal(check16,avar("TSShapeAlloc::checkGuard16: found %i, wanted %i",getSaveGuard16(),getPrevGuard16()));
   AssertFatal(check8 ,avar("TSShapeAlloc::checkGuard8:  found %i, wanted %i",getSaveGuard8() ,getPrevGuard8()));
}

void TSShapeAlloc::setGuard()
{
   setGuard32();
   setGuard16();
   setGuard8();
}


//#define IMPLEMENT_ALLOC(suffix,type)                          \
//                                                              \
//type TSShapeAlloc::get##suffix()                              \
//{                                                             \
//   readOnly();                                                \
//   return *(mMemBuffer##suffix++);                            \
//}                                                             \
//                                                              \
//void TSShapeAlloc::get##suffix(type * dest, S32 num)          \
//{                                                             \
//   readOnly();                                                \
//   dMemcpy(dest,mMemBuffer##suffix,sizeof(type)*num);         \
//   mMemBuffer##suffix += num;                                 \
//}                                                             \
//                                                              \
//type * TSShapeAlloc::allocShape##suffix(S32 num)              \
//{                                                             \
//   readOnly();                                                \
//   type * ret = (type*) mDest;                                \
//   if (mDest)                                                 \
//      mDest += mMult*num*sizeof(type);                        \
//   mSize += sizeof(type)*mMult*num;                           \
//   return ret;                                                \
//}                                                             \
//                                                              \
//type * TSShapeAlloc::getPointer##suffix(S32 num)              \
//{                                                             \
//   readOnly();                                                \
//   type * ret = (type*)mMemBuffer##suffix;                    \
//   mMemBuffer##suffix += num;                                 \
//   return ret;                                                \
//}                                                             \
//                                                              \
//type * TSShapeAlloc::copyToShape##suffix(S32 num, bool returnSomething) \
//{                                                             \
//   readOnly();                                                \
//   type * ret = (!returnSomething || mDest) ? (type*)mDest : mMemBuffer##suffix; \
//   if (mDest)                                                 \
//   {                                                          \
//      dMemcpy((S8*)mDest,(S8*)mMemBuffer##suffix,             \
//              mMult*num*sizeof(type));                        \
//      mDest += mMult*num*sizeof(type);                        \
//   }                                                          \
//   mMemBuffer##suffix += num;                                 \
//   mSize += sizeof(type)*mMult*num;                           \
//   return ret;                                                \
//}                                                             \
//                                                              \
//bool TSShapeAlloc::checkGuard##suffix()                       \
//{                                                             \
//   readOnly();                                                \
//   mSaveGuard##suffix=get##suffix();                          \
//   bool ret = (mSaveGuard##suffix==mMemGuard##suffix);        \
//   mMemGuard##suffix += 1;                                    \
//   return ret;                                                \
//}                                                             \
//                                                              \
//type TSShapeAlloc::getPrevGuard##suffix()                     \
//{                                                             \
//   readOnly();                                                \
//   return mMemGuard##suffix - 1;                              \
//}                                                             \
//                                                              \
//type TSShapeAlloc::getSaveGuard##suffix()                     \
//{                                                             \
//   readOnly();                                                \
//   return mSaveGuard##suffix;                                 \
//}                                                             \
//                                                              \
//type * TSShapeAlloc::getBuffer##suffix()                      \
//{                                                             \
//   writeOnly();                                               \
//   return mMemBuffer##suffix;                                 \
//}                                                             \
//                                                              \
//S32 TSShapeAlloc::getBufferSize##suffix()                     \
//{                                                             \
//   writeOnly();                                               \
//   return mSize##suffix;                                      \
//}                                                             \
//                                                              \
//type * TSShapeAlloc::extend##suffix(S32 add)                  \
//{                                                             \
//   writeOnly();                                               \
//   if (mSize##suffix+add>mFullSize##suffix)                   \
//   {                                                          \
//      S32 numPages = 1+(mFullSize##suffix+add)/TSShapeAlloc::PageSize; \
//      mFullSize##suffix = numPages*TSShapeAlloc::PageSize;    \
//      type * temp = new type[mFullSize##suffix];              \
//      dMemcpy(temp,mMemBuffer##suffix, mSize##suffix * sizeof(type)); \
//      delete [] mMemBuffer##suffix;                           \
//      mMemBuffer##suffix = temp;                              \
//   }                                                          \
//   type * ret = mMemBuffer##suffix + mSize##suffix;            \
//   mSize##suffix += add;                                      \
//   return ret;                                                \
//}                                                             \
//                                                              \
//type TSShapeAlloc::set##suffix(type entry)                    \
//{                                                             \
//   writeOnly();                                               \
//   *extend##suffix(1) = entry;                                \
//   return entry;                                              \
//}                                                             \
//                                                              \
//void TSShapeAlloc::copyToBuffer##suffix(type * entries, S32 count) \
//{                                                             \
//   writeOnly();                                               \
//   if (entries)                                               \
//      dMemcpy((U8*)extend##suffix(count),(U8*)entries,count*sizeof(type)); \
//   else                                                       \
//      dMemset((U8*)extend##suffix(count),0,count*sizeof(type)); \
//}                                                             \
//                                                              \
//void TSShapeAlloc::setGuard##suffix()                         \
//{                                                             \
//   writeOnly();                                               \
//   set##suffix(mMemGuard##suffix);                            \
//   mMemGuard##suffix += 1;                                    \
//}