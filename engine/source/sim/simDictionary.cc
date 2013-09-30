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

#include "sim/simDictionary.h"
#include "sim/simBase.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

SimNameDictionary::SimNameDictionary()
{
   mutex = Mutex::createMutex();
}

SimNameDictionary::~SimNameDictionary()
{
   Mutex::destroyMutex(mutex);
}

void SimNameDictionary::insert(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Mutex::lockMutex(mutex);
   
//   if(!hashTable)
//   {
//      hashTable = new SimObject *[DefaultTableSize];
//      hashTableSize = DefaultTableSize;
//      hashEntryCount = 0;
//      S32 i;
//      for(i = 0; i < hashTableSize; i++)
//         hashTable[i] = nullptr;
//   }
//   S32 idx = HashPointer(obj->objectName) % hashTableSize;
//   obj->nextNameObject = hashTable[idx];
//   hashTable[idx] = obj;
//   hashEntryCount++;
//   if(hashEntryCount > hashTableSize)
//   {
//      // resize the hash table
//      S32 i;
//      SimObject *head = nullptr, *walk, *temp;
//   	for(i = 0; i < hashTableSize; i++) {
//   		walk = hashTable[i];
//         while(walk)
//         {
//            temp = walk->nextNameObject;
//            walk->nextNameObject = head;
//            head = walk;
//            walk = temp;
//         }
//   	}
//      delete[] hashTable;
//      hashTableSize = hashTableSize * 2 + 1;
//      hashTable = new SimObject *[hashTableSize];
//
//      for(i = 0; i < hashTableSize;i++)
//         hashTable[i] = nullptr;
//      while(head)
//      {
//         temp = head->nextNameObject;
//         idx = HashPointer(head->objectName) % hashTableSize;
//         head->nextNameObject = hashTable[idx];
//         head = temp;
//      }
//   }
   hashTable[std::string(obj->objectName)] = obj;
   Mutex::unlockMutex(mutex);
}

SimObject* SimNameDictionary::find(StringTableEntry name)
{
   // nullptr is a valid lookup - it will always return nullptr
   if(hashTable.empty())
      return nullptr;
      
   Mutex::lockMutex(mutex);

   if (hashTable.count(std::string(name))>0)
   {
      Mutex::unlockMutex(mutex);
       return hashTable.at(std::string(name));
   }

//   S32 idx = HashPointer(name) % hashTableSize;
//   SimObject *walk = hashTable[idx];
//   while(walk)
//   {
//      if(walk->objectName == name)
//      {
//         Mutex::unlockMutex(mutex);
//         return walk;
//      }
//      walk = walk->nextNameObject;
//   }

   Mutex::unlockMutex(mutex);
   return nullptr;
}

void SimNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Mutex::lockMutex(mutex);

   hashTable.erase(std::string(obj->objectName));
//   SimObject **walk = &hashTable[HashPointer(obj->objectName) % hashTableSize];
//   while(*walk)
//   {
//      if(*walk == obj)
//      {
//         *walk = obj->nextNameObject;
//			obj->nextNameObject = (SimObject*)-1;
//         hashEntryCount--;
//
//         Mutex::unlockMutex(mutex);
//         return;
//      }
//      walk = &((*walk)->nextNameObject);
//   }

   Mutex::unlockMutex(mutex);
}	

//----------------------------------------------------------------------------

SimManagerNameDictionary::SimManagerNameDictionary()
{
//   hashTable = new SimObject *[DefaultTableSize];
//   hashTableSize = DefaultTableSize;
//   hashEntryCount = 0;
//   S32 i;
//   for(i = 0; i < hashTableSize; i++)
//      hashTable[i] = nullptr;
   mutex = Mutex::createMutex();
}

SimManagerNameDictionary::~SimManagerNameDictionary()
{
//   delete[] hashTable;
   Mutex::destroyMutex(mutex);
}

