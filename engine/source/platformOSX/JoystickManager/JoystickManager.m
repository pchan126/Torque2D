//
//  JoystickManager.m
//  JoystickHIDTest
//
//  Created by John Stringham on 12-05-01.
//  Copyright 2012 We Get Signal. All rights reserved.
//

#import "JoystickManager.h"
#import "JoystickNotificationDelegate.h"

@implementation JoystickManager

@synthesize joystickAddedDelegate;

static JoystickManager *instance;

- (id)init {
    self = [super init];
    
    if (self) {
        joysticks = [[NSMutableDictionary alloc] init];
        joystickIDIndex = 0;
        [self setupGamepads];
    }
    
    return self;
}

+ (void)initialize
{
    static BOOL initialized = NO;
    
    if (!initialized) {
        initialized = YES;
        instance = [[JoystickManager alloc] init];
    }
}

+ (JoystickManager *)sharedInstance {
    return instance;
}

- (unsigned long)connectedJoysticks {
    return [joysticks count];
}

- (int)deviceIDByReference:(IOHIDDeviceRef)deviceRef {
  //  NSLog(@"Searching for device id by pointer: %p",deviceRef);
    for (id key in joysticks) {
        Joystick *thisJoystick = joysticks[key];
    //    NSLog(@"Comparing to joystick id: %d with device: %p",[(NSNumber *)key intValue],[thisJoystick device]);
        
        if ([thisJoystick device] == deviceRef) {
          //  NSLog(@"Found.");
            return [((NSNumber *)key) intValue];
        }
    }
    
    return -1;
}

- (Joystick *)joystickByID:(int)joystickID {
    return joysticks[@(joystickID)];
}

- (void)registerNewJoystick:(Joystick *)joystick {
    joysticks[@(joystickIDIndex++)] = joystick;
    NSLog(@"Gamepad was plugged in");
    NSLog(@"Gamepads registered: %li", (unsigned long)joysticks.count);
    [joystickAddedDelegate joystickAdded:joystick];
}



void gamepadWasRemoved(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef device) {
    NSLog(@"Gamepad was unplugged");
}

void gamepadAction(void* inContext, IOReturn inResult, void* inSender, IOHIDValueRef value) {
    
    
  //  NSLog(@"gamepadAction from: %p",inSender);
    
    IOHIDElementRef element = IOHIDValueGetElement(value);
    IOHIDDeviceRef device = IOHIDElementGetDevice(element);
    
    
   // NSLog(@"or is it from: %p",device);
    
    int joystickID = [[JoystickManager sharedInstance] deviceIDByReference:device];
    
    if (joystickID == -1) {
        NSLog(@"Invalid device reported.");
        return;
    }
    
 //   NSLog(@"Device index %d reported",joystickID);
    
    Joystick *theJoystick = [[JoystickManager sharedInstance] joystickByID:joystickID];
    
   // NSLog(@"%@",[theJoystick giveButtonOrAxesIndex:element]);
    [theJoystick elementReportedChange:element];
    
}

void gamepadWasAdded(void* inContext, IOReturn inResult, void* inSender, IOHIDDeviceRef device) {
    IOHIDDeviceOpen(device, kIOHIDOptionsTypeNone);
	IOHIDDeviceRegisterInputValueCallback(device, gamepadAction, inContext);
    
    Joystick *newJoystick = [[Joystick alloc] initWithDevice:device];
    [[JoystickManager sharedInstance] registerNewJoystick:newJoystick];
    
}


-(void) setupGamepads {
    hidManager = IOHIDManagerCreate( kCFAllocatorDefault, kIOHIDOptionsTypeNone);
    
    int usageKeys[] = { kHIDUsage_GD_GamePad,kHIDUsage_GD_Joystick,kHIDUsage_GD_MultiAxisController };
    
    int i;
    
    NSMutableArray *criterionSets = [NSMutableArray arrayWithCapacity:3];
    
    for (i=0; i<3; ++i) {
        int usageKeyConstant = usageKeys[i];
        NSMutableDictionary* criterion = [[NSMutableDictionary alloc] init];
        criterion[(NSString*)CFSTR(kIOHIDDeviceUsagePageKey)] = @(kHIDPage_GenericDesktop);
        criterion[(NSString*)CFSTR(kIOHIDDeviceUsageKey)] = @(usageKeyConstant);
        [criterionSets addObject:criterion];
    }
    
	IOHIDManagerSetDeviceMatchingMultiple(hidManager, (CFArrayRef)CFBridgingRetain(criterionSets));
    //IOHIDManagerSetDeviceMatching(hidManager, (CFDictionaryRef)criterion);
    IOHIDManagerRegisterDeviceMatchingCallback(hidManager, gamepadWasAdded, (__bridge void*)self);
    IOHIDManagerRegisterDeviceRemovalCallback(hidManager, gamepadWasRemoved, (__bridge void*)self);
    IOHIDManagerScheduleWithRunLoop(hidManager, CFRunLoopGetCurrent(), kCFRunLoopDefaultMode);
    IOReturn tIOReturn = IOHIDManagerOpen(hidManager, kIOHIDOptionsTypeNone);
    (void)tIOReturn; // to suppress warnings
    
}


@end
