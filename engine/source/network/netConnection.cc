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
#include "io/resource/resourceManager.h"
#include "console/consoleTypes.h"
#include "netInterface.h"
#include "io/StreamFn.h"
#include <stdarg.h>
#include <sstream>

S32 gNetBitsSent = 0;
extern S32 gNetBitsReceived;
U32 gGhostUpdates = 0;

enum NetConnectionConstants {
   PingTimeout = 4500, ///< milliseconds
   DefaultPingRetryCount = 15,
};

SimObjectPtr<NetConnection> NetConnection::mServerConnection;
SimObjectPtr<NetConnection> NetConnection::mLocalClientConnection;

//----------------------------------------------------------------------
/// ConnectionMessageEvent
///
/// This event is used inside by the connection and subclasses to message
/// itself when sequencing events occur.  Right now, the message event
/// only uses 6 bits to transmit the message, so
class ConnectionMessageEvent : public NetEvent
{
   U32 sequence;
   U32 message;
   U32 ghostCount;
public:
   ConnectionMessageEvent(U32 msg=0, U32 seq=0, U32 gc=0)
      { message = msg; sequence = seq; ghostCount = gc;}

   void pack(NetConnection *, std::ostream &bstream)
   {
       bstream << sequence;
       StreamFn::writeInt(bstream, message, 3);
       StreamFn::writeInt(bstream, ghostCount, NetConnection::GhostIdBitSize + 1);
   }

   void write(NetConnection *, std::ostream &bstream)
   {
      bstream << (sequence);
      StreamFn::writeInt(bstream, message, 3);
      StreamFn::writeInt(bstream, ghostCount, NetConnection::GhostIdBitSize + 1);
   }

   void unpack(NetConnection *, std::istream &bstream)
   {
      bstream >> (sequence);
      message = StreamFn::readInt(bstream, 3);
      ghostCount = StreamFn::readInt(bstream, NetConnection::GhostIdBitSize + 1);
   }

   void process(NetConnection *ps)
   {
      ps->handleConnectionMessage(message, sequence, ghostCount);
   }

   DECLARE_CONOBJECT(ConnectionMessageEvent);
};

IMPLEMENT_CO_NETEVENT_V1(ConnectionMessageEvent);

void NetConnection::sendConnectionMessage(U32 message, U32 sequence, U32 ghostCount)
{
   postNetEvent(new ConnectionMessageEvent(message, sequence, ghostCount));
}

//--------------------------------------------------------------------
IMPLEMENT_CONOBJECT(NetConnection);

NetConnection* NetConnection::mConnectionList = nullptr;
NetConnection* NetConnection::mHashTable[NetConnection::HashTableSize] = { nullptr, };

bool NetConnection::mFilesWereDownloaded = false;

static inline U32 HashNetAddress(const NetAddress *addr)
{
   return *((U32 *)addr->netNum) % NetConnection::HashTableSize;
}

NetConnection *NetConnection::lookup(const NetAddress *addr)
{
   U32 hashIndex = HashNetAddress(addr);
   for(NetConnection *walk = mHashTable[hashIndex]; walk; walk = walk->mNextTableHash)
      if(Net::compareAddresses(addr, walk->getNetAddress()))
         return walk;
   return nullptr;
}

void NetConnection::netAddressTableInsert()
{
   U32 hashIndex = HashNetAddress(&mNetAddress);
   mNextTableHash = mHashTable[hashIndex];
   mHashTable[hashIndex] = this;
}

void NetConnection::netAddressTableRemove()
{
   U32 hashIndex = HashNetAddress(&mNetAddress);
   NetConnection **walk = &mHashTable[hashIndex];
   while(*walk)
   {
      if(*walk == this)
      {
         *walk = mNextTableHash;
         mNextTableHash = nullptr;
         return;
      }
      walk = &((*walk)->mNextTableHash);
   }
}

void NetConnection::setNetAddress(const NetAddress *addr)
{
   mNetAddress = *addr;
}

const NetAddress *NetConnection::getNetAddress()
{
   return &mNetAddress;
}

void NetConnection::setSequence(U32 sequence)
{
   mConnectSequence = sequence;
}

U32 NetConnection::getSequence()
{
   return mConnectSequence;
}

static U32 gPacketRateToServer = 32;
static U32 gPacketUpdateDelayToServer = 32;
static U32 gPacketRateToClient = 10;
static U32 gPacketSize = 200;

