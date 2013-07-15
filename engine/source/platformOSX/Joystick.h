//
//  Joystick.h
//  JoystickHIDTest
//
//  Created by John Stringham on 12-05-01.
//  Copyright 2012 We Get Signal. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <IOKit/hid/IOHIDLib.h>

@interface Joystick : NSObject {
    IOHIDDeviceRef  device;
    

@private
    NSArray  *elements;

    NSArray *_axes;
    NSArray *buttons;
    NSArray *hats;
    NSArray *misc;
    
}

@property(readwrite) IOHIDDeviceRef device;

@property(readonly) unsigned int numButtons;
@property(readonly) unsigned int numAxes;
@property(readonly) unsigned int numHats;

@property(nonatomic, retain) NSArray *axes;

- (id)initWithDevice:(IOHIDDeviceRef)theDevice;
- (int)getElementIndex:(IOHIDElementRef)theElement;
- (int)getButtonIndex:(IOHIDElementRef)theElement;
- (int)getAxisIndex:(IOHIDElementRef)theElement;

- (double)getRelativeValueOfAxesIndex:(int)index;

@end
