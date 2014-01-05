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
#include "console/consoleObject.h"
#include "string/stringTable.h"
#include "algorithm/crc.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/consoleTypeValidators.h"
#include "math/mMath.h"

AbstractClassRep *                 AbstractClassRep::classLinkList = nullptr;
static AbstractClassRep::FieldList sg_tempFieldList;
size_t                                AbstractClassRep::NetClassCount  [NetClassGroupsCount][NetClassTypesCount] = {{0, },};
size_t                                AbstractClassRep::NetClassBitSize[NetClassGroupsCount][NetClassTypesCount] = {{0, },};

AbstractClassRep **                AbstractClassRep::classTable[NetClassGroupsCount][NetClassTypesCount];

U32                                AbstractClassRep::classCRC[NetClassGroupsCount] = {INITIAL_CRC_VALUE, };
bool                               AbstractClassRep::initialized = false;

//--------------------------------------
const AbstractClassRep::Field *AbstractClassRep::findField(StringTableEntry name) const
{
   for(U32 i = 0; i < (U32)mFieldList.size(); i++)
      if(mFieldList[i].pFieldname == name)
         return &mFieldList[i];

   return nullptr;
}

//-----------------------------------------------------------------------------

AbstractClassRep* AbstractClassRep::findFieldRoot( StringTableEntry fieldName )
{
    // Find the field.
    const Field* pField = findField( fieldName );

    // Finish if not found.
    if ( pField == nullptr )
        return nullptr;

    // We're the root if we have no parent.
    if ( getParentClass() == nullptr )
        return this;

    // Find the field root via the parent.
    AbstractClassRep* pFieldRoot = getParentClass()->findFieldRoot( fieldName );

    // We're the root if the parent does not have it else return the field root.
    return pFieldRoot == nullptr ? this : pFieldRoot;
}

//-----------------------------------------------------------------------------

AbstractClassRep* AbstractClassRep::findContainerChildRoot( AbstractClassRep* pChild )
{
    // Fetch container child.
    AbstractClassRep* pContainerChildClass = getContainerChildClass( true );

    // Finish if not found.
    if ( pContainerChildClass == nullptr )
        return nullptr;

    // We're the root for the child if we have no parent.
    if ( getParentClass() == nullptr )
        return this;

    // Find child in parent.
    AbstractClassRep* pParentContainerChildClass = getParentClass()->findContainerChildRoot( pChild );

    // We;re the root if the parent does not contain the child else return the container root.
    return pParentContainerChildClass == nullptr ? this : pParentContainerChildClass;
}

//-----------------------------------------------------------------------------

AbstractClassRep* AbstractClassRep::findClassRep(const char* in_pClassName)
{
   AssertFatal(initialized,
      "AbstractClassRep::findClassRep() - Tried to find an AbstractClassRep before AbstractClassRep::initialize().");

   for (AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass)
      if (dStricmp(walk->getClassName(), in_pClassName) == 0)
         return walk;

   return nullptr;
}

//--------------------------------------
void AbstractClassRep::registerClassRep(AbstractClassRep* in_pRep)
{
   AssertFatal(in_pRep != nullptr, "AbstractClassRep::registerClassRep was passed a nullptr pointer!");
   assert(in_pRep != nullptr);

#ifdef TORQUE_DEBUG  // assert if this class is already registered.
   for(AbstractClassRep *walk = classLinkList; walk; walk = walk->nextClass)
   {
      AssertFatal(dStricmp(in_pRep->mClassName, walk->mClassName) != 0,
         "Duplicate class name registered in AbstractClassRep::registerClassRep()");
   }
#endif

   in_pRep->nextClass = classLinkList;
   classLinkList = in_pRep;
}

//--------------------------------------

ConsoleObject* AbstractClassRep::create(const char* in_pClassName)
{
   AssertFatal(initialized,
      "AbstractClassRep::create() - Tried to create an object before AbstractClassRep::initialize().");

   const AbstractClassRep *rep = AbstractClassRep::findClassRep(in_pClassName);
   if(rep)
      return rep->create();

   AssertWarn(0, avar("Couldn't find class rep for dynamic class: %s", in_pClassName));
   return nullptr;
}

//--------------------------------------
ConsoleObject* AbstractClassRep::create(const U32 groupId, const U32 typeId, const U32 in_classId)
{
   AssertFatal(initialized,
      "AbstractClassRep::create() - Tried to create an object before AbstractClassRep::initialize().");
   AssertFatal(in_classId < NetClassCount[groupId][typeId],
      "AbstractClassRep::create() - Class id out of range.");
   AssertFatal(classTable[groupId][typeId][in_classId] != nullptr,
      "AbstractClassRep::create() - No class with requested ID type.");

   // Look up the specified class and create it.
   if(classTable[groupId][typeId][in_classId])
      return classTable[groupId][typeId][in_classId]->create();

   return nullptr;
}

