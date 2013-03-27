//-----------------------------------------------------------------------------
// Copyright (c) 2013 GarageGames, LLC
//
// Permission is hereby granted, free of charge, to any person obtaining a copy
// of this software and associated documentation files (the "Software"), to
// deal in the Software without restriction, including without limitation the
// rights to use, copy, modify, merge, publish, distribute, sublicense, and/or
// sell copies of the Software, and to permit persons to whom the Software is
// furnished to do so, subject to the following conditions:
//
// The above copyright notice and this permission notice shall be included in
// all copies or substantial portions of the Software.
//
// THE SOFTWARE IS PROVIDED "AS IS", WITHOUT WARRANTY OF ANY KIND, EXPRESS OR
// IMPLIED, INCLUDING BUT NOT LIMITED TO THE WARRANTIES OF MERCHANTABILITY,
// FITNESS FOR A PARTICULAR PURPOSE AND NONINFRINGEMENT. IN NO EVENT SHALL THE
// AUTHORS OR COPYRIGHT HOLDERS BE LIABLE FOR ANY CLAIM, DAMAGES OR OTHER
// LIABILITY, WHETHER IN AN ACTION OF CONTRACT, TORT OR OTHERWISE, ARISING
// FROM, OUT OF OR IN CONNECTION WITH THE SOFTWARE OR THE USE OR OTHER DEALINGS
// IN THE SOFTWARE.
//-----------------------------------------------------------------------------

#include "torqueConfig.h"
#include "console/consoleInternal.h"
#include "debug/profiler.h"
#include "platform/event.h"
#include "platform/platform.h"
#include "gui/guiTypes.h"
#include "gui/guiControl.h"
#include "gui/guiCanvas.h"
#include "game/gameInterface.h"

#include "graphics/gfxInit.h"

IMPLEMENT_CONOBJECT(GuiCanvas);

GuiCanvas *Canvas = NULL;

ConsoleMethod( GuiCanvas, getContent, S32, 2, 2, "() Use the getContent method to get the ID of the control which is being used as the current canvas content.\n"
                                                                "@return Returns the ID of the current canvas content (a control), or 0 meaning the canvas is empty")
{
   GuiControl *ctrl = object->getContentControl();
   if(ctrl)
      return ctrl->getId();
   return -1;
}

ConsoleMethod( GuiCanvas, setContent, void, 3, 3, "( handle ) Use the setContent method to set the control identified by handle as the current canvas content.\n"
                                                                "@param handle The numeric ID or name of the control to be made the canvas contents.\n"
                                                                "@return No return value")
{
   GuiControl *gui = NULL;
   if(argv[2][0])
   {
      if (!Sim::findObject(argv[2], gui))
      {
         Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
         return;
      }
   }

   //set the new content control
   Canvas->setContentControl(gui);
}

ConsoleMethod( GuiCanvas, pushDialog, void, 3, 4, "( handle [ , layer ] ) Use the pushDialog method to open a dialog on a specific canvas layer, or in the same layer the last openned dialog. Newly placed dialogs placed in a layer with another dialog(s) will overlap the prior dialog(s).\n"
                                                                "@param handle The numeric ID or name of the dialog to be opened.\n"
                                                                "@param layer A integer value in the range [ 0 , inf ) specifying the canvas layer to place the dialog in.\n"
                                                                "@return No return value")
{
   GuiControl *gui;

   if (!	Sim::findObject(argv[2], gui))
   {
      Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
      return;
   }

   //find the layer
   S32 layer = 0;
   if (argc == 4)
      layer = dAtoi(argv[3]);

   //set the new content control
   Canvas->pushDialogControl(gui, layer);
}

ConsoleMethod( GuiCanvas, popDialog, void, 2, 3, "( handle ) Use the popDialog method to remove a currently showing dialog. If no handle is provided, the top most dialog is popped.\n"
                                                                "@param handle The ID or a previously pushed dialog.\n"
                                                                "@return No return value.\n"
                                                                "@sa pushDialog, popLayer")
{
   // Must initialize this to NULL to avoid crash on the if (gui) statement further down [KNM | 07/28/11 | ITGB-120]
   //GuiControl * gui;
   GuiControl * gui = NULL;
   if (argc == 3)
   {
      if (!Sim::findObject(argv[2], gui))
      {
         Con::printf("%s(): Invalid control: %s", argv[0], argv[2]);
         return;
      }
   }

   if (gui)
      Canvas->popDialogControl(gui);
   else
      Canvas->popDialogControl();
}

ConsoleMethod( GuiCanvas, popLayer, void, 2, 3, "( layer ) Use the popLayer method to remove (close) all dialogs in the specified canvas �layer�.\n"
                                                                "@param layer A integer value in the range [ 0 , inf ) specifying the canvas layer to clear.\n"
                                                                "@return No return value.\n"
                                                                "@sa pushDialog, popDialog")
{
   S32 layer = 0;
   if (argc == 3)
      layer = dAtoi(argv[2]);

   Canvas->popDialogControl(layer);
}

ConsoleMethod(GuiCanvas, cursorOn, void, 2, 2, "() Use the cursorOn method to enable the cursor.\n"
                                                                "@return No return value")
{
   Canvas->setCursorON(true);
}

ConsoleMethod(GuiCanvas, cursorOff, void, 2, 2, "() Use the cursorOff method to disable the cursor.\n"
                                                                "@return No return value")
{
   Canvas->setCursorON(false);
}

ConsoleMethod( GuiCanvas, setCursor, void, 3, 3, "( cursorHandle ) Use the setCursor method to select the current cursor.\n"
                                                                "@param cursorHandle The ID of a previously defined GuiCursor object.\n"
                                                                "@return No return value")
{
   GuiCursor *curs = NULL;
   if(argv[2][0])
   {
      if(!Sim::findObject(argv[2], curs))
      {
         Con::printf("%s is not a valid cursor.", argv[2]);
         return;
      }
   }
   Canvas->setCursor(curs);
}

ConsoleMethod( GuiCanvas, renderFront, void, 3, 3, "(bool enable)")
{
   Canvas->setRenderFront(dAtob(argv[2]));
}

ConsoleMethod( GuiCanvas, showCursor, void, 2, 2, "")
{
   Canvas->showCursor(true);
}

ConsoleMethod( GuiCanvas, hideCursor, void, 2, 2, "")
{
   Canvas->showCursor(false);
}

ConsoleMethod( GuiCanvas, isCursorOn, bool, 2, 2, "")
{
   return Canvas->isCursorON();
}

ConsoleMethod( GuiCanvas, setDoubleClickDelay, void, 3, 3, "")
{
   Canvas->setDoubleClickTime(dAtoi(argv[2]));
}

ConsoleMethod( GuiCanvas, setDoubleClickMoveBuffer, void, 4, 4, "")
{
   Canvas->setDoubleClickWidth(dAtoi(argv[2]));
   Canvas->setDoubleClickHeight(dAtoi(argv[3]));
}

ConsoleMethod( GuiCanvas, repaint, void, 2, 2, "() Use the repaint method to force the canvas to redraw all elements.\n"
                                                                "@return No return value")
{
   Canvas->paint();
}

ConsoleMethod( GuiCanvas, reset, void, 2, 2, "() Use the reset method to reset the current canvas update region.\n"
                                                                "@return No return value")
{
   Canvas->resetUpdateRegions();
}

