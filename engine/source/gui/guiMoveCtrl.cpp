//
// Created by Paul L Jan on 2013-07-29.
//


#include "gui/guiMoveCtrl.h"
#include "2d/core/Vector2.h"

IMPLEMENT_CONOBJECT(GuiMoveCtrl);


static EnumTable::Enums interpolationModeLookup[] =
        {
                { GuiMoveCtrl::LINEAR,   "LINEAR" },
                { GuiMoveCtrl::SIGMOID,  "SIGMOID" },
        };

static EnumTable interpolationModeTable(sizeof(interpolationModeLookup) / sizeof(EnumTable::Enums), &interpolationModeLookup[0]);


GuiMoveCtrl::InterpolationMode GuiMoveCtrl::getInterpolationModeEnum(const char* label)
{
    // Search for Mnemonic.
    for(U32 i = 0; i < (sizeof(interpolationModeLookup) / sizeof(EnumTable::Enums)); i++)
        if( dStricmp(interpolationModeLookup[i].label, label) == 0)
            return((InterpolationMode)interpolationModeLookup[i].index);

    // Warn.
    Con::warnf( "GuiMoveCtrl::getInterpolationModeEnum() - Invalid interpolation mode '%s'.", label );

    return GuiMoveCtrl::INVALID_INTERPOLATION_MODE;
}


GuiMoveCtrl::GuiMoveCtrl() :
    mInterpolationMode(SIGMOID),
    mTransitionTime(2.0f),
    mMoving(false)
{
    // Turn-on Tick Processing.
    setProcessTicks( true );
}

//-----------------------------------------------------------------------------

GuiMoveCtrl::~GuiMoveCtrl()
{
}


bool GuiMoveCtrl::onAdd()
{
   if (!Parent::onAdd())
      return false;
   
   mTargetRect = getBounds();
   return true;
}

F32 GuiMoveCtrl::interpolate( F32 from, F32 to, F32 delta )
{
    // Linear.
    if ( mInterpolationMode == LINEAR )
        return mLerp( from, to, delta );
            // Sigmoid.
    else if ( mInterpolationMode == SIGMOID )
        return mSmoothStep( from, to, delta );
    else
        return from;
}

void GuiMoveCtrl::setTargetPosition( const Point2I& position )
{
    if ( mMoving ) stopMove();
    mTargetRect.point = position;
}

void GuiMoveCtrl::setTargetSize( const Point2I& targetSize )
{
    if ( mMoving ) stopMove();
    mTargetRect.extent = targetSize;
}

void GuiMoveCtrl::setTargetArea( const RectI& targetArea )
{
    if ( mMoving ) stopMove();

    mTargetRect = targetArea;
}

void GuiMoveCtrl::startMove( const F32 interpolationTime )
{
//    // Are we mounted to an object and trying to move?
//    if ( isCameraMounted() )
//    {
//        if ( ( mCameraCurrent.mSourceArea.point  != mCameraTarget.mSourceArea.point ) ||
//                ( mCameraCurrent.mSourceArea.extent != mCameraTarget.mSourceArea.extent ) )
//        {
//            // Yes, so cannot use this command.
//            Con::warnf("SceneWindow::startCameraMove - Cannot use this command when camera is mounted!");
//            return;
//        }
//    }

    // Stop move if we're at target already.
    if (    getBounds().point  == mTargetRect.point &&
            getBounds().extent == mTargetRect.extent  )
    {
        // Reset Camera Move.
        mMoving = false;

        // Return here.
        return;
    }
    else
    {
        // Stop Camera Move ( if any ).
        if ( mMoving ) stopMove();
    }

    // Set Camera Interpolation Time.
    setInterpolationTime( interpolationTime );

    // Zero Camera Time.
    zeroTime();

    // Set Camera Move.
    mMoving = true;

    // Complete camera move if interpolate time is zero.
    if ( mIsZero(mTransitionTime) ) completeMove();
}


