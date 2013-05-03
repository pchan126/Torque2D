//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _IOSWINDOW_INPUTGENERATOR_H_
#define _IOSWINDOW_INPUTGENERATOR_H_

#include "windowManager/windowInputGenerator.h"
#include "platformiOS/windowManager/iOSWindow.h"

class iOSWindowInputGenerator : public WindowInputGenerator
{

    protected:
       iOSWindow *mWindow;

    // Event Handlers
    void handleTapEvent( WindowId did, U32 modifier, S32 x, S32 y, U32 eventType, U32 tapid );
    void handlePanEvent( WindowId did, U32 modifier, F32 translation_x,F32 translation_y, F32 velocity_x, F32 velocity_y, U32 action );
    void handleSwipeEvent( WindowId did, U32 modifier, U32 direction );
    void handleRotationEvent( WindowId did,U32 modifier,S32 x, S32 y, F32 rotation , F32 velocity );
    void handleLongPressEvent( WindowId did, U32 modifier,S32 x,S32 y, U32 action, U32 touchid );
    void handlePinchEvent( WindowId did, U32 modifier, F32 scale, F32 velocity, U32 action );
    void handleTouchEvent( WindowId did, U32 modifier,S32 x,S32 y, U32 touchid, U32 action, U32 numTouches );
    
   public:
   
      iOSWindowInputGenerator( iOSWindow *window );
      virtual ~iOSWindowInputGenerator();
};

#endif // _IOSWINDOW_INPUTGENERATOR_H_