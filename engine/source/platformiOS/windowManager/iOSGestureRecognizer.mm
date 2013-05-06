//
//  iOSGestureRecognizer.m
//  Torque2D
//
//  Created by Paul L Jan on 2013-04-29.
//

#import "iOSGestureRecognizer.h"
#import "platformiOS/windowManager/iOSWindow.h"
#include "gui/guiCanvas.h"
#include "gui/guiCanvas_ScriptBinding.h"

#define MAX_TOUCH_EVENTS 5

@implementation iOSGestureRecognizer
@synthesize tapGestureRecognizer;
@synthesize panGestureRecognizer;
@synthesize touchGestureRecognizer;
@synthesize rotationGestureRecognizer;
@synthesize swipeGestureRecognizer;
@synthesize longPressGestureRecognizer;
@synthesize pinchGestureRecognizer;
@synthesize window;

- (id)initWithT2DWindow:(iOSWindow *)in_window
{
    self.window = in_window;

    iOSGestureRecognizer* gestureRecognizer = self;
    
    T2DUITouchGestureRecognizer* newTouchGestureRecognizer = [[T2DUITouchGestureRecognizer alloc] initWithTarget:self action:@selector(handleTouch:)];
    
    gestureRecognizer.touchGestureRecognizer = newTouchGestureRecognizer;
    in_window->addGestureRecognizer( gestureRecognizer.touchGestureRecognizer );
    
    return self;
}

- (void) handleTaps:(UITapGestureRecognizer*)paramSender {
    NSUInteger touchCounter = 0;
    
    switch (paramSender.state)
    {
        case UIGestureRecognizerStateEnded:
             for (touchCounter = 0; touchCounter < paramSender.numberOfTouches; touchCounter++)
             {
                 CGPoint touchPoint = [paramSender locationOfTouch:touchCounter inView:paramSender.view];
                 window->tapEvent.trigger(window->getWindowId(), 0, touchPoint.x, touchPoint.y, SI_BREAK, touchCounter );
             }
            break;
            
        default:
            break;
    }
}

- (void) handleSwipe:(UISwipeGestureRecognizer*)paramSender {
    window->swipeEvent.trigger(window->getWindowId(), 0, paramSender.direction );
}

- (void) handleRotation:(UIRotationGestureRecognizer*)paramSender {
    NSUInteger touchCounter = 0;
    static CGPoint center;
    
    switch (paramSender.state)
    {
        case UIGestureRecognizerStateBegan:
        {
            center.x = 0;
            center.y = 0;
            for (touchCounter = 0; touchCounter < paramSender.numberOfTouches; touchCounter++)
            {
                CGPoint touchPoint = [paramSender locationOfTouch:touchCounter inView:paramSender.view];
                center.x += touchPoint.x;
                center.y += touchPoint.y;
            }
            center.x /= paramSender.numberOfTouches;
            center.y /= paramSender.numberOfTouches;
            break;
        }
        case UIGestureRecognizerStateChanged:
        {
            CGFloat rotation = [paramSender rotation];
            CGFloat velocity = [paramSender velocity];
            window->rotationEvent.trigger(window->getWindowId(), 0, center.x, center.y, rotation, velocity );
            break;
        }
        default:
        {
            break;
        }
    }
}

- (void) handlePan:(UIPanGestureRecognizer*)paramSender {
    NSUInteger touchCounter = 0;
    switch (paramSender.state)
    {
        case UIGestureRecognizerStateChanged:
            for (touchCounter = 0; touchCounter < paramSender.numberOfTouches; touchCounter++)
            {
                CGPoint touchPoint = [paramSender locationOfTouch:touchCounter inView:paramSender.view];
                CGPoint translation = [paramSender translationInView:paramSender.view];
                CGPoint velocity = [paramSender velocityInView:paramSender.view];
                window->tapEvent.trigger(window->getWindowId(), 0, touchPoint.x, touchPoint.y, SI_MOVE, touchCounter );
                window->panEvent.trigger(window->getWindowId(), 0, translation.x, translation.y, velocity.x, velocity.y, SI_MOVE);
            }
            break;
            
        default:
            break;
    }
}

- (void) handleLongPress:(UILongPressGestureRecognizer*)paramSender {
    NSUInteger touchCounter = 0;
    switch (paramSender.state)
    {
        case UIGestureRecognizerStateEnded:
            break;

        default:
            break;
    }
}

- (void) handlePinch:(UIPinchGestureRecognizer*)paramSender {
    NSUInteger touchCounter = 0;
    static CGPoint center;

    switch (paramSender.state)
    {
        case UIGestureRecognizerStateChanged:
        {
            CGFloat scale = [paramSender scale];
            CGFloat velocity = [paramSender velocity];
            window->pinchEvent.trigger(window->getWindowId(), 0, scale, velocity, SI_MOVE);
            break;
        }
        default:
        {
            break;
        }
    }
}

- (void) handleTouch:(T2DUITouchGestureRecognizer*)paramSender {

   UIView *view = (UIView*)window->view;
   for (int i = 0; i < 5; i++)
   {
       UITouch* touch = [paramSender touchAtLocation:i];
       if (touch != nil)
       {
           CGPoint point = [touch locationInView:view];
            U32 touchType = 0;
            switch ( touch.phase )
            {
                case UITouchPhaseBegan:
                    touchType = SI_MAKE;
                    break;
                case UITouchPhaseMoved:
                    touchType = SI_MOVE;
                    break;
                case UITouchPhaseEnded:
                    touchType = SI_BREAK;
                    point = [touch previousLocationInView:view];
                    break;
                case UITouchPhaseCancelled:
                    touchType = SI_BREAK;
                    break;
                default:
                    break;
            }
            if (touchType != 0) {
                window->touchEvent.trigger(window->getWindowId(), 0, point.x, point.y, i, touchType, 1);
            }
        }
   }
    [paramSender clearEndedTouches];
}

