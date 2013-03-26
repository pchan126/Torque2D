//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "windowManager/windowInputGenerator.h"
#include "windowManager/platformWindow.h"
#include "input/actionMap.h"
#include "component/interfaces/IProcessInput.h"


//extern InputModifiers convertModifierBits(const U32 in);


//-----------------------------------------------------------------------------
// Constructor/Destructor
//-----------------------------------------------------------------------------
WindowInputGenerator::WindowInputGenerator( PlatformWindow *window ) : 
                                             mWindow(window),
                                             mInputController(NULL),
                                             mLastCursorPos(0,0),
                                             mClampToWindow(true),
                                             mPixelsPerMickey(1.0f),
                                             mNotifyPosition(true),
                                             mFocused(false)
{
   AssertFatal(mWindow, "NULL PlatformWindow on WindowInputGenerator creation");

#ifdef TORQUE_OS_XENON
   mFocused = true;
#endif

   if (mWindow->getOffscreenRender())
      mFocused = true;

   mWindow->appEvent.notify(this, &WindowInputGenerator::handleAppEvent);
   mWindow->mouseEvent.notify(this, &WindowInputGenerator::handleMouseMove);
   mWindow->wheelEvent.notify(this, &WindowInputGenerator::handleMouseWheel);
   mWindow->buttonEvent.notify(this, &WindowInputGenerator::handleMouseButton);
   mWindow->keyEvent.notify(this, &WindowInputGenerator::handleKeyboard);
   mWindow->charEvent.notify(this, &WindowInputGenerator::handleCharInput);

   // We also want to subscribe to input events.
   Input::smInputEvent.notify(this, &WindowInputGenerator::handleInputEvent);
}

WindowInputGenerator::~WindowInputGenerator()
{
   if( mWindow )
   {
      mWindow->mouseEvent.remove(this, &WindowInputGenerator::handleMouseMove);
      mWindow->buttonEvent.remove(this, &WindowInputGenerator::handleMouseButton);
      mWindow->wheelEvent.remove(this, &WindowInputGenerator::handleMouseWheel);
      mWindow->keyEvent.remove(this, &WindowInputGenerator::handleKeyboard);
      mWindow->charEvent.remove(this, &WindowInputGenerator::handleCharInput);
      mWindow->appEvent.remove(this, &WindowInputGenerator::handleAppEvent);
   }

   Input::smInputEvent.remove(this, &WindowInputGenerator::handleInputEvent);
}

//-----------------------------------------------------------------------------
// Process an input event and pass it on.
// Respect the action map.
//-----------------------------------------------------------------------------
void WindowInputGenerator::generateInputEvent( InputEventInfo &inputEvent )
{
   if( !mInputController || !mFocused )
      return;

   // Give the ActionMap first shot.
   if (ActionMap::handleEventGlobal(&inputEvent))
      return;

   if( mInputController->processInputEvent( inputEvent ) )
      return;

   // If we get here we failed to process it with anything prior... so let
   // the ActionMap handle it.
   ActionMap::handleEvent(&inputEvent);

}

//-----------------------------------------------------------------------------
// Mouse Events
//-----------------------------------------------------------------------------
void WindowInputGenerator::handleMouseMove( WindowId did, U32 modifier, S32 x, S32 y, bool isRelative )
{
    if( !mInputController || !mFocused )
      return;

    // Generate a base Movement along and Axis event
    MouseMoveEventInfo event;
    event.modifier   = modifier;
    event.xPos = x;
    event.yPos = y;

    mInputController->processMouseMoveEvent( event );
}

//void WindowInputGenerator::handleMouseButton( WindowId did, U32 modifiers, U32 action, U16 button )
//{
//   if( !mInputController || !mFocused )
//      return;
//
//   InputEventInfo event;
//   event.deviceType = MouseDeviceType;
//   event.deviceInst = 0;
//   event.objType    = SI_BUTTON;
//   event.objInst    = (InputObjectInstances)(KEY_BUTTON0 + button);
//   event.modifier   = convertModifierBits(modifiers);
//   event.ascii      = 0;
//   event.action     = (action==IA_MAKE) ? SI_MAKE : SI_BREAK;
//   event.fValue     = (action==IA_MAKE) ? 1.0 : 0.0;
//
//   generateInputEvent(event);
//}
//
//void WindowInputGenerator::handleMouseWheel( WindowId did, U32 modifiers, S32 wheelDeltaX, S32 wheelDeltaY )
//{
//   if( !mInputController || !mFocused )
//      return;
//
//   InputEventInfo event;
//   event.deviceType = MouseDeviceType;
//   event.deviceInst = 0;
//   event.objType    = SI_AXIS;
//   event.modifier   = convertModifierBits(modifiers);
//   event.ascii      = 0;
//   event.action     = SI_MOVE;
//
//   if( wheelDeltaY ) // Vertical
//   {
//      event.objInst    = SI_ZAXIS;
//      event.fValue     = (F32)wheelDeltaY;
//
//      generateInputEvent(event);
//   }
//   if( wheelDeltaX ) // Horizontal
//   {
//      event.objInst    = SI_RZAXIS;
//      event.fValue     = (F32)wheelDeltaX;
//
//      generateInputEvent(event);
//   }
//}

