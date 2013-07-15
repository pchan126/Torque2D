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
#import "platformOSX/osxInputManager.h"
#include "actionMap.h"
#include "gameInterface.h"

#pragma mark ---- InputManager Handling ----


void gamepadAction(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef value) {


    //  NSLog(@"gamepadAction from: %p",inSender);

    IOHIDElementRef element = IOHIDValueGetElement(value);
    IOHIDDeviceRef device = IOHIDElementGetDevice(element);


    // NSLog(@"or is it from: %p",device);

    osxInputManager* JoystickManager = dynamic_cast<osxInputManager*>(Input::getManager());
    JoystickManager->elementReportedChange(device, element);
}

void gamepadWasAdded(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef device) {
    IOHIDDeviceOpen(device, kIOHIDOptionsTypeNone);
    IOHIDDeviceRegisterInputValueCallback(device, gamepadAction, inContext);

    osxInputManager* JoystickManager = dynamic_cast<osxInputManager*>(Input::getManager());

    Joystick *newJoystick = [[Joystick alloc] initWithDevice:device];
    JoystickManager->registerNewJoystick(newJoystick);
    IOHIDDeviceScheduleWithRunLoop(device, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    [newJoystick release];
}

void gamepadWasRemoved(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef device) {
    osxInputManager* JoystickManager = dynamic_cast<osxInputManager*>(Input::getManager());
    JoystickManager->unregisterJoystick(device);
    NSLog(@"Gamepad was unplugged");
}


//--------------------------------------------------------------------------
osxInputManager::osxInputManager()
{
    mKeyboardEnabled = false;
    mMouseEnabled = false;
    mEnabled = false;

    joysticks = [[NSMutableDictionary alloc] init];
    joystickIDIndex = 0;

    hidManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone);

    int usageKeys[] = { kHIDUsage_GD_GamePad,kHIDUsage_GD_Joystick,kHIDUsage_GD_MultiAxisController };

    int i;

    NSMutableArray *criterionSets = [NSMutableArray arrayWithCapacity:3];

    for (i=0; i<3; ++i) {
        int usageKeyConstant = usageKeys[i];
        NSMutableDictionary* criterion = [[NSMutableDictionary alloc] init];
        [criterion setObject: [NSNumber numberWithInt: kHIDPage_GenericDesktop] forKey: (NSString*)CFSTR(kIOHIDDeviceUsagePageKey)];
        [criterion setObject: [NSNumber numberWithInt: usageKeyConstant] forKey: (NSString*)CFSTR(kIOHIDDeviceUsageKey)];
        [criterionSets addObject:criterion];
        [criterion release];
    }

    IOHIDManagerSetDeviceMatchingMultiple(hidManager, (CFArrayRef)criterionSets);
    //IOHIDManagerSetDeviceMatching(hidManager, (CFDictionaryRef)criterion);
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager, gamepadWasAdded, (void*)this);
    IOHIDManagerRegisterDeviceRemovalCallback(hidManager, gamepadWasRemoved, (void*)this);
    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOReturn tIOReturn = IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
    (void)tIOReturn; // to suppress warnings
 }

//--------------------------------------------------------------------------
bool osxInputManager::enable()
{
    mEnabled = true;
    return mEnabled;
}

//--------------------------------------------------------------------------
void osxInputManager::disable()
{
    mEnabled = false;
}

//--------------------------------------------------------------------------
void osxInputManager::process()
{
    for ( S32 joystickID = 0; joystickID < joystickIDIndex; joystickID++)
    {
        Joystick* joystick = nil;
        joystick = [joysticks objectForKey:[NSNumber numberWithInt:joystickID]];

        if (joystick != nil)
        {
            for ( S32 index = 0; index < joystick.axes.count; index++)
            {
                IOHIDElementRef theElement = (IOHIDElementRef)[joystick.axes objectAtIndex:index];

                S32 elementUsage = IOHIDElementGetUsage(theElement);
                if (elementUsage >= kHIDUsage_GD_X && elementUsage <= kHIDUsage_GD_Slider)
                {
                    IOHIDValueRef pValue;

                    if (IOHIDDeviceGetValue(  joystick.device, theElement, &pValue) == kIOReturnSuccess)
                    {
                        F32 min = (F32)IOHIDElementGetPhysicalMin(theElement);
                        F32 max = (F32)IOHIDElementGetPhysicalMax(theElement);

                        F32 value = mLerp(-1.0f, 1.0f, ((F32)(IOHIDValueGetIntegerValue(pValue)-min) * (1/(max-min))));

                        InputEvent inputEvent;
                        inputEvent.fValue = (F32)value;
                        inputEvent.deviceType = JoystickDeviceType;
                        inputEvent.deviceInst = (U32)joystickID;

                        switch (elementUsage)
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
                            default:
                                break;
                        }

                        inputEvent.objInst = (U16)index;
                        inputEvent.action = SI_MOVE;
                        inputEvent.modifier = 0;

                        Game->postEvent(inputEvent);
                    }
                }
            }
        }
    }
}

