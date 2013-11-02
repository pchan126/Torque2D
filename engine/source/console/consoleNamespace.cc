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

#include "consoleNamespace.h"
#include "console/ast.h"
#include "console/consoleInternal.h"
#include "console/compiler.h"

U32 Namespace::mCacheSequence = 0;
DataChunker Namespace::mCacheAllocator;
DataChunker Namespace::mAllocator;
std::list<Namespace*> Namespace::mNamespaceList;
std::vector<Namespace::Entry*> Namespace::mNamespaceEntryList;
Namespace *Namespace::mGlobalNamespace = nullptr;


Namespace::Entry::Entry()
{
   mCode = nullptr;
   mType = InvalidFunctionType;
}

void Namespace::Entry::clear()
{
   if(mCode)
   {
      mCode->decRefCount();
      mCode = nullptr;
   }

   // Clean up usage strings generated for script functions.
   if( ( mType == Namespace::Entry::ScriptFunctionType ) && mUsage )
   {
      delete mUsage;
      mUsage = nullptr;
   }
}

Namespace::Namespace()
{
   mPackage = nullptr;
   mUsage = nullptr;
   mCleanUpUsage = false;
   mName = nullptr;
   mParent = nullptr;
   mHashTable = nullptr;
   mHashSequence = 0;
   mRefCountToParent = 0;
   mClassRep = 0;
}

Namespace::~Namespace()
{
   if( mUsage && mCleanUpUsage )
   {
      dFree (const_cast <char *> (mUsage));
      mUsage = nullptr;
      mCleanUpUsage = false;
   }
   SAFE_DELETE(mHashTable);
}

void Namespace::clearEntries()
{
   for(Entry *walk : mEntryList)
      walk->clear();
}

Namespace *Namespace::find(StringTableEntry name, StringTableEntry package)
{
   for(Namespace* walk : mNamespaceList)
      if(walk->mName == name && walk->mPackage == package)
         return walk;

   Namespace *ret = new Namespace;
   ret->mPackage = package;
   ret->mName = name;
   mNamespaceList.push_front(ret);
   return ret;
}

bool Namespace::canTabComplete(const char *prevText, const char *bestMatch, const char *newText, S32 baseLen, bool fForward)
{
   // test if it matches the first baseLen chars:
   if(dStrnicmp(newText, prevText, baseLen))
      return false;

   if (fForward)
   {
      if(!bestMatch)
         return dStricmp(newText, prevText) > 0;
      else
         return (dStricmp(newText, prevText) > 0) &&
                (dStricmp(newText, bestMatch) < 0);
   }
   else
   {
      if (dStrlen(prevText) == (U32) baseLen)
      {
         // look for the 'worst match'
         if(!bestMatch)
            return dStricmp(newText, prevText) > 0;
         else
            return dStricmp(newText, bestMatch) > 0;
      }
      else
      {
         if (!bestMatch)
            return (dStricmp(newText, prevText)  < 0);
         else
            return (dStricmp(newText, prevText)  < 0) &&
                   (dStricmp(newText, bestMatch) > 0);
      }
   }
}

bool Namespace::unlinkClass(Namespace *parent)
{
   Namespace *walk = this;
   while(walk->mParent && walk->mParent->mName == mName)
      walk = walk->mParent;

   if(walk->mParent && walk->mParent != parent)
   {
      Con::errorf(ConsoleLogEntry::General, "Namespace::unlinkClass - cannot unlink namespace parent linkage for %s for %s.",
         walk->mName, walk->mParent->mName);
      return false;
   }

   mRefCountToParent--;
   AssertFatal(mRefCountToParent >= 0, "Namespace::unlinkClass - reference count to parent is less than 0");

   if(mRefCountToParent == 0)
      walk->mParent = nullptr;

   trashCache();

   return true;
}


bool Namespace::classLinkTo(Namespace *parent)
{
   Namespace *walk = this;
   while(walk->mParent && walk->mParent->mName == mName)
      walk = walk->mParent;

   if(walk->mParent && walk->mParent != parent)
   {
      Con::errorf(ConsoleLogEntry::General, "Error: cannot change namespace parent linkage of %s from %s to %s.",
         walk->mName, walk->mParent->mName, parent->mName);
      return false;
   }
   mRefCountToParent++;
   walk->mParent = parent;

   trashCache();

   return true;
}

