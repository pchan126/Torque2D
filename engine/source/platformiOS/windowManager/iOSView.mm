//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import "./iOSView.h"
#import "platform/event.h"
#import "platform/platformInput.h"
#import "console/console.h"
#import "input/actionMap.h"
//#import "app/mainLoop.h"

// For left/right side definitions.
//#import <IOKit/hidsystem/IOLLEvent.h>

//
//#define WHEEL_DELTA ( 120 * 0.1 )
//
//static bool smApplicationInactive = false;
//
//
//extern U32 convertModifierBits( const U32 in );
//
//
//inline U32 NSModifiersToTorqueModifiers( NSUInteger mods )
//{
//   U32 torqueMods = 0;
//   
//   if( mods & NX_DEVICELSHIFTKEYMASK )
//      torqueMods |= IM_LSHIFT;
//   if( mods & NX_DEVICERSHIFTKEYMASK )
//      torqueMods |= IM_RSHIFT;
//   if( mods & NX_DEVICELALTKEYMASK )
//      torqueMods |= IM_LOPT;
//   if( mods & NX_DEVICERALTKEYMASK )
//      torqueMods |= IM_ROPT;
//   if( mods & NX_DEVICELCTLKEYMASK )
//      torqueMods |= IM_LCTRL;
//   if( mods & NX_DEVICERCTLKEYMASK )
//      torqueMods |= IM_RCTRL;
//   if( mods & NX_DEVICELCMDKEYMASK )
//      torqueMods |= IM_LALT;
//   if( mods & NX_DEVICERCMDKEYMASK )
//      torqueMods |= IM_RALT;
//      
//   Input::setModifierKeys( convertModifierBits( torqueMods ) );
//      
//   return torqueMods;
//}
//
//
//@implementation GGMacView
//- (void)setTorqueWindow:(iOSWindow*)theWindow
//{
//   mTorqueWindow = theWindow;
//   mLastMods = 0;
//}
//
//- (iOSWindow*)torqueWindow
//{
//   return mTorqueWindow;
//}
//
//-(void)trackModState:(U32)torqueKey withMacMods:(U32)macMods event:(NSEvent*)theEvent
//{
//   // track state:
//   //  translate the torque key code to the torque flag that changed
//   //  xor with existing mods for new mod state
//   //  clear anything that the mac says both siblings are not down ( to help stay in sync, a little bit )
//
//   //  ?set left sibling of anything that the mac says some sibling is down, but that we don't see as down?
//   
//   U32 torqueMod = 0;
//   switch(torqueKey)
//   {
//      case KEY_LSHIFT:     torqueMod = IM_LSHIFT;    break;
//      case KEY_RSHIFT:     torqueMod = IM_RSHIFT;    break;
//      case KEY_LCONTROL:   torqueMod = IM_LCTRL;     break;
//      case KEY_RCONTROL:   torqueMod = IM_RCTRL;     break;
//      case KEY_MAC_LOPT:   torqueMod = IM_LOPT;  break;
//      case KEY_MAC_ROPT:   torqueMod = IM_ROPT;  break;
//      case KEY_LALT:       torqueMod = IM_LALT;      break;
//      case KEY_RALT:       torqueMod = IM_RALT;      break;
//   };
//   
//   // flip the mod that we got an event for
//   U32 newMods = mLastMods ^ torqueMod;
//   
//   // clear left and right if mac thinks both are up.
//   if( !(macMods & NSShiftKeyMask))       newMods &= ~IM_LSHIFT, newMods &= ~IM_RSHIFT;
//   if( !(macMods & NSControlKeyMask))     newMods &= ~IM_LCTRL, newMods &= ~IM_RCTRL;
//   if( !(macMods & NSAlternateKeyMask))   newMods &= ~IM_LOPT, newMods &= ~IM_ROPT;
//   if( !(macMods & NSCommandKeyMask))     newMods &= ~IM_LALT, newMods &= ~IM_RALT;
//   
//   // Generate keyUp/Down event (allows us to use modifier keys for actions)
//   mLastMods = 0;
//   [self rawKeyUpDown:theEvent keyDown:(newMods & torqueMod)];
//   
//   mLastMods = newMods;
//   
//   Input::setModifierKeys( convertModifierBits( mLastMods ) );
//}
//
//- (Point2I)viewCoordsToTorqueCoords:(NSPoint)mousePoint
//{
//   if(mTorqueWindow->isFullscreen())
//   {
//      CGRect bounds = mTorqueWindow->getDisplayBounds();
//      CGRect mainbounds = mTorqueWindow->getMainDisplayBounds();
//      F32 offsetY = mainbounds.size.height - (bounds.size.height + bounds.origin.y);
//      Point2I pt(mousePoint.x - bounds.origin.x, bounds.size.height + offsetY - mousePoint.y);
//      return pt;
//   }
//   return Point2I(mousePoint.x, mTorqueWindow->getClientExtent().y - mousePoint.y);
//}
//
//- (void)signalGainFocus
//{
//   if(smApplicationInactive)
//      smApplicationInactive = false;
//   
//   bool gainFocus = static_cast<iOSWindowManager*>(WindowManager)->canWindowGainFocus(mTorqueWindow);   
//   if(gainFocus)
//      mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), GainFocus);
//}

