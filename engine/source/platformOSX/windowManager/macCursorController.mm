//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import "./macWindow.h"
#import "./macCursorController.h"

void MacCursorController::setCursorPosition(S32 x, S32 y)
{
   MacWindow* macWindow = dynamic_cast<MacWindow*>(mOwner);
   if(!macWindow || !macWindow->isVisible())
      return;
      
   CGPoint pt = { static_cast<CGFloat>(x), static_cast<CGFloat>(y) };
   CGWarpMouseCursorPosition(pt);
   
   macWindow->_skipAnotherMouseEvent();
}

void MacCursorController::getCursorPosition( Point2I &point )
{
   NSPoint pos = [NSEvent mouseLocation];
   point.x = pos.x;
   point.y = pos.y;
   
   //what does this do?? comment??
   
   MacWindow* macWindow = static_cast<MacWindow*>(mOwner);
   
   CGRect bounds = macWindow->getDisplayBounds();
   CGRect mainbounds = macWindow->getMainDisplayBounds();
   F32 offsetY = mainbounds.size.height - (bounds.size.height + bounds.origin.y);
   point.y = bounds.size.height + offsetY - point.y;
}

void MacCursorController::setCursorVisible(bool visible)
{
   visible ? [NSCursor unhide] : [NSCursor hide];
}

bool MacCursorController::isCursorVisible()
{
   return CGCursorIsVisible();
}

// a repository of custom cursors.
@interface TorqueCursors : NSObject { }
+(NSCursor*)resizeAll;
+(NSCursor*)resizeNWSE;
+(NSCursor*)resizeNESW;
@end
@implementation TorqueCursors
+(NSCursor*)resizeAll
{
   static NSCursor* cur = nil;
   if(!cur)
      cur = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"resizeAll"] hotSpot:NSMakePoint(8, 8)];
   return cur;
}
+(NSCursor*)resizeNWSE
{
   static NSCursor* cur = nil;
   if(!cur)
      cur = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"resizeNWSE"] hotSpot:NSMakePoint(8, 8)];
   return cur;
}
+(NSCursor*)resizeNESW
{
   static NSCursor* cur = nil;
   if(!cur)
      cur = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"resizeNESW"] hotSpot:NSMakePoint(8, 8)];
   return cur;
}
@end

void MacCursorController::setCursorShape(U32 cursorID)
{
   NSCursor *cur;
   switch(cursorID)
   {
      case PlatformCursorController::curArrow:
         [[NSCursor arrowCursor] set];
         break;
      case PlatformCursorController::curWait:
         // hack: black-sheep carbon call
//         [[NSCursor ]]
//         SetThemeCursor(kThemeWatchCursor);
         break;
      case PlatformCursorController::curPlus:
         [[NSCursor crosshairCursor] set];
         break;
      case PlatformCursorController::curResizeVert:
         [[NSCursor resizeLeftRightCursor] set];
         break;
      case PlatformCursorController::curIBeam:
         [[NSCursor IBeamCursor] set];
         break;
      case PlatformCursorController::curResizeAll:
         cur = [TorqueCursors resizeAll];
         [cur set];
         break;
      case PlatformCursorController::curResizeNESW:
         [[TorqueCursors resizeNESW] set];
         break;
      case PlatformCursorController::curResizeNWSE:
         [[TorqueCursors resizeNWSE] set];
         break;
      case PlatformCursorController::curResizeHorz:
         [[NSCursor resizeUpDownCursor] set];
      break;
   }
}

void MacCursorController::setCursorShape( const UTF8 *fileName, bool reload )
{
   //TODO: this is untested code
   
   NSString* strFileName = [ NSString stringWithUTF8String: fileName ];
   
   // Load image file.
   
   NSImage* image = [ NSImage alloc ];
   if( [ image initWithContentsOfFile: strFileName ] == nil )
      return;

   // Allocate cursor.
   
   NSCursor* cursor = [ NSCursor alloc ];
   [ cursor initWithImage: image hotSpot: NSMakePoint( 0.5, 0.5 ) ];
}

////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void MacCursorController::pushCursor(S32 cursorID)
//{
//}
//
////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void MacCursorController::popCursor()
//{
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void MacCursorController::refreshCursor()
//{
//}

//------------------------------------------------------------------------------
U32 MacCursorController::getDoubleClickTime()
{
    // Get system specified double click time
    NSTimeInterval doubleInterval = [NSEvent doubleClickInterval];
    
    return doubleInterval * 1000;
}

//------------------------------------------------------------------------------
S32 MacCursorController::getDoubleClickWidth()
{
    // this is an arbitrary value.
    return 10;
}

//------------------------------------------------------------------------------
S32 MacCursorController::getDoubleClickHeight()
{
    return getDoubleClickWidth();
}

////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void MacCursorController::setCursorPos(S32 x, S32 y)
//{
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void MacCursorController::setCursorState(bool on)
//{
//    
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void MacCursorController::setCursorShape(U32 cursorID)
//{
//    
//}

