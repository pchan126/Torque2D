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
#include "network/connectionProtocol.h"
#include "sim/simBase.h"
#include "network/netConnection.h"
#include "console/consoleTypes.h"
#include "StreamFn.h"

class NetStringEvent : public NetEvent
{
   NetStringHandle mString;
   U32 mIndex;
public:
   NetStringEvent(U32 index = 0, NetStringHandle string = NetStringHandle())
   {
      mIndex = index;
      mString = string;
   }
   virtual void pack(NetConnection * /*ps*/, std::ostream &bstream)
   {
       StreamFn::writeInt(bstream, mIndex, ConnectionStringTable::EntryBitSize);
       StreamFn::writeString(bstream, mString.getString());
   }
   virtual void write(NetConnection * /*ps*/, std::ostream &bstream)
   {
       StreamFn::writeInt(bstream, mIndex, ConnectionStringTable::EntryBitSize);
      StreamFn::writeString(bstream, mString.getString());
   }
   virtual void unpack(NetConnection * /*con*/, std::istream &bstream)
   {
      char buf[256];
      mIndex = StreamFn::readInt(bstream, ConnectionStringTable::EntryBitSize);
      StreamFn::readString(bstream, buf);
      mString = NetStringHandle(buf);
   }
   virtual void notifyDelivered(NetConnection *ps, bool madeit)
   {
      if(madeit)
         ps->confirmStringReceived(mString, mIndex);
   }
   virtual void process(NetConnection *connection)
   {
      Con::printf("Mapping string: %s to index: %d", mString.getString(), mIndex);
      connection->mapString(mIndex, mString);
   }
#ifdef TORQUE_DEBUG_NET
   const char *getDebugName()
   {
      static char buffer[512];
      dSprintf(buffer, sizeof(buffer), "%s - \"", getClassName());
      expandEscape(buffer + dStrlen(buffer), mString.getString());
      dStrcat(buffer, "\"");
      return buffer;
   }
#endif
   DECLARE_CONOBJECT(NetStringEvent);
};

IMPLEMENT_CO_NETEVENT_V1(NetStringEvent);

//--------------------------------------------------------------------


ConnectionStringTable::ConnectionStringTable(NetConnection *parent)
{
   mParent = parent;
   for(U32 i = 0; i < EntryCount; i++)
   {
      mEntryTable[i].nextHash = nullptr;
      mEntryTable[i].nextLink = &mEntryTable[i+1];
      mEntryTable[i].prevLink = &mEntryTable[i-1];
      mEntryTable[i].index = i;

      mHashTable[i] = nullptr;
   }
   mLRUHead.nextLink = &mEntryTable[0];
   mEntryTable[0].prevLink = &mLRUHead;
   mLRUTail.prevLink = &mEntryTable[EntryCount-1];
   mEntryTable[EntryCount-1].nextLink = &mLRUTail;
}

U32 ConnectionStringTable::getNetSendId(NetStringHandle &string)
{
   // see if the entry is in the hash table right now
   U32 hashIndex = string.getIndex() % EntryCount;
   for(Entry *walk = mHashTable[hashIndex]; walk; walk = walk->nextHash)
      if(walk->string == string)
         return walk->index;
   AssertFatal(0, "Net send id is not in the table.  Error!");
   return 0;
}

U32 ConnectionStringTable::checkString(NetStringHandle &string, bool *isOnOtherSide)
{
   // see if the entry is in the hash table right now
   U32 hashIndex = string.getIndex() % EntryCount;
   for(Entry *walk = mHashTable[hashIndex]; walk; walk = walk->nextHash)
   {
      if(walk->string == string)
      {
         pushBack(walk);
         if(isOnOtherSide)
             *isOnOtherSide = walk->receiveConfirmed;
         return walk->index;
      }
   }

   // not in the hash table, means we have to add it
   Entry *newEntry;

   // pull the new entry from the LRU list.
   newEntry = mLRUHead.nextLink;
   pushBack(newEntry);

   // remove the string from the hash table
   Entry **hashWalk;
   for (hashWalk = &mHashTable[newEntry->string.getIndex() % EntryCount]; *hashWalk; hashWalk = &((*hashWalk)->nextHash))
   {
      if(*hashWalk == newEntry)
      {
         *hashWalk = newEntry->nextHash;
         break;
      }
   }

   newEntry->string = string;
   newEntry->receiveConfirmed = false;
   newEntry->nextHash = mHashTable[hashIndex];
   mHashTable[hashIndex] = newEntry;

   mParent->postNetEvent(new NetStringEvent(newEntry->index, string));
   if(isOnOtherSide)
      *isOnOtherSide = false;
   return newEntry->index;
}

void ConnectionStringTable::mapString(U32 netId, NetStringHandle &string)
{
   // the netId is sent by the other side... throw it in our mapping table:
   mRemoteStringTable[netId] = string;
}

void ConnectionStringTable::readDemoStartBlock(std::istream &stream)
{
   // ok, reading the demo start block
   for(U32 i = 0; i < EntryCount; i++)
   {
      if(StreamFn::readFlag(stream))
      {
         char buffer[256];
          StreamFn::readString(stream, buffer);
         mRemoteStringTable[i] = NetStringHandle(buffer);
      }
   }
}

void ConnectionStringTable::writeDemoStartBlock(std::ostream &stream)
{
   // ok, writing the demo start block
   for(U32 i = 0; i < EntryCount; i++)
   {
      if(stream << (mRemoteStringTable[i].isValidString()))
      {
          StreamFn::writeString(stream, mRemoteStringTable[i].getString());
//         stream->validate();
      }
   }
}
