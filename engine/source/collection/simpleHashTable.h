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
#include <string>

#ifndef _TSIMPLEHASHTABLE_H
#define _TSIMPLEHASHTABLE_H

template <class T> class SimpleHashTable
{
    bool mCaseSensitive;

public:
   SimpleHashTable(bool caseSensitive = true): mCaseSensitive(caseSensitive)
   {
   }
   std::unordered_map<std::string, T*> table;

   void insert(T* pObject, const char *key);
   T*   remove(const char *key);
   T*   retrieve(const char *key);

   void clearTables();                       // Note: _deletes_ the objects!
};

template <class T> inline void SimpleHashTable<T>::insert(T* pObject, const char *key)
{
    std::string val(key);
    if (mCaseSensitive)
        std::transform(val.begin(), val.end(), val.begin(), tolower);

    table.insert(std::pair<std::string, T*>(std::string(key), pObject));
}

template <class T> T* SimpleHashTable<T>::remove(const char *key)
{
    std::string val(key);
    if (mCaseSensitive)
        std::transform(val.begin(), val.end(), val.begin(), tolower);

    T* ret = table[val];
    table.erase(val);
    return ret;
}

template <class T> T* SimpleHashTable<T>::retrieve(const char *key)
{
    std::string val(key);
    if (mCaseSensitive)
        std::transform(val.begin(), val.end(), val.begin(), tolower);

    auto itr = table.find(val);
    if (itr == table.end())
        return nullptr;
    std::pair<std::string, T*> temp = (*itr);
    return temp.second;
}

template <class T>
inline void SimpleHashTable<T>::clearTables()
{
   for (std::pair<std::string, T*> itr:table)
      delete itr.second;

   table.clear();
}

#endif // _TSIMPLEHASHTABLE_H