void NetConnection::consoleInit()
{
   Con::addVariable("pref::Net::PacketRateToServer",  TypeS32, &gPacketRateToServer);
   Con::addVariable("pref::Net::PacketRateToClient",  TypeS32, &gPacketRateToClient);
   Con::addVariable("pref::Net::PacketSize",          TypeS32, &gPacketSize);
   Con::addVariable("Stats::netBitsSent",       TypeS32, &gNetBitsSent);
   Con::addVariable("Stats::netBitsReceived",   TypeS32, &gNetBitsReceived);
   Con::addVariable("Stats::netGhostUpdates",   TypeS32, &gGhostUpdates);
}

void NetConnection::checkMaxRate()
{
   // Limit packet rate to server.
   if(gPacketRateToServer > 32)
      gPacketRateToServer = 32;
   if(gPacketRateToServer < 8)
      gPacketRateToServer = 8;

   // Limit packet rate to client.
   if(gPacketRateToClient > 32)
      gPacketRateToClient = 32;
   if(gPacketRateToClient < 1)
      gPacketRateToClient = 1;

   // Limit packet size.
   if(gPacketSize > 450)
      gPacketSize = 450;
   if(gPacketSize < 100)
      gPacketSize = 100;

   gPacketUpdateDelayToServer = 1024 / gPacketRateToServer;
   U32 toClientUpdateDelay = 1024 / gPacketRateToClient;

   if(mMaxRate.updateDelay != toClientUpdateDelay || mMaxRate.packetSize != gPacketSize)
   {
      mMaxRate.updateDelay = toClientUpdateDelay;
      mMaxRate.packetSize = gPacketSize;
      mMaxRate.changed = true;
   }
}

void NetConnection::setSendingEvents(bool sending)
{
   AssertFatal(!mEstablished, "Error, cannot change event behavior after a connection has been established.");
   mSendingEvents = sending;
}

void NetConnection::setTranslatesStrings(bool xl)
{
   AssertFatal(!mEstablished, "Error, cannot change event behavior after a connection has been established.");
   mTranslateStrings = xl;
   if(mTranslateStrings)
      mStringTable = new ConnectionStringTable(this);
}

void NetConnection::setNetClassGroup(U32 grp)
{
   AssertFatal(!mEstablished, "Error, cannot change net class group after a connection has been established.");
   mNetClassGroup = grp;
}

NetConnection::NetConnection()
{
   mTranslateStrings = false;
   mConnectSequence = 0;

   mStringTable = nullptr;
   mSendingEvents = true;
   mNetClassGroup = NetClassGroupGame;
   AssertFatal(mNetClassGroup >= NetClassGroupGame && mNetClassGroup < NetClassGroupsCount,
            "Invalid net event class type.");

   mSimulatedPing = 0;
   mSimulatedPacketLoss = 0;
#ifdef TORQUE_DEBUG_NET
   mLogging = false;
#endif
   mEstablished = false;
   mLastUpdateTime = 0;
   mRoundTripTime = 0;
   mPacketLoss = 0;
   mNextTableHash = nullptr;
   mSendDelayCredit = 0;
   mConnectionState = NotConnected;

   mCurrentDownloadingFile = nullptr;
   mCurrentFileBuffer = nullptr;

   mNextConnection = nullptr;
   mPrevConnection = nullptr;

   mNotifyQueueHead = nullptr;
   mNotifyQueueTail = nullptr;

   mCurRate.updateDelay = 102;
   mCurRate.packetSize = 200;
   mCurRate.changed = false;
   mMaxRate.updateDelay = 102;
   mMaxRate.packetSize = 200;
   mMaxRate.changed = false;
   checkMaxRate();

   // event management data:

   mNotifyEventList = nullptr;
   mSendEventQueueHead = nullptr;
   mSendEventQueueTail = nullptr;
   mUnorderedSendEventQueueHead = nullptr;
   mUnorderedSendEventQueueTail = nullptr;
   mWaitSeqEvents = nullptr;

   mNextSendEventSeq = FirstValidSendEventSeq;
   mNextRecvEventSeq = FirstValidSendEventSeq;
   mLastAckedEventSeq = -1;

   // ghost management data:

   mScopeObject = nullptr;
   mGhostingSequence = 0;
   mGhosting = false;
   mScoping = false;
   mGhostArray = nullptr;
   mGhostRefs = nullptr;
   mGhostLookupTable = nullptr;
   mLocalGhosts = nullptr;

   mGhostsActive = 0;

   mMissionPathsSent = false;
   mDemoWriteStream = nullptr;
   mDemoReadStream = nullptr;

   mPingSendCount = 0;
   mPingRetryCount = DefaultPingRetryCount;
   mLastPingSendTime = Platform::getVirtualMilliseconds();

   mCurrentDownloadingFile = nullptr;
   mCurrentFileBuffer = nullptr;
   mCurrentFileBufferSize = 0;
   mCurrentFileBufferOffset = 0;
   mNumDownloadedFiles = 0;
}

