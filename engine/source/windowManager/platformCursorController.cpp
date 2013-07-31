//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#include "platform/platform.h"
#include "./platformCursorController.h"

void PlatformCursorController::pushCursor( S32 cursorID )
{
   // Place the new cursor shape onto the stack
   mCursors.increment();

   CursorShape &shape = mCursors.back();
   shape.mCursorType  = CursorShape::TYPE_RESOURCE;
   shape.mCursorID    = cursorID;

   // Now Change the Cursor Shape.
   setCursorShape( shape.mCursorID );
}

void PlatformCursorController::pushCursor( const UTF8 *fileName )
{
   // Place the new cursor shape onto the stack
   mCursors.increment();

   // Store the Details.
   CursorShape &shape = mCursors.back();
   shape.mCursorType  = CursorShape::TYPE_FILE;
   shape.mCursorFile  = String::ToString( "%s", fileName );

   // Now Change the Cursor Shape.
   setCursorShape( shape.mCursorFile.c_str(), true );
}

void PlatformCursorController::popCursor()
{
   // Before poping the stack, make sure we're not trying to remove the last cursor shape
   if ( mCursors.size() <= 1 )
   {
      return;
   }

   // Clear the Last Cursor.
   mCursors.pop_back();

   // Now Change the Cursor Shape.
   setCursorShape( mCursors.back(), true );
}

void PlatformCursorController::refreshCursor()
{
   // Refresh the Cursor Shape.
   setCursorShape( mCursors.back(), false );
}

void PlatformCursorController::setCursorShape( const CursorShape &shape, bool reload )
{
    switch( shape.mCursorType )
    {
        case CursorShape::TYPE_RESOURCE :
            {

                // Set Resource.
                setCursorShape( shape.mCursorID );

            } break;

        case CursorShape::TYPE_FILE :
            {

                // Set File.
                setCursorShape( shape.mCursorFile.c_str(), reload );

            } break;
    }
}
