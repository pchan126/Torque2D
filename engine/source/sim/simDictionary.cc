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

SimNameDictionary::SimNameDictionary()
{

}

SimNameDictionary::~SimNameDictionary()
{

}

void SimNameDictionary::insert(SimObject* obj)
{
    if(!obj->objectName)
       return;

    std::lock_guard<std::mutex> lock(m_mutex);
    hashTable[std::string(obj->objectName)] = obj;
}

SimObject* SimNameDictionary::find(StringTableEntry name)
{
   // nullptr is a valid lookup - it will always return nullptr
   if(hashTable.empty())
      return nullptr;

   if (hashTable.count(std::string(name))>0)
       return hashTable.at(std::string(name));

   return nullptr;
}

void SimNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

    std::lock_guard<std::mutex> lock(m_mutex);
    hashTable.erase(std::string(obj->objectName));
}

//----------------------------------------------------------------------------

SimManagerNameDictionary::SimManagerNameDictionary()
{

}

SimManagerNameDictionary::~SimManagerNameDictionary()
{

}

void SimManagerNameDictionary::insert(SimObject* obj)
{
   if(!obj->objectName)
      return;

//   Con::printf("SimManagerNameDictionary::insert %s", obj->objectName);

    std::lock_guard<std::mutex> lock(m_mutex);
   hashTable[obj->objectName] = obj;
}

SimObject* SimManagerNameDictionary::find(StringTableEntry name)
{
   // nullptr is a valid lookup - it will always return nullptr
   SimObject* ret = nullptr;

   if (hashTable.count(name)>0)
       ret = hashTable.at(name);

   return ret;
}

void SimManagerNameDictionary::remove(SimObject* obj)
{
   if(!obj->objectName)
      return;

    std::lock_guard<std::mutex> lock(m_mutex);
   hashTable.erase(obj->objectName);
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

SimIdDictionary::SimIdDictionary()
{
}

SimIdDictionary::~SimIdDictionary()
{
}

void SimIdDictionary::insert(SimObject* obj)
{
    std::lock_guard<std::mutex> lock(m_mutex);
   table[obj->getId()] = obj;
}

SimObject* SimIdDictionary::find(SimObjectId id)
{
   if (table.count(id) > 0)
      return table[id];

   return nullptr;
}

void SimIdDictionary::remove(SimObject* obj)
{
    std::lock_guard<std::mutex> lock(m_mutex);
   table.erase(obj->getId());
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

