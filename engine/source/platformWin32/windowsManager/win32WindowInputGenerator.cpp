//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformWin32/windowsManager/win32WindowInputGenerator.h"
#include "platformWin32/windowsManager/win32Window.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


extern U32 convertModifierBits(const U32 in);


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
win32WindowInputGenerator::win32WindowInputGenerator( Win32Window *window ):
    WindowInputGenerator((PlatformWindow*)window)
{
    mWindow = window;
   mWindow->mouseButtonEvent.notify(this, &win32WindowInputGenerator::handleMouseButtonEvent);
   mWindow->mouseEvent.notify(this, &win32WindowInputGenerator::handleMouseMove);
}

win32WindowInputGenerator::~win32WindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseButtonEvent.remove(this, &win32WindowInputGenerator::handleMouseButtonEvent);
   }
}


//-----------------------------------------------------------------------------
// Touch Screen Events
//-----------------------------------------------------------------------------
void win32WindowInputGenerator::handleMouseButtonEvent( WindowId did, U32 modifier, S32 x, S32 y, U32 action, U16 button )
{
    if( !mInputController || !mFocused )
      return;

    // Generate a base Movement along and Axis event
    ButtonEventInfo event;
    event.modifier   = modifier;
    event.xPos = x;
    event.yPos = y;
    event.action = action;
    event.buttonID = button;
//    Con::printf("handleTapEvent %i %i", x, y);
    mInputController->processMouseButtonEvent( event );
}



