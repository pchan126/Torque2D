//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformWin32/windowManager/Win32WindowInputGenerator.h"
#include "platformWin32/windowManager/win32Window.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


extern U32 convertModifierBits(const U32 in);


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
Win32WindowInputGenerator::Win32WindowInputGenerator( Win32Window *window ):
    WindowInputGenerator((PlatformWindow*)window)
{
    mWindow = window;
   mWindow->mouseButtonEvent.notify(this, &Win32WindowInputGenerator::handleMouseButton);
   mWindow->mouseEvent.notify(this, &Win32WindowInputGenerator::handleMouseMove);
}

Win32WindowInputGenerator::~Win32WindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseButtonEvent.remove(this, &Win32WindowInputGenerator::handleMouseButton);
   }
}