void Namespace::buildHashTable()
{
   if(mHashSequence == mCacheSequence)
      return;

   if(mEntryList.empty() && mParent)
   {
      mParent->buildHashTable();
      mHashTable = mParent->mHashTable;
      mHashSequence = mCacheSequence;
      return;
   }

   U32 entryCount = 0;
   Namespace * ns;
   for(ns = this; ns; ns = ns->mParent)
      for(Entry *walk : ns->mEntryList)
         if(lookupRecursive(walk->mFunctionName) == walk)
            entryCount++;

   mHashTable = new std::unordered_map<StringTableEntry, Entry*>;

   for(ns = this; ns; ns = ns->mParent)
   {
      for(Entry *walk : ns->mEntryList)
      {
         if ((*mHashTable).count(walk->mFunctionName) == 0)
            (*mHashTable)[walk->mFunctionName] = walk;
      }
   }
   mHashSequence = mCacheSequence;
}

void Namespace::init()
{
   // create the global namespace
   mGlobalNamespace = find(nullptr);
}

Namespace *Namespace::global()
{
   return mGlobalNamespace;
}

void Namespace::shutdown()
{
   for(Namespace* walk : mNamespaceList)
      walk->clearEntries();
}

void Namespace::trashCache()
{
   mCacheSequence++;
   mCacheAllocator.freeBlocks();
}

const char *Namespace::tabComplete(const char *prevText, S32 baseLen, bool fForward)
{
   if(mHashSequence != mCacheSequence)
      buildHashTable();

   const char *bestMatch = nullptr;
   for(auto itr: *mHashTable)
   {
       Entry* temp = itr.second;
       if( temp != nullptr && canTabComplete(prevText, bestMatch, temp->mFunctionName, baseLen, fForward))
           bestMatch = temp->mFunctionName;
   }

   return bestMatch;
}

Namespace::Entry *Namespace::lookupRecursive(StringTableEntry name)
{
   for(Namespace *ns = this; ns; ns = ns->mParent)
      for(Entry *walk : ns->mEntryList)
         if(walk->mFunctionName == name)
            return walk;

   return nullptr;
}

Namespace::Entry *Namespace::lookup(StringTableEntry name)
{
   if(mHashSequence != mCacheSequence)
      buildHashTable();

   assert(mHashTable != nullptr);
   
   if (mHashTable->count(name) > 0)
       return mHashTable->at(name);
   else
       return nullptr;
}

//static S32 QSORT_CALLBACK compareEntries(const void* a,const void* b)
//{
//   const Namespace::Entry* fa = *((Namespace::Entry**)a);
//   const Namespace::Entry* fb = *((Namespace::Entry**)b);
//
//   return dStricmp(fa->mFunctionName, fb->mFunctionName);
//}

static bool compareEntries( Namespace::Entry* a, Namespace::Entry* b)
{
    return dStricmp(a->mFunctionName, b->mFunctionName) == -1;
}

void Namespace::getEntryList(Vector<Entry *> *vec)
{
   if(mHashSequence != mCacheSequence)
      buildHashTable();

    for (auto itr: *mHashTable)
    {
        Namespace::Entry *temp = itr.second;
        vec->push_back(temp);
    }

//   dQsort(vec->address(),vec->size(),sizeof(Namespace::Entry *),compareEntries);
    std::sort(vec->begin(), vec->end(), compareEntries);
}

Namespace::Entry *Namespace::createLocalEntry(StringTableEntry name)
{
   for(Entry *walk : mEntryList)
   {
      if(walk->mFunctionName == name)
      {
         walk->clear();
         return walk;
      }
   }

   Entry *ent = (Entry *) mAllocator.alloc(sizeof(Entry));
   constructInPlace(ent);

   ent->mNamespace = this;
   ent->mFunctionName = name;
   ent->mPackage = mPackage;
   mEntryList.push_back( ent );

   mNamespaceEntryList.push_back(ent);
   ent->mID = (U32)mNamespaceEntryList.size();

   return ent;
}

void Namespace::addFunction(StringTableEntry name, CodeBlock *cb, U32 functionOffset, const char* usage)
{
   Entry *ent = createLocalEntry(name);
   trashCache();

   ent->mUsage = usage;
   ent->mCode = cb;
   ent->mFunctionOffset = functionOffset;
   ent->mCode->incRefCount();
   ent->mType = Entry::ScriptFunctionType;
}

void Namespace::addCommand(StringTableEntry name,StringCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Entry *ent = createLocalEntry(name);
   trashCache();

   ent->mUsage = usage;
   ent->mMinArgs = minArgs;
   ent->mMaxArgs = maxArgs;

   ent->mType = Entry::StringCallbackType;
   ent->cb.mStringCallbackFunc = cb;
}

