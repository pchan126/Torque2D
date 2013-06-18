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
   mWindow->mouseButtonEvent.notify(this, &osxWindowInputGenerator::handleMouseButton);
   mWindow->mouseEvent.notify(this, &osxWindowInputGenerator::handleMouseMove);
}

osxWindowInputGenerator::~osxWindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseButtonEvent.remove(this, &osxWindowInputGenerator::handleMouseButton);
   }
}


