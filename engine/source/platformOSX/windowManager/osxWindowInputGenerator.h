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
   
   public:
   
      osxWindowInputGenerator( MacWindow *window );
      virtual ~osxWindowInputGenerator();
   

};

#endif // _osxWINDOW_INPUTGENERATOR_H_