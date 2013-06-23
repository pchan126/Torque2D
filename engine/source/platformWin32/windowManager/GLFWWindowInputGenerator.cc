//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "./GLFWWindowInputGenerator.h"
#include "./GLFWWindow.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
GLFWWindowInputGenerator::GLFWWindowInputGenerator( GLFWWindow *window ):
    WindowInputGenerator((PlatformWindow*)window)
{
    mWindow = window;
   mWindow->mouseButtonEvent.notify(this, &GLFWWindowInputGenerator::handleMouseButton);
   mWindow->mouseEvent.notify(this, &GLFWWindowInputGenerator::handleMouseMove);
   mWindow->mouseWheelEvent.notify(this, &GLFWWindowInputGenerator::handleMouseWheel);
}

GLFWWindowInputGenerator::~GLFWWindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseButtonEvent.remove(this, &GLFWWindowInputGenerator::handleMouseButton);
      mWindow->mouseEvent.remove(this, &GLFWWindowInputGenerator::handleMouseMove);
      mWindow->mouseWheelEvent.remove(this, &GLFWWindowInputGenerator::handleMouseWheel);
   }
}