//#pragma mark -
//#pragma mark Mouse Input
//// We're funnelling all the standard cocoa event handlers to -mouseUpDown and -mouseMotion.
//- (void)mouseDown:(NSEvent *)theEvent         { [self mouseUpDown:theEvent mouseDown:YES]; }
//- (void)rightMouseDown:(NSEvent *)theEvent    { [self mouseUpDown:theEvent mouseDown:YES]; }
//- (void)otherMouseDown:(NSEvent *)theEvent    { [self mouseUpDown:theEvent mouseDown:YES]; }
//- (void)mouseUp:(NSEvent *)theEvent           { [self mouseUpDown:theEvent mouseDown:NO]; }
//- (void)rightMouseUp:(NSEvent *)theEvent      { [self mouseUpDown:theEvent mouseDown:NO]; }
//- (void)otherMouseUp:(NSEvent *)theEvent      { [self mouseUpDown:theEvent mouseDown:NO]; }
//- (void)mouseDragged:(NSEvent *)theEvent      { [self mouseMotion:theEvent]; }
//- (void)rightMouseDragged:(NSEvent *)theEvent { [self mouseMotion:theEvent]; }
//- (void)otherMouseDragged:(NSEvent *)theEvent { [self mouseMotion:theEvent]; }
//- (void)mouseMoved:(NSEvent *)theEvent        { [self mouseMotion:theEvent]; }
//
//- (void)mouseUpDown:(NSEvent *)theEvent mouseDown:(BOOL)isMouseDown
//{
//   Point2I eventLocation = [self viewCoordsToTorqueCoords: [theEvent locationInWindow]];
//   U16 buttonNumber = [theEvent buttonNumber];  
//   U32 action  = isMouseDown ? SI_MAKE : SI_BREAK;
//
//   // If the event location is negative then it occurred in the structure region (e.g. title bar, resize corner), and we don't want
//   // to lock the mouse/drop into fullscreen for that.
//   if(WindowManager->getFocusedWindow() != mTorqueWindow && eventLocation.x > 0 && eventLocation.y > 0)
//      [self signalGainFocus];
//      
//   mLastMods = NSModifiersToTorqueModifiers( [ theEvent modifierFlags ] );
//
//   mTorqueWindow->buttonEvent.trigger(mTorqueWindow->getWindowId(), mLastMods, action, buttonNumber);
//}
//
//- (void)mouseMotion:(NSEvent *)theEvent
//{
//   mTorqueWindow->_doMouseLockNow();
//   
//   // when moving the mouse to the center of the window for mouse locking, we need
//   // to skip the next mouse delta event
//   if(mTorqueWindow->isMouseLocked() && mTorqueWindow->_skipNextMouseEvent())
//   {
//      mTorqueWindow->_skippedMouseEvent();
//      return;
//   }
//   
//   Point2I eventLocation;
//   if(mTorqueWindow->isMouseLocked())
//   {
//      eventLocation.x = [theEvent deltaX]; 
//      eventLocation.y = [theEvent deltaY];
//   }
//   else
//   {
//      eventLocation = [self viewCoordsToTorqueCoords:[theEvent locationInWindow]];  
//   }
//   
//   mLastMods = NSModifiersToTorqueModifiers( [ theEvent modifierFlags ] );
//
//   mTorqueWindow->mouseEvent.trigger(mTorqueWindow->getWindowId(), mLastMods, eventLocation.x, eventLocation.y, mTorqueWindow->isMouseLocked());
//}
//
//- (void)scrollWheel:(NSEvent *)theEvent
//{
//   float deltaX = [ theEvent deltaX ];
//   float deltaY = [ theEvent deltaY ];
//
//   if( mIsZero( deltaX ) && mIsZero( deltaY ) )
//      return;
//   
//   mLastMods = NSModifiersToTorqueModifiers( [ theEvent modifierFlags ] );
//
//   mTorqueWindow->wheelEvent.trigger( mTorqueWindow->getWindowId(), mLastMods, S32( deltaX * WHEEL_DELTA ), S32( deltaY * WHEEL_DELTA ) );
//}
//
//#pragma mark -
//#pragma mark Keyboard Input
//- (BOOL)performKeyEquivalent:(NSEvent *)theEvent
//{
//   // Pass it off to the main menu.  If the main menu handled it, we're done.
//   if([[NSApp mainMenu] performKeyEquivalent:theEvent])
//      return YES;
//   
//   // cmd-q will quit.  End of story.
//   if(([theEvent modifierFlags] & NSCommandKeyMask && !([theEvent modifierFlags] & NSAlternateKeyMask) && !([theEvent modifierFlags] & NSControlKeyMask)) && [theEvent keyCode] == 0x0C)
//   {
//      StandardMainLoop::shutdown();
//      [NSApp terminate:self];
//      return YES;
//   }
//   
//   // In fullscreen we grab ALL keyboard events, including ones which would normally be handled by the system,
//   // like cmd-tab.  Thus, we need to specifically check for cmd-tab and bail out of fullscreen and hide the app
//   // to switch to the next application.
//   // 0x30 is tab, so this grabs command-tab and command-shift-tab
//   if([theEvent keyCode] == 0x30 && mTorqueWindow->isFullscreen())
//   {
//      // Bail!
//      mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseFocus);
//      [NSApp hide:nil];
//      return YES;
//   }
//   
//   // All other events are uninteresting to us and can be handled by Torque.
//   if([theEvent type] == NSKeyDown)
//      [self keyDown:theEvent];
//   else if([theEvent type] == NSKeyUp)
//      [self keyUp:theEvent];
//      
//   // Don't let the default handler continue processing these events, it does things we don't like.
//   return YES;
//}
//
//- (void)flagsChanged:(NSEvent *)theEvent
//{
//   U32 torqueKeyCode = TranslateOSKeyCode([theEvent keyCode]);
//   U32 macMods = [theEvent modifierFlags];
//   [self trackModState:torqueKeyCode withMacMods:macMods event:theEvent];
//}
//
//- (void)keyDown:(NSEvent *)theEvent
//{
//   // If keyboard translation is on and the key isn't bound in the
//   // global action map, first try turning this into one or more
//   // character events.
//   
//   if(    mTorqueWindow->getKeyboardTranslation()
//       && !mTorqueWindow->shouldNotTranslate(
//               convertModifierBits( NSModifiersToTorqueModifiers( [ theEvent modifierFlags ] ) ),
//               ( InputObjectInstances ) TranslateOSKeyCode( [ theEvent keyCode ] ) ) )
//   {
//      mHandledAsCharEvent = false;
//      [ self interpretKeyEvents: [ NSArray arrayWithObject: theEvent ] ];
//      
//      if( mHandledAsCharEvent )
//         return;
//   }
//   
//   // Fire as raw keyboard event.
//   
//   [ self rawKeyUpDown: theEvent keyDown: YES ];
//}
//
//- (void)keyUp:(NSEvent *)theEvent
//{
//   [self rawKeyUpDown:theEvent keyDown:NO];
//}
//
//- (void)rawKeyUpDown:(NSEvent *)theEvent keyDown:(BOOL)isKeyDown
//{
//   U32 action;
//   if([theEvent type] != NSFlagsChanged)
//      action = isKeyDown ? ([theEvent isARepeat] ? SI_REPEAT : SI_MAKE) : SI_BREAK;
//   else
//      action = isKeyDown ? SI_MAKE : SI_BREAK;
//   
//   U32 torqueKeyCode = TranslateOSKeyCode([theEvent keyCode]);
//   mLastMods = NSModifiersToTorqueModifiers( [ theEvent modifierFlags ] );
//
//   mTorqueWindow->keyEvent.trigger(mTorqueWindow->getWindowId(), mLastMods, action, torqueKeyCode);
//}
//
//- (void)insertText:(id)_inString
//{
//   // input string may be an NSString or an NSAttributedString
//   NSString *text = [_inString isKindOfClass:[NSAttributedString class]] ? [_inString string] : _inString;
//   for(int i = 0; i < [text length]; i++)
//   {
//      mTorqueWindow->charEvent.trigger(mTorqueWindow->getWindowId(), mLastMods, [text characterAtIndex:i]);
//      mHandledAsCharEvent = true;
//   }
//}
//
//#pragma mark -
//#pragma mark Application Delegate
//- (void)applicationDidResignActive:(NSNotification *)aNotification
//{
//   smApplicationInactive = true;
//   mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseFocus);
//   [NSApp setDelegate:nil];
//}
//
//- (void)applicationDidHide:(NSNotification *)aNotification
//{
//   mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseFocus);
//}
//
//- (void)applicationDidUnhide:(NSNotification *)aNotification
//{
//   mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), GainFocus);   
//}
//
//#ifndef TORQUE_SHARED
//
//- (NSApplicationTerminateReply)applicationShouldTerminate:(NSApplication*)sender
//{
//   Platform::postQuitMessage(0);
//   return NSTerminateCancel;
//}
//
//#endif

