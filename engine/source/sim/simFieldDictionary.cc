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

#include "sim/simFieldDictionary.h"
//#include "memory/dataChunker.h"
#include "console/consoleInternal.h"
#include "memory/frameAllocator.h"

//-----------------------------------------------------------------------------

std::list<SimFieldDictionary::Entry*> *SimFieldDictionary::mFreeList = new std::list<SimFieldDictionary::Entry*>;

//static Chunker<SimFieldDictionary::Entry> fieldChunker;

//U32 SimFieldDictionary::getHashValue( StringTableEntry slotName )
//{
//   return HashPointer( slotName ) % HashTableSize;
//}
//
//U32 SimFieldDictionary::getHashValue( const String& fieldName )
//{
//   return getHashValue( StringTable->insert( fieldName ) );
//}

//SimFieldDictionary::Entry *SimFieldDictionary::allocEntry()
//{
//   if(mFreeList)
//   {
//      Entry *ret = mFreeList;
//      mFreeList = ret->next;
//      return ret;
//   }
//   else
//      return fieldChunker.alloc();
//}

//void SimFieldDictionary::freeEntry(SimFieldDictionary::Entry *ent)
//{
////   ent->next = mFreeList;
////   mFreeList = ent;
//}

SimFieldDictionary::SimFieldDictionary()
{
   mHashTable.clear();
   mVersion = 0;
}

SimFieldDictionary::~SimFieldDictionary()
{
   for (auto itr:mHashTable)
   {
       Entry *walk = itr.second;
       dFree(walk->value);
       mFreeList->push_back(walk);
//       freeEntry(temp);
   }

//   for(U32 i = 0; i < HashTableSize; i++)
//   {
//      for(Entry *walk = mHashTable[i]; walk;)
//      {
//         Entry *temp = walk;
//         walk = temp->next;
//
//         dFree(temp->value);
//         freeEntry(temp);
//      }
//   }
}

void SimFieldDictionary::setFieldValue(StringTableEntry slotName, const char *value)
{
//   U32 bucket = HashPointer(slotName) % HashTableSize;
//   Entry **walk = &mHashTable[bucket];
//   while(*walk && (*walk)->slotName != slotName)
//      walk = &((*walk)->next);

//   Entry *field = *walk;
   auto itr = mHashTable.find(std::string(slotName));
   if(!*value)
   {
        if(itr != mHashTable.end())
        {
            Entry *field = itr->second;
            mVersion++;

            dFree(field->value);
            mHashTable.erase(itr);
            mFreeList->push_back(field);
        }
   }
   else
   {
      if(itr != mHashTable.end())
      {
         Entry *field = itr->second;
         dFree(field->value);
         field->value = dStrdup(value);
      }
      else
      {
         mVersion++;
         Entry *field = new Entry;

//         field = allocEntry();
         field->value = dStrdup(value);
         field->slotName = slotName;
//         field->next = nullptr;
//         *walk = field;
          mHashTable[std::string(slotName)] = field;
      }
   }
}

const char *SimFieldDictionary::getFieldValue(StringTableEntry slotName)
{
//   U32 bucket = getHashValue( slotName );
//
//   for(Entry *walk = mHashTable[bucket];walk;walk = walk->next)
//      if(walk->slotName == slotName)
//         return walk->value;

    auto itr = mHashTable.find(std::string(slotName));
    if (itr != mHashTable.end())
        return itr->second->value;

   return nullptr;
}

SimFieldDictionary::Entry  *SimFieldDictionary::findDynamicField(const String &fieldName) const
{
//   U32 bucket = getHashValue( fieldName );
//
//   for( Entry *walk = mHashTable[bucket]; walk; walk = walk->next )
//   {
//      if( fieldName.equal(walk->slotName, String::NoCase) )
//         return walk;
//   }
    auto itr = mHashTable.find(std::string(fieldName));
    if (itr != mHashTable.end())
        return itr->second;

   return nullptr;
}

void SimFieldDictionary::assignFrom(SimFieldDictionary *dict)
{
   mVersion++;

   mHashTable.insert(dict->mHashTable.begin(), dict->mHashTable.end());
//   for(U32 i = 0; i < HashTableSize; i++)
//      for(Entry *walk = dict->mHashTable[i];walk; walk = walk->next)
//         setFieldValue(walk->slotName, walk->value);
}

