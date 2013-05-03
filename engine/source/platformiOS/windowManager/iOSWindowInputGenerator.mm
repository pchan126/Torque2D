//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platformiOS/windowManager/iOSWindowInputGenerator.h"
#include "platformiOS/windowManager/iOSWindow.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


extern U32 convertModifierBits(const U32 in);


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
iOSWindowInputGenerator::iOSWindowInputGenerator( iOSWindow *window ):
    WindowInputGenerator((PlatformWindow*)window)
{
    mWindow = window;
   mWindow->tapEvent.notify(this, &iOSWindowInputGenerator::handleTapEvent);
    mWindow->panEvent.notify(this, &iOSWindowInputGenerator::handlePanEvent);
    mWindow->pinchEvent.notify(this, &iOSWindowInputGenerator::handlePinchEvent);
    mWindow->swipeEvent.notify(this, &iOSWindowInputGenerator::handleSwipeEvent);
    mWindow->rotationEvent.notify(this, &iOSWindowInputGenerator::handleRotationEvent);
    mWindow->longPressEvent.notify(this, &iOSWindowInputGenerator::handleLongPressEvent);
    mWindow->touchEvent.notify(this, &iOSWindowInputGenerator::handleTouchEvent);
}

iOSWindowInputGenerator::~iOSWindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->tapEvent.remove(this, &iOSWindowInputGenerator::handleTapEvent);
       mWindow->panEvent.remove(this, &iOSWindowInputGenerator::handlePanEvent);
       mWindow->pinchEvent.remove(this, &iOSWindowInputGenerator::handlePinchEvent);
       mWindow->swipeEvent.remove(this, &iOSWindowInputGenerator::handleSwipeEvent);
       mWindow->rotationEvent.remove(this, &iOSWindowInputGenerator::handleRotationEvent);
       mWindow->longPressEvent.remove(this, &iOSWindowInputGenerator::handleLongPressEvent);
       mWindow->touchEvent.remove(this, &iOSWindowInputGenerator::handleTouchEvent);
   }
}


//-----------------------------------------------------------------------------
// Touch Screen Events
//-----------------------------------------------------------------------------
void iOSWindowInputGenerator::handleTapEvent( WindowId did, U32 modifier, S32 x, S32 y, U32 eventType, U32 tapid )
{
    if( !mInputController || !mFocused )
      return;

    // Generate a base Movement along and Axis event
    ScreenTouchEventInfo event;
    event.modifier   = modifier;
    event.xPos = x * mWindow->mDisplayScale;
    event.yPos = y * mWindow->mDisplayScale;
    event.action = eventType;
//    Con::printf("handleTapEvent %i %i", x, y);
    mInputController->processScreenTouchEvent( event );
}

void iOSWindowInputGenerator::handlePanEvent( WindowId did, U32 modifier, F32 translation_x,F32 translation_y, F32 velocity_x, F32 velocity_y, U32 action )
{
    
}

void iOSWindowInputGenerator::handleSwipeEvent( WindowId did, U32 modifier, U32 direction )
{
    
}

void iOSWindowInputGenerator::handleRotationEvent( WindowId did, U32 modifier,S32 x, S32 y, F32 rotation , F32 velocity )
{
    
}

void iOSWindowInputGenerator::handleLongPressEvent( WindowId did, U32 modifier,S32 x,S32 y, U32 action, U32 touchid)
{
    
}

void iOSWindowInputGenerator::handlePinchEvent( WindowId did, U32 modifier, F32 scale, F32 velocity, U32 action )
{
    
}

void iOSWindowInputGenerator::handleTouchEvent( WindowId did, U32 modifier,S32 x,S32 y, U32 touchid, U32 action, U32 numTouches )
{
    if( !mInputController || !mFocused )
        return;

    
    // Generate a base Movement along and Axis event
    ScreenTouchEventInfo event;
    event.modifier   = modifier;
    event.xPos = x * mWindow->mDisplayScale;
    event.yPos = y * mWindow->mDisplayScale;
    event.action = action;
    event.numTouches = numTouches;
    mInputController->processScreenTouchEvent( event );
}


