
#include "./platformTimer.h"
#include "delegates/process.h"

void TimeManager::_updateTime()
{
   S32 delta = mTimer->getElapsedMs();
   
   S32 msTillThresh = (mBackground ? mBackgroundThreshold : mForegroundThreshold) - delta;
   
   if (msTillThresh > 0)
   {
      Platform::sleep( msTillThresh );
   }
   
   S32 finalDelta = mTimer->getElapsedMs();
   mTimer->reset();
   
   timeEvent.trigger(finalDelta);
}


TimeManager::TimeManager()
{
   mBackground = false;
   mTimer = PlatformTimer::create();
   Process::notify(this, &TimeManager::_updateTime, PROCESS_TIME_ORDER);

   mForegroundThreshold = 5;
   mBackgroundThreshold = 10;
}

TimeManager::~TimeManager()
{
   Process::remove(this, &TimeManager::_updateTime);
   delete mTimer;
}

void TimeManager::setForegroundThreshold(const S32 msInterval)
{
   AssertFatal(msInterval > 0, "TimeManager::setForegroundThreshold - should have at least 1 ms between time events to avoid math problems!");
   mForegroundThreshold = msInterval;
}

void TimeManager::setBackgroundThreshold(const S32 msInterval)
{
   AssertFatal(msInterval > 0, "TimeManager::setBackgroundThreshold - should have at least 1 ms between time events to avoid math problems!");
   mBackgroundThreshold = msInterval;
}

PlatformTimer::PlatformTimer()
{
   
}

PlatformTimer::~PlatformTimer()
{
   
}


