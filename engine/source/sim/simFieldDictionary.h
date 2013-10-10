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

#ifndef _SIM_FIELD_DICTIONARY_H_
#define _SIM_FIELD_DICTIONARY_H_

#include "console/console.h"
#include <unordered_map>
#include <list>

//-----------------------------------------------------------------------------

class SimObject;

//-----------------------------------------------------------------------------

/// Dictionary to keep track of dynamic fields on SimObject.

class SimFieldDictionary
{
  public:
   struct Entry
   {
      StringTableEntry slotName;
      char *value;
   };
   std::unordered_map<StringTableEntry, Entry*> mHashTable;

  private:

   static std::list<Entry *>* mFreeList;
   static void freeEntry(Entry *entry);
   
   U32   mNumFields;

   /// In order to efficiently detect when a dynamic field has been
   /// added or deleted, we increment this every time we add or
   /// remove a field.
   U32 mVersion;

public:
   const U32 getVersion() const { return mVersion; }

   SimFieldDictionary();
   ~SimFieldDictionary();
   void setFieldValue(StringTableEntry slotName, const char *value);
   const char *getFieldValue(StringTableEntry slotName);
   Entry  *findDynamicField(const String &fieldName) const;
   Entry  *findDynamicField( StringTableEntry fieldName) const {return findDynamicField(String(fieldName)); };
   void writeFields(SimObject *obj, std::iostream &stream, U32 tabStop);
   void printFields(SimObject *obj);
   void assignFrom(SimFieldDictionary *dict);
   U32   getNumFields() const { return mNumFields; }

   std::unordered_map<StringTableEntry, Entry*>::iterator begin() { return mHashTable.begin(); };
   std::unordered_map<StringTableEntry, Entry*>::iterator end() { return mHashTable.end(); };
   size_t size() {return mHashTable.size(); };

   typedef std::unordered_map<StringTableEntry, Entry*>::iterator Iterator;
};

//-----------------------------------------------------------------------------

#endif // _SIM_FIELD_DICTIONARY_H_
