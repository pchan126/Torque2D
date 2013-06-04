//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _osxWINDOW_INPUTGENERATOR_H_
#define _osxWINDOW_INPUTGENERATOR_H_

#include "windowManager/windowInputGenerator.h"
#include "platformOSX/windowManager/macWindow.h"

class osxWindowInputGenerator : public WindowInputGenerator
{

    protected:
       MacWindow *mWindow;

    // Event Handlers
    void handleMouseButtonEvent( WindowId did, U32 modifier, S32 x, S32 y, U32 action, U16 button );
   
   public:
   
      osxWindowInputGenerator( MacWindow *window );
      virtual ~osxWindowInputGenerator();
};

#endif // _osxWINDOW_INPUTGENERATOR_H_