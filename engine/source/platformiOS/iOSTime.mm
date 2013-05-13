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

#import "game/gameInterface.h"
#import "platformiOS/platformiOS.h"
#import "platform/event.h"
#import "game/gameInterface.h"
#import "platform/platformTimer.h"

#pragma mark ---- TimeManager Class Methods ----

//--------------------------------------

//static void _iOSUpdateSleepTicks()
//{
//    iOSPlatState * platState = [iOSPlatState sharedPlatState];
//
//    if (platState.backgrounded)
//        platState.sleepTicks = Platform::getBackgroundSleepTime();
//    else
//        platState.sleepTicks = sgTimeManagerProcessInterval;
//}
//
////------------------------------------------------------------------------------
//// Responsible for calculating ticks and posting the TimeEvent
//void TimeManager::process( double elapsedTime )
//{
//    iOSPlatState * platState = [iOSPlatState sharedPlatState];
////
////    _iOSUpdateSleepTicks();
////    
////    U32 curTime = Platform::getRealMilliseconds(); // GTC returns Milliseconds, FYI.
////   
////   Con::printf("%u", curTime);
//    S32 _elapsedTime = (S32)(elapsedTime * 1000);
//    
////    if(_elapsedTime <= platState.sleepTicks)
////    {
////        Platform::sleep(platState.sleepTicks - _elapsedTime);
////    }
//   
//    platState.lastTimeTick += _elapsedTime;
//    
//    TimeEvent event;
//    event.elapsedTime = _elapsedTime;
//    Game->postEvent(event);
//}


#pragma mark ---- Platform Namespace Time Functions  ----

//------------------------------------------------------------------------------
void Platform::getLocalTime(LocalTime &lt)
{
   struct tm systime;
   time_t long_time;

   /// Get time as long integer.
   time( &long_time );
   /// Convert to local time, thread safe.
   localtime_r( &long_time, &systime );
   
   /// Fill the return struct
   lt.sec      = systime.tm_sec;
   lt.min      = systime.tm_min;
   lt.hour     = systime.tm_hour;
   lt.month    = systime.tm_mon;
   lt.monthday = systime.tm_mday;
   lt.weekday  = systime.tm_wday;
   lt.year     = systime.tm_year;
   lt.yearday  = systime.tm_yday;
   lt.isdst    = systime.tm_isdst;
}   

/// Gets the time in seconds since the Epoch
U32 Platform::getTime()
{
   time_t epoch_time;
   time( &epoch_time );
   return epoch_time;
}   


//------------------------------------------------------------------------------
// Gets the time in milliseconds since some epoch. In this case, the current system
// absolute time. Storing milisec in a U32 overflows every 49.71 days.
U32 Platform::getRealMilliseconds()
{
    return (U32)([NSDate timeIntervalSinceReferenceDate] * 1000);
}

U32 Platform::getVirtualMilliseconds()
{
    return [[iOSPlatState sharedPlatState] currentSimTime];
}

void Platform::advanceTime(U32 delta)
{
    iOSPlatState * platState = [iOSPlatState sharedPlatState];
    platState.currentSimTime += delta;
}

/// Asks the operating system to put the process to sleep for at least ms milliseconds
void Platform::sleep(U32 ms)
{
   // note: this will overflow if you want to sleep for more than 49 days. just so ye know.
   
   //Luma:	Re-enable sleeping... why was it commented out? No sense in that!
   usleep( ms * 1000 );
}

PlatformTimer *PlatformTimer::create()
{
   return new DefaultPlatformTimer();
}

