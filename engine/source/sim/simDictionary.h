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

#ifndef _SIMDICTIONARY_H_
#define _SIMDICTIONARY_H_

#include "platform/platform.h"
#include "string/stringTable.h"
#include "platform/threads/mutex.h"
#include <unordered_map>
#include "simObject.h"


//----------------------------------------------------------------------------
/// Map of names to SimObjects
///
/// Provides fast lookup for name->object and
/// for fast removal of an object given object*
class SimNameDictionary
{
   enum
   {
      DefaultTableSize = 29
   };

   std::unordered_map<std::string, SimObject *> hashTable;  // hash the pointers of the names...

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(StringTableEntry name);

   SimNameDictionary();
   ~SimNameDictionary();
};

class SimManagerNameDictionary
{
   enum
   {
      DefaultTableSize = 29
   };

   std::unordered_map<StringTableEntry, SimObject*> hashTable;  // hash the pointers of the names...

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(StringTableEntry name);

   SimManagerNameDictionary();
   ~SimManagerNameDictionary();
};

//----------------------------------------------------------------------------
/// Map of ID's to SimObjects.
///
/// Provides fast lookup for ID->object and
/// for fast removal of an object given object*
class SimIdDictionary
{
   std::unordered_map<SimObjectId, SimObject*> table;

   void *mutex;

public:
   void insert(SimObject* obj);
   void remove(SimObject* obj);
   SimObject* find(SimObjectId id);

   SimIdDictionary();
   ~SimIdDictionary();
};

#endif //_SIMDICTIONARY_H_
