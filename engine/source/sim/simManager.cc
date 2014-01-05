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
#include "platform/threads/mutex.h"
#include "sim/simBase.h"
#include "string/stringTable.h"
#include "console/console.h"
#include "io/resource/resourceManager.h"
#include "io/fileObject.h"
#include "console/consoleInternal.h"
#include "memory/safeDelete.h"

//---------------------------------------------------------------------------

// We comment out the implementation of the Con namespace when doxygenizing because
// otherwise Doxygen decides to ignore our docs in console.h
#ifndef DOXYGENIZING

namespace Sim
{
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// event queue variables:

SimTime gCurrentTime;
SimTime gTargetTime;

std::recursive_mutex gEventQueueMutex;
SimEvent *gEventQueue;
U32 gEventSequence;

//---------------------------------------------------------------------------
// event queue init/shutdown

void initEventQueue()
{
   gCurrentTime = 0;
   gTargetTime = 0;
   gEventSequence = 1;
   gEventQueue = nullptr;
}

void shutdownEventQueue()
{
   // Delete all pending events
   std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   SimEvent *walk = gEventQueue;
   while(walk)
   {
      SimEvent *temp = walk->nextEvent;
      delete walk;
      walk = temp;
   }
}

//---------------------------------------------------------------------------
// event post

U32 postEvent(SimObject *destObject, SimEvent* event,U32 time)
{
    AssertFatal(time == -1 || time >= getCurrentTime(),
        "Sim::postEvent: Cannot go back in time. (flux capacitor unavailable -- BJG)");
   AssertFatal(destObject, "Destination object for event doesn't exist.");

//    std::lock_guard<std::mutex> lock(gEventQueueMutex);

   if( time == -1 )
      time = gCurrentTime;

   event->time = time;
   event->startTime = gCurrentTime;
   event->destObject = destObject;

   if(!destObject)
   {
      delete event;
      return InvalidEventId;
   }
   event->sequenceCount = gEventSequence++;
   SimEvent **walk = &gEventQueue;
   SimEvent *current;
   
   while((current = *walk) != nullptr && (current->time < event->time))
      walk = &(current->nextEvent);
   
   // [tom, 6/24/2005] This ensures that SimEvents are dispatched in the same order that they are posted.
   // This is needed to ensure Con::threadSafeExecute() executes script code in the correct order.
   while((current = *walk) != nullptr && (current->time == event->time))
      walk = &(current->nextEvent);
   
   event->nextEvent = current;
   *walk = event;

   U32 seqCount = event->sequenceCount;

   return seqCount;
}

//---------------------------------------------------------------------------
// event cancellation

void cancelEvent(U32 eventSequence)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);

   SimEvent **walk = &gEventQueue;
   SimEvent *current;
   
   while((current = *walk) != nullptr)
   {
      if(current->sequenceCount == eventSequence)
      {
         *walk = current->nextEvent;
         delete current;
         return;
      }
      else
         walk = &(current->nextEvent);
   }
}

void cancelPendingEvents(SimObject *obj)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);

   SimEvent **walk = &gEventQueue;
   SimEvent *current;
   
   while((current = *walk) != nullptr)
   {
      if(current->destObject == obj)
      {
         *walk = current->nextEvent;
         delete current;
      }
      else
         walk = &(current->nextEvent);
   }
}

//---------------------------------------------------------------------------
// event pending test

bool isEventPending(U32 eventSequence)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);

   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
         return true;

   return false;
}

U32 getEventTimeLeft(U32 eventSequence)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);

   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
      {
         SimTime t = walk->time - getCurrentTime();
         return t;
      }

   return 0;
}

U32 getScheduleDuration(U32 eventSequence)
{
   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
         return (walk->time-walk->startTime);
   return 0;
}

U32 getTimeSinceStart(U32 eventSequence)
{
   for(SimEvent *walk = gEventQueue; walk; walk = walk->nextEvent)
      if(walk->sequenceCount == eventSequence)
         return (getCurrentTime()-walk->startTime);
   return 0;
}

//---------------------------------------------------------------------------
// event timing
    