ConsoleMethod( GuiCanvas, getCursorPos, const char*, 2, 2, "() Use the getCursorPos method to retrieve the current position of the mouse pointer.\n"
                                                                "@return Returns a vector containing the �x y� coordinates of the cursor in the canvas")
{
   Point2I pos = Canvas->getCursorPos();
   char * ret = Con::getReturnBuffer(32);
   dSprintf(ret, 32, "%d %d", pos.x, pos.y);
   return(ret);
}

ConsoleMethod( GuiCanvas, setCursorPos, void, 3, 4, "( ) Use the setCursorPos method to set the position of the cursor in the cavas.\n"
                                                                "@param position An \"x y\" position vector specifying the new location of the cursor.\n"
                                                                "@return No return value")
{
   Point2I pos(0,0);

   if(argc == 4)
      pos.set(dAtoi(argv[2]), dAtoi(argv[3]));
   else
      dSscanf(argv[2], "%d %d", &pos.x, &pos.y);

   Canvas->setCursorPos(pos);
}

ConsoleMethod( GuiCanvas, getMouseControl, S32, 2, 2, "Gets the gui control under the mouse.")
{
   GuiControl* control = object->getMouseControl();
   if (control)
      return control->getId();
   
   return NULL;
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiCanvas, setBackgroundColor, void, 3, 6,    "(float red, float green, float blue, [float alpha = 1.0]) - Sets the background color for the canvas."
                                                            "@param red The red value.\n"
                                                            "@param green The green value.\n"
                                                            "@param blue The blue value.\n"
                                                            "@param alpha The alpha value.\n"
                                                            "@return No return Value.")
{
    // The colors.
    F32 red;
    F32 green;
    F32 blue;
    F32 alpha = 1.0f;

    // Space separated.
    if (argc == 3)
    {
        // Grab the element count.
        const U32 elementCount = Utility::mGetStringElementCount(argv[2]);

        // Has a single argument been specified?
        if ( elementCount == 1 )
        {
            object->setDataField( StringTable->insert("BackgroundColor"), NULL, argv[2] );
            return;
        }

        // ("R G B [A]")
        if ((elementCount == 3) || (elementCount == 4))
        {
            // Extract the color.
            red   = dAtof(Utility::mGetStringElement(argv[2], 0));
            green = dAtof(Utility::mGetStringElement(argv[2], 1));
            blue  = dAtof(Utility::mGetStringElement(argv[2], 2));

            // Grab the alpha if it's there.
            if (elementCount > 3)
                alpha = dAtof(Utility::mGetStringElement(argv[2], 3));
        }

        // Invalid.
        else
        {
            Con::warnf("GuiCanvas::setBackgroundColor() - Invalid Number of parameters!");
            return;
        }
    }

    // (R, G, B)
    else if (argc >= 5)
    {
        red   = dAtof(argv[2]);
        green = dAtof(argv[3]);
        blue  = dAtof(argv[4]);

        // Grab the alpha if it's there.
        if (argc > 5)
            alpha = dAtof(argv[5]);
    }

    // Invalid.
    else
    {
        Con::warnf("GuiCanvas::setBackgroundColor() - Invalid Number of parameters!");
        return;
    }

    // Set background color.
    object->setBackgroundColor(ColorF(red, green, blue, alpha) );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiCanvas, getBackgroundColor, const char*, 2, 2, "Gets the background color for the canvas.\n"
                                                                "@return (float red / float green / float blue / float alpha) The background color for the canvas.")
{
    // Get the background color.
    const ColorF& color = object->getBackgroundColor();

    // Fetch color name.
    StringTableEntry colorName = StockColor::name( color );

    // Return the color name if it's valid.
    if ( colorName != StringTable->EmptyString )
        return colorName;

    // Create Returnable Buffer.
    char* pBuffer = Con::getReturnBuffer(64);

    // Format Buffer.
    dSprintf(pBuffer, 64, "%g %g %g %g", color.red, color.green, color.blue, color.alpha );

    // Return buffer.
    return pBuffer;
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiCanvas, setUseBackgroundColor, void, 3, 3, "Sets whether to use the canvas background color or not.\n"
                                                            "@param useBackgroundColor Whether to use the canvas background color or not.\n"
                                                            "@return No return value." )
{
    // Fetch flag.
    const bool useBackgroundColor = dAtob(argv[2]);

    // Set the flag.
    object->setUseBackgroundColor( useBackgroundColor );
}

//-----------------------------------------------------------------------------

ConsoleMethod(GuiCanvas, getUseBackgroundColor, bool, 2, 2, "Gets whether the canvas background color is in use or not.\n"
                                                            "@return Whether the canvas background color is in use or not." )
{
    // Get the flag.
    return object->getUseBackgroundColor();
}


ConsoleMethod( GuiCanvas, setWindowTitle, void, 2, 2, "(string windowTitle) Sets the title to the provided string\n"
                "@param windowTitle The desired title\n"
                "@return No Return Value")
{
   return object->setWindowTitle( argv[1] );
}

ConsoleFunction(screenShot, void, 3, 3, "(string file, string format)"
                "Take a screenshot.\n\n"
                "@param format One of JPEG or PNG.")
{
#ifndef TORQUE_OS_IOS
// PUAP -Mat no screenshots on iPhone can do it from Xcode
    FileStream fStream;
   if(!fStream.open(argv[1], FileStream::Write))
   {   
      Con::printf("Failed to open file '%s'.", argv[1]);
      return;
   }
    
   glReadBuffer(GL_FRONT);
   
   Point2I extent = Canvas->getExtent();
   U8 * pixels = new U8[extent.x * extent.y * 4];
   glReadPixels(0, 0, extent.x, extent.y, GL_RGB, GL_UNSIGNED_BYTE, pixels);
   
   GBitmap * bitmap = new GBitmap;
   bitmap->allocateBitmap(U32(extent.x), U32(extent.y));
   
   // flip the rows
   for(U32 y = 0; y < (U32)extent.y; y++)
      dMemcpy(bitmap->getAddress(0, extent.y - y - 1), pixels + y * extent.x * 3, U32(extent.x * 3));

   if ( dStrcmp( argv[2], "JPEG" ) == 0 )
      bitmap->writeJPEG(fStream);
   else if( dStrcmp( argv[2], "PNG" ) == 0)
      bitmap->writePNG(fStream);
   else
      bitmap->writePNG(fStream);

   fStream.close();
   delete [] pixels;
   delete bitmap;
#endif
}


GuiCanvas::GuiCanvas():GuiControl(),
                        mShowCursor(true),
                        mMouseControl(NULL),
                        mMouseCapturedControl(NULL),
                        mMouseControlClicked(false),
                        mMouseButtonDown(false),
                        mMouseRightButtonDown(false),
                        mMouseMiddleButtonDown(false),
                        mLastMouseClickCount(0),
                        mLastMouseDownTime(0),
                        mPrevMouseTime(0),
                        mRenderFront(false),
                        mLeftMouseLast(false),
                        mMiddleMouseLast(false),
                        mRightMouseLast(false),
                        mPlatformWindow(NULL)
{
#ifdef TORQUE_OS_IOS
   setBounds(0, 0, IOS_DEFAULT_RESOLUTION_X, IOS_DEFAULT_RESOLUTION_Y);
#else
   setBounds(0, 0, MIN_RESOLUTION_X, MIN_RESOLUTION_Y);
#endif
    
   mAwake = true;
   mPixelsPerMickey = 1.0f;

   cursorON    = true;
   mShowCursor = false;

   lastCursorON = false;
   rLastFrameTime = 0.0f;

   mMouseCapturedControl = NULL;
   mMouseControl = NULL;
   mMouseControlClicked = false;
   mMouseButtonDown = false;
   mMouseRightButtonDown = false;
   mMouseMiddleButtonDown = false;

   mLastCursor = NULL;
   mLastCursorPt.set(0,0);
   mCursorPt.set(0,0);

    mLastMouseClickCount = 0;
    mLastMouseDownTime = 0;
    mPrevMouseTime = 0;
    mDefaultCursor = NULL;

   mRenderFront = false;

   hoverControlStart = Platform::getRealMilliseconds();
   hoverControl = NULL;
   hoverPosition = getCursorPos();
   hoverPositionSet = false;
   hoverLeftControlTime = 0;

   mLeftMouseLast = false;
   mMiddleMouseLast = false;
   mRightMouseLast = false;

    /// Background color.
    mBackgroundColor.set( 0.0f, 0.0f, 0.0f, 0.0f );
    mUseBackgroundColor = true;
}