//--------------------------------------

//static S32 QSORT_CALLBACK ACRCompare(const void *aptr, const void *bptr)
//{
//   const AbstractClassRep *a = *((const AbstractClassRep **) aptr);
//   const AbstractClassRep *b = *((const AbstractClassRep **) bptr);
//
//   if(a->mClassType != b->mClassType)
//      return a->mClassType - b->mClassType;
//   return dStricmp(a->getClassName(), b->getClassName());
//}

static bool ACRCompare(const AbstractClassRep *aptr, AbstractClassRep *bptr)
{
    if(aptr->mClassType != bptr->mClassType)
        return aptr->mClassType < bptr->mClassType;
    return dStricmp(aptr->getClassName(), bptr->getClassName()) == -1;
}


void AbstractClassRep::initialize()
{
   AssertFatal(!initialized, "Duplicate call to AbstractClassRep::initialize()!");
   Vector<AbstractClassRep *> dynamicTable(__FILE__, __LINE__);

   AbstractClassRep *walk;

   // Initialize namespace references...
   for (walk = classLinkList; walk; walk = walk->nextClass)
   {
      walk->mNamespace = Con::lookupNamespace(StringTable->insert(walk->getClassName()));
      walk->mNamespace->mClassRep = walk;
   }

   // Initialize field lists... (and perform other console registration).
   for (walk = classLinkList; walk; walk = walk->nextClass)
   {
      // sg_tempFieldList is used as a staging area for field lists
      // (see addField, addGroup, etc.)
      sg_tempFieldList.setSize(0);

      walk->init();

      // So if we have things in it, copy it over...
      if (sg_tempFieldList.size() != 0)
      {
         if( !walk->mFieldList.size())
            walk->mFieldList = sg_tempFieldList;
         else
            destroyFieldValidators( sg_tempFieldList );
      }

      // And of course delete it every round.
      sg_tempFieldList.clear();
   }

   // Calculate counts and bit sizes for the various NetClasses.
   for (U32 group = 0; group < NetClassGroupsCount; group++)
   {
      U32 groupMask = 1 << group;

      // Specifically, for each NetClass of each NetGroup...
      for(U32 type = 0; type < NetClassTypesCount; type++)
      {
         // Go through all the classes and find matches...
         for (walk = classLinkList; walk; walk = walk->nextClass)
         {
            if(walk->mClassType == type && walk->mClassGroupMask & groupMask)
               dynamicTable.push_back(walk);
         }

         // Set the count for this NetGroup and NetClass
         NetClassCount[group][type] = dynamicTable.size();
         if(!NetClassCount[group][type])
            continue; // If no classes matched, skip to next.

         // Sort by type and then by name.
//         dQsort((void *)dynamicTable.address(), dynamicTable.size(), sizeof(AbstractClassRep *), ACRCompare);
          std::sort(dynamicTable.begin(), dynamicTable.end(), ACRCompare);

         // Allocate storage in the classTable
         classTable[group][type] = new AbstractClassRep*[NetClassCount[group][type]];

         // Fill this in and assign class ids for this group.
         for(U32 i = 0; i < NetClassCount[group][type];i++)
         {
            classTable[group][type][i] = dynamicTable[i];
            dynamicTable[i]->mClassId[group] = i;
         }

         // And calculate the size of bitfields for this group and type.
         NetClassBitSize[group][type] =
               getBinLog2(getNextPow2((U32)(NetClassCount[group][type] + 1)));

         dynamicTable.clear();
      }
   }

   // Ok, we're golden!
   initialized = true;

}

void AbstractClassRep::destroyFieldValidators( AbstractClassRep::FieldList &mFieldList )
{
   for(S32 i = (S32)mFieldList.size()-1; i>=0; i-- )
   {
      ConsoleTypeValidator **p = &mFieldList[i].validator;
      if( *p )
      {
         delete *p;
         *p = nullptr;
      }
   }
}

//------------------------------------------------------------------------------
//-------------------------------------- ConsoleObject

char replacebuf[1024];
char* suppressSpaces(const char* in_pname)
{
    U32 i = 0;
    char chr;
    do
    {
        chr = in_pname[i];
        replacebuf[i++] = (chr != 32) ? chr : '_';
    } while(chr);

    return replacebuf;
}

