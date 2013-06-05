//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _win32WINDOW_INPUTGENERATOR_H_
#define _win32WINDOW_INPUTGENERATOR_H_

#include "windowManager/windowInputGenerator.h"
#include "platformWin32/windowsManager/win32Window.h"

class win32WindowInputGenerator : public WindowInputGenerator
{

    protected:
       Win32Window *mWindow;

    // Event Handlers
    void handleMouseButtonEvent( WindowId did, U32 modifier, S32 x, S32 y, U32 action, U16 button );
   
   public:
   
      win32WindowInputGenerator( Win32Window *window );
      virtual ~win32WindowInputGenerator();
};

#endif // _osxWINDOW_INPUTGENERATOR_H_