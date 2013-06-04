//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformOSX/windowManager/osxWindowInputGenerator.h"
#include "platformOSX/windowManager/macWindow.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


extern U32 convertModifierBits(const U32 in);


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
osxWindowInputGenerator::osxWindowInputGenerator( MacWindow *window ):
    WindowInputGenerator((PlatformWindow*)window)
{
    mWindow = window;
   mWindow->mouseButtonEvent.notify(this, &osxWindowInputGenerator::handleMouseButtonEvent);
   mWindow->mouseEvent.notify(this, &osxWindowInputGenerator::handleMouseMove);
}

osxWindowInputGenerator::~osxWindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseButtonEvent.remove(this, &osxWindowInputGenerator::handleMouseButtonEvent);
   }
}


//-----------------------------------------------------------------------------
// Touch Screen Events
//-----------------------------------------------------------------------------
void osxWindowInputGenerator::handleMouseButtonEvent( WindowId did, U32 modifier, S32 x, S32 y, U32 action, U16 button )
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



