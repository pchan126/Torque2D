//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _GLFW_WINDOW_INPUTGENERATOR_H_
#define _GLFW_WINDOW_INPUTGENERATOR_H_

#include "windowManager/windowInputGenerator.h"
#include "./GLFWWindow.h"

class GLFWWindowInputGenerator : public WindowInputGenerator
{

    protected:
       GLFWWindow *mWindow;
   
   public:
   
      GLFWWindowInputGenerator( GLFWWindow *window );
      virtual ~GLFWWindowInputGenerator();
   

};

#endif // _osxWINDOW_INPUTGENERATOR_H_