////-----------------------------------------------------------------------------
//// Key/Character Input
////-----------------------------------------------------------------------------
//void WindowInputGenerator::handleCharInput( WindowId did, U32 modifier, U16 key )
//{
//   if( !mInputController || !mFocused )
//      return;
//
//   InputEventInfo event;
//   event.deviceType  = KeyboardDeviceType;
//   event.deviceInst  = 0;
//   event.objType     = SI_KEY;
//   event.objInst     = KEY_NULL;
//   event.modifier    = convertModifierBits(modifier);
//   event.ascii       = key;
//   event.action      = SI_MAKE;
//   event.fValue      = 1.0;
//   generateInputEvent(event);
//
//   event.action = SI_BREAK;
//   event.fValue = 0.f;
//   generateInputEvent(event);
//}

//
//void WindowInputGenerator::handleKeyboard( WindowId did, U32 modifier, U32 action, U16 key )
//{
//   if( !mInputController || !mFocused )
//      return;
//
//   InputEventInfo event;
//   event.deviceType  = KeyboardDeviceType;
//   event.deviceInst  = 0;
//   event.objType     = SI_KEY;
//   event.objInst     = (InputObjectInstances)key;
//   event.modifier    = convertModifierBits(modifier);
//   event.ascii       = 0;
//
//   switch(action)
//   {
//   case IA_MAKE:
//      event.action = SI_MAKE;
//      event.fValue = 1.f;
//      break;
//
//   case IA_REPEAT:
//      event.action = SI_REPEAT;
//      event.fValue = 1.f;
//      break;
//
//   case IA_BREAK:
//      event.action = SI_BREAK;
//      event.fValue = 0.f;
//      break;
//
//      // If we encounter an unknown don't submit the event.
//   default:
//      //Con::warnf("GuiCanvas::handleKeyboard - got an unknown action type %d!", action);
//      return;
//   }
//
//   generateInputEvent(event);
//}

////-----------------------------------------------------------------------------
//// Raw input 
////-----------------------------------------------------------------------------
//void WindowInputGenerator::handleInputEvent( U32 deviceInst,F32 fValue, U16 deviceType, U16 objType, U16 ascii, U16 objInst, U8 action, U8 modifier )
//{
//   // Skip it if we don't have focus.
//   if(!mInputController || !mFocused)
//      return;
//
//   // Convert to an InputEventInfo and pass it around for processing.
//   InputEventInfo event;
//   event.deviceInst  = deviceInst;
//   event.fValue      = fValue;
//   event.deviceType  = (InputDeviceTypes)deviceType;
//   event.objType     = (InputEventType)objType;
//   event.ascii       = ascii;
//   event.objInst     = (InputObjectInstances)objInst;
//   event.action      = (InputActionType)action;
//   event.modifier    = (InputModifiers)modifier;
//   
//   generateInputEvent(event);
//}

//-----------------------------------------------------------------------------
// Window Events
//-----------------------------------------------------------------------------
void WindowInputGenerator::handleAppEvent( WindowId did, S32 event )
{
   if(event == LoseFocus)
   {
      // Fire all breaks; this will prevent issues with dangling keys.
//      ActionMap::clearAllBreaks();
      mFocused = false;
   }
   else if(event == GainFocus)
   {
      // Set an update flag to notify the consumer of the absolute mouse position next move
      mNotifyPosition = true;
      mFocused = true;
   }

   // always focused with offscreen rendering
   if (mWindow->getOffscreenRender())
      mFocused = true;
}

//-----------------------------------------------------------------------------
// Character Input Mapping
//-----------------------------------------------------------------------------

bool WindowInputGenerator::wantAsKeyboardEvent( U32 modifiers, U32 keyCode )
{
   // Disallow translation on keys that are bound in the global action map.
   
   return ActionMap::getGlobalMap()->isAction(
      KeyboardDeviceType,
      0,
      modifiers,
      keyCode
   );
}
