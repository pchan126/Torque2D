//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import "./macWindow.h"
#import "./macCursorController.h"

void MacCursorController::setCursorPosition(S32 x, S32 y)
{
   MacWindow* macWindow = static_cast<MacWindow*>(mOwner);
   macWindow->setCursorPosition(Point2D(x, y));
}

void MacCursorController::getCursorPosition( Point2I &point )
{
   MacWindow* macWindow = static_cast<MacWindow*>(mOwner);
   macWindow->getCursorPosition(point);
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