NetConnection::~NetConnection()
{
   AssertFatal(mNotifyQueueHead == nullptr, "Uncleared notifies remain.");
   netAddressTableRemove();

   dFree(mCurrentFileBuffer);
   if(mCurrentDownloadingFile)
      ResourceManager->closeStream(mCurrentDownloadingFile);

   delete[] mLocalGhosts;
   delete[] mGhostLookupTable;
   delete[] mGhostRefs;
   delete[] mGhostArray;
   delete mStringTable;
   if(mDemoWriteStream)
      delete mDemoWriteStream;
   if(mDemoReadStream)
      ResourceManager->closeStream(mDemoReadStream);
}

NetConnection::PacketNotify::PacketNotify()
{
   rateChanged = false;
   maxRateChanged = false;
   sendTime = 0;
   eventList = 0;
   ghostList = 0;
}

bool NetConnection::checkTimeout(U32 time)
{
   if(!isNetworkConnection())
      return false;

   if(time > mLastPingSendTime + PingTimeout)
   {
      if(mPingSendCount >= mPingRetryCount)
         return true;
      mLastPingSendTime = time;
      mPingSendCount++;
      sendPingPacket();
   }
   return false;
}

void NetConnection::keepAlive()
{
   mLastPingSendTime = Platform::getVirtualMilliseconds();
   mPingSendCount = 0;
}

void NetConnection::handleConnectionEstablished()
{
}

//--------------------------------------------------------------------------

void NetConnection::setEstablished()
{
   AssertFatal(!mEstablished, "NetConnection::setEstablished - Error, this NetConnection has already been established.");

   mEstablished = true;
   mNextConnection = mConnectionList;
   if(mConnectionList)
      mConnectionList->mPrevConnection = this;
   mConnectionList = this;

   if(isNetworkConnection())
      netAddressTableInsert();

}

void NetConnection::onRemove()
{
   // delete any ghosts that may exist for this connection, but aren't added
   while(mGhostAlwaysSaveList.size())
   {
      delete mGhostAlwaysSaveList[0].ghost;
      mGhostAlwaysSaveList.pop_front();
   }
   if(mNextConnection)
      mNextConnection->mPrevConnection = mPrevConnection;
   if(mPrevConnection)
      mPrevConnection->mNextConnection = mNextConnection;
   if(mConnectionList == this)
      mConnectionList = mNextConnection;
   while(mNotifyQueueHead)
      handleNotify(false);

   ghostOnRemove();
   eventOnRemove();

   Parent::onRemove();
}

char NetConnection::mErrorBuffer[256];

void NetConnection::setLastError(const char *fmt, ...)
{
   va_list argptr;
   va_start(argptr, fmt);
   dVsprintf(mErrorBuffer, sizeof(mErrorBuffer), fmt, argptr);
   va_end(argptr);

#ifdef TORQUE_DEBUG_NET
   // setLastErrors assert in net_debug builds
   AssertFatal(false, mErrorBuffer);
#endif

}

//--------------------------------------------------------------------

void NetConnection::handleNotify(bool recvd)
{
//   Con::printf("NET  %d: NOTIFY - %d %s", getId(), gPacketId, recvd ? "RECVD" : "DROPPED");

   PacketNotify *note = mNotifyQueueHead;
   AssertFatal(note != nullptr, "Error: got a notify with a null notify head.");
   mNotifyQueueHead = mNotifyQueueHead->nextPacket;

   if(note->rateChanged && !recvd)
      mCurRate.changed = true;
   if(note->maxRateChanged && !recvd)
      mMaxRate.changed = true;

   if(recvd) 
   {
      // Running average of roundTrip time
      U32 curTime = Platform::getVirtualMilliseconds();
      mRoundTripTime = (mRoundTripTime + (curTime - note->sendTime)) * 0.5f;
      packetReceived(note);
   }
   else
      packetDropped(note);

   delete note;
}

