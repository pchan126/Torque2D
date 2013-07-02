//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#import "AppDelegate.h"
#import "platformOSX/platformOSX.h"
#import "platformOSX/JoystickManager/JoystickManager.h"
#import "platformOSX/JoystickManager/Joystick.h"
#include "actionMap.h"

@implementation AppDelegate

@synthesize window;

//-----------------------------------------------------------------------------

- (void)dealloc
{
    [super dealloc];
}

//-----------------------------------------------------------------------------

- (void)applicationDidFinishLaunching:(NSNotification *)aNotification
{
    // Parse command line arguments
    const int kMaxCmdlineArgs = 32; // arbitrary
    const char* newArgv[kMaxCmdlineArgs];
    
    NSArray* arguments = [[NSProcessInfo processInfo] arguments];
    
    osxPlatState * platState = [osxPlatState sharedPlatState];
    
    for (NSUInteger i = 0; i < platState.argc; i++)
    {
        const char* pArg = [[arguments objectAtIndex:i] UTF8String];
        newArgv[i] = pArg;
    }
    
    platState.argv = newArgv;
    
   JoystickManager *theJoystickManager = [JoystickManager sharedInstance];
   [theJoystickManager setJoystickAddedDelegate:self];

   // With the command line arguments stored, let's run Torque
    [platState runTorque2D];
}

//-----------------------------------------------------------------------------

- (BOOL) applicationShouldTerminateAfterLastWindowClosed:(NSApplication *)theApplication
{
    return YES;
}

//-----------------------------------------------------------------------------

- (void)applicationWillTerminate:(NSNotification *)aNotification
{
    osxPlatState * platState = [osxPlatState sharedPlatState];
    [platState shutDownTorque2D];
}

- (void)joystickAdded:(Joystick *)joystick {
   [joystick registerForNotications:self];
   NSLog(@"added %@", joystick);
}

- (void)joystickStateChanged:(Joystick *)joystick {
   
   for (int i = 0; i< [joystick numAxes]; i++)
   {
      double value = mLerp(-1.0f, 1.0f, [joystick getRelativeValueOfAxesIndex:(i)]);
      {
         InputEventInfo inputEvent;

         inputEvent.deviceInst = 0;
         inputEvent.fValue = value;
         inputEvent.deviceType = JoystickDeviceType;
         inputEvent.objType = (i % 2) == 0 ? SI_XAXIS : SI_YAXIS;
         inputEvent.objInst = i/2;
         inputEvent.action = SI_MOVE;
         inputEvent.modifier = 0;

           // Give the ActionMap first shot.
           if (ActionMap::handleEventGlobal(&inputEvent))
               return;

           // If we get here we failed to process it with anything prior... so let
           // the ActionMap handle it.
           ActionMap::handleEvent(&inputEvent);
      }
   }
}

- (void) joystickButtonPushed:(int)buttonIndex onJoystick:(Joystick *)joystick {
   // Build the input event
    InputEventInfo inputEvent;

    inputEvent.deviceType = JoystickDeviceType;
    inputEvent.deviceInst = 0;
    inputEvent.objType = SI_BUTTON;
    inputEvent.objInst = KEY_BUTTON0+buttonIndex;
    inputEvent.ascii = 0;
    inputEvent.action = SI_MAKE;
    inputEvent.fValue = 1.0;

    // Give the ActionMap first shot.
    if (ActionMap::handleEventGlobal(&inputEvent))
        return;

    // If we get here we failed to process it with anything prior... so let
    // the ActionMap handle it.
    ActionMap::handleEvent(&inputEvent);
}

- (void) joystickButtonReleased:(int)buttonIndex onJoystick:(Joystick *)joystick {
   // Build the input event
    InputEventInfo inputEvent;

    inputEvent.deviceType = JoystickDeviceType;
    inputEvent.deviceInst = 0;
    inputEvent.objType = SI_BUTTON;
    inputEvent.objInst = KEY_BUTTON0+buttonIndex;
    inputEvent.ascii = 0;
    inputEvent.action = SI_BREAK;
    inputEvent.fValue = 1.0;

    // Give the ActionMap first shot.
    if (ActionMap::handleEventGlobal(&inputEvent))
        return;

    // If we get here we failed to process it with anything prior... so let
    // the ActionMap handle it.
    ActionMap::handleEvent(&inputEvent);
}
@end
