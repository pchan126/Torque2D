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

#include "platform/platform.h"
#include <functional>
#include <unordered_map>

#ifndef _TSIMPLEHASHTABLE_H
#define _TSIMPLEHASHTABLE_H

template <class T> class SimpleHashTable
{
    bool mCaseSensitive;

    char mCaseConvBuf[1024];

    // [tom, 9/21/2006] This is incredibly lame and adds a pretty big speed penalty
    inline const char *caseConv(const char *str)
    {
        if(mCaseSensitive)   return str;

        dsize_t len = dStrlen(str);
        if(len >= sizeof(mCaseConvBuf))  len = sizeof(mCaseConvBuf) - 1;

        char *dptr = mCaseConvBuf;
        const char *sptr = str;
        while(*sptr)
        {
            *dptr = dTolower(*sptr);
            ++sptr;
            ++dptr;
        }
        *dptr = 0;

        return mCaseConvBuf;
    }

public:
   SimpleHashTable(bool caseSensitive = true): mCaseSensitive(caseSensitive)
   {
   }
   std::unordered_map<size_t, T*> table;

   std::hash<U8*> ptr_hash;

   void insert(T* pObject, U8 *key);
   T*   remove(U8 *key);
   T*   retrieve(U8 *key);

   void insert(T* pObject, const char *key);
   T*   remove(const char *key);
   T*   retrieve(const char *key);

   void clearTables();                       // Note: _deletes_ the objects!
};

template <class T> inline void SimpleHashTable<T>::insert(T* pObject, U8 *key)
{
   table.insert(std::pair<size_t, T*>(ptr_hash(key), pObject));
}

template <class T> inline T* SimpleHashTable<T>::remove(U8 *key)
{
   T* ret = table[ptr_hash(key)];
   table.erase(ptr_hash(key));
   return ret;
}

template <class T> inline T* SimpleHashTable<T>::retrieve(U8 *key)
{
   return table[ptr_hash(key)];
}

template <class T> inline void SimpleHashTable<T>::insert(T* pObject, const char *key)
{
   key = caseConv(key);
   insert(pObject, (U8*)key);
}

template <class T> T* SimpleHashTable<T>::remove(const char *key)
{
   key = caseConv(key);
   return remove((U8 *)key);
}

template <class T> T* SimpleHashTable<T>::retrieve(const char *key)
{
   key = caseConv(key);
   return retrieve((U8 *)key);
}

template <class T>
inline void SimpleHashTable<T>::clearTables()
{
   for (std::pair<size_t, T*> itr:table)
      delete itr.second;

   table.clear();
}

#endif // _TSIMPLEHASHTABLE_H