void NetConnection::processRawPacket(std::iostream &bstream)
{
//   if(mDemoWriteStream)
//      recordBlock(BlockTypePacket, StreamFn::getReadByteSize(bstream), bstream->getBuffer());

   ConnectionProtocol::processRawPacket(bstream);
}

void NetConnection::handlePacket(std::iostream &bstream)
{
//   Con::printf("NET  %d: RECV - %d", getId(), mLastSeqRecvd);
   // clear out any errors

   mErrorBuffer[0] = 0;

   if(StreamFn::readFlag(bstream))
   {
      mCurRate.updateDelay = StreamFn::readInt(bstream, 10);
      mCurRate.packetSize = StreamFn::readInt(bstream, 10);
   }

   if(StreamFn::readFlag(bstream))
   {
      U32 omaxDelay = StreamFn::readInt(bstream, 10);
      S32 omaxSize = StreamFn::readInt(bstream, 10);
      if(omaxDelay < mMaxRate.updateDelay)
         omaxDelay = mMaxRate.updateDelay;
      if(omaxSize > mMaxRate.packetSize)
         omaxSize = mMaxRate.packetSize;
      if(omaxDelay != mCurRate.updateDelay || omaxSize != mCurRate.packetSize)
      {
         mCurRate.updateDelay = omaxDelay;
         mCurRate.packetSize = omaxSize;
         mCurRate.changed = true;
      }
   }
   readPacket(bstream);

   if(mErrorBuffer[0])
      connectionError(mErrorBuffer);
}

void NetConnection::connectionError(const char *errorString)
{
}

//--------------------------------------------------------------------

NetConnection::PacketNotify *NetConnection::allocNotify()
{
   return new PacketNotify;
}

/// Used when simulating lag.
///
/// We post this SimEvent when we want to send a packet; it delays for a bit, then
/// sends the actual packet.
class NetDelayEvent : public SimEvent
{
   U8 buffer[MaxPacketDataSize];
   std::stringstream stream;
public:
   NetDelayEvent(std::stringstream& inStream) : stream(nullptr, 0)
   {
//      dMemcpy(buffer, inStream->getBuffer(), inStream->getPosition());
//      stream.setBuffer(buffer, inStream->getPosition());
//      stream.setPosition(inStream->getPosition());
   }

   void process(SimObject *object)
   {
      ((NetConnection *) object)->sendPacket(stream);
   }
};

void NetConnection::checkPacketSend(bool force)
{
   U32 curTime = Platform::getVirtualMilliseconds();
   U32 delay = isConnectionToServer() ? gPacketUpdateDelayToServer : mCurRate.updateDelay;

   if(!force)
   {
      if(curTime < mLastUpdateTime + delay - mSendDelayCredit)
         return;

      mSendDelayCredit = curTime - (mLastUpdateTime + delay - mSendDelayCredit);
      if(mSendDelayCredit > 1000)
         mSendDelayCredit = 1000;

      if(mDemoWriteStream)
         recordBlock(BlockTypeSendPacket, 0, 0);
   }
   if(windowFull())
      return;

   std::stringstream *stream = new std::stringstream(); // = BitStream::getPacketStream(mCurRate.packetSize);
   buildSendPacketHeader(*stream);

   mLastUpdateTime = curTime;

   PacketNotify *note = allocNotify();
   if(!mNotifyQueueHead)
      mNotifyQueueHead = note;
   else
      mNotifyQueueTail->nextPacket = note;
   mNotifyQueueTail = note;
   note->nextPacket = nullptr;
   note->sendTime = curTime;

   note->rateChanged = mCurRate.changed;
   note->maxRateChanged = mMaxRate.changed;

   if(*stream << mCurRate.changed)
   {
      StreamFn::writeInt(*stream, mCurRate.updateDelay, 10);
      StreamFn::writeInt(*stream, mCurRate.packetSize, 10);
      mCurRate.changed = false;
   }
   *stream << mMaxRate.changed;
   if(mMaxRate.changed)
   {
      StreamFn::writeInt(*stream, mMaxRate.updateDelay, 10);
      StreamFn::writeInt(*stream, mMaxRate.packetSize, 10);
      mMaxRate.changed = false;
   }
   DEBUG_LOG(("PKLOG %d START", getId()) );
   writePacket(*stream, note);
   DEBUG_LOG(("PKLOG %d END - %d", getId(), stream->getCurPos() - start) );
   if(mSimulatedPacketLoss && Platform::getRandom() < mSimulatedPacketLoss)
   {
      //Con::printf("NET  %d: SENDDROP - %d", getId(), mLastSendSeq);
      return;
   }
   if(mSimulatedPing)
   {
      Sim::postEvent(getId(), new NetDelayEvent(*stream), Sim::getCurrentTime() + mSimulatedPing);
      return;
   }
   sendPacket(*stream);
}

