//
//  Joystick.m
//  JoystickHIDTest
//
//  Created by John Stringham on 12-05-01.
//  Copyright 2012 We Get Signal. All rights reserved.
//

#import "Joystick.h"
#import "JoystickHatswitch.h"

@implementation Joystick

@synthesize device;

- (id)initWithDevice:(IOHIDDeviceRef)theDevice
{
    NSArray *axisTypes = [NSArray arrayWithObjects:kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z, kHIDUsage_GD_Rz, kHIDUsage_GD_Rx, kHIDUsage_GD_Ry, kHIDUsage_GD_Slider, nil];
    self = [super init];
    if (self) {
        device = theDevice;
        
        delegates = [[NSMutableArray alloc] initWithCapacity:0];
        
        elements = (NSArray *)IOHIDDeviceCopyMatchingElements(theDevice, NULL, kIOHIDOptionsTypeNone);
        
        NSMutableArray *tempButtons = [NSMutableArray array];
        NSMutableArray *tempAxes = [NSMutableArray array];
        NSMutableArray *tempHats = [NSMutableArray array];
        NSMutableArray *tempMisc = [NSMutableArray array];
        
        int i;
        for (i=0; i<elements.count; ++i) {
            IOHIDElementRef thisElement = (IOHIDElementRef)[elements objectAtIndex:i];
            
            int elementType = IOHIDElementGetType(thisElement);
            int elementUsage = IOHIDElementGetUsage(thisElement);
            
            if (elementUsage == kHIDUsage_GD_Hatswitch) {
                JoystickHatswitch *hatSwitch = [[JoystickHatswitch alloc] initWithElement:thisElement andOwner:self];
                [tempHats addObject:hatSwitch];
                [hatSwitch release];
//            if (elementUsage == kHIDUsage_GD_X || elementUsage == kHIDUsage_GD_Y)
            } else if ([axisTypes containsObject: elementUsage] ) {
                [tempAxes addObject:thisElement];
            } else if (elementType == kIOHIDElementTypeInput_Button) {
                [tempButtons addObject:thisElement];
            } else if ( elementType == kIOHIDElementTypeInput_Misc) {
                [tempMisc addObject:thisElement];
            } else
                NSLog(@"unidentified element");

        }
        buttons = [[NSArray arrayWithArray:tempButtons] retain];
        axes = [[NSArray arrayWithArray:tempAxes] retain];
        hats = [[NSArray arrayWithArray:tempHats] retain];
        misc = [[NSArray arrayWithArray:tempMisc] retain];
        
        NSLog(@"New device address: %p from %p",device,theDevice);
        NSLog(@"found %lu buttons, %lu axes, %lu hats, %lu misc",tempButtons.count,tempAxes.count,tempHats.count, tempMisc.count);
        // For more detailed info there are Usage tables
        // eg: kHIDUsage_GD_X
        // declared in IOHIDUsageTables.h
        // could use to determine major axes
    }
    
    return self;
}