void ConsoleObject::addGroup(const char* in_pGroupname, const char* in_pGroupDocs)
{
   // Remove spaces.
   char* pFieldNameBuf = suppressSpaces(in_pGroupname);

   // Append group type to fieldname.
   dStrcat(pFieldNameBuf, "_begingroup");

   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(pFieldNameBuf);
   f.pGroupname   = StringTable->insert(in_pGroupname);

   if(in_pGroupDocs)
      f.pFieldDocs   = StringTable->insert(in_pGroupDocs);
   else
      f.pFieldDocs   = nullptr;

   f.type         = AbstractClassRep::StartGroupFieldType;
   f.elementCount = 0;
   f.groupExpand  = false;
   f.validator    = nullptr;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn  = &defaultProtectedWriteFn;

   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::endGroup(const char*  in_pGroupname)
{
   // Remove spaces.
   char* pFieldNameBuf = suppressSpaces(in_pGroupname);

   // Append group type to fieldname.
   dStrcat(pFieldNameBuf, "_endgroup");

   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(pFieldNameBuf);
   f.pGroupname   = StringTable->insert(in_pGroupname);
   f.pFieldDocs   = nullptr;
   f.type         = AbstractClassRep::EndGroupFieldType;
   f.groupExpand  = false;
   f.validator    = nullptr;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn  = &defaultProtectedWriteFn;
   f.elementCount = 0;

   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addArray( const char *arrayName, S32 count )
{
   char *nameBuff = suppressSpaces(arrayName);
   dStrcat(nameBuff, "_beginarray");
   
   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(nameBuff);
   f.pGroupname   = arrayName;
   
   f.type         = AbstractClassRep::StartArrayFieldType;
   f.elementCount = count;
   f.groupExpand  = false;
   f.validator    = nullptr;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   
   // Add to field list.
   sg_tempFieldList.push_back(f);
}

void ConsoleObject::endArray( const char *arrayName )
{
   char *nameBuff = suppressSpaces(arrayName);
   dStrcat(nameBuff, "_endarray");
   
   // Create Field.
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(nameBuff);
   f.pGroupname   = arrayName;
   f.type         = AbstractClassRep::EndArrayFieldType;
   f.groupExpand  = false;
   f.validator    = nullptr;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.elementCount = 0;
   
   // Add to field list.
   sg_tempFieldList.push_back(f);
}


void ConsoleObject::addField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       const char* in_pFieldDocs)
{
   addField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      &defaultProtectedWriteFn,
      1,
           nullptr,
      in_pFieldDocs);
}

void ConsoleObject::addField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       AbstractClassRep::WriteDataNotify in_writeDataFn,
                       const char* in_pFieldDocs)
{
   addField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_writeDataFn,
      1,
           nullptr,
      in_pFieldDocs);
}

void ConsoleObject::addField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       const U32 in_elementCount,
                       EnumTable *in_table,
                       const char* in_pFieldDocs)
{
   addField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      &defaultProtectedWriteFn,
      1,
      in_table,
      in_pFieldDocs);
}

void ConsoleObject::addField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       AbstractClassRep::WriteDataNotify in_writeDataFn,
                       const U32 in_elementCount,
                       EnumTable *in_table,
                       const char* in_pFieldDocs)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(in_pFieldname);
   f.pGroupname   = nullptr;

   if(in_pFieldDocs)
      f.pFieldDocs   = StringTable->insert(in_pFieldDocs);
   else
      f.pFieldDocs   = nullptr;

   f.type         = in_fieldType;
   f.offset       = in_fieldOffset;
   f.elementCount = in_elementCount;
   f.table        = in_table;
   f.validator    = nullptr;

   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn  = in_writeDataFn;

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       AbstractClassRep::SetDataNotify in_setDataFn,
                       AbstractClassRep::GetDataNotify in_getDataFn,
                       const char* in_pFieldDocs)
{
   addProtectedField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_setDataFn,
      in_getDataFn,
      &defaultProtectedWriteFn,
      1,
           nullptr,
      in_pFieldDocs);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       AbstractClassRep::SetDataNotify in_setDataFn,
                       AbstractClassRep::GetDataNotify in_getDataFn,
                       AbstractClassRep::WriteDataNotify in_writeDataFn,
                       const char* in_pFieldDocs)
{
   addProtectedField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_setDataFn,
      in_getDataFn,
      in_writeDataFn,
      1,
      nullptr,
      in_pFieldDocs);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       AbstractClassRep::SetDataNotify in_setDataFn,
                       AbstractClassRep::GetDataNotify in_getDataFn,
                       const U32 in_elementCount,
                       EnumTable *in_table,
                       const char* in_pFieldDocs)
{
   addProtectedField(
      in_pFieldname,
      in_fieldType,
      in_fieldOffset,
      in_setDataFn,
      in_getDataFn,
      &defaultProtectedWriteFn,
      in_elementCount,
      in_table,
      in_pFieldDocs);
}

