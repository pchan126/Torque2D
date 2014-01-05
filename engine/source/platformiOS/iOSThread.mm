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

#include <pthread.h>
#include <stdlib.h>
#include "memory/safeDelete.h"
#include "platform/threads/thread.h"
#include "platform/platformSemaphore.h"
#include "platform/threads/mutex.h"
#import "console/console.h"

//-----------------------------------------------------------------------------

#pragma mark ---- Thread Class Methods ----

struct PlatformThreadData
{
   ThreadRunFunction       mRunFunc;
   void*                   mRunArg;
   Thread*                 mThread;
   Semaphore               mGateway; // default count is 1
   PTR                     mThreadID;
};

//-----------------------------------------------------------------------------
// Function:    ThreadRunHandler
// Summary:     Calls Thread::run() with the thread's specified run argument.
//               Neccesary because Thread::run() is provided as a non-threaded
//               way to execute the thread's run function. So we have to keep
//               track of the thread's lock here.
static void *ThreadRunHandler(void * arg)
{
   PlatformThreadData *mData = reinterpret_cast<PlatformThreadData*>(arg);
   Thread *thread = mData->mThread;
	@autoreleasepool {
      mData->mThreadID = ThreadManager::getCurrentThreadId();
      ThreadManager::addThread(thread);
      thread->run(mData->mRunArg);
   	}
	mData->mGateway.release();
   // we could delete the Thread here, if it wants to be auto-deleted...
   if(thread->autoDelete)
   {
      ThreadManager::removeThread(thread);
      delete thread;
   }
   // return value for pthread lib's benefit
   return NULL;
   // the end of this function is where the created pthread will die.
}
   
//-----------------------------------------------------------------------------
Thread::Thread(ThreadRunFunction func, void* arg, bool start_thread, bool autodelete)
{
   mData = new PlatformThreadData;
   mData->mRunFunc = func;
   mData->mRunArg = arg;
   mData->mThread = this;
   mData->mThreadID = 0;
   autoDelete = autodelete;
   
   if(start_thread)
      start();
}

Thread::~Thread()
{
   stop();
   join();

   SAFE_DELETE(mData);
}

void Thread::start()
{
   if(isAlive())
      return;

   // cause start to block out other pthreads from using this Thread, 
   // at least until ThreadRunHandler exits.
   mData->mGateway.acquire();

   // reset the shouldStop flag, so we'll know when someone asks us to stop.
   shouldStop = false;

   pthread_create((pthread_t*)(&mData->mThreadID), NULL, ThreadRunHandler, mData);
}

bool Thread::join()
{
   if(!isAlive())
      return true;

   // not using pthread_join here because pthread_join cannot deal
   // with multiple simultaneous calls.
   mData->mGateway.acquire();
   mData->mGateway.release();
   return true;
}

void Thread::run(void* arg)
{
   if(mData->mRunFunc)
      mData->mRunFunc(arg);
}

bool Thread::isAlive()
{
   if(mData->mThreadID == 0)
      return false;

   if( mData->mGateway.acquire(false) ) 
   {
     mData->mGateway.release();
     return false; // we got the lock, it aint alive.
   }
   else
     return true; // we could not get the lock, it must be alive.
}

PTR Thread::getId()
{
   return mData->mThreadID;
}

PTR ThreadManager::getCurrentThreadId()
{
   return (PTR)pthread_self();
}

bool ThreadManager::compare(PTR threadId_1, PTR threadId_2)
{
   return (bool)pthread_equal((pthread_t)threadId_1, (pthread_t)threadId_2);
}


#pragma mark ---- ShellExecute ----
class ExecuteThread : public Thread
{
    const char* zargs;
    const char* directory;
    const char* executable;
public:
    ExecuteThread(const char *_executable, const char *_args /* = NULL */, const char *_directory /* = NULL */) : Thread(0, NULL, false, true)
    {
        zargs = dStrdup(_args);
        directory = dStrdup(_directory);
        executable = dStrdup(_executable);
        start();
    }
    
    virtual void run(void* arg);
};

static char* _unDoubleQuote(char* arg)
{
    size_t len = dStrlen(arg);
    if(!len)
        return arg;
    
    if(arg[0] == '"' && arg[len-1] == '"')
    {
        arg[len - 1] = '\0';
        return arg + 1;
    }
    return arg;
}

void ExecuteThread::run(void* arg)
{
#if 0 // FIXME: HACK PUAP -Mat		Threads
    // 2k should be enough. if it's not, bail.
    //   char buf[2048];
    //   U32 len = dSprintf(buf, sizeof(buf), "%s %s -workingDir %s", executable, args, directory);
    //   if( len >= sizeof(buf))
    //   {
    //      Con::errorf("shellExecute(): the command was too long, and won't be run.");
    //      return;
    //   }
    //   // calls sh with the string and blocks until the command returns.
    //   system(buf);
    
    // FIXME: there is absolutely no error checking in here.
    printf("creating nstask\n");
    NSTask *aTask = [[NSTask alloc] init];
    NSMutableArray *array = [NSMutableArray array];
    
    // scan the args list, breaking it up, space delimited, backslash escaped.
    U32 len = dStrlen(zargs);
    char args[len+1];
    dStrncpy(args, zargs, len+1);
    char *lastarg = args;
    bool escaping = false;
    for(int i = 0; i< len; i++)
    {
        char c = args[i];
        // a backslash escapes the next character
        if(escaping)
            continue;
        if(c == '\\')
            escaping = true;
        
        if(c == ' ')
        {
            args[i] = '\0';
            if(*lastarg)
                [array addObject:[NSString stringWithUTF8String: _unDoubleQuote(lastarg)]];
            lastarg = args + i + 1;
        }
    }
    if(*lastarg)
        [array addObject:[NSString stringWithUTF8String: _unDoubleQuote(lastarg)]];
    
    [aTask setArguments: array];
    
    [aTask setCurrentDirectoryPath:[NSString stringWithUTF8String: this->directory]];
    [aTask setLaunchPath:[NSString stringWithUTF8String:executable]];
    [aTask launch];
    [aTask waitUntilExit];
    U32 ret = [aTask terminationStatus];
    Con::executef(2, "onExecuteDone", Con::getIntArg(ret));
    printf("done nstask\n");
#endif
}

ConsoleFunction(shellExecute, bool, 2, 4, "(executable, [args], [directory])")
{
    return true; // Bug: BPNC error: need feedback on whether the command was sucessful
}