void SimManagerNameDictionary::insert(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Con::printf("SimManagerNameDictionary::insert %s", obj->objectName);
   
   Mutex::lockMutex(mutex);

   hashTable[obj->objectName] = obj;
//   S32 idx = HashPointer(obj->objectName) % hashTableSize;
//   obj->nextManagerNameObject = hashTable[idx];
//   hashTable[idx] = obj;
//   hashEntryCount++;
//   if(hashEntryCount > hashTableSize)
//   {
//      // resize the hash table
//      S32 i;
//      SimObject *head = nullptr, *walk, *temp;
//   	for(i = 0; i < hashTableSize; i++) {
//   		walk = hashTable[i];
//         while(walk)
//         {
//            temp = walk->nextManagerNameObject;
//            walk->nextManagerNameObject = head;
//            head = walk;
//            walk = temp;
//         }
//   	}
//      delete[] hashTable;
//      hashTableSize = hashTableSize * 2 + 1;
//      hashTable = new SimObject *[hashTableSize];
//
//      for(i = 0; i < hashTableSize;i++)
//         hashTable[i] = nullptr;
//      while(head)
//      {
//         temp = head->nextManagerNameObject;
//         idx = HashPointer(head->objectName) % hashTableSize;
//         head->nextManagerNameObject = hashTable[idx];
//         hashTable[idx] = head;
//         head = temp;
//      }
//   }
   
   Mutex::unlockMutex(mutex);
}

SimObject* SimManagerNameDictionary::find(StringTableEntry name)
{
   Con::printf("SimManagerNameDictionary::find %s", name);
   // nullptr is a valid lookup - it will always return nullptr
   SimObject* ret = nullptr;
   Mutex::lockMutex(mutex);

   if (hashTable.count(name)>0)
       ret = hashTable.at(name);

   Mutex::unlockMutex(mutex);
   return ret;
}

void SimManagerNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

   Mutex::lockMutex(mutex);
   hashTable.erase(obj->objectName);

//   SimObject **walk = &hashTable[HashPointer(obj->objectName) % hashTableSize];
//   while(*walk)
//   {
//      if(*walk == obj)
//      {
//         *walk = obj->nextManagerNameObject;
//			obj->nextManagerNameObject = (SimObject*)-1;
//         hashEntryCount--;
//
//         Mutex::unlockMutex(mutex);
//         return;
//      }
//      walk = &((*walk)->nextManagerNameObject);
//   }

   Mutex::unlockMutex(mutex);
}	

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

SimIdDictionary::SimIdDictionary()
{
   for(S32 i = 0; i < DefaultTableSize; i++)
      table[i] = nullptr;
   mutex = Mutex::createMutex();
}

SimIdDictionary::~SimIdDictionary()
{
   Mutex::destroyMutex(mutex);
}

void SimIdDictionary::insert(SimObject* obj)
{
   Mutex::lockMutex(mutex);

   S32 idx = obj->getId() & TableBitMask;
   obj->nextIdObject = table[idx];
   AssertFatal( obj->nextIdObject != obj, "SimIdDictionary::insert - Creating Infinite Loop linking to self!" );
   table[idx] = obj;

   Mutex::unlockMutex(mutex);
}

SimObject* SimIdDictionary::find(S32 id)
{
   Mutex::lockMutex(mutex);

   S32 idx = id & TableBitMask;
   SimObject *walk = table[idx];
   while(walk)
   {
      if(walk->getId() == U32(id))
      {
         Mutex::unlockMutex(mutex);
         return walk;
      }
      walk = walk->nextIdObject;
   }

   Mutex::unlockMutex(mutex);

   return nullptr;
}

void SimIdDictionary::remove(SimObject* obj)
{
   Mutex::lockMutex(mutex);

   SimObject **walk = &table[obj->getId() & TableBitMask];
   while(*walk && *walk != obj)
      walk = &((*walk)->nextIdObject);
   if(*walk)
      *walk = obj->nextIdObject;

   Mutex::unlockMutex(mutex);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