GuiCanvas::~GuiCanvas()
{
   if(Canvas == this)
      Canvas = 0;
}


//-----------------------------------------------------------------------------

void GuiCanvas::initPersistFields()
{
    // Call Parent.
    Parent::initPersistFields();

    // Physics.
    addField("UseBackgroundColor", TypeBool, Offset(mUseBackgroundColor, GuiCanvas), "" );
    addField("BackgroundColor", TypeColorF, Offset(mBackgroundColor, GuiCanvas), "" );
}

//------------------------------------------------------------------------------

bool GuiCanvas::onAdd()
{
#ifndef TORQUE_OS_IOS
    // ensure that we have a cursor
    setCursor(dynamic_cast<GuiCursor*>(Sim::findObject("mDefaultCursor")));
#endif
    
    // Enumerate things for GFX before we have an active device.
    GFXInit::enumerateAdapters();
    
    // Create a device.
    GFXAdapter *a = GFXInit::getBestAdapterChoice();
    
    // Do we have a global device already? (This is the site if you want
    // to start rendering to multiple devices simultaneously)
    GFXDevice *newDevice = GFX;
    if(newDevice == NULL)
        newDevice = GFXInit::createDevice(a);
    
    newDevice->setAllowRender( false );
    
    // Initialize the window...
    GFXVideoMode vm = GFXInit::getInitialVideoMode();
    
    if (a && a->mType != NullDevice)
    {
        mPlatformWindow = WindowManager->createWindow(newDevice, vm);
        
		// Set a minimum on the window size so people can't break us by resizing tiny.
		mPlatformWindow->setMinimumWindowSize(Point2I(640,480));
        
        // Now, we have to hook in our event callbacks so we'll get
        // appropriate events from the window.
        mPlatformWindow->resizeEvent .notify(this, &GuiCanvas::handleResize);
        mPlatformWindow->appEvent    .notify(this, &GuiCanvas::handleAppEvent);
        mPlatformWindow->displayEvent.notify(this, &GuiCanvas::handlePaintEvent);
        mPlatformWindow->setInputController( dynamic_cast<IProcessInput*>(this) );
    }
    
//    // Need to get painted, too! :)
//    Process::notify(this, &GuiCanvas::paint, PROCESS_RENDER_ORDER);
    
//    // Set up the fences
//    setupFences();
    
    // Make sure we're able to render.
    newDevice->setAllowRender( true );
    
    // Propagate add to parents.
    // CodeReview - if GuiCanvas fails to add for whatever reason, what happens to
    // all the event registration above?
    bool parentRet = Parent::onAdd();
    
//    // Define the menu bar for this canvas (if any)
//    Con::executef(this, "onCreateMenu");
    
#ifdef TORQUE_DEMO_PURCHASE
    mPurchaseScreen = new PurchaseScreen;
    mPurchaseScreen->init();
    
    mLastPurchaseHideTime = 0;
#endif
    
    return parentRet;
}

void GuiCanvas::onRemove()
{
#ifdef TORQUE_DEMO_PURCHASE
    if (mPurchaseScreen && mPurchaseScreen->isAwake())
        removeObject(mPurchaseScreen);
#endif
    
//    // And the process list
//    Process::remove(this, &GuiCanvas::paint);
    
//    // Destroy the menu bar for this canvas (if any)
//    Con::executef(this, "onDestroyMenu");
    
    Parent::onRemove();
}

void GuiCanvas::setWindowTitle(const char *newTitle)
{
    if (mPlatformWindow)
        mPlatformWindow->setCaption(newTitle);
}

void GuiCanvas::handleResize( WindowId did, S32 width, S32 height )
{
//	if (Journal::IsPlaying() && mPlatformWindow)
//	{
//		mPlatformWindow->lockSize(false);
//		mPlatformWindow->setSize(Point2I(width, height));
//		mPlatformWindow->lockSize(true);
//	}
    
    // Notify the scripts
    if ( isMethod( "onResize" ) )
        Con::executef( this, 3, "onResize", Con::getIntArg( width ), Con::getIntArg( height ) );
}

void GuiCanvas::handlePaintEvent(WindowId did)
{
    bool canRender = mPlatformWindow->isVisible() && GFX->allowRender() && !GFX->canCurrentlyRender();
    
//	// Do the screenshot first.
//    if ( gScreenShot != NULL && gScreenShot->isPending() && canRender )
//		gScreenShot->capture( this );
//    
//    // If the video capture is waiting for a canvas, start the capture
//    if ( VIDCAP->isWaitingForCanvas() && canRender )
//        VIDCAP->begin( this );
//    
//    // Now capture the video
//    if ( VIDCAP->isRecording() && canRender )
//        VIDCAP->capture();
    
    renderFrame(false);
}

void GuiCanvas::handleAppEvent( WindowId did, S32 event )
{
    // Notify script if we gain or lose focus.
    if(event == LoseFocus)
    {
        if(isMethod("onLoseFocus"))
            Con::executef(this, 1, "onLoseFocus");
    }
    
    if(event == GainFocus)
    {
        if(isMethod("onGainFocus"))
            Con::executef(this, 1, "onGainFocus");
    }
    
    if(event == WindowClose || event == WindowDestroy)
    {
        
        if(isMethod("onWindowClose"))
        {
            // First see if there is a method on this window to handle
            //  it's closure
            Con::executef(this, 1, "onWindowClose");
        }
        else if(Con::isFunction("onWindowClose"))
        {
            // otherwise check to see if there is a global function handling it
            Con::executef(2, "onWindowClose", getIdString());
        }
//        else
//        {
//            // Else just shutdown
//            Process::requestShutdown();
//        }
    }
}

Point2I GuiCanvas::getWindowSize()
{
    // CodeReview Asserting on this breaks previous logic
    // and code assumptions.  It seems logical that we would
    // handle this and return an error value rather than implementing
    // if(!mPlatformWindow) whenever we need to call getWindowSize.
    // This should help keep our API error free and easy to use, while
    // cutting down on code duplication for sanity checking.  [5/5/2007 justind]
    if( !mPlatformWindow )
        return Point2I(-1,-1);
    
    return mPlatformWindow->getClientExtent();
}

//------------------------------------------------------------------------------
void GuiCanvas::setCursor(GuiCursor *curs)
{
   mDefaultCursor = curs;
}

void GuiCanvas::setCursorON(bool onOff)
{
   cursorON = onOff;
   if(!cursorON)
      mMouseControl = NULL;
}


