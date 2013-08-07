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
#include "stringTable.h"
#include "console/console.h"

_StringTable *_gStringTable = NULL;
const U32 _StringTable::csm_stInitSize = 29;
StringTableEntry _StringTable::EmptyString;

//---------------------------------------------------------------
//
// StringTable functions
//
//---------------------------------------------------------------

namespace {
bool sgInitTable = true;
U8   sgHashTable[256];

void initTolowerTable()
{
   for (U32 i = 0; i < 256; i++) {
      U8 c = dTolower(i);
      sgHashTable[i] = c * c;
   }

   sgInitTable = false;
}

} // namespace {}

U32 _StringTable::hashString(const char* str)
{
   if (sgInitTable)
      initTolowerTable();

   U32 ret = 0;
   char c;
   while((c = *str++) != 0) {
      ret <<= 1;
      ret ^= sgHashTable[c];
   }
   return ret;
}

U32 _StringTable::hashStringn(const char* str, S32 len)
{
   if (sgInitTable)
      initTolowerTable();

   U32 ret = 0;
   char c;
   while((c = *str++) != 0 && len--) {
      ret <<= 1;
      ret ^= sgHashTable[c];
   }
   return ret;
}

//--------------------------------------
_StringTable::_StringTable()
{
   // Insert empty string.
   EmptyString = insert("");
}

//--------------------------------------
_StringTable::~_StringTable()
{
    for(std::pair<std::string, StringTableEntry> itr:_table)
        delete (itr.second);
}


//--------------------------------------
void _StringTable::create()
{
    if(!_gStringTable)
    {
        _gStringTable = new _StringTable;
    }
}


//--------------------------------------
void _StringTable::destroy()
{
   AssertFatal(StringTable != NULL, "StringTable::destroy: StringTable does not exist.");
   delete StringTable;
   _gStringTable = NULL;
}

//--------------------------------------
StringTableEntry _StringTable::insert(const char* src, const bool  caseSens)
{
   if ( src == NULL )
       return StringTable->EmptyString;

   MutexHandle mutex;
   mutex.lock(&mMutex, true);

    std::string val(src);
    if (!caseSens)
        std::transform(val.begin(), val.end(), val.begin(), tolower);

   if (_table.count(val) == 0)
   {
       char * cstr = new char [val.length()+1];
       std::strcpy (cstr, src);

       _table[val] = cstr;
   }

   return _table[val];
}

//--------------------------------------
StringTableEntry _StringTable::insertn(const char* src, S32 len, const bool  caseSens)
{
   if ( src == NULL )
       return StringTable->EmptyString;

   MutexHandle mutex;
   mutex.lock(&mMutex, true);

   char val[1024];
   AssertFatal(len < sizeof(val), "Invalid string to insertn");
   dStrncpy(val, src, len);
   val[len] = 0;
   return insert(val, caseSens);
}

//--------------------------------------
StringTableEntry _StringTable::lookup(const char* src, const bool  caseSens)
{
   if ( src == NULL )
       return StringTable->EmptyString;

   MutexHandle mutex;
   mutex.lock(&mMutex, true);

    std::string val(src);
    if (!caseSens)
        std::transform(val.begin(), val.end(), val.begin(), tolower);

   if (_table.find(val) == _table.end())
       return NULL;

   return _table[val];
}

//--------------------------------------
StringTableEntry _StringTable::lookupn(const char* src, S32 len, const bool  caseSens)
{
   if ( src == NULL )
       return StringTable->EmptyString;

    char val[1024];
    AssertFatal(len < sizeof(val), "Invalid string to insertn");
    dStrncpy(val, src, len);
    val[len] = 0;

   return lookup(val, caseSens);
}



