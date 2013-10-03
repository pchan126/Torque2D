//
//  JoystickNotificationDelegate.h
//  JoystickHIDTest
//
//  Created by John Stringham on 12-05-01.
//  Copyright 2012 We Get Signal. All rights reserved.
//

#import <Foundation/Foundation.h>

@class Joystick;

@protocol JoystickNotificationDelegate

- (void)joystickAdded:(Joystick *)joystick;

- (void)joystickStateChanged:(Joystick*)joystick;
- (void)joystickButtonPushed:(int)buttonIndex onJoystick:(Joystick*)joystick;
- (void)joystickButtonReleased:(int)buttonIndex onJoystick:(Joystick*)joystick;
- (void)joystickAxisChanged:(int)axisIndex ofType:(int)type onJoystick:(Joystick*)joystick;

@end