void GuiCanvas::setCursorPos(const Point2I &pt)   
{ 
    AssertISV(mPlatformWindow, "GuiCanvas::setCursorPos - no window present!");
    
    if ( mPlatformWindow->isMouseLocked() )
    {
        mCursorPt.x = F32( pt.x );
        mCursorPt.y = F32( pt.y );
    }
    else
    {
        Point2I screenPt( mPlatformWindow->clientToScreen( pt ) );
        mPlatformWindow->setCursorPosition( screenPt.x, screenPt.y );
    }
}

void GuiCanvas::addAcceleratorKey(GuiControl *ctrl, U32 index, U32 keyCode, U32 modifier)
{
   if (keyCode > 0 && ctrl)
   {
      AccKeyMap newMap;
      newMap.ctrl = ctrl;
      newMap.index = index;
      newMap.keyCode = keyCode;
      newMap.modifier = modifier;
      mAcceleratorMap.push_back(newMap);
   }
}

bool GuiCanvas::tabNext(void)
{
   GuiControl *ctrl = static_cast<GuiControl *>(last());
   if (ctrl)
   {
      //save the old
      GuiControl *oldResponder = mFirstResponder;

        GuiControl* newResponder = ctrl->findNextTabable(mFirstResponder);
      if ( !newResponder )
         newResponder = ctrl->findFirstTabable();

        if ( newResponder && newResponder != oldResponder )
        {
            newResponder->setFirstResponder();
        if ( oldResponder )
            oldResponder->onLoseFirstResponder();
         return true;
        }
   }
   return false;
}

bool GuiCanvas::tabPrev(void)
{
   GuiControl *ctrl = static_cast<GuiControl *>(last());
   if (ctrl)
   {
      //save the old
      GuiControl *oldResponder = mFirstResponder;

        GuiControl* newResponder = ctrl->findPrevTabable(mFirstResponder);
        if ( !newResponder )
         newResponder = ctrl->findLastTabable();

        if ( newResponder && newResponder != oldResponder )
        {
            newResponder->setFirstResponder();
    
          if ( oldResponder )
             oldResponder->onLoseFirstResponder();

         return true;
        }
   }
   return false;
}

void GuiCanvas::processScreenTouchEvent(const ScreenTouchEventInfo &event)
{
    //copy the cursor point into the event
    mLastEvent.mousePoint.x = S32(event.xPos);
    mLastEvent.mousePoint.y = S32(event.yPos);
    mLastEvent.eventID = event.touchID;
    mLastEvent.mouseClickCount = event.numTouches;
    
    //see if button was pressed
    if (event.action == SI_MAKE)
    {
        U32 curTime = Platform::getVirtualMilliseconds();
        mNextMouseTime = curTime + mInitialMouseDelay;
        
        mLastMouseDownTime = curTime;
//		mLastEvent.mouseClickCount = mLastMouseClickCount;
        
        rootScreenTouchDown(mLastEvent);
    }
    else if(event.action == SI_MOVE)
    {
        rootScreenTouchMove(mLastEvent);
    }
    //else button was released
    else if(event.action == SI_BREAK)
    {
        mNextMouseTime = 0xFFFFFFFF;
        rootScreenTouchUp(mLastEvent);
    }
}

void GuiCanvas::processMouseMoveEvent(const MouseMoveEventInfo &event)
{
   if( cursorON )
   {
        //copy the modifier into the new event
        mLastEvent.modifier = event.modifier;

      mCursorPt.x += ( F32(event.xPos - mCursorPt.x) * mPixelsPerMickey);
      mCursorPt.y += ( F32(event.yPos - mCursorPt.y) * mPixelsPerMickey);

      // clamp the cursor to the window, or not
      if( ! Con::getBoolVariable( "$pref::Gui::noClampTorqueCursorToWindow", true ))
      {
         mCursorPt.x =(F32) getMax(0, getMin((S32)mCursorPt.x, getWidth() - 1));
         mCursorPt.y = (F32)getMax(0, getMin((S32)mCursorPt.y, getHeight() - 1));
      }
      
        mLastEvent.mousePoint.x = S32(mCursorPt.x);
        mLastEvent.mousePoint.y = S32(mCursorPt.y);
        mLastEvent.eventID = 0;

      Point2F movement = mMouseDownPoint - mCursorPt;
      if ((mAbs((S32)movement.x) > mDoubleClickWidth) || (mAbs((S32)movement.y) > mDoubleClickHeight))
      {
         mLeftMouseLast = false;
         mMiddleMouseLast = false;
         mRightMouseLast = false;
      }

        if (mMouseButtonDown)
            rootMouseDragged(mLastEvent);
        else if (mMouseRightButtonDown)
            rootRightMouseDragged(mLastEvent);
        else if(mMouseMiddleButtonDown)
            rootMiddleMouseDragged(mLastEvent);
        else
            rootMouseMove(mLastEvent);
    }
}