- (void)elementReportedChange:(IOHIDElementRef)theElement {

    NSArray *axisTypes = [NSArray arrayWithObjects:kHIDUsage_GD_X, kHIDUsage_GD_Y, kHIDUsage_GD_Z, kHIDUsage_GD_Rz, kHIDUsage_GD_Rx, kHIDUsage_GD_Ry, kHIDUsage_GD_Slider, nil];
    int elementType = IOHIDElementGetType(theElement);
    IOHIDValueRef pValue;
    IOHIDDeviceGetValue(device, theElement, &pValue);
    
    int elementUsage = IOHIDElementGetUsage(theElement);
    int value = IOHIDValueGetIntegerValue(pValue);
    int i;
    
    if (elementUsage == kHIDUsage_GD_Hatswitch) {
        
        // determine a unique offset. index is buttons.count
        // so all dpads will report buttons.count+(hats.indexOfObject(hatObject)*5)
        // 8 ways are interpreted as UP DOWN LEFT RIGHT so this is fine.
        int offset = (int)[buttons count];
        JoystickHatswitch *hatswitch = nil;

        for (i=0; i<hats.count; ++i) {
            hatswitch = [hats objectAtIndex:i];
            
            if ([hatswitch element] == theElement) {
                offset += i*5;
                break;
            }
        }
        
        if (hatswitch)
        {
            for (i=0; i<delegates.count; ++i) {
                id <JoystickNotificationDelegate> delegate = [delegates objectAtIndex:i];
                [hatswitch checkValue:value andDispatchButtonPressesWithIndexOffset:offset toDelegate:delegate];
            }
        }
        
        return;
    }
    
    if (elementType == kIOHIDElementTypeInput_Button)
    {
        for (i=0; i<delegates.count; ++i) {
            id <JoystickNotificationDelegate> delegate = [delegates objectAtIndex:i];

            if (value==1)
                [delegate joystickButtonPushed:[buttons indexOfObject:theElement] onJoystick:self];
            else
                [delegate joystickButtonReleased:[buttons indexOfObject:theElement] onJoystick:self];
                 
        }
        NSLog(@"button #%i reported value of %d", [buttons indexOfObject:theElement], value);
        return;
    }

//    if (elementType == kIOHIDElementTypeInput_Axis) {
    if ( [axisTypes containsObject: elementUsage] ) {
        for (i=0; i<delegates.count; ++i) {
            id <JoystickNotificationDelegate> delegate = [delegates objectAtIndex:i];

            [delegate joystickAxisChanged:[axes indexOfObject:theElement] ofType:elementUsage onJoystick:self];
        }

//        CFIndex lmin = IOHIDElementGetLogicalMin(theElement);
//        CFIndex lmax = IOHIDElementGetLogicalMax(theElement);

//        if (elementUsage == kHIDUsage_GD_X)
//        {
//            NSLog(@"X Axis #%i reported value of %d", [axes indexOfObject:theElement], value);
//        }
//        else if ( elementUsage == kHIDUsage_GD_Y)
//        {
//            NSLog(@"Y Axis #%i reported value of %d", [axes indexOfObject:theElement], value);
//        }
//        else if ( elementUsage == kHIDUsage_GD_Z )
//        {
//            NSLog(@"Z Axis #%i reported value of %d", [axes indexOfObject:theElement], value);
//        }

    }
}

- (void)registerForNotications:(id <JoystickNotificationDelegate>)delegate {
    [delegates addObject:delegate];
}

- (void)deregister:(id<JoystickNotificationDelegate>)delegate {
    [delegates removeObject:delegate];
}

- (int)getElementIndex:(IOHIDElementRef)theElement {
    int elementType = IOHIDElementGetType(theElement);
    
    NSArray *searchArray;
    NSString *returnString = @"";
    
    if (elementType == kIOHIDElementTypeInput_Button) {
        searchArray = buttons;
        returnString = @"Button";
    } else {
        searchArray = axes;
        returnString = @"Axis";
    }
    
    int i;
    
    for (i=0; i<searchArray.count; ++i) {
        if ([searchArray objectAtIndex:i] == theElement)
            return i;
            //  returnString = [NSString stringWithFormat:@"%@_%d",returnString,i];
    }
    
    return -1;
}

- (double)getRelativeValueOfAxesIndex:(int)index {
    IOHIDElementRef theElement = [axes objectAtIndex:index];
    
    double value;
    double min = IOHIDElementGetPhysicalMin(theElement);
    double max = IOHIDElementGetPhysicalMax(theElement);
    
    IOHIDValueRef pValue;
    IOHIDDeviceGetValue(device, theElement, &pValue);
    
    value = ((double)IOHIDValueGetIntegerValue(pValue)-min) * (1/(max-min));
    
    return value;
}

- (unsigned int)numButtons {
    return (unsigned int)[buttons count];
}

- (unsigned int)numAxes {
    return (unsigned int)[axes count];
}

- (unsigned int)numHats {
    return (unsigned int)[hats count];
}

- (void)dealloc
{
    [delegates release];
    
    [axes release];
    [hats release];
    [buttons release];
    
    [elements release];
    
    [super dealloc];
}

@end
