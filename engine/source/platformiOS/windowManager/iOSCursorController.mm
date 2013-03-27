//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import "./iOSWindow.h"
#import "./iOSCursorController.h"

// <Mat> just some random number 50, we'll get a proper value later
#define IOS_DOUBLE_CLICK_TIME	( 50 * 60.0f * 1000.0f)


void iOSCursorController::setCursorPosition(S32 x, S32 y)
{
//   iOSWindow* iOSWindow = dynamic_cast<iOSWindow*>(mOwner);
//   if(!iOSWindow || !iOSWindow->isVisible())
//      return;
//      
//   CGPoint pt = { static_cast<CGFloat>(x), static_cast<CGFloat>(y) };
//   CGWarpMouseCursorPosition(pt);
//   
//   iOSWindow->_skipAnotherMouseEvent();
}

void iOSCursorController::getCursorPosition( Point2I &point )
{
//   NSPoint pos = [NSEvent mouseLocation];
//   point.x = pos.x;
//   point.y = pos.y;
//   
//   //what does this do?? comment??
//   
//   iOSWindow* iOSWindow = static_cast<iOSWindow*>(mOwner);
//   
//   CGRect bounds = iOSWindow->getDisplayBounds();
//   CGRect mainbounds = iOSWindow->getMainDisplayBounds();
//   F32 offsetY = mainbounds.size.height - (bounds.size.height + bounds.origin.y);
//   point.y = bounds.size.height + offsetY - point.y;
}

void iOSCursorController::setCursorVisible(bool visible)
{
//   visible ? [NSCursor unhide] : [NSCursor hide];
}

bool iOSCursorController::isCursorVisible()
{
//   return CGCursorIsVisible();
}

// a repository of custom cursors.
//@interface TorqueCursors : NSObject { }
//+(NSCursor*)resizeAll;
//+(NSCursor*)resizeNWSE;
//+(NSCursor*)resizeNESW;
//@end
//@implementation TorqueCursors
//+(NSCursor*)resizeAll
//{
//   static NSCursor* cur = nil;
//   if(!cur)
//      cur = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"resizeAll"] hotSpot:NSMakePoint(8, 8)];
//   return cur;
//}
//+(NSCursor*)resizeNWSE
//{
//   static NSCursor* cur = nil;
//   if(!cur)
//      cur = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"resizeNWSE"] hotSpot:NSMakePoint(8, 8)];
//   return cur;
//}
//+(NSCursor*)resizeNESW
//{
//   static NSCursor* cur = nil;
//   if(!cur)
//      cur = [[NSCursor alloc] initWithImage:[NSImage imageNamed:@"resizeNESW"] hotSpot:NSMakePoint(8, 8)];
//   return cur;
//}
//@end

void iOSCursorController::setCursorShape(U32 cursorID)
{
//   NSCursor *cur;
//   switch(cursorID)
//   {
//      case PlatformCursorController::curArrow:
//         [[NSCursor arrowCursor] set];
//         break;
//      case PlatformCursorController::curWait:
//         // hack: black-sheep carbon call
////         [[NSCursor ]]
////         SetThemeCursor(kThemeWatchCursor);
//         break;
//      case PlatformCursorController::curPlus:
//         [[NSCursor crosshairCursor] set];
//         break;
//      case PlatformCursorController::curResizeVert:
//         [[NSCursor resizeLeftRightCursor] set];
//         break;
//      case PlatformCursorController::curIBeam:
//         [[NSCursor IBeamCursor] set];
//         break;
//      case PlatformCursorController::curResizeAll:
//         cur = [TorqueCursors resizeAll];
//         [cur set];
//         break;
//      case PlatformCursorController::curResizeNESW:
//         [[TorqueCursors resizeNESW] set];
//         break;
//      case PlatformCursorController::curResizeNWSE:
//         [[TorqueCursors resizeNWSE] set];
//         break;
//      case PlatformCursorController::curResizeHorz:
//         [[NSCursor resizeUpDownCursor] set];
//      break;
//   }
}

void iOSCursorController::setCursorShape( const UTF8 *fileName, bool reload )
{
   //TODO: this is untested code
   
//   NSString* strFileName = [ NSString stringWithUTF8String: fileName ];
//   
//   // Load image file.
//   
//   NSImage* image = [ NSImage alloc ];
//   if( [ image initWithContentsOfFile: strFileName ] == nil )
//      return;
//
//   // Allocate cursor.
//   
//   NSCursor* cursor = [ NSCursor alloc ];
//   [ cursor initWithImage: image hotSpot: NSMakePoint( 0.5, 0.5 ) ];
}

////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void iOSCursorController::pushCursor(S32 cursorID)
//{
//}
//
////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void iOSCursorController::popCursor()
//{
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void iOSCursorController::refreshCursor()
//{
//}

//------------------------------------------------------------------------------
U32 iOSCursorController::getDoubleClickTime()
{
//    // Get system specified double click time
//    NSTimeInterval doubleInterval = [NSEvent doubleClickInterval];
//    
//    return doubleInterval * 1000;
    return IOS_DOUBLE_CLICK_TIME;
}

//------------------------------------------------------------------------------
S32 iOSCursorController::getDoubleClickWidth()
{
    // this is an arbitrary value.
    return 10;
}

//------------------------------------------------------------------------------
S32 iOSCursorController::getDoubleClickHeight()
{
    return getDoubleClickWidth();
}

////------------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void iOSCursorController::setCursorPos(S32 x, S32 y)
//{
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void iOSCursorController::setCursorState(bool on)
//{
//    
//}
//
////-----------------------------------------------------------------------------
//// Not yet implemented. Will resolve in the next platform update
//void iOSCursorController::setCursorShape(U32 cursorID)
//{
//    
//}

