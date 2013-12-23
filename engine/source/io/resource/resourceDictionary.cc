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
#include "io/resource/resourceManager.h"

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

ResDictionary::ResDictionary()
{
}

ResDictionary::~ResDictionary()
{
}

void ResDictionary::insert(ResourceObject *obj, StringTableEntry path, StringTableEntry file)
{
   if(path)
   {
      char fullPath[1024];
      Platform::makeFullPathName(path, fullPath, sizeof(fullPath));
      path = StringTable->insert(fullPath);
   }

   obj->name = file;
   obj->path = path;

   std::string path_and_file(path);
   path_and_file.append("/");
   path_and_file.append(file);

    ResEntry newEntry(path_and_file, obj);
    hashTable.insert(newEntry);
}

ResourceObject* ResDictionary::find(StringTableEntry path, StringTableEntry name)
{
   if(path)
   {
      char fullPath[1024];
      Platform::makeFullPathName(path, fullPath, sizeof(fullPath));
      path = StringTable->insert(fullPath);
   }

    std::string path_and_file(path);
    path_and_file.append("/");
    path_and_file.append(name);
   

    auto itr = hashTable.find(path_and_file);
    if (itr != hashTable.end())
        return (itr->second);

   return nullptr;
}

ResourceObject* ResDictionary::find(StringTableEntry path, StringTableEntry name, StringTableEntry zipPath, StringTableEntry zipName)
{
   if(path)
   {
      char fullPath[1024];
      Platform::makeFullPathName(path, fullPath, sizeof(fullPath));
      path = StringTable->insert(fullPath);
   }

    std::string path_and_file(path);
    path_and_file.append(name);

    auto range = hashTable.equal_range(path_and_file);
    for (auto itr = range.first; itr != range.second; ++itr )
    {
        ResourceObject* walk = itr->second;
        if(walk->name == name && walk->path == path && walk->zipName == zipName && walk->zipPath == zipPath)
             return walk;
    }

   return nullptr;
}

ResourceObject* ResDictionary::find(StringTableEntry path, StringTableEntry name, U32 flags)
{
   if(path)
   {
      char fullPath[1024];
      Platform::makeFullPathName(path, fullPath, sizeof(fullPath));
      path = StringTable->insert(fullPath);
   }

    std::string path_and_file(path);
    path_and_file.append(name);

    auto range = hashTable.equal_range(path_and_file);
    for (auto itr = range.first; itr != range.second; ++itr )
    {
        ResourceObject* walk = itr->second;
        if(walk->name == name && walk->path == path && U32(walk->flags) == flags)
            return walk;
    }
   return nullptr;
}

void ResDictionary::pushBehind(ResourceObject *resObj, S32 flagMask)
{
   remove(resObj);
   insert(resObj, resObj->path, resObj->name);
}

void ResDictionary::remove(ResourceObject *resObj)
{
   std::string path_and_file(resObj->path);
   path_and_file.append(resObj->name);

   auto range = hashTable.equal_range(path_and_file);
   for (auto itr = range.first; itr != range.second; ++itr )
   {
      ResourceObject* walk = itr->second;
      if(walk == resObj)
      {
         hashTable.erase(itr);
         return;
      }
   }
}
