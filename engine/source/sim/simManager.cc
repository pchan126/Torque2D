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
#include <forward_list>

//---------------------------------------------------------------------------

namespace Sim
{
//---------------------------------------------------------------------------

//---------------------------------------------------------------------------
// event queue variables:

SimTime gCurrentTime;
SimTime gTargetTime;

std::recursive_mutex gEventQueueMutex;
std::forward_list<std::shared_ptr<SimEvent>> gEventQueue;
U32 gEventSequence;

//---------------------------------------------------------------------------
// event queue init/shutdown

void initEventQueue()
{
   gCurrentTime = 0;
   gTargetTime = 0;
   gEventSequence = 1;
}

void shutdownEventQueue()
{
   // Delete all pending events
   std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   gEventQueue.clear();
}

//---------------------------------------------------------------------------
// event post

U32 postEvent(SimObject *destObject, SimEvent* event,U32 time)
{
    AssertFatal(time == -1 || time >= getCurrentTime(),
        "Sim::postEvent: Cannot go back in time. (flux capacitor unavailable -- BJG)");
   AssertFatal(destObject, "Destination object for event doesn't exist.");

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

   gEventQueue.emplace_front(event);
   gEventQueue.sort(SimEvent::compare_time);

   U32 seqCount = event->sequenceCount;

   return seqCount;
}


//---------------------------------------------------------------------------
// event cancellation

/*! cancel a previously scheduled event.
	@param eventSequence The numeric ID of a previously scheduled event.
	@return No return value.
	@sa getEventTimeLeft, getScheduleDuration, getTimeSinceStart, isEventPending, schedule, obj.schedule
*/
void cancelEvent(U32 eventSequence)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);

   gEventQueue.remove_if(SimEvent::i_match(eventSequence));
}

void cancelPendingEvents(SimObject *obj)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   gEventQueue.remove_if(SimEvent::e_match(obj));
}

//---------------------------------------------------------------------------
// event pending test

/*!	see if the event associated with eventID is still pending.

	When an event passes, the eventID is removed from the event queue, becoming invalid, so there is no discernable difference between a completed event and a bad event ID.
	@param eventID The numeric ID of a previously scheduled event.
	@return true if this event is still outstanding and false if it has passed or eventID is invalid.

	@sa cancel, getEventTimeLeft, getScheduleDuration, getTimeSinceStart, schedule, obj.schedule
*/
bool isEventPending(U32 eventSequence)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   
   for (auto walk: gEventQueue)
   {
      if(walk->sequenceCount == eventSequence)
         return true;
   }

   return false;
}

/*!
	determines how much time remains until the event specified by eventID occurs.

    @param eventID The numeric ID of a previously scheduled event.
    @return a non-zero integer value equal to the milliseconds until the event specified by eventID will occur. However, if eventID is invalid, or the event has passed, this function will return zero.
    @sa cancel, getScheduleDuration, getTimeSinceStart, isEventPending, schedule, SimObject::schedule
*/
U32 getEventTimeLeft(U32 eventSequence)
{
    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   for (auto walk: gEventQueue)
   {
      if(walk->sequenceCount == eventSequence)
      {
         SimTime t = walk->time - getCurrentTime();
         return t;
      }
   }
   return 0;
}

/*!
	Determines how long the event associated with eventID was scheduled for.

	@param eventID The numeric ID of a previously scheduled event.
	@return a non-zero integer value equal to the milliseconds used in the schedule call that created this event. However, if eventID is invalid, this function will return zero.
	@sa cancel, getEventTimeLeft, getTimeSinceStart, isEventPending, schedule, SimObject::schedule
*/
U32 getScheduleDuration(U32 eventSequence)
{
   for (auto walk: gEventQueue)
   {
      if(walk->sequenceCount == eventSequence)
         return (walk->time-walk->startTime);
   }
   return 0;
}

/*!
	Determines how much time has passed since the event specified by eventID was scheduled.

    @param eventID The numeric ID of a previously scheduled event.
    @return a non-zero integer value equal to the milliseconds that have passed since this event was scheduled. However, if eventID is invalid, or the event has passed, this function will return zero.
    @sa cancel, getEventTimeLeft, getScheduleDuration, isEventPending, schedule, SimObject::schedule
*/
U32 getTimeSinceStart(U32 eventSequence)
{
   for (auto walk: gEventQueue)
   {
      if(walk->sequenceCount == eventSequence)
         return (getCurrentTime()-walk->startTime);
   }
   return 0;
}

//---------------------------------------------------------------------------
// event timing
    
void advanceToTime(SimTime targetTime)
{
   AssertFatal(targetTime >= getCurrentTime(), "EventQueue::process: cannot advance to time in the past.");

    std::lock_guard<std::recursive_mutex> lock(gEventQueueMutex);
   gTargetTime = targetTime;
   
   while (!gEventQueue.empty() && gEventQueue.front()->time <= targetTime)
   {
      AssertFatal(gEventQueue.front()->time >= gCurrentTime,
            "SimEventQueue::pop: Cannot go back in time (flux capacitor not installed - BJG).");
      gCurrentTime = gEventQueue.front()->time;
      SimObject *obj = gEventQueue.front()->destObject;

      if(!obj->isDeleted())
      {
         gEventQueue.front()->process(obj);
      }

       gEventQueue.pop_front();
   }
   gCurrentTime = targetTime;
}

void advanceTime(SimTime delta)
{
   advanceToTime(getCurrentTime() + delta);
}

/*! get the time, in ticks, that has elapsed since the engine started executing.

    @return the time in ticks since the engine was started.
    @sa getRealTime
*/
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
std::unique_ptr<SimManagerNameDictionary> gNameDictionary = nullptr;
std::unique_ptr<SimIdDictionary> gIdDictionary = nullptr;
U32 gNextObjectId;

void initRoot()
{
   gIdDictionary = std::unique_ptr<SimIdDictionary>(new SimIdDictionary);
   gNameDictionary = std::unique_ptr<SimManagerNameDictionary>(new SimManagerNameDictionary);

   gRootGroup = new SimGroup();
   gRootGroup->setId(RootGroupId);
   gRootGroup->assignName("RootGroup");
   gRootGroup->registerObject();

   gNextObjectId = DynamicObjectIdFirst;
}

void shutdownRoot()
{
   gRootGroup->deleteObject();

   gNameDictionary = nullptr;
   gIdDictionary = nullptr;
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
   InstantiateNamedSet(MaterialSet);
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

SimDataBlockGroup::SimDataBlockGroup()
{
   mLastModifiedKey = 0;
}

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
      std::sort(objectList.begin(),objectList.end(),compareModifiedKey);
   }
}