bool GuiCanvas::processInputEvent(const InputEventInfo &event)
{
    // First call the general input handler (on the extremely off-chance that it will be handled):
    if ( mFirstResponder )
   {
      if ( mFirstResponder->onInputEvent( event ) )
           return( true );
   }

   if(event.deviceType == KeyboardDeviceType)
   {
      mLastEvent.ascii = event.ascii;
      mLastEvent.modifier = event.modifier;
      mLastEvent.keyCode = (U8)event.objInst;

      U32 eventModifier = event.modifier;
      if(eventModifier & SI_SHIFT)
         eventModifier |= SI_SHIFT;
      if(eventModifier & SI_CTRL)
         eventModifier |= SI_CTRL;
      if(eventModifier & SI_ALT)
         eventModifier |= SI_ALT;

      if (event.action == SI_MAKE)
      {
         //see if we should tab next/prev

         //see if we should now pass the event to the first responder
         if (mFirstResponder)
         {
            if(mFirstResponder->onKeyDown(mLastEvent))
               return true;
         }

         if ( isCursorON() && ( event.objInst == KEY_TAB ) )
         {
            if (size() > 0)
            {
               if (event.modifier & SI_SHIFT)
               {
                  if(tabPrev())
                     return true;
               }
               else if (event.modifier == 0)
               {
                  if(tabNext())
                     return true;
               }
            }
         }

         //if not handled, search for an accelerator
         for (U32 i = 0; i < (U32)mAcceleratorMap.size(); i++)
         {
            if ((U32)mAcceleratorMap[i].keyCode == (U32)event.objInst && (U32)mAcceleratorMap[i].modifier == eventModifier)
            {
               mAcceleratorMap[i].ctrl->acceleratorKeyPress(mAcceleratorMap[i].index);
               return true;
            }
         }
      }
      else if(event.action == SI_BREAK)
      {
         if(mFirstResponder)
            if(mFirstResponder->onKeyUp(mLastEvent))
               return true;

         //see if there's an accelerator
         for (U32 i = 0; i < (U32)mAcceleratorMap.size(); i++)
         {
            if ((U32)mAcceleratorMap[i].keyCode == (U32)event.objInst && (U32)mAcceleratorMap[i].modifier == eventModifier)
            {
               mAcceleratorMap[i].ctrl->acceleratorKeyRelease(mAcceleratorMap[i].index);
               return true;
            }
         }
      }
      else if(event.action == SI_REPEAT)
      {

         //if not handled, search for an accelerator
         for (U32 i = 0; i < (U32)mAcceleratorMap.size(); i++)
         {
            if ((U32)mAcceleratorMap[i].keyCode == (U32)event.objInst && (U32)mAcceleratorMap[i].modifier == eventModifier)
            {
               mAcceleratorMap[i].ctrl->acceleratorKeyPress(mAcceleratorMap[i].index);
               return true;
            }
         }

         if(mFirstResponder)
            mFirstResponder->onKeyRepeat(mLastEvent);
         return true;
      }
   }
   else if(event.deviceType == MouseDeviceType && cursorON)
   {
      //copy the modifier into the new event
      mLastEvent.modifier = event.modifier;

      if(event.objType == SI_XAXIS || event.objType == SI_YAXIS)
      {
         bool moved = false;
         Point2I oldpt((S32)mCursorPt.x, (S32)mCursorPt.y);
         Point2F pt(mCursorPt.x, mCursorPt.y);

         if (event.objType == SI_XAXIS)
         {
            pt.x += (event.fValue * mPixelsPerMickey);
            mCursorPt.x = (F32)getMax(0, getMin((S32)pt.x, getWidth() - 1));
            if (oldpt.x != S32(mCursorPt.x))
               moved = true;
         }
         else
         {
            pt.y += (event.fValue * mPixelsPerMickey);
            mCursorPt.y = (F32)getMax(0, getMin((S32)pt.y, getHeight() - 1));
            if (oldpt.y != S32(mCursorPt.y))
               moved = true;
         }
         if (moved)
         {
            mLastEvent.mousePoint.x = S32(mCursorPt.x);
            mLastEvent.mousePoint.y = S32(mCursorPt.y);
            mLastEvent.eventID = 0;

#ifdef	TORQUE_ALLOW_JOURNALING
            // [tom, 9/8/2006] If we're journaling, we need to update the plat cursor
            if(Game->isJournalReading())
               Input::setCursorPos((S32)mCursorPt.x, (S32)mCursorPt.y);
#endif	//TORQUE_ALLOW_JOURNALING

            if (mMouseButtonDown)
               rootMouseDragged(mLastEvent);
            else if (mMouseRightButtonDown)
               rootRightMouseDragged(mLastEvent);
            else if(mMouseMiddleButtonDown)
               rootMiddleMouseDragged(mLastEvent);
            else
               rootMouseMove(mLastEvent);
         }
         return true;
      }
        else if ( event.objType == SI_ZAXIS )
        {
         mLastEvent.mousePoint.x = S32( mCursorPt.x );
         mLastEvent.mousePoint.y = S32( mCursorPt.y );
         mLastEvent.eventID = 0;

            if ( event.fValue < 0.0f )
            rootMouseWheelDown( mLastEvent );
            else
            rootMouseWheelUp( mLastEvent );
      }
      else if(event.objType == SI_BUTTON)
      {
         //copy the cursor point into the event
         mLastEvent.mousePoint.x = S32(mCursorPt.x);
         mLastEvent.mousePoint.y = S32(mCursorPt.y);
         mLastEvent.eventID = 0;
         mMouseDownPoint = mCursorPt;

         if(event.objInst == KEY_BUTTON0) // left button
         {
            //see if button was pressed
            if (event.action == SI_MAKE)
            {
               U32 curTime = Platform::getVirtualMilliseconds();
               mNextMouseTime = curTime + mInitialMouseDelay;

               //if the last button pressed was the left...
               if (mLeftMouseLast)
               {
                  //if it was within the double click time count the clicks
                  if ((S32)curTime - mLastMouseDownTime <= mDoubleClickTime)
                     mLastMouseClickCount++;
                  else
                     mLastMouseClickCount = 1;
               }
               else
               {
                  mLeftMouseLast = true;
                  mLastMouseClickCount = 1;
               }

               mLastMouseDownTime = curTime;
               mLastEvent.mouseClickCount = mLastMouseClickCount;

               rootMouseDown(mLastEvent);
            }
            //else button was released
            else
            {
               mNextMouseTime = 0xFFFFFFFF;
               rootMouseUp(mLastEvent);
            }
            return true;
         }
         else if(event.objInst == KEY_BUTTON1) // right button
         {
            if(event.action == SI_MAKE)
            {
               U32 curTime = Platform::getVirtualMilliseconds();

               //if the last button pressed was the right...
               if (mRightMouseLast)
               {
                  //if it was within the double click time count the clicks
                  if ((S32)curTime - mLastMouseDownTime <= mDoubleClickTime)
                     mLastMouseClickCount++;
                  else
                     mLastMouseClickCount = 1;
               }
               else
               {
                  mRightMouseLast = true;
                  mLastMouseClickCount = 1;
               }

               mLastMouseDownTime = curTime;
               mLastEvent.mouseClickCount = mLastMouseClickCount;

               rootRightMouseDown(mLastEvent);
            }
            else // it was a mouse up
               rootRightMouseUp(mLastEvent);
            return true;
         }
         else if(event.objInst == KEY_BUTTON2) // middle button
         {
            if(event.action == SI_MAKE)
            {
               U32 curTime = Platform::getVirtualMilliseconds();

               //if the last button pressed was the right...
               if (mMiddleMouseLast)
               {
                  //if it was within the double click time count the clicks
                  if ((S32)curTime - mLastMouseDownTime <= mDoubleClickTime)
                     mLastMouseClickCount++;
                  else
                     mLastMouseClickCount = 1;
               }
               else
               {
                  mMiddleMouseLast = true;
                  mLastMouseClickCount = 1;
               }

               mLastMouseDownTime = curTime;
               mLastEvent.mouseClickCount = mLastMouseClickCount;

               rootMiddleMouseDown(mLastEvent);
            }
            else // it was a mouse up
               rootMiddleMouseUp(mLastEvent);
            return true;
         }
      }
   }
   return false;
}

void GuiCanvas::rootMouseDown(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseButtonDown = true;

   //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseDown(event);

   //else pass it to whoever is underneath the cursor
   else
   {
      iterator i;
      i = end();
      while (i != begin())
      {
         i--;
         GuiControl *ctrl = static_cast<GuiControl *>(*i);
         GuiControl *controlHit = ctrl->findHitControl(event.mousePoint);

         //see if the controlHit is a modeless dialog...
         if ((! controlHit->mActive) && (! controlHit->mProfile->mModal))
            continue;
         else
         {
            controlHit->onMouseDown(event);
            break;
         }
      }
   }
   if (bool(mMouseControl))
      mMouseControlClicked = true;
}

void GuiCanvas::findMouseControl(const GuiEvent &event)
{
   if(size() == 0)
   {
      mMouseControl = NULL;
      return;
   }
   GuiControl *controlHit = findHitControl(event.mousePoint);
   if(controlHit != static_cast<GuiControl*>(mMouseControl))
   {
      if(bool(mMouseControl))
         mMouseControl->onMouseLeave(event);
      mMouseControl = controlHit;
      mMouseControl->onMouseEnter(event);
   }
}

//Luma: Some fixes from the forums, Dave Calabrese
//http://www.garagegames.com/community/forums/viewthread/93467/1#comment-669559
void GuiCanvas::rootScreenTouchDown(const GuiEvent &event)
{
    mPrevMouseTime = Platform::getVirtualMilliseconds();  
    mMouseButtonDown = true;  
  
        iterator i;  
        i = end();  
        while (i != begin())  
        {  
            i--;  
            GuiControl *ctrl = static_cast<GuiControl *>(*i);  
            GuiControl *controlHit = ctrl->findHitControl(event.mousePoint);  
              
            //If the control we hit is not the same one that is locked,  
            // then unlock the existing control.  
            if (bool(mMouseCapturedControl))  
            {  
                if(mMouseCapturedControl->isMouseLocked())  
                {  
                    if(mMouseCapturedControl != controlHit)  
                    {  
                        mMouseCapturedControl->onMouseLeave(event);  
                    }  
                }  
            }  
              
            //see if the controlHit is a modeless dialog...  
            if ((! controlHit->mActive) && (! controlHit->mProfile->mModal))  
                continue;  
            else  
            {  
                controlHit->onMouseDown(event);  
                break;  
            }  
        }  
      
    if (bool(mMouseControl))  
        mMouseControlClicked = true;  
}