void GuiMoveCtrl::stopMove( void )
{
    // Quit if we're not moving.
    if ( !mMoving ) return;

    // Reset Tick Camera Time.
    resetTickTime();

    // Set target to Current.
    mTargetRect = mBounds;

    // Reset Camera Move.
    mMoving = false;
}


//-----------------------------------------------------------------------------

void GuiMoveCtrl::completeMove( void )
{
    // Quit if we're not moving.
    if ( !mMoving ) return;

    // Reset Tick Camera Time.
    resetTickTime();

    // Move straight to target.
    setBounds(mTargetRect);

    // Reset Camera Move.
    mMoving = false;

    // Script callback.
    Con::executef( this, 2, "onMoveToComplete" );
}


void GuiMoveCtrl::interpolateTick(F32 timeDelta) {
    // Are we moving the camera.
    if ( mMoving )
    {
        // Calculate Render Tick Position.
        mRenderTime = (mPreTime * timeDelta) + ((1.0f-timeDelta) * mPostTime);
    }
}


void GuiMoveCtrl::processTick() {

    // Are we moving.
    if ( mMoving )
    {
        // Yes, so add Elapsed Time (scaled appropriately).
        mCurrentTime += Tickable::smTickSec;

        // Update Tick Camera Time.
        updateTickTime();

        // Update Camera.
        updateMove();
    }
}


//-----------------------------------------------------------------------------

void GuiMoveCtrl::updateTickTime( void )
{
    // Store Pre Camera Time.
    mPreTime = mPostTime;

    // Store Current Camera Time.
    mPostTime = mCurrentTime;

    // Render Camera Time is at Pre-Tick Time.
    mRenderTime = mPreTime;
}


void GuiMoveCtrl::updateMove()
{
    // Calculate Normalised Time.
    const F32 normTime = mRenderTime / mTransitionTime;

    // Have we finished the interpolation?
    if ( mGreaterThanOrEqual(normTime, 1.0f) )
    {
        // Yes, so complete camera move.
        completeMove();
        // Finish here.
        return;
    }

    mBounds.point.x    = interpolate( mBounds.point.x, mTargetRect.point.x, normTime );
    mBounds.point.y    = interpolate( mBounds.point.y, mTargetRect.point.y, normTime );
    mBounds.extent.x   = interpolate( mBounds.extent.x, mTargetRect.extent.x, normTime );
    mBounds.extent.y   = interpolate( mBounds.extent.y, mTargetRect.extent.y, normTime );
}

void GuiMoveCtrl::onRender(Point2I offset, const RectI &updateRect)
{
   //render the child controls
   renderChildControls(offset, updateRect);
}


//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, setTargetPosition, void, 3, 4,     "(x , y) - Set the target position.\n"
        "@param X Position along the X axis.\n"
        "@param Y Position along the Y axis.\n"
        "@return No return value.")
{
    if ( argc == 3 )
    {
        Point2I temp;
        temp.x = dAtoi(Utility::mGetStringElement(argv[2],0));
        temp.y = dAtoi(Utility::mGetStringElement(argv[2],1));
        object->setTargetPosition( temp );
        return;
    }

    object->setTargetPosition( Point2I(dAtoi(argv[2]), dAtoi(argv[3])) );
}