void advanceToTime(SimTime targetTime)
{
   AssertFatal(targetTime >= getCurrentTime(), "EventQueue::process: cannot advance to time in the past.");

    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   gTargetTime = targetTime;
   while(gEventQueue && gEventQueue->time <= targetTime)
   {
      SimEvent *event = gEventQueue;
      gEventQueue = gEventQueue->nextEvent;
      AssertFatal(event->time >= gCurrentTime,
            "SimEventQueue::pop: Cannot go back in time (flux capacitor not installed - BJG).");
      gCurrentTime = event->time;
      SimObject *obj = event->destObject;

      if(!obj->isDeleted())
         event->process(obj);
      delete event;
   }
    gCurrentTime = targetTime;
}

void advanceTime(SimTime delta)
{
   advanceToTime(getCurrentTime() + delta);
}

U32 getCurrentTime()
{
   SimTime t = gCurrentTime;
   return t;
}

U32 getTargetTime()
{
   return gTargetTime;
}

//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

SimGroup *gRootGroup = nullptr;
SimManagerNameDictionary *gNameDictionary = nullptr;
SimIdDictionary *gIdDictionary;
U32 gNextObjectId;

void initRoot()
{
   gIdDictionary = new SimIdDictionary;
   gNameDictionary = new SimManagerNameDictionary;

   gRootGroup = new SimGroup();
   gRootGroup->setId(RootGroupId);
   gRootGroup->assignName("RootGroup");
   gRootGroup->registerObject();

   gNextObjectId = DynamicObjectIdFirst;
}

void shutdownRoot()
{
   gRootGroup->deleteObject();

   SAFE_DELETE(gNameDictionary);
   SAFE_DELETE(gIdDictionary);
}

//---------------------------------------------------------------------------

SimObject* findObject(const char* name)
{
   // Play nice with bad code - JDD
   if( !name )
      return nullptr;

   SimObject *obj;
   char c = *name;
   if(c == '/')
      return gRootGroup->findObject(name + 1 );
   if(c >= '0' && c <= '9')
   {
      // it's an id group
      const char* temp = name + 1;
      for(;;)
      {
         c = *temp++;
         if(!c)
            return findObject(dAtoi(name));
         else if(c == '/')
         {
            obj = findObject(dAtoi(name));
            if(!obj)
               return nullptr;
            return obj->findObject(temp);
         }
      }
   }
   S32 len;

   //-Mat ensure > 0, instead of just != 0 (prevent running through bogus memory on non-nullptr-terminated strings)
   for(len = 0; name[len] > 0 && name[len] != '/'; len++)
      ;
   StringTableEntry stName = StringTable->lookupn(name, len);
   if(!stName)
      return nullptr;
   obj = gNameDictionary->find(stName);
   if(!name[len])
      return obj;
   if(!obj)
      return nullptr;
   return obj->findObject(name + len + 1);
}

SimObject* findObject(SimObjectId id)
{
    return gIdDictionary->find(id);
}

