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

#include <fstream>
#include "platform/platform.h"
#include "platform/event.h"
#include "game/gameInterface.h"
#include "console/console.h"
#include "platform/threads/mutex.h"

// Script binding.
#include "game/gameInterface_ScriptBinding.h"

GameInterface *Game = nullptr;
void *gGameEventQueueMutex = nullptr;
std::fstream gJournalStream;

//-----------------------------------------------------------------------------

GameInterface::GameInterface()
{
   AssertFatal(Game == nullptr, "ERROR: Multiple games declared.");
   Game = this;
   mJournalMode = JournalOff;
   mRunning = true;
   mRequiresRestart = false;
   if(!gGameEventQueueMutex)
      gGameEventQueueMutex = Mutex::createMutex();
}

void GameInterface::journalProcess()
{
#ifdef TORQUE_ALLOW_JOURNALING
   if(mJournalMode == JournalPlay)
   {
      ReadEvent journalReadEvent;
// used to be:
//      if(gJournalStream.read(&journalReadEvent.type))
//        if(gJournalStream.read(&journalReadEvent.size))
// for proper non-endian stream handling, the read-ins should match the write-out by using bytestreams read:
      if(gJournalStream.read(sizeof(Event), &journalReadEvent))
      {
         if(gJournalStream.read(journalReadEvent.size - sizeof(Event), &journalReadEvent.data))
         {
            if(gJournalStream.getPosition() == gJournalStream.getStreamSize() && mJournalBreak)
               Platform::debugBreak();
            processEvent(&journalReadEvent);
            return;
         }
      }
      // JournalBreak is used for debugging, so halt all game
      // events if we get this far.
      if(mJournalBreak)
         mRunning = false;
      else
         mJournalMode = JournalOff;
   }
#endif //TORQUE_ALLOW_JOURNALING
}

void GameInterface::saveJournal(const char *fileName)
{
   mJournalMode = JournalSave;
   gJournalStream.open(fileName, std::fstream::out);
}

void GameInterface::playJournal(const char *fileName,bool journalBreak)
{
   mJournalMode = JournalPlay;
   mJournalBreak = journalBreak;
   gJournalStream.open(fileName, std::fstream::in);
}

std::fstream *GameInterface::getJournalStream()
{
   return &gJournalStream;
}

void GameInterface::journalRead(U32 &val)
{
   gJournalStream >> val;
}

void GameInterface::journalWrite(U32 val)
{
   gJournalStream << (val);
}

void GameInterface::journalRead(U32 size, char *buffer)
{
   gJournalStream.read(buffer, size);
}

void GameInterface::journalWrite(U32 size, char const *buffer)
{
   gJournalStream.write(buffer, size);
}