#pragma mark ---- Keyboard Handling Functions ----

//--------------------------------------------------------------------------
void osxInputManager::enableKeyboard()
{
    if (!mEnabled)
        mEnabled = true;

    mKeyboardEnabled = true;
}

//--------------------------------------------------------------------------
void osxInputManager::disableKeyboard()
{
    mKeyboardEnabled = false;
}

//--------------------------------------------------------------------------
bool osxInputManager::isKeyboardEnabled()
{
    if (mEnabled)
        return mKeyboardEnabled;

    return false;
}

#pragma mark ---- Mouse Handling Functions ----

//--------------------------------------------------------------------------
void osxInputManager::enableMouse()
{
    if (!mEnabled)
        mEnabled = true;

    mMouseEnabled = true;
}

//--------------------------------------------------------------------------
void osxInputManager::disableMouse()
{
    mMouseEnabled = false;
}

//--------------------------------------------------------------------------
bool osxInputManager::isMouseEnabled()
{
    if (mEnabled)
        return mMouseEnabled;

    return false;
}

void osxInputManager::registerNewJoystick(Joystick *joystick)
{
    [joysticks setObject:joystick forKey:[NSNumber numberWithInt:joystickIDIndex++]];
    NSLog(@"Gamepad was plugged in");
    NSLog(@"Gamepads registered: %u", (U32)joysticks.count);
}

void osxInputManager::unregisterJoystick(IOHIDDeviceRef deviceRef)
{
//    S32 joystickID = getDeviceIDByReference(deviceRef);
//    [joysticks removeObjectForKey:[NSNumber numberWithInt:joystickID]];
}

S32 osxInputManager::getDeviceIDByReference(IOHIDDeviceRef deviceRef)
{
    //  NSLog(@"Searching for device id by pointer: %p",deviceRef);
    for (id key in joysticks) {
        Joystick *thisJoystick = [joysticks objectForKey:key];
        //    NSLog(@"Comparing to joystick id: %d with device: %p",[(NSNumber *)key intValue],[thisJoystick device]);

        if ([thisJoystick device] == deviceRef) {
            //  NSLog(@"Found.");
            return [((NSNumber *)key) intValue];
        }
    }

    return -1;
}


void osxInputManager::elementReportedChange(IOHIDDeviceRef device, IOHIDElementRef theElement)
{
    S32 joystickID = getDeviceIDByReference(device);

    if (joystickID == -1) {
        NSLog(@"Invalid device reported.");
        return;
    }

    //   NSLog(@"Device index %d reported",joystickID);

    Joystick* joystick = [joysticks objectForKey:[NSNumber numberWithInt:joystickID]];
    S32 elementType = IOHIDElementGetType(theElement);
    IOHIDValueRef pValue;
    IOHIDDeviceGetValue(joystick.device, theElement, &pValue);

    S32 elementUsage = IOHIDElementGetUsage(theElement);
    S32 value = IOHIDValueGetIntegerValue(pValue);
    S32 buttonIndex = [joystick getButtonIndex:theElement];

    if (elementType == kIOHIDElementTypeInput_Button)
    {
        InputEvent inputEvent;

        inputEvent.deviceType = JoystickDeviceType;
        inputEvent.deviceInst = (U32)joystickID;
        inputEvent.objType = SI_BUTTON;
        inputEvent.objInst = KEY_BUTTON0+buttonIndex;
        inputEvent.ascii = 0;
        inputEvent.fValue = 1.0;

        if (value==1)
            inputEvent.action = SI_MAKE;
        else
            inputEvent.action = SI_BREAK;


        Game->postEvent(inputEvent);

        NSLog(@"button #%i reported value of %d", buttonIndex, value);
        return;
    }
}

