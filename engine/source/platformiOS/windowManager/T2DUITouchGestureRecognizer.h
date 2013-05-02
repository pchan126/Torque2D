//
//  T2DUITouchGestureRecognizer.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-05-02.
//

#import <UIKit/UIKit.h>
#import <UIKit/UIGestureRecognizerSubclass.h>

@interface T2DUITouchGestureRecognizer : UIGestureRecognizer
{
    UITouch *touchArray[5];
}

- (UITouch*) touchAtLocation:(NSInteger)location;
- (void) clearEndedTouches;
@end