void Namespace::addCommand(StringTableEntry name,IntCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Entry *ent = createLocalEntry(name);
   trashCache();

   ent->mUsage = usage;
   ent->mMinArgs = minArgs;
   ent->mMaxArgs = maxArgs;

   ent->mType = Entry::IntCallbackType;
   ent->cb.mIntCallbackFunc = cb;
}

void Namespace::addCommand(StringTableEntry name,VoidCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Entry *ent = createLocalEntry(name);
   trashCache();

   ent->mUsage = usage;
   ent->mMinArgs = minArgs;
   ent->mMaxArgs = maxArgs;

   ent->mType = Entry::VoidCallbackType;
   ent->cb.mVoidCallbackFunc = cb;
}

void Namespace::addCommand(StringTableEntry name,FloatCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Entry *ent = createLocalEntry(name);
   trashCache();

   ent->mUsage = usage;
   ent->mMinArgs = minArgs;
   ent->mMaxArgs = maxArgs;

   ent->mType = Entry::FloatCallbackType;
   ent->cb.mFloatCallbackFunc = cb;
}

void Namespace::addCommand(StringTableEntry name,BoolCallback cb, const char *usage, S32 minArgs, S32 maxArgs)
{
   Entry *ent = createLocalEntry(name);
   trashCache();

   ent->mUsage = usage;
   ent->mMinArgs = minArgs;
   ent->mMaxArgs = maxArgs;

   ent->mType = Entry::BoolCallbackType;
   ent->cb.mBoolCallbackFunc = cb;
}

void Namespace::addOverload(const char * name, const char *altUsage)
{
   static U32 uid=0;
   char buffer[1024];
   char lilBuffer[32];
   dStrcpy(buffer, name);
   dSprintf(lilBuffer, 32, "_%d", uid++);
   dStrcat(buffer, lilBuffer);

   Entry *ent = createLocalEntry(StringTable->insert( buffer ));
   trashCache();

   ent->mUsage = altUsage;
   ent->mMinArgs = -1;
   ent->mMaxArgs = -2;

   ent->mType = Entry::OverloadMarker;
   ent->cb.mGroupName = name;
}

void Namespace::markGroup(const char* name, const char* usage)
{
   static U32 uid=0;
   char buffer[1024];
   char lilBuffer[32];
   dStrcpy(buffer, name);
   dSprintf(lilBuffer, 32, "_%d", uid++);
   dStrcat(buffer, lilBuffer);

   Entry *ent = createLocalEntry(StringTable->insert( buffer ));
   trashCache();

   if(usage != nullptr)
      lastUsage = (char*)(ent->mUsage = usage);
   else
      ent->mUsage = lastUsage;

   ent->mMinArgs = -1; // Make sure it explodes if somehow we run this entry.
   ent->mMaxArgs = -2;

   ent->mType = Entry::GroupMarker;
   ent->cb.mGroupName = name;
}

extern S32 executeBlock(StmtNode *block, ExprEvalState *state);

const char *Namespace::Entry::execute(S32 argc, const char **argv, ExprEvalState *state)
{
   if(mType == ScriptFunctionType)
   {
      if(mFunctionOffset)
         return mCode->exec(mFunctionOffset, argv[0], mNamespace, argc, argv, false, mPackage);
      else
         return "";
   }

   if((mMinArgs && argc < mMinArgs) || (mMaxArgs && argc > mMaxArgs))
   {
      Con::warnf(ConsoleLogEntry::Script, "%s::%s - wrong number of arguments.", mNamespace->mName, mFunctionName);
      Con::warnf(ConsoleLogEntry::Script, "usage: %s", mUsage);
      return "";
   }

   static char returnBuffer[32];
   switch(mType)
   {
      case StringCallbackType:
         return cb.mStringCallbackFunc(state->thisObject, argc, argv);
      case IntCallbackType:
         dSprintf(returnBuffer, sizeof(returnBuffer), "%d",
            cb.mIntCallbackFunc(state->thisObject, argc, argv));
         return returnBuffer;
      case FloatCallbackType:
         dSprintf(returnBuffer, sizeof(returnBuffer), "%.9g",
            cb.mFloatCallbackFunc(state->thisObject, argc, argv));
         return returnBuffer;
      case VoidCallbackType:
         cb.mVoidCallbackFunc(state->thisObject, argc, argv);
         return "";
      case BoolCallbackType:
         dSprintf(returnBuffer, sizeof(returnBuffer), "%d",
            (U32)cb.mBoolCallbackFunc(state->thisObject, argc, argv));
         return returnBuffer;
   }

   return "";
}

