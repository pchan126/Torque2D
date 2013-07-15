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
#include "actionMap.h"
#include "gameInterface.h"

@implementation AppDelegate

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
    
    const char* cwd = Platform::getMainDotCsDir();
    Platform::setCurrentDirectory(cwd);
    
    osxPlatState * platState = [osxPlatState sharedPlatState];
    
    platState.argc = [arguments count];
    
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

    CFStringRef valueRef = 0;

    // Get product string
    valueRef = (CFStringRef) IOHIDDeviceGetProperty(joystick.device, CFSTR(kIOHIDProductKey));
    char HIDName[1024];
    if (valueRef)
    {
        CFStringGetCString(valueRef,
                HIDName,
                sizeof(HIDName),
                kCFStringEncodingUTF8);

    }
    NSLog(@"added %@ %s", joystick, HIDName);
}

- (void)joystickStateChanged:(Joystick *)joystick {
   
}

- (void) joystickButtonPushed:(int)buttonIndex onJoystick:(Joystick *)joystick {
   // Build the input event
    InputEvent inputEvent;

    JoystickManager *theJoystickManager = [JoystickManager sharedInstance];
    int val = [theJoystickManager deviceIDByReference:joystick.device];

    Con::printf("button: %i", buttonIndex);

    inputEvent.deviceInst = val;
    inputEvent.deviceType = JoystickDeviceType;
    inputEvent.objType = SI_BUTTON;
    inputEvent.objInst = KEY_BUTTON0+buttonIndex;
    inputEvent.ascii = 0;
    inputEvent.action = SI_MAKE;
    inputEvent.fValue = 1.0;

    Game->postEvent(inputEvent);
}

- (void) joystickButtonReleased:(int)buttonIndex onJoystick:(Joystick *)joystick {
   // Build the input event
    InputEvent inputEvent;

    JoystickManager *theJoystickManager = [JoystickManager sharedInstance];
    int val = [theJoystickManager deviceIDByReference:joystick.device];

    inputEvent.deviceType = JoystickDeviceType;
    inputEvent.deviceInst = val;
    inputEvent.objType = SI_BUTTON;
    inputEvent.objInst = KEY_BUTTON0+buttonIndex;
    inputEvent.ascii = 0;
    inputEvent.action = SI_BREAK;
    inputEvent.fValue = 1.0;

    Game->postEvent(inputEvent);
}

- (void)joystickAxisChanged:(int)axisIndex ofType:(int)type onJoystick:(Joystick*)joystick
{
    int i = axisIndex;
    double value = mLerp(-1.0f, 1.0f, [joystick getRelativeValueOfAxesIndex:(i)]);
    {
        InputEvent inputEvent;

        JoystickManager *theJoystickManager = [JoystickManager sharedInstance];
        int val = [theJoystickManager deviceIDByReference:joystick.device];

        inputEvent.deviceInst = val;
        inputEvent.fValue = value;
        inputEvent.deviceType = JoystickDeviceType;

        switch (type)
        {
            case kHIDUsage_GD_X:
            {
                inputEvent.objType = SI_XAXIS;
                break;
            }
            case kHIDUsage_GD_Y:
            {
                inputEvent.objType = SI_YAXIS;
                break;
            }
            case kHIDUsage_GD_Z:
            {
                inputEvent.objType = SI_ZAXIS;
                break;
            }
            case kHIDUsage_GD_Rx:
            {
                inputEvent.objType = SI_RXAXIS;
                break;
            }
            case kHIDUsage_GD_Ry:
            {
                inputEvent.objType = SI_RYAXIS;
                break;
            }
            case kHIDUsage_GD_Rz:
            {
                inputEvent.objType = SI_RZAXIS;
                break;
            }
            case kHIDUsage_GD_Slider:
            {
                inputEvent.objType = SI_SLIDER;
                break;
            }
        }

        inputEvent.objInst = 0;
        inputEvent.action = SI_MOVE;
        inputEvent.modifier = 0;

        Game->postEvent(inputEvent);
    }
}



@end
