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

#include "platformiOS/platformiOS.h"
#include "platform/event.h"
#include "platform/platformInput.h"
#include "game/gameInterface.h"
#include "platform/threads/thread.h"
#include <pthread.h>

#include "platformiOS/iOSUtil.h"
#include "platformiOS/iOSEvents.h"
#include "platformiOS/iOSAlerts.h"
#include "gui/guiCanvas.h"



/*
EventHandlerRef gWinMouseEventHandlerRef  = NULL;
EventHandlerRef gAppMouseEventHandlerRef  = NULL;
*/



//-----------------------------------------------------------------------------
static void _OnActivate(bool activating)
{
//    iOSPlatState * platState = [iOSPlatState sharedPlatState];
//
//    if(activating)
//   {
//      Input::activate();
//      Game->refreshWindow();
//      platState.backgrounded = false;
//      pthread_kill((pthread_t)platState.torqueThreadId, SIGALRM);
//   }
//   else
//   {
//      Input::deactivate();
//      platState.backgrounded = true;
//   }
}

//-----------------------------------------------------------------------------
// here we manually poll for events, and send them to the dispatcher.
// we only use this in single-threaded mode.
static void _iOSPollEvents()
{
}



//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
void Platform::enableKeyboardTranslation(void)
{
    
}

//-----------------------------------------------------------------------------
// Not yet implemented. Will resolve in the next platform update
void Platform::disableKeyboardTranslation(void)
{
    
}


//-----------------------------------------------------------------------------
void Platform::process()
{

}