void ConsoleObject::addProtectedField(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       AbstractClassRep::SetDataNotify in_setDataFn,
                       AbstractClassRep::GetDataNotify in_getDataFn,
                       AbstractClassRep::WriteDataNotify in_writeDataFn,
                       const U32 in_elementCount,
                       EnumTable *in_table,
                       const char* in_pFieldDocs)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(in_pFieldname);
   f.pGroupname   = nullptr;

   if(in_pFieldDocs)
      f.pFieldDocs   = StringTable->insert(in_pFieldDocs);
   else
      f.pFieldDocs   = nullptr;

   f.type         = in_fieldType;
   f.offset       = in_fieldOffset;
   f.elementCount = in_elementCount;
   f.table        = in_table;
   f.validator    = nullptr;

   f.setDataFn    = in_setDataFn;
   f.getDataFn    = in_getDataFn;
   f.writeDataFn  = in_writeDataFn;

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addFieldV(const char*  in_pFieldname,
                       const U32 in_fieldType,
                       const dsize_t in_fieldOffset,
                       ConsoleTypeValidator *v,
                       const char* in_pFieldDocs)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(in_pFieldname);
   f.pGroupname   = nullptr;
   if(in_pFieldDocs)
      f.pFieldDocs   = StringTable->insert(in_pFieldDocs);
   else
      f.pFieldDocs   = nullptr;
   f.type         = in_fieldType;
   f.offset       = in_fieldOffset;
   f.elementCount = 1;
   f.table        = nullptr;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn  = &defaultProtectedWriteFn;
   f.validator    = v;
   v->fieldIndex  = (S32)sg_tempFieldList.size();

   sg_tempFieldList.push_back(f);
}

void ConsoleObject::addDeprecatedField(const char *fieldName)
{
   AbstractClassRep::Field f;
   f.pFieldname   = StringTable->insert(fieldName);
   f.pGroupname   = nullptr;
   f.pFieldDocs   = nullptr;
   f.type         = AbstractClassRep::DeprecatedFieldType;
   f.offset       = 0;
   f.elementCount = 0;
   f.table        = nullptr;
   f.validator    = nullptr;
   f.setDataFn    = &defaultProtectedSetFn;
   f.getDataFn    = &defaultProtectedGetFn;
   f.writeDataFn  = &defaultProtectedWriteFn;

   sg_tempFieldList.push_back(f);
}


bool ConsoleObject::removeField(const char* in_pFieldname)
{
   for (U32 i = 0; i < (U32)sg_tempFieldList.size(); i++) {
      if (dStricmp(in_pFieldname, sg_tempFieldList[i].pFieldname) == 0) {
         sg_tempFieldList.erase(i);
         return true;
      }
   }

   return false;
}

//--------------------------------------
void ConsoleObject::initPersistFields()
{
}

//--------------------------------------
void ConsoleObject::consoleInit()
{
}

ConsoleObject::~ConsoleObject()
{
}

//--------------------------------------
AbstractClassRep* ConsoleObject::getClassRep() const
{
   return nullptr;
}

ConsoleFunction( enumerateConsoleClasses, const char*, 1, 2, "enumerateConsoleClasses(<\"base class\">);")
{
   AbstractClassRep *base = nullptr;    
   if(argc > 1)
   {
      base = AbstractClassRep::findClassRep(argv[1]);
      if(!base)
         return "";
   }
   
   Vector<AbstractClassRep*> classes;
   U32 bufSize = 0;
   for(AbstractClassRep *rep = AbstractClassRep::getClassList(); rep; rep = rep->getNextClass())
   {
      if( !base || rep->isClass(base))
      {
         classes.push_back(rep);
         bufSize += dStrlen(rep->getClassName()) + 1;
      }
   }
   
   if(classes.empty())
      return "";

//   dQsort(classes.address(), classes.size(), sizeof(AbstractClassRep*), ACRCompare);
    std::sort(classes.begin(), classes.end(), ACRCompare);

   char* ret = Con::getReturnBuffer(bufSize);
   dStrcpy( ret, classes[0]->getClassName());
   for( auto itr:classes)
   {
      dStrcat( ret, "\t" );
      dStrcat( ret, itr->getClassName() );
   }
   
   return ret;
}

String ConsoleObject::_getLogMessage(const char* fmt, va_list args) const
{
    String objClass = "UnknownClass";
    if(getClassRep())
        objClass = getClassRep()->getClassName();

    String formattedMessage = String::VToString(fmt, args);
    return String::ToString("%s - Object at %x - %s",
            objClass.c_str(), this, formattedMessage.c_str());
}

void ConsoleObject::logMessage(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    Con::printf(_getLogMessage(fmt, args));
    va_end(args);
}

void ConsoleObject::logWarning(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    Con::warnf(_getLogMessage(fmt, args));
    va_end(args);
}

void ConsoleObject::logError(const char* fmt, ...) const
{
    va_list args;
    va_start(args, fmt);
    Con::errorf(_getLogMessage(fmt, args));
    va_end(args);
}
