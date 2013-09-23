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

#ifndef _HASHTABLE_H
#define _HASHTABLE_H

#include "vector.h"
#include "platform/platform.h"
#include "string/stringTable.h"
#include <unordered_map>
#include <functional>

/// A HashTable template class.
///
/// The hash table class maps between a key and an associated value. Access
/// using the key is performed using a hash table.  The class provides
/// methods for both unique and equal keys. The global ::hash(Type) function
/// is used for hashing, see util/hash.h
/// @ingroup UtilContainers
template<typename Key, typename Value >
class HashTable
{
public:
    typedef typename std::pair<Key, Value> pair;

private:
    std::unordered_map<Key, Value> mTable;

public:
    typedef typename std::unordered_map<Key, Value>::iterator iterator;
    typedef typename std::unordered_map<Key, Value>::const_iterator const_iterator;

   // Initialization
   HashTable();
   ~HashTable();
   HashTable(const HashTable& p);

   // Management
   U32  size() const       { return mTable.size(); };                  ///< Return the number of elements
   void clear()            { return mTable.clear(); };                       ///< Empty the HashTable
   void resize(U32 size);
   bool isEmpty() const    { return mTable.empty(); };               ///< Returns true if the table is empty

   // Insert & erase elements
   void insertEqual(const Key& key, const Value&);
   void insertUnique(const Key& key, const Value&);
   void erase(iterator);               ///< Erase the given entry
   U32 erase(const Key& key);         ///< Erase all matching keys from the table

   // HashTable lookup
   iterator find(const Key& key)               { return mTable.find(key); }   ///< Find the first entry for the given key
   const_iterator find(const Key& key) const   { return mTable.find(key); }   ///< Find the first entry for the given key
   S32 count(const Key& key)                   { return mTable.count(key); }   ///< Count the number of matching keys in the table

   std::pair<iterator, iterator> equal_range( const Key& key)   { return mTable.equal_range(key); };
   std::pair<const_iterator, const_iterator> equal_range(const Key& key)  const { return mTable.equal_range(key); };

   // Forward iterator access
   iterator       begin()                      { return mTable.begin(); }            ///< iterator to first element
   const_iterator begin() const                { return mTable.begin(); }       ///< iterator to first element
   iterator       end()                        { return mTable.end(); }              ///< iterator to last element + 1
   const_iterator end() const                  { return mTable.end(); }         ///< iterator to last element + 1

   void operator=(const HashTable& p)          { mTable = p.mTable; };
};


template<typename Key, typename Value> HashTable<Key,Value>::HashTable()
{
}

template<typename Key, typename Value> HashTable<Key,Value>::HashTable(const HashTable& p)
{
   mTable = 0;
   *this = p;
}

template<typename Key, typename Value> HashTable<Key,Value>::~HashTable()
{
}

//-----------------------------------------------------------------------------
// add & remove elements

/// Insert the key value pair but don't insert duplicates.
template<typename Key, typename Value>
void HashTable<Key,Value>::insertUnique(const Key& key, const Value& x)
{
    iterator itr = mTable.find(key);

    if (itr != mTable.end())
        return;

    mTable[key] = x;
}

/// Insert the key value pair and allow duplicates.
/// This insert method allows duplicate keys.  Keys are grouped together but
/// are not sorted.
template<typename Key, typename Value>
void HashTable<Key,Value>::insertEqual(const Key& key, const Value& x)
{
    pair temp(key, x);
    mTable.insert(temp);
}

template<typename Key, typename Value>
U32 HashTable<Key,Value>::erase(const Key& key)
{
    return (U32)mTable.erase(key);
}

template<typename Key, typename Value>
void HashTable<Key,Value>::erase(iterator node)
{
    mTable.erase(node);
}


#include <map>

//-----------------------------------------------------------------------------
// iterator class

/// A HashMap template class.
/// The map class maps between a key and an associated value. Keys
/// are unique.
/// The hash table class is used as the default implementation so the
/// the key must be hashable, see util/hash.h for details.
/// @ingroup UtilContainers
template<typename Key, typename Value>
class HashMap
{
private:
   std::map<Key, Value> mHashMap;

public:
   // types
   typedef typename std::pair<Key, Value> pair;
   typedef typename std::map<Key, Value>::iterator  iterator;
   typedef typename std::map<Key, Value>::const_iterator const_iterator;

   // initialization
   HashMap() {}
   ~HashMap() {}
   HashMap(const HashMap& p);

   // management
   U32  size() const                       { return mHashMap.size(); } ///< Return the number of elements
   void clear()                            { mHashMap.clear(); }; ///< Empty the HashMap
   bool isEmpty() const                    { return mHashMap.isEmpty(); };               ///< Returns true if the map is empty

   // insert & erase elements
   iterator insert(const Key& key, const Value&); // Documented below...
   void erase(iterator itr)                { mHashMap.erase(itr); };               ///< Erase the given entry
   void erase(const Key& key)              { mHashMap.erase(key); };         ///< Erase the key from the map

   // HashMap lookup
   iterator find(const Key& key)                 { return mHashMap.find(key); }          ///< Find entry for the given key
   const_iterator find(const Key& key) const     { return mHashMap.find(key); }          ///< Find entry for the given key
   bool contains(const Key&a)                    { return mHashMap.count(a) > 0; }

   // forward iterator access
   iterator       begin()           {  return mHashMap.begin(); };   ///< iterator to first element
   const_iterator begin() const     {  return mHashMap.begin(); };       ///< iterator to first element
   iterator       end()             {  return mHashMap.end(); };   ///< IIterator to last element + 1
   const_iterator end() const       {  return mHashMap.end(); };   ///< iterator to last element + 1

   // operators
   Value& operator[](const Key& key ) { return mHashMap[key]; };      ///< Index using the given key. If the key is not currently in the map it is added.
};

template<typename Key, typename Value> HashMap<Key,Value>::HashMap(const HashMap& p)
{
   *this = p;
}


//-----------------------------------------------------------------------------
// add & remove elements

/// Insert the key value pair but don't allow duplicates.
/// The map class does not allow duplicates keys. If the key already exists in
/// the map the function will fail and return end().
template<typename Key, typename Value>
typename HashMap<Key,Value>::iterator HashMap<Key,Value>::insert(const Key& key, const Value& x)
{
    if (mHashMap.find(key) != mHashMap.end())
        return mHashMap.end();

    mHashMap.insert(pair(key,x));
    return mHashMap.find(key);
}


#endif// _HASHTABLE_H
