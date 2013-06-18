//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformOSX/windowManager/osxWindowInputGenerator.h"
#include "platformOSX/windowManager/macWindow.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
osxWindowInputGenerator::osxWindowInputGenerator( MacWindow *window ):
    WindowInputGenerator((PlatformWindow*)window)
{
    mWindow = window;
   mWindow->mouseButtonEvent.notify(this, &osxWindowInputGenerator::handleMouseButton);
   mWindow->mouseEvent.notify(this, &osxWindowInputGenerator::handleMouseMove);
   mWindow->mouseWheelEvent.notify(this, &osxWindowInputGenerator::handleMouseWheel);
}

osxWindowInputGenerator::~osxWindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseButtonEvent.remove(this, &osxWindowInputGenerator::handleMouseButton);
      mWindow->mouseEvent.remove(this, &osxWindowInputGenerator::handleMouseMove);
      mWindow->mouseWheelEvent.remove(this, &osxWindowInputGenerator::handleMouseWheel);
   }
}


