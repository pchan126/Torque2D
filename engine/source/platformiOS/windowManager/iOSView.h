//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#ifndef _IOSVIEW_H_
#define _IOSVIEW_H_

#import <GLKit/GLKit.h>
#import "./iOSWindow.h"

/// GGMacView handles displaying content and responding to user input.
@interface iOSView : GLKView
{
   iOSWindow* mTorqueWindow;
   U32 mLastMods;
   bool mHandledAsCharEvent;
}
- (void)setTorqueWindow:(iOSWindow*)theWindow;
- (iOSWindow*)torqueWindow;

///// @name Inherited Mouse Input methods
///// @{
//- (void)mouseDown:(NSEvent *)theEvent;
//- (void)rightMouseDown:(NSEvent *)theEvent;
//- (void)mouseDragged:(NSEvent *)theEvent;
//- (void)rightMouseDragged:(NSEvent *)theEvent;
//- (void)mouseUp:(NSEvent *)theEvent;
//- (void)rightMouseUp:(NSEvent *)theEvent;
//- (void)mouseMoved:(NSEvent *)theEvent;
//- (void)scrollWheel:(NSEvent *)theEvent;
///// @}
//
///// @name Inherited Keyboard Input methods
///// @{
//- (void)keyDown:(NSEvent *)theEvent;
//- (void)keyUp:(NSEvent *)theEvent;
///// @}
//
///// @name Keyboard Input Common Code
///// @{
//- (void)rawKeyUpDown:(NSEvent *)theEvent keyDown:(BOOL)isKeyDown;
///// @}
//
///// @name Mouse Input Common Code
///// @{
//- (void)mouseUpDown:(NSEvent *)theEvent mouseDown:(BOOL)isMouseDown;
//- (void)mouseMotion:(NSEvent *)theEvent;
///// @}

- (BOOL)acceptsFirstResponder;
- (BOOL)becomeFirstResponder;
- (BOOL)resignFirstResponder;


@end

#endif
