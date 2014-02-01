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

/*
** Alive and Ticking
** (c) Copyright 2006 Burnt Wasp
**     All Rights Reserved.
**
** Filename:    dispatcher.cc
** Author:      Tom Bampton
** Created:     19/8/2006
** Purpose:
**   Message Dispatcher
**
*/

#include "messaging/dispatcher.h"
#include "collection/simpleHashTable.h"
#include "memory/safeDelete.h"

#include "dispatcher_ScriptBinding.h"

namespace Dispatcher
{

//////////////////////////////////////////////////////////////////////////
// IMessageListener Methods
//////////////////////////////////////////////////////////////////////////

IMessageListener::~IMessageListener()
{
   for(S32 i = 0;i < mQueues.size();i++)
   {
      unregisterMessageListener(mQueues[i], this);
   }
}

void IMessageListener::onAddToQueue(StringTableEntry queue)
{
   // [tom, 8/20/2006] The dispatcher won't let us get added twice, so no need
   // to worry about it here.

   mQueues.push_back(queue);
}

void IMessageListener::onRemoveFromQueue(StringTableEntry queue)
{
   for(S32 i = 0;i < mQueues.size();i++)
   {
      if(mQueues[i] == queue)
      {
         mQueues.erase(i);
         return;
      }
   }
}

//////////////////////////////////////////////////////////////////////////
// Global State
//////////////////////////////////////////////////////////////////////////

//////////////////////////////////////////////////////////////////////////
/// @brief Internal class used by the dispatcher
//////////////////////////////////////////////////////////////////////////
static struct _DispatchData
{
   std::mutex mMutex;
   SimpleHashTable<MessageQueue> mQueues;

   _DispatchData()
   {
   }

   ~_DispatchData()
   {
      if(mMutex.try_lock())
      {
         mQueues.clearTables();
         mMutex.unlock();
      }
   }
} gDispatchData;

//////////////////////////////////////////////////////////////////////////
// Queue Registration
//////////////////////////////////////////////////////////////////////////

bool isQueueRegistered(const char *name)
{
   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   {
      return gDispatchData.mQueues.retrieve(name) != nullptr;
   }

   return false;
}

void registerMessageQueue(const char *name)
{
   if(isQueueRegistered(name))
      return;

   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   MessageQueue *queue = new MessageQueue;
   queue->mQueueName = StringTable->insert(name);
   gDispatchData.mQueues.insert(queue, name);
}

void unregisterMessageQueue(const char *name)
{
   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   MessageQueue *queue = gDispatchData.mQueues.remove(name);
   if(queue == nullptr)
      return;

   // Tell the listeners about it
   for(S32 i = 0;i < queue->mListeners.size();i++)
   {
      queue->mListeners[i]->onRemoveFromQueue(name);
   }

   delete queue;

}

//////////////////////////////////////////////////////////////////////////
// Message Listener Registration
//////////////////////////////////////////////////////////////////////////

bool registerMessageListener(const char *queue, IMessageListener *listener)
{
   if(! isQueueRegistered(queue))
      registerMessageQueue(queue);

   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   MessageQueue *q = gDispatchData.mQueues.retrieve(queue);
   if(q == nullptr)
   {
      Con::errorf("Dispatcher::registerMessageListener - Queue '%s' not found?! It should have been added automatically!", queue);
      return false;
   }

   for( IMessageListener* i:q->mListeners)
   {
      if(i == listener)
         return false;
   }

   q->mListeners.push_front(listener);
   listener->onAddToQueue(StringTable->insert(queue));
   return true;
}

void unregisterMessageListener(const char *queue, IMessageListener *listener)
{
   if(! isQueueRegistered(queue))
      return;

   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   MessageQueue *q = gDispatchData.mQueues.retrieve(queue);
   if(q == nullptr)
      return;

   for(std::deque<IMessageListener *>::iterator i = q->mListeners.begin();i != q->mListeners.end();i++)
   {
      if(*i == listener)
      {
         listener->onRemoveFromQueue(StringTable->insert(queue));
         q->mListeners.erase(i);
         return;
      }
   }
}

//////////////////////////////////////////////////////////////////////////
// Dispatcher
//////////////////////////////////////////////////////////////////////////

bool dispatchMessage(const char *queue, const char *msg, const char *data)
{
   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   MessageQueue *q = gDispatchData.mQueues.retrieve(queue);
   if(q == nullptr)
   {
      Con::errorf("Dispatcher::dispatchMessage - Attempting to dispatch to unknown queue '%s'", queue);
      return true;
   }

   return q->dispatchMessage(msg, data);
}


bool dispatchMessageObject(const char *queue, Message *msg)
{
   if(msg == nullptr)
      return true;

   msg->addReference();

   std::lock_guard<std::mutex> mh(gDispatchData.mMutex);

   MessageQueue *q = gDispatchData.mQueues.retrieve(queue);
   if(q == nullptr)
   {
      Con::errorf("Dispatcher::dispatchMessage - Attempting to dispatch to unknown queue '%s'", queue);
      msg->freeReference();
      return true;
   }

   // [tom, 8/19/2006] Make sure that the message is registered with the sim, since
   // when it's ref count is zero it'll be deleted with deleteObject()
   if(! msg->isProperlyAdded())
   {
      SimObjectId id = Message::getNextMessageID();
      if(id != 0xffffffff)
         msg->registerObject(id);
      else
      {
         Con::errorf("dispatchMessageObject: Message was not registered and no more object IDs are available for messages");

         msg->freeReference();
         return false;
      }
   }

   bool bResult = q->dispatchMessageObject(msg);
   msg->freeReference();

   return bResult;
}

//////////////////////////////////////////////////////////////////////////
// Internal Functions
//////////////////////////////////////////////////////////////////////////

MessageQueue * getMessageQueue(const char *name)
{
   return gDispatchData.mQueues.retrieve(name);
}

extern bool lockDispatcherMutex()
{
   return gDispatchData.mMutex.try_lock();
}

extern void unlockDispatcherMutex()
{
   gDispatchData.mMutex.unlock();
}

} // end namespace Dispatcher