//#pragma mark ---- OSXTorqueView Input Handling ----
//
////-----------------------------------------------------------------------------
//// Fills out the modifiers based on key presses such as shift, alt, etc
//- (void)getModifierKey:(U32&)modifiers event:(NSEvent *)event;
//{
//    /*
//     NSAlphaShiftKeyMask = 1 << 16,
//     NSShiftKeyMask      = 1 << 17,
//     NSControlKeyMask    = 1 << 18,
//     NSAlternateKeyMask  = 1 << 19,
//     NSCommandKeyMask    = 1 << 20,
//     NSNumericPadKeyMask = 1 << 21,
//     NSHelpKeyMask       = 1 << 22,
//     NSFunctionKeyMask   = 1 << 23,
//     NSDeviceIndependentModifierFlagsMask = 0xffff0000U
//     */
//    
//    U32 keyMods = [event modifierFlags];
//    
//    if (keyMods & NSShiftKeyMask)
//        modifiers |= SI_SHIFT;
//    
//    if (keyMods & NSCommandKeyMask)
//        modifiers |= SI_ALT;
//    
//    if (keyMods & NSAlternateKeyMask)
//        modifiers |= SI_MAC_OPT;
//    
//    if (keyMods & NSControlKeyMask)
//        modifiers |= SI_CTRL;
//}
//
////-----------------------------------------------------------------------------
//// Processes mouse up and down events, posts to the event system
//- (void)processMouseButton:(NSEvent *)event button:(KeyCodes)button action:(U8)action
//{
//    // Get the click location
//    NSPoint clickLocation = [self convertPoint:[event locationInWindow] fromView:nil];
//    
//    CGRect bounds = [self bounds];
//    
//    clickLocation.y = bounds.size.height - clickLocation.y;
//    
////    // Move the cursor
////    Canvas->setCursorPos(Point2I((S32) clickLocation.x, (S32) clickLocation.y));
//    
//    // Grab any modifiers
//    U32 modifiers = 0;
//    [self getModifierKey:modifiers event:event];
//    
//    // Build the input event
//    InputEventInfo torqueEvent;
//    
//    torqueEvent.deviceType = MouseDeviceType;
//    torqueEvent.deviceInst = 0;
//    torqueEvent.objType = SI_BUTTON;
//    torqueEvent.objInst = button;
//    torqueEvent.modifier = modifiers;
//    torqueEvent.ascii = 0;
//    torqueEvent.action = action;
//    torqueEvent.fValue = 1.0;
//    
//    // Post the input event
////    mTorqueWindow->mouseEvent.trigger(mTorqueWindow->getWindowId(), mLastMods, action, torqueKeyCode);
////    Game->postEvent(torqueEvent);
//}
//
////-----------------------------------------------------------------------------
//// Processes keyboard up and down events, posts to the event system
//- (void)processKeyEvent:(NSEvent *)event make:(BOOL)make
//{
//    // If input and keyboard are enabled
//    if (!Input::isEnabled() && !Input::isKeyboardEnabled())
//        return;
//    
//    unichar chars = [[event charactersIgnoringModifiers] characterAtIndex:0];
//    
//    // Get the key code for the event
//    U32 keyCode = [event keyCode];
//    
//    U16 objInst = TranslateOSKeyCode(keyCode);
//    
//    // Grab any modifiers
//    U32 modifiers = 0;
//    [self getModifierKey:modifiers event:event];
//    
//    // Build the input event
//    InputEventInfo torqueEvent;
//    
//    F32 fValue = 1.0f;
//    U8 action = SI_MAKE;
//    
//    if (!make)
//    {
//        action = SI_BREAK;
//        fValue = 0.0f;
//    }
//    else if(make && [event isARepeat])
//    {
//        action = SI_REPEAT;
//    }
//    
//    torqueEvent.deviceType = KeyboardDeviceType;
//    torqueEvent.deviceInst = 0;
//    torqueEvent.objType = SI_KEY;
//    torqueEvent.objInst = objInst;
//    torqueEvent.modifier = modifiers;
//    torqueEvent.ascii = 0;
//    torqueEvent.action = action;
//    torqueEvent.fValue = fValue;
//    torqueEvent.ascii = chars;
//    
//    // Post the input event
////    mTorqueWindow->keyEvent.trigger(mTorqueWindow->getWindowId(), mLastMods, action, torqueKeyCode);
////    Game->postEvent(torqueEvent);
//}
//
////-----------------------------------------------------------------------------
//// Default mouseDown override
//- (void)mouseDown:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self processMouseButton:event button:KEY_BUTTON0 action:SI_MAKE];
//}
//
////-----------------------------------------------------------------------------
//// Default rightMouseDown override
//- (void)rightMouseDown:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self processMouseButton:event button:KEY_BUTTON1 action:SI_MAKE];
//}
//
////-----------------------------------------------------------------------------
//// Default otherMouseDown override
//- (void)otherMouseDown:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self processMouseButton:event button:KEY_BUTTON2 action:SI_MAKE];
//}
//
////-----------------------------------------------------------------------------
//// Default mouseUp override
//- (void)mouseUp:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self processMouseButton:event button:KEY_BUTTON0 action:SI_BREAK];
//}
//
////-----------------------------------------------------------------------------
//// Default rightMouseUp override
//- (void)rightMouseUp:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self processMouseButton:event button:KEY_BUTTON1 action:SI_BREAK];
//}
//
////-----------------------------------------------------------------------------
//// Default otherMouseUp override
//- (void)otherMouseUp:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self processMouseButton:event button:KEY_BUTTON2 action:SI_BREAK];
//}
//
////-----------------------------------------------------------------------------
//// Default otherMouseDown override
//- (void)mouseMoved:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    // Get the mouse location
//    NSPoint location = [self convertPoint:[event locationInWindow] fromView:nil];
//    
//    // NSViews increase the Y the higher the cursor
//    // Torque needs that to be inverted
//    CGRect bounds = [self bounds];
//    location.y = bounds.size.height - location.y;
//    
//    // Grab any modifiers
//    U32 modifiers = 0;
//    [self getModifierKey:modifiers event:event];
//    
////    // Move the cursor
////    Canvas->setCursorPos(Point2I((S32) location.x, (S32) location.y));
//    
//    // Post the event
//    mTorqueWindow->mouseEvent.trigger(mTorqueWindow->getWindowId(), modifiers, (S32)location.x, (S32)location.y, mTorqueWindow->isMouseLocked());
//}
//
////-----------------------------------------------------------------------------
//// Default mouseDragged override
//- (void)mouseDragged:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self mouseMoved:event];
//}
//
////-----------------------------------------------------------------------------
//// Default rightMouseDragged override
//- (void)rightMouseDragged:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self mouseMoved:event];
//}
//
////-----------------------------------------------------------------------------
//// Default otherMouseDragged override
//- (void)otherMouseDragged:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    [self mouseMoved:event];
//}
//
////-----------------------------------------------------------------------------
//// Default scrollWheel override
//- (void)scrollWheel:(NSEvent *)event
//{
//    if (!Input::isEnabled() && !Input::isMouseEnabled())
//        return;
//    
//    F32 deltaY = [event deltaY];
//    
//    if (deltaY == 0)
//        return;
//    
//    // Grab any modifiers
//    U32 modifiers = 0;
//    [self getModifierKey:modifiers event:event];
//    
//    InputEventInfo torqueEvent;
//    
//    torqueEvent.deviceType = MouseDeviceType;
//    torqueEvent.deviceInst = 0;
//    torqueEvent.objType = SI_ZAXIS;
//    torqueEvent.objInst = 0;
//    torqueEvent.modifier = modifiers;
//    torqueEvent.ascii = 0;
//    torqueEvent.action = SI_MOVE;
//    torqueEvent.fValue = deltaY;
////    Game->postEvent(torqueEvent);
//}
//
////-----------------------------------------------------------------------------
//// Default keyDown override
//- (void)keyDown:(NSEvent *)event
//{
//    // If input and keyboard are enabled
//    if (!Input::isEnabled() && !Input::isKeyboardEnabled())
//        return;
//    
//    [self processKeyEvent:event make:YES];
//}
//
////-----------------------------------------------------------------------------
//// Default keyUp override
//- (void)keyUp:(NSEvent *)event
//{
//    // If input and keyboard are enabled
//    if (!Input::isEnabled() && !Input::isKeyboardEnabled())
//        return;
//    
//    [self processKeyEvent:event make:NO];
//}
//
//
//#pragma mark -
//#pragma mark Window Delegate
//- (BOOL)windowShouldClose:(NSWindow *)sender
//{
//   // We close the window ourselves
//   mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), WindowDestroy);
//   return NO;
//}
//
//- (void)windowWillClose:(NSNotification *) notification
//{
//   mTorqueWindow->_disassociateCocoaWindow();
//}
//
//- (void)windowDidBecomeKey:(NSNotification *)notification
//{
//   // when our window is the key window, we become the app delegate.
//   PlatformWindow* focusWindow = WindowManager->getFocusedWindow();
//   if(focusWindow && focusWindow != mTorqueWindow)
//      focusWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseFocus);
//   [NSApp setDelegate:self];
//   [self signalGainFocus];
//}
//
//- (void)windowDidResignKey:(NSNotification*)notification
//{
//   mTorqueWindow->appEvent.trigger(mTorqueWindow->getWindowId(), LoseScreen);
//   mTorqueWindow->_associateMouse();
//   mTorqueWindow->setCursorVisible(true);
//   [NSApp setDelegate:nil];
//}
//
//- (void)windowDidChangeScreen:(NSNotification*)notification
//{
//   NSWindow* wnd = [notification object];
//   // TODO: Add a category to UIScreen to deal with this
//   CGDirectDisplayID disp = (CGDirectDisplayID)[[[[wnd screen] deviceDescription] valueForKey:@"UIScreenNumber"] unsignedIntValue];
//   mTorqueWindow->setDisplay(disp);
//}
//
//- (void)windowDidResize:(NSNotification*)notification
//{
//   Point2I clientExtent = mTorqueWindow->getClientExtent();
//   mTorqueWindow->resizeEvent.trigger(mTorqueWindow->getWindowId(), clientExtent.x, clientExtent.y);
//}
//
//#pragma mark -
//#pragma mark responder status
//- (BOOL)acceptsFirstResponder { return YES; }
//- (BOOL)becomeFirstResponder  { return YES; }
//- (BOOL)resignFirstResponder  { return YES; }
//
//// Basic implementation because NSResponder's default implementation can cause infinite loops when our keyDown: method calls interpretKeyEvents:
//- (void)doCommandBySelector:(SEL)aSelector
//{
//   if([self respondsToSelector:aSelector])
//      [self performSelector:aSelector withObject:nil];
//}

//@end