Net::Error NetConnection::sendPacket(std::iostream &stream)
{
   //Con::printf("NET  %d: SEND - %d", getId(), mLastSendSeq);
   // do nothing on send if this is a demo replay.
   if(mDemoReadStream)
      return Net::NoError;

   gNetBitsSent = StreamFn::getStreamSize(stream);

//   if(isLocalConnection())
//   {
//      // short circuit connection to the other side.
//      // handle the packet, then force a notify.
//      stream->setBuffer(stream->getBuffer(), stream->getPosition(), stream->getPosition());
//      mRemoteConnection->processRawPacket(stream);
//
      return Net::NoError;
//   }
//   else
//   {
//      return Net::sendto(getNetAddress(), stream->getBuffer(), stream->getPosition());
//   }
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// these are the virtual function defs for Connection -
// if your subclass has additional data to read / write / notify, add it in these functions.

void NetConnection::readPacket(std::iostream &bstream)
{
   eventReadPacket(bstream);
   ghostReadPacket(bstream);
}

void NetConnection::writePacket(std::iostream &bstream, PacketNotify *note)
{
   eventWritePacket(bstream, note);
   ghostWritePacket(bstream, note);
}

void NetConnection::packetReceived(PacketNotify *note)
{
   eventPacketReceived(note);
   ghostPacketReceived(note);
}

void NetConnection::packetDropped(PacketNotify *note)
{
   eventPacketDropped(note);
   ghostPacketDropped(note);
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

void NetConnection::writeDemoStartBlock(std::ostream &stream)
{
   ConnectionProtocol::writeDemoStartBlock(stream);

   stream << (mRoundTripTime);
   stream << (mPacketLoss);
//   stream->validate();
   mStringTable->writeDemoStartBlock(stream);

   U32 start = 0;
   PacketNotify *note = mNotifyQueueHead;
   while(note)
   {
      start++;
      note = note->nextPacket;
   }
   stream << start;

   eventWriteStartBlock(stream);
   ghostWriteStartBlock(stream);
}

bool NetConnection::readDemoStartBlock(std::istream &stream)
{
   ConnectionProtocol::readDemoStartBlock(stream);
   
   stream >> mRoundTripTime;
   stream >> mPacketLoss;

   // Read
   mStringTable->readDemoStartBlock(stream);
   U32 pos;
   stream >> pos; // notify count
   for(U32 i = 0; i < pos; i++)
   {
      PacketNotify *note = allocNotify();
      note->nextPacket = nullptr;
      if(!mNotifyQueueHead)
         mNotifyQueueHead = note;
      else
         mNotifyQueueTail->nextPacket = note;
      mNotifyQueueTail = note;
   }
   eventReadStartBlock(stream);
   ghostReadStartBlock(stream);
   return true;
}

bool NetConnection::startDemoRecord(const char *fileName)
{
    std::fstream *fs = new std::fstream;

   if(!ResourceManager->openFileForWrite(*fs, fileName))
   {
      delete fs;
      return false;
   }

//   mDemoWriteStream = fs;
//   *mDemoWriteStream << (mProtocolVersion);
//   std::stringstream bs;
//
//   // then write out the start block
//   writeDemoStartBlock(bs);
//   U32 size = bs.getPosition() + 1;
//   *mDemoWriteStream << size;
//   mDemoWriteStream->write((char*)bs.getBuffer(), size);
   return true;
}

bool NetConnection::replayDemoRecord(const char *fileName)
{
//    std::iostream *fs = ResourceManager->openStream(fileName);
//   if(!fs)
//      return false;
//
//   mDemoReadStream = fs;
//   *mDemoReadStream >> mProtocolVersion;
//   U32 size;
//   *mDemoReadStream >> size;
//   U8 *block = new U8[size];
//   mDemoReadStream->read( (char*)block, size);
//   BitStream bs(block, size);
//
//   bool res = readDemoStartBlock(&bs);
//   delete[] block;
//   if(!res)
//      return false;
//
//   // prep for first block read
//   // type/size stored in U16: [type:4][size:12]
//   U16 typeSize;
//   *mDemoReadStream >> typeSize;
//
//   mDemoNextBlockType = typeSize >> 12;
//   mDemoNextBlockSize = typeSize & 0xFFF;
//
//   if(mDemoReadStream->bad())
//      return false;
   return true;
}

void NetConnection::stopRecording()
{
   if(mDemoWriteStream)
   {
      delete mDemoWriteStream;
      mDemoWriteStream = nullptr;
   }
}

void NetConnection::recordBlock(U32 type, U32 size, void *data)
{
   AssertFatal(type < MaxNumBlockTypes, "NetConnection::recordBlock: invalid type");
   AssertFatal(size < MaxBlockSize, "NetConnection::recordBlock: invalid size");
   if((type >= MaxNumBlockTypes) || (size >= MaxBlockSize))
      return;

   if(mDemoWriteStream)
   {
      // store type/size in U16: [type:4][size:12]
      U16 typeSize = (type << 12) | size;
      *mDemoWriteStream << (typeSize);
      if(size)
         mDemoWriteStream->write( (char*)data, size);
   }
}

void NetConnection::handleRecordedBlock(U32 type, U32 size, void *data)
{
//   switch(type)
//   {
//      case BlockTypePacket: {
//         BitStream bs(data, size);
//         processRawPacket(&bs);
//         break;
//      }
//      case BlockTypeSendPacket:
//         checkPacketSend(true);
//         break;
//   }
}

void NetConnection::demoPlaybackComplete()
{
}

void NetConnection::stopDemoPlayback()
{
   demoPlaybackComplete();
   deleteObject();
}

bool NetConnection::processNextBlock()
{
   U8 buffer[MaxPacketDataSize];
   // read in and handle
   if(mDemoReadStream->read( (char*)buffer, mDemoNextBlockSize))
      handleRecordedBlock(mDemoNextBlockType, mDemoNextBlockSize, buffer);

   U16 typeSize;
   *mDemoReadStream >> typeSize;

   mDemoNextBlockType = typeSize >> 12;
   mDemoNextBlockSize = typeSize & 0xFFF;

   if(mDemoReadStream->bad())
   {
      stopDemoPlayback();
      return false;
   }
   return true;
}

//--------------------------------------------------------------------
//--------------------------------------------------------------------

// some handy string functions for compressing strings over a connection:
enum NetStringConstants
{
   NullString = 0,
   CString,
   TagString,
   Integer
};

void NetConnection::validateSendString(const char *str)
{
   if(U8(*str) == StringTagPrefixByte)
   {
      NetStringHandle strHandle(dAtoi(str + 1));
      checkString(strHandle);
   }
}

void NetConnection::packString(std::ostream &stream, const char *str)
{
   char buf[16];
   if(!*str)
   {
      StreamFn::writeInt(stream, NullString, 2);
      return;
   }
   if(U8(str[0]) == StringTagPrefixByte)
   {
      StreamFn::writeInt(stream, TagString, 2);
      StreamFn::writeInt(stream, dAtoi(str + 1), ConnectionStringTable::EntryBitSize);
      return;
   }
   if(str[0] == '-' || (str[0] >= '0' && str[0] <= '9'))
   {
      S32 num = dAtoi(str);
      dSprintf(buf, sizeof(buf), "%d", num);
      if(!dStrcmp(buf, str))
      {
         StreamFn::writeInt(stream, Integer, 2);
         if(StreamFn::writeFlag(stream, num < 0))
            num = -num;
         if(StreamFn::writeFlag(stream, num < 128))
         {
            StreamFn::writeInt(stream, num, 7);
            return;
         }
         if(StreamFn::writeFlag(stream, num < 32768))
         {
            StreamFn::writeInt(stream, num, 15);
            return;
         }
         else
         {
            StreamFn::writeInt(stream, num, 31);
            return;
         }
      }
   }
   StreamFn::writeInt(stream, CString, 2);
   StreamFn::writeString(stream, str);
}

void NetConnection::unpackString(std::istream &stream, char readBuffer[1024])
{
   U32 code = StreamFn::readInt(stream, 2);
   switch(code)
   {
      case NullString:
         readBuffer[0] = 0;
         return;
      case CString:
         StreamFn::readString(stream, readBuffer);
         return;
      case TagString:
         U32 tag;
         tag = StreamFn::readInt(stream, ConnectionStringTable::EntryBitSize);
         readBuffer[0] = StringTagPrefixByte;
         dSprintf(readBuffer+1, 1023, "%d", tag);
         return;
      case Integer:
         bool neg;
         neg = StreamFn::readFlag(stream);
         S32 num;
         if(StreamFn::readFlag(stream))
            num = StreamFn::readInt(stream, 7);
         else if(StreamFn::readFlag(stream))
            num = StreamFn::readInt(stream, 15);
         else
            num = StreamFn::readInt(stream, 31);
         if(neg)
            num = -num;
         dSprintf(readBuffer, 1024, "%d", num);
   }
}

void NetConnection::packNetStringHandleU(std::iostream &stream, NetStringHandle &h)
{
   if(StreamFn::writeFlag(stream, h.isValidString() ))
   {
      bool isReceived = false;
      U32 netIndex = checkString(h, &isReceived);
      if(StreamFn::writeFlag(stream, isReceived))
         StreamFn::writeInt(stream, netIndex, ConnectionStringTable::EntryBitSize);
      else
         StreamFn::writeString(stream, h.getString());
   }
}

NetStringHandle NetConnection::unpackNetStringHandleU(std::iostream &stream)
{
   NetStringHandle ret;
   if(StreamFn::readFlag(stream))
   {
      if(StreamFn::readFlag(stream))
         ret = mStringTable->lookupString(StreamFn::readInt(stream, ConnectionStringTable::EntryBitSize));
      else
      {
         char buf[256];
         StreamFn::readString(stream, buf);
         ret = NetStringHandle(buf);
      }
   }
   return ret;
}

//----------------------------------------------------------------------------

//----------------------------------------------------------------------------
//----------------------------------------------------------------------------

void NetConnection::setAddressDigest(U32 digest[4])
{
   mAddressDigest[0] = digest[0];
   mAddressDigest[1] = digest[1];
   mAddressDigest[2] = digest[2];
   mAddressDigest[3] = digest[3];
}

void NetConnection::getAddressDigest(U32 digest[4])
{
   digest[0] = mAddressDigest[0];
   digest[1] = mAddressDigest[1];
   digest[2] = mAddressDigest[2];
   digest[3] = mAddressDigest[3];
}

bool NetConnection::canRemoteCreate()
{
   return false;
}

void NetConnection::onTimedOut()
{

}

void NetConnection::connect(const NetAddress *address)
{
   mNetAddress = *address;
   GNet->startConnection(this);
}

void NetConnection::onConnectTimedOut()
{

}

void NetConnection::sendDisconnectPacket(const char *reason)
{
   GNet->sendDisconnectPacket(this, reason);
}

void NetConnection::onDisconnect(const char *reason)
{
}

void NetConnection::onConnectionRejected(const char *reason)
{
}

void NetConnection::onConnectionEstablished(bool isInitiator)
{

}

void NetConnection::handleStartupError(const char *errorString)
{

}

void NetConnection::writeConnectRequest(std::iostream &stream)
{
   stream << mNetClassGroup;
   stream << (U32(AbstractClassRep::getClassCRC(mNetClassGroup)));
}

bool NetConnection::readConnectRequest(std::iostream &stream, const char **errorString)
{
   U32 classGroup, classCRC;
   stream >> classGroup;
   stream >> classCRC;

   if(classGroup == mNetClassGroup && classCRC == AbstractClassRep::getClassCRC(mNetClassGroup))
      return true;

   *errorString = "CHR_INVALID";
   return false;
}

void NetConnection::writeConnectAccept(std::iostream &stream)
{
}

bool NetConnection::readConnectAccept(std::iostream &stream, const char **errorString)
{
   return true;
}
