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

#ifndef _PLATFORM_TIME_MANAGER_H_
#define _PLATFORM_TIME_MANAGER_H_

#include "platform/platform.h"
#include "delegates/delegateSignal.h"

//------------------------------------------------------------------------------

extern S32 sgTimeManagerProcessInterval;

class PlatformTimer
{
protected:
   PlatformTimer();
public:
   virtual ~PlatformTimer();

   virtual const S32 getElapsedMs() = 0;
   virtual void reset() = 0;

   static PlatformTimer *create();
};


class TimeManager
{
   PlatformTimer *mTimer;
   S32 mForegroundThreshold, mBackgroundThreshold;
   bool mBackground;
   
   void _updateTime();
   
public:
   TimeManagerEvent timeEvent;
   
   TimeManager();
   ~TimeManager();
   
   void setForegroundThreshold(const S32 msInterval);
   const S32 getForegroundThreshold() const { return mForegroundThreshold; };
   
   void setBackgroundThreshold(const S32 msInterval);
   const S32 getBackgroundThreshold() const { return mBackgroundThreshold; };
   
   void setBackground(const bool isBackground) { mBackground = isBackground; };
   const bool getBackground() const { return mBackground; };
};

class DefaultPlatformTimer : public PlatformTimer
{
   S32 mLastTime, mNextTime;
   
public:
   DefaultPlatformTimer()
   {
      mLastTime = mNextTime = Platform::getRealMilliseconds();
   }
   
   const S32 getElapsedMs()
   {
      mNextTime = Platform::getRealMilliseconds();
      return (mNextTime - mLastTime);
   }
   
   void reset()
   {
      mLastTime = mNextTime;
   }
};

#endif // _PLATFORM_TIME_MANAGER_H_
