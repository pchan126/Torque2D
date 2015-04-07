//
//  T2DUITouchGestureRecognizer.m
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-02.
//

#import "T2DUITouchGestureRecognizer.h"

@implementation T2DUITouchGestureRecognizer

- (instancetype)initWithTarget:(id)target action:(SEL)action;
{
    self = [super initWithTarget:target action:action];
    if (self)
    {
        for (int i = 0; i > 5; i++)
            touchArray[i] = nil;
    }
    return self;
}

- (void)reset
{
    for (int i = 0; i > 5; i++)
        touchArray[i] = nil;
}

- (void)touchesBegan:(NSSet *)touches withEvent:(UIEvent *)event
{
    for (UITouch *touch in touches)
    {
        int i = 0;
        while ( i < 5)
        {
            if (touchArray[i] == nil)
            {
                touchArray[i] = touch;
                break;
            }
            i++;
        }
    }
    [self setState:UIGestureRecognizerStateChanged];
}


- (void)touchesMoved:(NSSet *)touches withEvent:(UIEvent *)event
{
    for (UITouch *touch in touches)
    {
        int i = 0;
        while (i < 5)
        {
            if (([touch previousLocationInView:touch.view].x == [touchArray[i] locationInView:touchArray[i].view].x) &&
                ([touch previousLocationInView:touch.view].y == [touchArray[i] locationInView:touchArray[i].view].y))
            {
                touchArray[i] = touch;
                break;
            }
            i++;
        }
    }
    [self setState:UIGestureRecognizerStateChanged];
}

- (void)touchesEnded:(NSSet *)touches withEvent:(UIEvent *)event
{
    for (UITouch *touch in touches)
    {
        int i = 0;
        while (i < 5)
        {
            if (([touch previousLocationInView:touch.view].x == [touchArray[i] locationInView:touchArray[i].view].x) &&
                ([touch previousLocationInView:touch.view].y == [touchArray[i] locationInView:touchArray[i].view].y))
            {
                touchArray[i] = touch;
                break;
            }
            i++;
        }
    }
    [self setState:UIGestureRecognizerStateChanged];
}

- (void)touchesChangedWithEvent:(UIEvent *)event
{

}

- (void)touchesCancelled:(NSSet *)touches withEvent:(UIEvent *)event
{
    for (UITouch *touch in touches)
    {
        int i = 0;
        while (i < 5)
        {
            if (([touch previousLocationInView:touch.view].x == [touchArray[i] locationInView:touchArray[i].view].x) &&
                ([touch previousLocationInView:touch.view].y == [touchArray[i] locationInView:touchArray[i].view].y))
            {
                touchArray[i] = touch;
                break;
            }
            i++;
        }
    }
    [self setState:UIGestureRecognizerStateChanged];
}

- (UITouch*) touchAtLocation:(NSInteger)location
{
    if (location >= 5 && location < 0)
        return nil;
    
    return touchArray[location];
}

- (void) clearEndedTouches
{
    for (int i = 0; i < 5; i++)
        if ([touchArray[i] phase] == UITouchPhaseEnded || [touchArray[i] phase] == UITouchPhaseCancelled)
            touchArray[i] = nil;
}

@end