static S32 QSORT_CALLBACK compareEntries(const void* a,const void* b)
{
   SimFieldDictionary::Entry *fa = *((SimFieldDictionary::Entry **)a);
   SimFieldDictionary::Entry *fb = *((SimFieldDictionary::Entry **)b);
   return dStricmp(fa->slotName, fb->slotName);
}

void SimFieldDictionary::writeFields(SimObject *obj, Stream &stream, U32 tabStop)
{

   const AbstractClassRep::FieldList &list = obj->getFieldList();
   Vector<Entry *> flist(__FILE__, __LINE__);

    for (auto itr:mHashTable)
    {
        Entry *walk = itr.second;
        // make sure we haven't written this out yet:
        U32 i;
        for(i = 0; i < (U32)list.size(); i++)
            if(list[i].pFieldname == walk->slotName)
                break;

        if(i != list.size())
            continue;


        if (!obj->writeField(walk->slotName, walk->value))
            continue;

        flist.push_back(walk);
    }


//   for(U32 i = 0; i < HashTableSize; i++)
//   {
//      for(Entry *walk = mHashTable[i];walk; walk = walk->next)
//      {
//          // make sure we haven't written this out yet:
//          U32 i;
//          for(i = 0; i < (U32)list.size(); i++)
//              if(list[i].pFieldname == walk->slotName)
//                  break;
//
//          if(i != list.size())
//              continue;
//
//
//          if (!obj->writeField(walk->slotName, walk->value))
//              continue;
//
//          flist.push_back(walk);
//      }
//   }

   // Sort Entries to prevent version control conflicts
   dQsort(flist.address(),flist.size(),sizeof(Entry *),compareEntries);

   // Save them out
   for(auto itr : flist )
   {
      U32 nBufferSize = (dStrlen( (itr)->value ) * 2) + dStrlen( (itr)->slotName ) + 16;
      FrameTemp<char> expandedBuffer( nBufferSize );

      stream.writeTabs(tabStop+1);

      dSprintf(expandedBuffer, nBufferSize, "%s = \"", (itr)->slotName);
      expandEscape((char*)expandedBuffer + dStrlen(expandedBuffer), (itr)->value);
      dStrcat(expandedBuffer, "\";\r\n");

      stream.write(dStrlen(expandedBuffer),expandedBuffer);
   }

}
void SimFieldDictionary::printFields(SimObject *obj)
{
   const AbstractClassRep::FieldList &list = obj->getFieldList();
   char expandedBuffer[4096];
   Vector<Entry *> flist(__FILE__, __LINE__);

    for (auto itr:mHashTable)
    {
        Entry *walk = itr.second;
         // make sure we haven't written this out yet:
         U32 i;
         for(i = 0; i < (U32)list.size(); i++)
            if(list[i].pFieldname == walk->slotName)
               break;

         if(i != list.size())
            continue;

         flist.push_back(walk);
   }

   dQsort(flist.address(),flist.size(),sizeof(Entry *),compareEntries);

   for(auto itr: flist)
   {
      dSprintf(expandedBuffer, sizeof(expandedBuffer), "  %s = \"", (itr)->slotName);
      expandEscape(expandedBuffer + dStrlen(expandedBuffer), (itr)->value);
      Con::printf("%s\"", expandedBuffer);
   }
}

//------------------------------------------------------------------------------

//SimFieldDictionaryIterator::SimFieldDictionaryIterator(SimFieldDictionary * dictionary)
//{
//   mDictionary = dictionary;
//   mHashIndex = -1;
//   mEntry = 0;
//   operator++();
//}
//
//SimFieldDictionary::Entry* SimFieldDictionaryIterator::operator++()
//{
//   if(!mDictionary)
//      return(mEntry);
//
//   if(mEntry)
//      mEntry = mEntry->next;
//
//   while(!mEntry && (mHashIndex < (SimFieldDictionary::HashTableSize-1)))
//      mEntry = mDictionary->mHashTable[++mHashIndex];
//
//   return(mEntry);
//}
//
//SimFieldDictionary::Entry* SimFieldDictionaryIterator::operator*()
//{
//   return(mEntry);
//}
