//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _win32WINDOW_INPUTGENERATOR_H_
#define _win32WINDOW_INPUTGENERATOR_H_

#include "windowManager/windowInputGenerator.h"
#include "platformWin32/windowManager/win32Window.h"

class Win32WindowInputGenerator : public WindowInputGenerator
{

    protected:
       Win32Window *mWindow;

   public:
   
      Win32WindowInputGenerator( Win32Window *window );
      virtual ~Win32WindowInputGenerator();
};

#endif // _osxWINDOW_INPUTGENERATOR_H_