void GuiCanvas::rootScreenTouchUp(const GuiEvent &event)
{
    mPrevMouseTime = Platform::getVirtualMilliseconds();
    mMouseButtonDown = false;

    iterator i;
    i = end();
    while (i != begin())
    {
        i--;    
        GuiControl *ctrl = static_cast<GuiControl *>(*i);
        GuiControl *controlHit = ctrl->findHitControl(event.mousePoint);
        
        //see if the controlHit is a modeless dialog...
        if ((! controlHit->mActive) && (! controlHit->mProfile->mModal))
            continue;
        else
        {
            controlHit->onMouseUp(event);
            break;
        }
    }
}

void GuiCanvas::rootScreenTouchMove(const GuiEvent &event)
{
    //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
   {
      checkLockMouseMove(event);
      if(!mMouseCapturedControl.isNull())
            mMouseCapturedControl->onMouseDragged(event);
   }
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
      {
          mMouseControl->onMouseDragged(event);		  
      }
   }
}
void GuiCanvas::refreshMouseControl()
{
   GuiEvent evt;
   evt.mousePoint.x = S32(mCursorPt.x);
   evt.mousePoint.y = S32(mCursorPt.y);
   findMouseControl(evt);
}

void GuiCanvas::rootMouseUp(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseButtonDown = false;

   //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseUp(event);
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
         mMouseControl->onMouseUp(event);
   }
}

void GuiCanvas::checkLockMouseMove(const GuiEvent &event)
{
    GuiControl *controlHit = findHitControl(event.mousePoint);
   if(controlHit != mMouseControl)
   {
      if(mMouseControl == mMouseCapturedControl)
         mMouseCapturedControl->onMouseLeave(event);
      else if(controlHit == mMouseCapturedControl)
         mMouseCapturedControl->onMouseEnter(event);
      mMouseControl = controlHit;
   }
}

void GuiCanvas::rootMouseDragged(const GuiEvent &event)
{
   //pass the event to the mouse locked control
   if (bool(mMouseCapturedControl))
   {
      checkLockMouseMove(event);
      if(!mMouseCapturedControl.isNull())
            mMouseCapturedControl->onMouseDragged(event);
       //Luma: Mouse dragged calls mouse Moved on iPhone
#ifdef TORQUE_OS_IOS
       mMouseCapturedControl->onMouseMove(event);
#endif //TORQUE_OS_IOS
   }
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
      {
          mMouseControl->onMouseDragged(event);
#ifdef TORQUE_OS_IOS
          mMouseControl->onMouseMove(event);
#endif //TORQUE_OS_IOS
          
      }
   }
}

void GuiCanvas::rootMouseMove(const GuiEvent &event)
{
   if(mMouseCapturedControl != NULL)
   {
      checkLockMouseMove(event);
      if(mMouseCapturedControl != NULL)
        mMouseCapturedControl->onMouseMove(event);
   }
   else
   {
      findMouseControl(event);
      if(bool(mMouseControl))
         mMouseControl->onMouseMove(event);
   }
}

void GuiCanvas::rootRightMouseDown(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseRightButtonDown = true;

   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onRightMouseDown(event);
   else
   {
      findMouseControl(event);

      if(bool(mMouseControl))
      {
         mMouseControl->onRightMouseDown(event);
      }
   }
}

void GuiCanvas::rootRightMouseUp(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseRightButtonDown = false;

   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onRightMouseUp(event);
   else
   {
      findMouseControl(event);

      if(bool(mMouseControl))
         mMouseControl->onRightMouseUp(event);
   }
}

void GuiCanvas::rootRightMouseDragged(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();

   if (bool(mMouseCapturedControl))
   {
      checkLockMouseMove(event);
      mMouseCapturedControl->onRightMouseDragged(event);
   }
   else
   {
      findMouseControl(event);

      if(bool(mMouseControl))
         mMouseControl->onRightMouseDragged(event);
   }
}

void GuiCanvas::rootMiddleMouseDown(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseMiddleButtonDown = true;

   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMiddleMouseDown(event);
   else
   {
      findMouseControl(event);

      if(bool(mMouseControl))
      {
         mMouseControl->onMiddleMouseDown(event);
      }
   }
}

void GuiCanvas::rootMiddleMouseUp(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();
   mMouseMiddleButtonDown = false;

   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMiddleMouseUp(event);
   else
   {
      findMouseControl(event);

      if(bool(mMouseControl))
         mMouseControl->onMiddleMouseUp(event);
   }
}

void GuiCanvas::rootMiddleMouseDragged(const GuiEvent &event)
{
   mPrevMouseTime = Platform::getVirtualMilliseconds();

   if (bool(mMouseCapturedControl))
   {
      checkLockMouseMove(event);
      mMouseCapturedControl->onMiddleMouseDragged(event);
   }
   else
   {
      findMouseControl(event);

      if(bool(mMouseControl))
         mMouseControl->onMiddleMouseDragged(event);
   }
}

void GuiCanvas::rootMouseWheelUp(const GuiEvent &event)
{
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseWheelUp(event);
   else
   {
      findMouseControl(event);

      if (bool(mMouseControl))
         mMouseControl->onMouseWheelUp(event);
   }
}

void GuiCanvas::rootMouseWheelDown(const GuiEvent &event)
{
   if (bool(mMouseCapturedControl))
      mMouseCapturedControl->onMouseWheelDown(event);
   else
   {
      findMouseControl(event);

      if (bool(mMouseControl))
         mMouseControl->onMouseWheelDown(event);
   }
}

void GuiCanvas::setContentControl(GuiControl *gui)
{
   if(!gui)
      return;

   // If we're setting the same content, don't do anything
   if( gui == at(0) )
      return;

   //remove all dialogs on layer 0
   U32 index = 0;
   while ((U32)size() > index)
   {
      GuiControl *ctrl = static_cast<GuiControl*>((*this)[index]);
      if (ctrl == gui || ctrl->mLayer != 0)
         index++;

      removeObject(ctrl);
      Sim::getGuiGroup()->addObject(ctrl);
   }

   // lose the first responder from the old GUI
   GuiControl* responder = gui->findFirstTabable();
   if(responder)
      responder->setFirstResponder();

   //add the gui to the front
   if(!size() || gui != (*this)[0])
   {
      // automatically wakes objects in GuiControl::onWake
      addObject(gui);
      if (size() >= 2)
         reOrder(gui, *begin());
   }
   //refresh the entire gui
   resetUpdateRegions();

   //rebuild the accelerator map
   mAcceleratorMap.clear();

   for(iterator i = end(); i != begin() ; )
   {
      i--;
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->buildAcceleratorMap();

      if (ctrl->mProfile->mModal)
         break;
   }
   refreshMouseControl();

   // Force the canvas to update the sizing of the new content control
   maintainSizing();
}

GuiControl *GuiCanvas::getContentControl()
{
   if(size() > 0)
      return (GuiControl *) first();
   return NULL;
}

