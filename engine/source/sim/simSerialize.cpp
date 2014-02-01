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

#include "console/console.h"
#include "sim/simBase.h"
#include "io/StreamFn.h"
#include "io/resource/resourceManager.h"

#include "simSerialize_ScriptBinding.h"

//////////////////////////////////////////////////////////////////////////
// SimObject Methods
//////////////////////////////////////////////////////////////////////////

bool SimObject::writeObject(std::iostream &stream)
{
   clearFieldFilters();
   buildFilterList();

//    if (getName())
//    {
//        S32 len = dStrlen(getName());
//    }

   StreamFn::writeString(stream, getName() ? getName() : "");

   // Static fields
   AbstractClassRep *rep = getClassRep();
   AbstractClassRep::FieldList &fieldList = rep->mFieldList;

   auto savePos = stream.tellp();
   auto numFields = fieldList.size();
   stream << numFields;

   for(auto itr : fieldList )
   {
      if(itr.type >= AbstractClassRep::StartGroupFieldType || isFiltered(itr.pFieldname))
      {
         numFields--;
         continue;
      }

      const char *field = getDataField(itr.pFieldname, nullptr);
      if(field == nullptr)
         field = "";

       StreamFn::writeString(stream, itr.pFieldname);
       StreamFn::writeString(stream, field);
   }

   // Dynamic Fields
   if(mCanSaveFieldDictionary)
   {
      SimFieldDictionary * fieldDictionary = getFieldDictionary();
      for(SimFieldDictionary::Iterator ditr = fieldDictionary->begin(); ditr != fieldDictionary->end(); ++ditr)
      {
         SimFieldDictionary::Entry * entry = ditr->second;

         if(isFiltered(entry->slotName))
            continue;

          StreamFn::writeString(stream, entry->slotName);
          StreamFn::writeString(stream, entry->value);
         numFields++;
      }
   }

   // Overwrite the number of fields with the correct value
   auto savePos2 = stream.tellp();
   stream.seekp(savePos);
   stream << (numFields);
   stream.seekp(savePos2);

   return true;
}

bool SimObject::readObject(std::iostream &stream)
{
   const char *name = StringTable->readStream(stream, true);
   if(name && *name)
      assignName(name);

   U32 numFields;
   stream >> numFields;

   for(U32 i = 0;i < numFields;i++)
   {
      const char *fieldName = StringTable->readStream(stream);
      const char *data = StringTable->readStream(stream);

      setDataField(fieldName, nullptr, data);
   }
   return true;
}

//////////////////////////////////////////////////////////////////////////

void SimObject::buildFilterList()
{
   Con::executef(this, 1, "buildFilterList");
}

//////////////////////////////////////////////////////////////////////////

void SimObject::addFieldFilter( const char *fieldName )
{
   StringTableEntry st = StringTable->insert(fieldName);
   for(S32 i = 0;i < mFieldFilter.size();i++)
   {
      if(mFieldFilter[i] == st)
         return;
   }

   mFieldFilter.push_back(st);
}

void SimObject::removeFieldFilter( const char *fieldName )
{
   StringTableEntry st = StringTable->insert(fieldName);
   for(S32 i = 0;i < mFieldFilter.size();i++)
   {
      if(mFieldFilter[i] == st)
      {
         mFieldFilter.erase(i);
         return;
      }
   }
}

void SimObject::clearFieldFilters()
{
   mFieldFilter.clear();
}

//////////////////////////////////////////////////////////////////////////

bool SimObject::isFiltered( const char *fieldName )
{
   StringTableEntry st = StringTable->insert(fieldName);
   for(S32 i = 0;i < mFieldFilter.size();i++)
   {
      if(mFieldFilter[i] == st)
         return true;
   }

   return false;
}

//////////////////////////////////////////////////////////////////////////
// SimSet Methods
//////////////////////////////////////////////////////////////////////////

bool SimSet::writeObject(std::iostream &stream)
{
   if(! Parent::writeObject(stream))
      return false;

   stream << (U32)(size());
   for(auto i:*this)
   {
      if(! Sim::saveObject(i, stream))
         return false;
   }
   return true;
}

bool SimSet::readObject(std::iostream &stream)
{
   if(! Parent::readObject(stream))
      return false;

   U32 numObj;
   stream >> numObj;

   for(U32 i = 0;i < numObj;i++)
   {
      SimObject *obj = Sim::loadObjectStream(stream);
      if(obj == nullptr)
         return false;

      addObject(obj);
   }

   return true;
}

//////////////////////////////////////////////////////////////////////////
// Sim Functions
//////////////////////////////////////////////////////////////////////////

namespace Sim
{

bool saveObject(SimObject *obj, const char *filename)
{
    std::fstream fs;
   if(ResourceManager->openFileForWrite(fs, filename, std::fstream::out))
   {
      bool ret = saveObject(obj, fs);
      fs.close();

      return ret;
   }
   return false;
}

bool saveObject(SimObject *obj, std::iostream &stream)
{
   stream.write(obj->getClassName(), strlen(obj->getClassName()));
   return obj->writeObject(stream);
}

//////////////////////////////////////////////////////////////////////////

SimObject *loadObjectStream(const char *filename)
{
    std::iostream * stream = ResourceManager->openStream(filename);
   
   if (stream)
   {
      SimObject *ret = loadObjectStream(*stream);
      ResourceManager->closeStream(stream);
      return ret;
   }

   return nullptr;
}

SimObject *loadObjectStream(std::iostream& stream)
{
   const char *className = StringTable->readStream(stream, true);
   ConsoleObject *conObj = ConsoleObject::create(className);
   if(conObj == nullptr)
   {
      Con::errorf("Sim::restoreObjectStream - Could not create object of class \"%s\"", className);
      return nullptr;
   }

   SimObject *simObj = dynamic_cast<SimObject *>(conObj);
   if(simObj == nullptr)
   {
      Con::errorf("Sim::restoreObjectStream - Object of class \"%s\" is not a SimObject", className);
      delete simObj;
      return nullptr;
   }

   if(simObj->readObject(stream))
   {
      simObj->registerObject();
      return simObj;
   }

   delete simObj;
   return nullptr;
}

} // end namespace Sim
