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
#include "console/console.h"
#include "console/consoleTypes.h"
#include "platform/threads/mutex.h"

//Added for the cprintf below
#include <stdarg.h>
#include <stdio.h>
#include <chrono>

S32 sgBackgroundProcessSleepTime = 200;
S32 sgTimeManagerProcessInterval = 0;


void Platform::initConsole()
{
   Con::addVariable("Pref::backgroundSleepTime", TypeS32, &sgBackgroundProcessSleepTime);
   Con::addVariable("Pref::timeManagerProcessInterval", TypeS32, &sgTimeManagerProcessInterval);
}

S32 Platform::getBackgroundSleepTime()
{
   return sgBackgroundProcessSleepTime;
}

void Platform::cprintf( const char* str )
{
    printf( "%s \n", str );
}

bool Platform::hasExtension(const char* pFilename, const char* pExtension)
{
    // Sanity!
    AssertFatal( pFilename != nullptr, "Filename cannot be nullptr." );
    AssertFatal( pExtension != nullptr, "Extension cannot be nullptr." );

    // Find filename length.
    const size_t filenameLength = dStrlen( pFilename );

    // Find extension length.
    const size_t extensionLength = dStrlen( pExtension );

    // Skip if extension is longer than filename.
    if ( extensionLength >= filenameLength )
        return false;

    // Check if extension exists.
    return dStricmp( pFilename + filenameLength - extensionLength, pExtension ) == 0;
}

/// Gets the time in seconds since the Epoch
U32 Platform::getTime()
{
    using namespace std::chrono;
    std::chrono::seconds tp = std::chrono::duration_cast<std::chrono::seconds>(system_clock::now().time_since_epoch());
    return (U32)tp.count();
}

//------------------------------------------------------------------------------
// Gets the time in milliseconds since some epoch. In this case, the current system
// absolute time. Storing milisec in a U32 overflows every 49.71 days.
U32 Platform::getRealMilliseconds()
{
    using namespace std::chrono;
    std::chrono::milliseconds tp = std::chrono::duration_cast<std::chrono::milliseconds>(system_clock::now().time_since_epoch());
    return (U32)tp.count();
};

ConsoleFunction( createUUID, const char*, 1, 1, "() - Creates a UUID string." )
{
    return Platform::createUUID();
}
