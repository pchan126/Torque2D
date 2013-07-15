//
//  Joystick.m
//  JoystickHIDTest
//
//  Created by John Stringham on 12-05-01.
//  Copyright 2012 We Get Signal. All rights reserved.
//

#import "Joystick.h"

@implementation Joystick

@synthesize device;
@synthesize axes = _axes;

- (id)initWithDevice:(IOHIDDeviceRef)theDevice
{
    self = [super init];
    if (self) {
        device = theDevice;
        
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
            
            if (elementUsage >= kHIDUsage_GD_X && elementUsage <= kHIDUsage_GD_Slider) {
                [tempAxes addObject:(id)thisElement];
            } else if (elementType == kIOHIDElementTypeInput_Button) {
                [tempButtons addObject:(id)thisElement];
            } else if ( elementType == kIOHIDElementTypeInput_Misc) {
                [tempMisc addObject:(id)thisElement];
            }

        }
        buttons = [[NSArray arrayWithArray:tempButtons] retain];
        _axes = [[NSArray arrayWithArray:tempAxes] retain];
        hats = [[NSArray arrayWithArray:tempHats] retain];
        misc = [[NSArray arrayWithArray:tempMisc] retain];
        
        NSLog(@"New device address: %p from %p",device,theDevice);
        NSLog(@"found %u buttons, %u axes, %u hats, %u misc",tempButtons.count,tempAxes.count,tempHats.count, tempMisc.count);
    }
    
    return self;
}

- (int)getElementIndex:(IOHIDElementRef)theElement {
    int elementType = IOHIDElementGetType(theElement);
    
    NSArray *searchArray;
    NSString *returnString = @"";
    
    if (elementType == kIOHIDElementTypeInput_Button) {
        searchArray = buttons;
        returnString = @"Button";
    } else {
        searchArray = self.axes;
        returnString = @"Axis";
    }
    
    int i;
    
    for (i=0; i<searchArray.count; ++i) {
        if ((IOHIDElementRef)[searchArray objectAtIndex:i] == theElement)
            return i;
            //  returnString = [NSString stringWithFormat:@"%@_%d",returnString,i];
    }
    
    return -1;
}

- (int)getButtonIndex:(IOHIDElementRef)theElement {
    return [buttons indexOfObject:(id)theElement];
}

- (int)getAxisIndex:(IOHIDElementRef)theElement {
    return [self.axes indexOfObject:(id)theElement];
}


- (double)getRelativeValueOfAxesIndex:(int)index {
    IOHIDElementRef theElement = (IOHIDElementRef)[self.axes objectAtIndex:index];
    
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
    return (unsigned int)[self.axes count];
}

- (unsigned int)numHats {
    return (unsigned int)[hats count];
}

- (void)dealloc
{
    [self.axes release];
    [hats release];
    [buttons release];
    
    [elements release];

    [super dealloc];
}

@end