SimGroup *getRootGroup()
{
   return gRootGroup;
}

   String getUniqueName( const char *inName )
   {
      String outName( inName );
      
      if ( outName.isEmpty() )
         return String::EmptyString;
      
      SimObject *dummy;
      
      if ( !Sim::findObject( outName, dummy ) )
         return outName;
      
      S32 suffixNumb = -1;
      String nameStr( String::GetTrailingNumber( outName, suffixNumb ) );
      suffixNumb = mAbs( suffixNumb ) + 1;
      
#define MAX_TRIES 100
      
      for ( U32 i = 0; i < MAX_TRIES; i++ )
      {
         outName = String::ToString( "%s%d", nameStr.c_str(), suffixNumb );
         
         if ( !Sim::findObject( outName, dummy ) )
            return outName;
         
         suffixNumb++;
      }
      
      Con::errorf( "Sim::getUniqueName( %s ) - failed after %d attempts", inName, MAX_TRIES );
      return String::EmptyString;
   }
   
   String getUniqueInternalName( const char *inName, SimSet *inSet, bool searchChildren )
   {
      // Since SimSet::findObjectByInternalName operates with StringTableEntry(s)
      // we have to muck up the StringTable with our attempts.
      // But then again, so does everywhere else.
      
      StringTableEntry outName = StringTable->insert( inName );
      
      if ( !outName || !outName[0] )
         return String::EmptyString;
      
      if ( !inSet->findObjectByInternalName( outName, searchChildren ) )
         return String(outName);
      
      S32 suffixNumb = -1;
      String nameStr( String::GetTrailingNumber( outName, suffixNumb ) );
      suffixNumb++;
      
      static char tempStr[512];
      
#define MAX_TRIES 100
      
      for ( U32 i = 0; i < MAX_TRIES; i++ )
      {
         dSprintf( tempStr, 512, "%s%d", nameStr.c_str(), suffixNumb );
         outName = StringTable->insert( tempStr );
         
         if ( !inSet->findObjectByInternalName( outName, searchChildren ) )
            return String(outName);         
         
         suffixNumb++;
      }
      
      Con::errorf( "Sim::getUniqueInternalName( %s ) - failed after %d attempts", inName, MAX_TRIES );
      return String::EmptyString;
   }
   
   bool isValidObjectName( const char* name )
   {
      if( !name || !name[ 0 ] )
         return true; // Anonymous object.
      
      if( !dIsalpha( name[ 0 ] ) && name[ 0 ] != '_' )
         return false;
      
      for( U32 i = 1; name[ i ]; ++ i )
         if( !dIsalnum( name[ i ] ) && name[ i ] != '_' )
            return false;
      
      return true;
   }
//---------------------------------------------------------------------------
//---------------------------------------------------------------------------

#define InstantiateNamedSet(set) g##set = new SimSet; g##set->registerObject(#set); gRootGroup->addObject(g##set); SIMSET_SET_ASSOCIATION((*g##set))
#define InstantiateNamedGroup(set) g##set = new SimGroup; g##set->registerObject(#set); gRootGroup->addObject(g##set); SIMSET_SET_ASSOCIATION((*g##set))

SimDataBlockGroup *gDataBlockGroup;
SimDataBlockGroup *getDataBlockGroup()
{
   return gDataBlockGroup;
}


void init()
{
   initEventQueue();
   initRoot();

   InstantiateNamedSet(ActiveActionMapSet);
   InstantiateNamedSet(GhostAlwaysSet);
   InstantiateNamedGroup(ActionMapGroup);
   InstantiateNamedGroup(ClientGroup);
   InstantiateNamedGroup(GuiGroup);
   InstantiateNamedGroup(GuiDataGroup);
   InstantiateNamedGroup(TCPGroup);
   InstantiateNamedGroup(ClientConnectionGroup);
   InstantiateNamedGroup(ChunkFileGroup);

   InstantiateNamedSet(BehaviorSet);
   InstantiateNamedSet(AchievementSet);

   gDataBlockGroup = new SimDataBlockGroup();
   gDataBlockGroup->registerObject("DataBlockGroup");
   gRootGroup->addObject(gDataBlockGroup);
}

void shutdown()
{
   shutdownRoot();
   shutdownEventQueue();
}

} // Sim Namespace.


#endif // DOXYGENIZING.

SimDataBlockGroup::SimDataBlockGroup()
{
   mLastModifiedKey = 0;
}

//S32 QSORT_CALLBACK SimDataBlockGroup::compareModifiedKey(const void* a,const void* b)
//{
//    return (reinterpret_cast<const SimDataBlock* >(a))->getModifiedKey() -
//        (reinterpret_cast<const SimDataBlock*>(b))->getModifiedKey();
//}

bool SimDataBlockGroup::compareModifiedKey(const SimObject * a, const SimObject * b)
{
    return (reinterpret_cast<const SimDataBlock* >(a))->getModifiedKey() <
             (reinterpret_cast<const SimDataBlock*>(b))->getModifiedKey();
}

void SimDataBlockGroup::sort()
{
   if(mLastModifiedKey != SimDataBlock::getNextModifiedKey())
   {
      mLastModifiedKey = SimDataBlock::getNextModifiedKey();
//        dQsort(objectList.address(),objectList.size(),sizeof(SimObject *),compareModifiedKey);
       std::sort(objectList.begin(),objectList.end(),compareModifiedKey);
   }
}