void GuiCanvas::pushDialogControl(GuiControl *gui, S32 layer)
{
   //add the gui
   gui->mLayer = layer;

   // GuiControl::addObject wakes the object
   addObject(gui);

   //reorder it to the correct layer
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl*>(*i);
      if (ctrl->mLayer > gui->mLayer)
      {
         reOrder(gui, ctrl);
         break;
      }
   }

   //call the dialog push method
   gui->onDialogPush();

   //find the top most dialog

   //find the first responder
   GuiControl* responder = gui->findFirstTabable();
   if(responder)
      responder->setFirstResponder();

   // call the 'onWake' method?
   //if(wakedGui)
   //   Con::executef(gui, 1, "onWake");

   //refresh the entire gui
   resetUpdateRegions();

   //rebuild the accelerator map
   mAcceleratorMap.clear();
   if (size() > 0)
   {
      GuiControl *ctrl = static_cast<GuiControl*>(last());
      ctrl->buildAcceleratorMap();
   }
   refreshMouseControl();

    // I don't see the purpose of this, and it's causing issues when showing, for instance the
    //  metrics dialog while in a 3d scene, causing the cursor to be shown even when the mouse
    //  is locked [4/25/2007 justind]
    if(gui->mProfile && gui->mProfile->mModal)
        mPlatformWindow->getCursorController()->pushCursor(PlatformCursorController::curArrow);
}

void GuiCanvas::popDialogControl(GuiControl *gui)
{
   if (size() < 1)
      return;

   //first, find the dialog, and call the "onDialogPop()" method
   GuiControl *ctrl = NULL;
   if (gui)
   {
      //make sure the gui really exists on the stack
      iterator i;
      bool found = false;
      for(i = begin(); i != end(); i++)
      {
         GuiControl *check = static_cast<GuiControl *>(*i);
         if (check == gui)
         {
            ctrl = check;
            found = true;
         }
      }
      if (! found)
         return;
   }
   else
      ctrl = static_cast<GuiControl*>(last());

   //call the "on pop" function
   ctrl->onDialogPop();

   // sleep the object

   //now pop the last child (will sleep if awake)
   removeObject(ctrl);

   // Save the old responder:

   Sim::getGuiGroup()->addObject(ctrl);

   if (size() > 0)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(last());
      if(ctrl->mFirstResponder)
         ctrl->mFirstResponder->setFirstResponder();
   }
   else
   {
      setFirstResponder(NULL);
   }

   //refresh the entire gui
   resetUpdateRegions();

   //rebuild the accelerator map
   mAcceleratorMap.clear();
   if (size() > 0)
   {
      GuiControl *ctrl = static_cast<GuiControl*>(last());
      ctrl->buildAcceleratorMap();
   }
   refreshMouseControl();
}

void GuiCanvas::popDialogControl(S32 layer)
{
   if (size() < 1)
      return;

   GuiControl *ctrl = NULL;
   iterator i = end(); // find in z order (last to first)
   while (i != begin())
   {
      i--;
      ctrl = static_cast<GuiControl*>(*i);
      if (ctrl->mLayer == layer)
         break;
   }
   if (ctrl)
      popDialogControl(ctrl);
}

void GuiCanvas::mouseLock(GuiControl *lockingControl)
{
   if (bool(mMouseCapturedControl))
      return;
   mMouseCapturedControl = lockingControl;
   if(mMouseControl && mMouseControl != mMouseCapturedControl)
   {
      GuiEvent evt;
      evt.mousePoint.x = S32(mCursorPt.x);
      evt.mousePoint.y = S32(mCursorPt.y);

      mMouseControl->onMouseLeave(evt);
   }
}

void GuiCanvas::mouseUnlock(GuiControl *lockingControl)
{
   if (static_cast<GuiControl*>(mMouseCapturedControl) != lockingControl)
      return;

   GuiEvent evt;
   evt.mousePoint.x = S32(mCursorPt.x);
   evt.mousePoint.y = S32(mCursorPt.y);

   GuiControl * controlHit = findHitControl(evt.mousePoint);
   if(controlHit != mMouseCapturedControl)
   {
      mMouseControl = controlHit;
      mMouseControlClicked = false;
      if(bool(mMouseControl))
         mMouseControl->onMouseEnter(evt);
   }
   mMouseCapturedControl = NULL;
}

void GuiCanvas::paint()
{
   resetUpdateRegions();

//   // inhibit explicit refreshes in the case we're swapped out
//   if (TextureManager::mDGLRender)
//      renderFrame(false);
}

void GuiCanvas::maintainSizing()
{
    Point2I size = getWindowSize();

   if(size.x == 0 || size.y == 0)
      return;

   RectI screenRect(0, 0, size.x, size.y);
   mBounds = screenRect;

   //all bottom level controls should be the same dimensions as the canvas
   //this is necessary for passing mouse events accurately
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      AssertFatal(static_cast<GuiControl*>((*i))->isAwake(), "GuiCanvas::renderFrame: ctrl is not awake");
      GuiControl *ctrl = static_cast<GuiControl*>(*i);
      Point2I ext = ctrl->getExtent();
      Point2I pos = ctrl->getPosition();

      if(pos != screenRect.point || ext != screenRect.extent)
      {
         ctrl->resize(screenRect.point, screenRect.extent);
         resetUpdateRegions();
      }
   }

}

