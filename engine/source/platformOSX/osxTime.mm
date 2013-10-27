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
#import "platform/platform.h"
#import "platformOSX/platformOSX.h"
#import "platform/event.h"
#import "game/gameInterface.h"
#import "delegates/process.h"

#pragma mark ---- TimeManager Class Methods ----

extern S32 sgTimeManagerProcessInterval;

//------------------------------------------------------------------------------

static void _OSXUpdateSleepTicks()
{
    osxPlatState * platState = [osxPlatState sharedPlatState];
    
    if (platState.backgrounded)
        platState.sleepTicks = (U32)Platform::getBackgroundSleepTime();
    else
        platState.sleepTicks = (U32)sgTimeManagerProcessInterval;
}

#pragma mark ---- Platform Namespace Time Functions  ----

//------------------------------------------------------------------------------
// Gets the local time on an OS X device
void Platform::getLocalTime(LocalTime &lt)
{
    struct tm systime;
    time_t long_time;
    
    /// Get time as long integer.
    time(&long_time);
    
    /// Convert to local time, thread safe.
    localtime_r(&long_time, &systime);
    
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


//------------------------------------------------------------------------------
// Gets the running time for this app in milliseconds
U32 Platform::getVirtualMilliseconds()
{
    return [[osxPlatState sharedPlatState] currentSimTime];
}

//------------------------------------------------------------------------------
// Moves the simulation time forward
void Platform::advanceTime(U32 delta)
{
    osxPlatState * platState = [osxPlatState sharedPlatState];
    platState.currentSimTime += delta;
}

//------------------------------------------------------------------------------
// Asks the operating system to put the process to sleep for at least ms milliseconds
void Platform::sleep(U32 ms)
{
    // Note: This will overflow if you want to sleep for more than 49 days
    usleep(ms * 1000);
}