StringTableEntry Namespace::mActivePackages[Namespace::MaxActivePackages];
U32 Namespace::mNumActivePackages = 0;
U32 Namespace::mOldNumActivePackages = 0;

bool Namespace::isPackage(StringTableEntry name)
{
   for(Namespace *walk : mNamespaceList) 
      if(walk->mPackage == name)
         return true;
   return false;
}

void Namespace::activatePackage(StringTableEntry name)
{
   if(mNumActivePackages == MaxActivePackages)
   {
      Con::printf("ActivatePackage(%s) failed - Max package limit reached: %d", name, MaxActivePackages);
      return;
   }
   if(!name)
      return;

   // see if this one's already active
   for(U32 i = 0; i < mNumActivePackages; i++)
      if(mActivePackages[i] == name)
         return;

   // kill the cache
   trashCache();

   // find all the package namespaces...
   for(Namespace *walk :mNamespaceList)
   {
      if(walk->mPackage == name)
      {
         Namespace *parent = Namespace::find(walk->mName);
         // hook the parent
         walk->mParent = parent->mParent;
         parent->mParent = walk;

         // now swap the entries:
         for(Entry *ew : parent->mEntryList)
            ew->mNamespace = walk;

         for(Entry *ew : walk->mEntryList )
            ew->mNamespace = parent;

         walk->mEntryList.swap( parent->mEntryList );
      }
   }
   mActivePackages[mNumActivePackages++] = name;
}

void Namespace::deactivatePackage(StringTableEntry name)
{
   S32 i, j;
   for(i = 0; i < (S32)mNumActivePackages; i++)
      if(mActivePackages[i] == name)
         break;
   if(i == mNumActivePackages)
      return;

   trashCache();

   for(j = mNumActivePackages - 1; j >= i; j--)
   {
      // gotta unlink em in reverse order...
      for(Namespace *walk : mNamespaceList)
      {
         if(walk->mPackage == mActivePackages[j])
         {
            Namespace *parent = Namespace::find(walk->mName);
            // hook the parent
            parent->mParent = walk->mParent;
            walk->mParent = nullptr;

            // now swap the entries:
            for(Entry *ew : parent->mEntryList)
               ew->mNamespace = walk;

            for(Entry *ew : walk->mEntryList)
               ew->mNamespace = parent;

            walk->mEntryList.swap( parent->mEntryList );
         }
      }
   }
   mNumActivePackages = i;
}

void Namespace::unlinkPackages()
{
   mOldNumActivePackages = mNumActivePackages;
   if(!mNumActivePackages)
      return;
   deactivatePackage(mActivePackages[0]);
}

void Namespace::relinkPackages()
{
   if(!mOldNumActivePackages)
      return;
   for(U32 i = 0; i < mOldNumActivePackages; i++)
      activatePackage(mActivePackages[i]);
}

ConsoleFunctionGroupBegin( Packages, "Functions relating to the control of packages.");

ConsoleFunction(isPackage,bool,2,2,"( packageName ) Use the isPackage function to check if the name or ID specified in packageName is a valid package.\n"
                                                                "@param packagename The name or ID of an existing package.\n"
                                                                "@return Returns true if packageName is a valid package, false otherwise.\n"
                                                                "@sa activatePackage, deactivatePackage")
{
   StringTableEntry packageName = StringTable->insert(argv[1]);
   return Namespace::isPackage(packageName);
}

ConsoleFunction(activatePackage, void,2,2,"( packageName ) Use the activatePackage function to activate a package definition and to re-define all functions named within this package with the definitions provided in the package body.\n"
                                                                "This pushes the newly activated package onto the top of the package stack.\n"
                                                                "@param packagename The name or ID of an existing package.\n"
                                                                "@return No return value.\n"
                                                                "@sa deactivatePackage, isPackage")
{
   StringTableEntry packageName = StringTable->insert(argv[1]);
   Namespace::activatePackage(packageName);
}

ConsoleFunction(deactivatePackage, void,2,2,"( packageName ) Use the deactivatePackage function to deactivate a package definition and to pop any definitions from this package off the package stack.\n"
                                                                "This also causes any subsequently stacked packages to be popped. i.e. If any packages were activated after the one specified in packageName, they too will be deactivated and popped.\n"
                                                                "@param packagename The name or ID of an existing package.\n"
                                                                "@return No return value.\n"
                                                                "@sa activatePackage, isPackage")
{
   StringTableEntry packageName = StringTable->insert(argv[1]);
   Namespace::deactivatePackage(packageName);
}

ConsoleFunctionGroupEnd( Packages );