void GuiCanvas::renderFrame(bool preRenderOnly, bool bufferSwap /* = true */)
{
   PROFILE_START(CanvasPreRender);

#ifndef TORQUE_OS_IOS
    
   if(mRenderFront)
      glDrawBuffer(GL_FRONT);
   else
      glDrawBuffer(GL_BACK);
#endif

    // Set our window as the current render target so we can see outputs.
    GFX->setActiveRenderTarget(mPlatformWindow->getGFXTarget());
    
    if (!GFX->getActiveRenderTarget())
    {
        PROFILE_END();
        return;
    }
    
#ifdef TORQUE_GFX_STATE_DEBUG
    GFX->getDebugStateManager()->startFrame();
#endif
    
    GFXTarget* renderTarget = GFX->getActiveRenderTarget();
    if (renderTarget == NULL)
    {
        PROFILE_END();
        return;
    }
    
    // Make sure the root control is the size of the canvas.
    Point2I size = renderTarget->getSize();
    
    if(size.x == 0 || size.y == 0)
    {
        PROFILE_END();
        return;
    }
    
    RectI screenRect(0, 0, size.x, size.y);
    
//    // Make sure the root control is the size of the canvas.
//   Point2I size = Platform::getWindowSize();
//
//   if(size.x == 0 || size.y == 0)
//   {
//       //Luma: Fixed missing PROFILE_END()
//       PROFILE_END();
//       return;
//   }
//
//   RectI screenRect(0, 0, size.x, size.y);

   maintainSizing();

   //preRender (recursive) all controls
   preRender();
   PROFILE_END();
   if(preRenderOnly)
      return;

   // for now, just always reset the update regions - this is a
   // fix for FSAA on ATI cards
   resetUpdateRegions();

// Moved this below object integration for performance reasons. -JDD
//   // finish the gl render so we don't get too far ahead of ourselves
//#if defined(TORQUE_OS_WIN32)
//   PROFILE_START(glFinish);
//   glFinish();
//   PROFILE_END();
//#endif

   //draw the mouse, but not using tags...
   PROFILE_START(CanvasRenderControls);

   GuiCursor *mouseCursor = NULL;
   bool cursorVisible = true;

   if(bool(mMouseCapturedControl))
      mMouseCapturedControl->getCursor(mouseCursor, cursorVisible, mLastEvent);
   else if(bool(mMouseControl))
      mMouseControl->getCursor(mouseCursor, cursorVisible, mLastEvent);

   Point2I cursorPos((S32)mCursorPt.x, (S32)mCursorPt.y);
   if(!mouseCursor)
      mouseCursor = mDefaultCursor;

   if(lastCursorON && mLastCursor)
   {
      Point2I spot = mLastCursor->getHotSpot();
      Point2I cext = mLastCursor->getExtent();
      Point2I pos = mLastCursorPt - spot;
      addUpdateRegion(pos - Point2I(2, 2), Point2I(cext.x + 4, cext.y + 4));
   }
   if(cursorVisible && mouseCursor)
   {
      Point2I spot = mouseCursor->getHotSpot();
      Point2I cext = mouseCursor->getExtent();
      Point2I pos = cursorPos - spot;

      addUpdateRegion(pos - Point2I(2, 2), Point2I(cext.x + 4, cext.y + 4));
   }

    lastCursorON = cursorVisible;
    mLastCursor = mouseCursor;
    mLastCursorPt = cursorPos;

    bool beginSceneRes = GFX->beginScene();
    if ( !beginSceneRes )
    {
//        GuiCanvas::getGuiCanvasFrameSignal().trigger(false);
        return;
    }
    
    GFX->setWorldMatrix( MatrixF::Identity );
    GFX->setViewMatrix( MatrixF::Identity );
    GFX->setProjectionMatrix( MatrixF::Identity );

    RectI updateUnion;
   buildUpdateUnion(&updateUnion);
   if (updateUnion.intersect(screenRect))
   {
    // Clear the background color if requested.
//    if ( mUseBackgroundColor )
    {
        GFX->clear( GFXClearZBuffer | GFXClearStencil | GFXClearTarget, mBackgroundColor, 1.0f, 0 );
    }

       //render the dialogs
      iterator i;
      for(i = begin(); i != end(); i++)
      {
         GuiControl *contentCtrl = static_cast<GuiControl*>(*i);
         GFX->setClipRect(updateUnion);
         GFX->setStateBlock(mDefaultGuiSB);
         contentCtrl->onRender(contentCtrl->getPosition(), updateUnion);
      }

      GFX->setClipRect(updateUnion);

      //temp draw the mouse
      if (cursorON && mShowCursor && !mouseCursor)
      {
#ifdef TORQUE_OS_IOS
         
#pragma message ("removed")
//         glColor4ub(255, 0, 0, 255);
//         GLfloat vertices[] = {
//              (GLfloat)(mCursorPt.x),(GLfloat)(mCursorPt.y),
//              (GLfloat)(mCursorPt.x + 2),(GLfloat)(mCursorPt.y),
//              (GLfloat)(mCursorPt.x + 2),(GLfloat)(mCursorPt.y + 2),
//              (GLfloat)(mCursorPt.x),(GLfloat)(mCursorPt.y + 2),
//          };
//          glEnableClientState(GL_VERTEX_ARRAY);
//          glVertexPointer(2, GL_FLOAT, 0, vertices);
//          glDrawArrays(GL_LINE_LOOP, 0, 4);
#else
//         glColor4ub(255, 0, 0, 255);
//         glRecti((S32)mCursorPt.x, (S32)mCursorPt.y, (S32)(mCursorPt.x + 2), (S32)(mCursorPt.y + 2));
#endif
      }
       
      //DEBUG
      //draw the help ctrl
      //if (helpCtrl)
      //{
      //   helpCtrl->render(srf);
      //}

      if (cursorON && mouseCursor && mShowCursor)
      {
         Point2I pos((S32)mCursorPt.x, (S32)mCursorPt.y);
         Point2I spot = mouseCursor->getHotSpot();

         pos -= spot;
         mouseCursor->render(pos);
      }
   }

   PROFILE_END();

    GFX->endScene();

   if( bufferSwap )
      swapBuffers();
    
//#if defined(TORQUE_OS_WIN32)
//   PROFILE_START(glFinish);
//   glFinish(); // This was changed to work with the D3D layer -pw
//   PROFILE_END();
//#endif

}

void GuiCanvas::swapBuffers()
{
//   PROFILE_START(SwapBuffers);
//   //flip the surface
//   if(!mRenderFront)
//      Video::swapBuffers();
//   PROFILE_END();

    AssertISV(mPlatformWindow, "GuiCanvas::swapBuffers - no window present!");
    if(!mPlatformWindow->isVisible())
        return;
    
    PROFILE_START(SwapBuffers);
    mPlatformWindow->getGFXTarget()->present();
    PROFILE_END();
}

void GuiCanvas::buildUpdateUnion(RectI *updateUnion)
{
   *updateUnion = mOldUpdateRects[0];

   //the update region should encompass the oldUpdateRects, and the curUpdateRect
   Point2I upperL;
   Point2I lowerR;

   upperL.x = getMin(mOldUpdateRects[0].point.x, mOldUpdateRects[1].point.x);
   upperL.x = getMin(upperL.x, mCurUpdateRect.point.x);

   upperL.y = getMin(mOldUpdateRects[0].point.y, mOldUpdateRects[1].point.y);
   upperL.y = getMin(upperL.y, mCurUpdateRect.point.y);

   lowerR.x = getMax(mOldUpdateRects[0].point.x + mOldUpdateRects[0].extent.x, mOldUpdateRects[1].point.x + mOldUpdateRects[1].extent.x);
   lowerR.x = getMax(lowerR.x, mCurUpdateRect.point.x + mCurUpdateRect.extent.x);

   lowerR.y = getMax(mOldUpdateRects[0].point.y + mOldUpdateRects[0].extent.y, mOldUpdateRects[1].point.y + mOldUpdateRects[1].extent.y);
   lowerR.y = getMax(lowerR.y, mCurUpdateRect.point.y + mCurUpdateRect.extent.y);

   updateUnion->point = upperL;
   updateUnion->extent = lowerR - upperL;

   //shift the oldUpdateRects
   mOldUpdateRects[0] = mOldUpdateRects[1];
   mOldUpdateRects[1] = mCurUpdateRect;

   mCurUpdateRect.point.set(0,0);
   mCurUpdateRect.extent.set(0,0);
}

void GuiCanvas::addUpdateRegion(Point2I pos, Point2I ext)
{
   if(mCurUpdateRect.extent.x == 0)
   {
      mCurUpdateRect.point = pos;
      mCurUpdateRect.extent = ext;
   }
   else
   {
      Point2I upperL;
      upperL.x = getMin(mCurUpdateRect.point.x, pos.x);
      upperL.y = getMin(mCurUpdateRect.point.y, pos.y);
      Point2I lowerR;
      lowerR.x = getMax(mCurUpdateRect.point.x + mCurUpdateRect.extent.x, pos.x + ext.x);
      lowerR.y = getMax(mCurUpdateRect.point.y + mCurUpdateRect.extent.y, pos.y + ext.y);
      mCurUpdateRect.point = upperL;
      mCurUpdateRect.extent = lowerR - upperL;
   }
}

void GuiCanvas::resetUpdateRegions()
{
   //DEBUG - get surface width and height
   mOldUpdateRects[0].set(getPosition(), getExtent());
   mOldUpdateRects[1] = mOldUpdateRects[0];
   mCurUpdateRect = mOldUpdateRects[0];
}

void GuiCanvas::setFirstResponder( GuiControl* newResponder )
{
    GuiControl* oldResponder = mFirstResponder;
    Parent::setFirstResponder( newResponder );

    if ( oldResponder && ( oldResponder != mFirstResponder ) )
        oldResponder->onLoseFirstResponder();
}