//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, getTargetPosition, const char*, 2, 2,  "() Get the target position.\n"
        "@return The target camera position.")
{
    Point2I pos = object->getTargetPosition();
    char* pBuffer = Con::getReturnBuffer(32);
    dSprintf(pBuffer, 32, "%i %i", pos.x, pos.y );
    return pBuffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, setTargetSize, void, 3, 4,    "(width , height) - Set the target camera position.\n"
        "@param width Size along the X axis.\n"
        "@param height Size along the Y axis.\n"
        "@return No return value.")
{
    if ( argc == 3 )
    {
        Point2I temp;
        temp.x = dAtoi(Utility::mGetStringElement(argv[2],0));
        temp.y = dAtoi(Utility::mGetStringElement(argv[2],1));
        object->setTargetSize( temp );
        return;
    }

    object->setTargetSize( Point2I(dAtoi(argv[2]), dAtoi(argv[3])) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, getTargetSize, const char*, 2, 2, "() Get the target camera size.\n"
        "@return The target camera width and height.")
{
    Point2I size = object->getTargetSize();
    char* pBuffer = Con::getReturnBuffer(32);
    dSprintf(pBuffer, 32, "%i %i", size.x, size.y );
    return pBuffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, setTargetArea, void, 3, 6, "(x / y / width / height) - Set the target camera area."
        "@return No return value.")
{
    // Upper left bound.
    Vector2 v1;
    // Lower right bound.
    Vector2 v2;

    // Grab the number of elements in the first two parameters.
    U32 elementCount1 = Utility::mGetStringElementCount(argv[2]);
    U32 elementCount2 = 1;
    if (argc > 3)
        elementCount2 = Utility::mGetStringElementCount(argv[3]);

    // ("x1 y1 x2 y2")
    if ((elementCount1 == 4) && (argc == 3))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[2], 2);
    }

            // ("x1 y1", "x2 y2")
    else if ((elementCount1 == 2) && (elementCount2 == 2) && (argc == 4))
    {
        v1 = Utility::mGetStringElementVector(argv[2]);
        v2 = Utility::mGetStringElementVector(argv[3]);
    }

            // (x1, y1, x2, y2)
    else if (argc == 6)
    {
        v1 = Vector2(dAtof(argv[2]), dAtof(argv[3]));
        v2 = Vector2(dAtof(argv[4]), dAtof(argv[5]));
    }

            // Invalid
    else
    {
        Con::warnf("SceneWindow::setTargetCameraArea() - Invalid number of parameters!");
        return;
    }

    // Calculate Normalised Rectangle.
    Vector2 topLeft( (v1.x <= v2.x) ? v1.x : v2.x, (v1.y <= v2.y) ? v1.y : v2.y );
    Vector2 bottomRight( (v1.x > v2.x) ? v1.x : v2.x, (v1.y > v2.y) ? v1.y : v2.y );

    // Set Target Camera Area.
    object->setTargetArea( RectI(topLeft.x, topLeft.y, bottomRight.x-topLeft.x+1, bottomRight.y-topLeft.y+1) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, getTargetArea, const char*, 2, 2, "() Get the target camera Area.\n"
        "@return The camera area formatted as \"x1 y1 x2 y2\"")
{
    // Fetch Camera Window.
    const RectI cameraWindow = object->getTargetArea();

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(64);

    // Format Buffer.
    dSprintf(pBuffer, 64, "%.5g %.5g %.5g %.5g", cameraWindow.point.x, cameraWindow.point.y, cameraWindow.point.x+cameraWindow.extent.x, cameraWindow.point.y+cameraWindow.extent.y);

    // Return Buffer.
    return pBuffer;
}


//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, setInterpolationTime, void, 3, 3, "(interpolationTime) - Set camera interpolation time."
        "@return No return value")
{
    // Set Camera Interpolation Time.
    object->setInterpolationTime( dAtof(argv[2]) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, setInterpolationMode, void, 3, 3, "(interpolationMode) - Set camera interpolation mode."
        "@return No return value.")
{
    // Set Camera Interpolation Mode.
    object->setInterpolationMode( GuiMoveCtrl::getInterpolationModeEnum(argv[2]) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, startMove, void, 2, 3, "([interpolationTime]) - Start Camera Move."
        "@return No return value.")
{
    F32 interpolationTime;

    // Interpolation Time?
    if ( argc >= 3 )
        interpolationTime = dAtof(argv[2]);
    else
        interpolationTime = object->getInterpolationTime();

    // Start Camera Move.
    object->startMove( interpolationTime );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, stopMove, void, 2, 2, "() Stops current movement"
        "@return No return value.")
{
    // Stop Camera Move.
    object->stopMove();
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiMoveCtrl, completeMove, void, 2, 2, "() Moves directly to target.\n"
        "@return No return value.")
{
    // Complete Camera Move.
    object->completeMove();
}