ConsoleMethod(GuiCanvas, startTapRecognizer, void, 2, 2, "() Use the startTapRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    
    UITapGestureRecognizer* newTapGestureRecognizer = [[UITapGestureRecognizer alloc] initWithTarget:gestureRecognizer action:@selector(handleTaps:)];
    
    gestureRecognizer.tapGestureRecognizer = newTapGestureRecognizer;
    window->addGestureRecognizer( gestureRecognizer.tapGestureRecognizer );
}

ConsoleMethod(GuiCanvas, removeTapRecognizer, void, 2, 2, "() Use the removeTapRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    window->removeGestureRecognizer( gestureRecognizer.tapGestureRecognizer );
    gestureRecognizer.tapGestureRecognizer = nil;
}

ConsoleMethod(GuiCanvas, startPinchRecognizer, void, 2, 2, "() Use the startPinchRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    
    UIPinchGestureRecognizer* newPinchGestureRecognizer = [[UIPinchGestureRecognizer alloc] initWithTarget:gestureRecognizer action:@selector(handlePinch:)];
    
    
    
    gestureRecognizer.pinchGestureRecognizer = newPinchGestureRecognizer;
    window->addGestureRecognizer( gestureRecognizer.pinchGestureRecognizer );
}

ConsoleMethod(GuiCanvas, removePinchRecognizer, void, 2, 2, "() Use the removePinchRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    window->removeGestureRecognizer( gestureRecognizer.pinchGestureRecognizer );
    gestureRecognizer.pinchGestureRecognizer = nil;
}

ConsoleMethod(GuiCanvas, startRotationRecognizer, void, 2, 2, "() Use the startRotationRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    
    UIRotationGestureRecognizer* newRotationGestureRecognizer = [[UIRotationGestureRecognizer alloc] initWithTarget:gestureRecognizer action:@selector(handleRotation:)];
    
    gestureRecognizer.rotationGestureRecognizer = newRotationGestureRecognizer;
    window->addGestureRecognizer( gestureRecognizer.rotationGestureRecognizer );
}

ConsoleMethod(GuiCanvas, removeRotationRecognizer, void, 2, 2, "() Use the removeRotationRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    window->removeGestureRecognizer( gestureRecognizer.rotationGestureRecognizer );
    gestureRecognizer.rotationGestureRecognizer = nil;
}


ConsoleMethod(GuiCanvas, startSwipeRecognizer, void, 2, 2, "() Use the startSwipeRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    
    UISwipeGestureRecognizer* newSwipeGestureRecognizer = [[UISwipeGestureRecognizer alloc] initWithTarget:gestureRecognizer action:@selector(handleSwipe:)];
    
    gestureRecognizer.swipeGestureRecognizer = newSwipeGestureRecognizer;
    window->addGestureRecognizer( gestureRecognizer.swipeGestureRecognizer );
}

ConsoleMethod(GuiCanvas, removeSwipeRecognizer, void, 2, 2, "() Use the removeSwipeRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    window->removeGestureRecognizer( gestureRecognizer.swipeGestureRecognizer );
    gestureRecognizer.swipeGestureRecognizer = nil;
}


ConsoleMethod(GuiCanvas, startPanRecognizer, void, 2, 2, "() Use the startPanRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    
    UIPanGestureRecognizer* newPanGestureRecognizer = [[UIPanGestureRecognizer alloc] initWithTarget:gestureRecognizer action:@selector(handlePan:)];
    
    gestureRecognizer.panGestureRecognizer = newPanGestureRecognizer;
    window->addGestureRecognizer( gestureRecognizer.panGestureRecognizer );
}

ConsoleMethod(GuiCanvas, removePanRecognizer, void, 2, 2, "() Use the removePanRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    window->removeGestureRecognizer( gestureRecognizer.panGestureRecognizer );
    gestureRecognizer.panGestureRecognizer = nil;
}

ConsoleMethod(GuiCanvas, startLongPressRecognizer, void, 2, 2, "() Use the startLongPressRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    
    UILongPressGestureRecognizer* newLongPressGestureRecognizer = [[UILongPressGestureRecognizer alloc] initWithTarget:gestureRecognizer action:@selector(handlePan:)];
    
    gestureRecognizer.longPressGestureRecognizer = newLongPressGestureRecognizer;
    window->addGestureRecognizer( gestureRecognizer.longPressGestureRecognizer );
}

ConsoleMethod(GuiCanvas, removeLongPressRecognizer, void, 2, 2, "() Use the removeLongPressRecognizer.\n"
              "@return No return value")
{
    iOSWindow* window = dynamic_cast<iOSWindow*>(object->getPlatformWindow());
    if (!window)
    {
        Con::errorf("Not on iOS platform or missing window");
        return;
    }
    iOSGestureRecognizer* gestureRecognizer = window->gestureRecognizer;
    window->removeGestureRecognizer( gestureRecognizer.longPressGestureRecognizer );
    gestureRecognizer.longPressGestureRecognizer = nil;
}


@end
