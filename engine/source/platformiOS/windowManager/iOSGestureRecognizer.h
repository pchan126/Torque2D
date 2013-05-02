//
//  iOSGestureRecognizer.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-04-29.
//  Copyright (c) 2013 Michael Perry. All rights reserved.
//

#import <Foundation/Foundation.h>
#import <UIKit/UIKit.h>
#import "platformiOS/windowManager/iOSWindow.h"
#import "platformiOS/windowManager/T2DUITouchGestureRecognizer.h"

@interface iOSGestureRecognizer : NSObject {
    @public
    UITapGestureRecognizer *tapGestureRecognizer;
    UISwipeGestureRecognizer *swipeGestureRecognizer;
    UIRotationGestureRecognizer *rotationGestureRecognizer;
    UIPanGestureRecognizer *panGestureRecognizer;
    UILongPressGestureRecognizer *longPressGestureRecognizer;
    UIPinchGestureRecognizer *pinchGestureRecognizer;
    T2DUITouchGestureRecognizer *touchGestureRecognizer;
    iOSWindow *window;
    
}

@property (nonatomic, strong) UITapGestureRecognizer *tapGestureRecognizer;
@property (nonatomic, strong) UISwipeGestureRecognizer *swipeGestureRecognizer;
@property (nonatomic, strong) UIRotationGestureRecognizer *rotationGestureRecognizer;
@property (nonatomic, strong) UIPanGestureRecognizer *panGestureRecognizer;
@property (nonatomic, strong) UILongPressGestureRecognizer *longPressGestureRecognizer;
@property (nonatomic, strong) UIPinchGestureRecognizer *pinchGestureRecognizer;
@property (nonatomic, strong) T2DUITouchGestureRecognizer *touchGestureRecognizer;
@property (nonatomic) iOSWindow *window;

- (id)initWithT2DWindow:(iOSWindow *)window;
- (void) handleTaps:(UITapGestureRecognizer*)paramSender;
- (void) handleSwipe:(UISwipeGestureRecognizer*)paramSender;
- (void) handleRotation:(UIRotationGestureRecognizer*)paramSender;
- (void) handlePan:(UIPanGestureRecognizer*)paramSender;
- (void) handleLongPress:(UILongPressGestureRecognizer*)paramSender;
- (void) handlePinch:(UIPinchGestureRecognizer*)paramSender;
- (void) handleTouch:(T2DUITouchGestureRecognizer*)paramSender;

@end
