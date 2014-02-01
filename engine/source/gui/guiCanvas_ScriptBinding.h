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

#ifndef Torque2D_guiCanvas_ScriptBinding_h
#define Torque2D_guiCanvas_ScriptBinding_h

ConsoleMethodGroupBeginWithDocs(GuiCanvas, GuiControl)

ConsoleMethod( GuiCanvas, isFullscreen, bool, 2, 2, "() - Is this canvas currently fullscreen?" )
{
//    if (Platform::getWebDeployment())
//        return false;
    
    if (!object->getPlatformWindow())
        return false;
    
    return object->getPlatformWindow()->getVideoMode().fullScreen;
}

ConsoleMethod( GuiCanvas, minimizeWindow, void, 2, 2, "() - minimize this canvas' window." )
{
    PlatformWindow* window = object->getPlatformWindow();
    if ( window )
        window->minimize();
}

ConsoleMethod( GuiCanvas, isMinimized, bool, 2, 2, "()" )
{
    PlatformWindow* window = object->getPlatformWindow();
    if ( window )
        return window->isMinimized();
    
    return false;
}

ConsoleMethod( GuiCanvas, isMaximized, bool, 2, 2, "()" )
{
    PlatformWindow* window = object->getPlatformWindow();
    if ( window )
        return window->isMaximized();
    
    return false;
}

ConsoleMethod( GuiCanvas, maximizeWindow, void, 2, 2, "() - maximize this canvas' window." )
{
    PlatformWindow* window = object->getPlatformWindow();
    if ( window )
        window->maximize();
}

ConsoleMethod( GuiCanvas, restoreWindow, void, 2, 2, "() - restore this canvas' window." )
{
    PlatformWindow* window = object->getPlatformWindow();
    if( window )
        window->restore();
}

ConsoleMethod( GuiCanvas, setFocus, void, 2,2, "() - Claim OS input focus for this canvas' window.")
{
    PlatformWindow* window = object->getPlatformWindow();
    if( window )
        window->setFocus();
}

ConsoleMethod( GuiCanvas, setVideoMode, void, 5, 8,
              "(int width, int height, bool fullscreen, [int bitDepth], [int refreshRate], [int antialiasLevel] )\n"
              "Change the video mode of this canvas. This method has the side effect of setting the $pref::Video::mode to the new values.\n\n"
              "\\param width The screen width to set.\n"
              "\\param height The screen height to set.\n"
              "\\param fullscreen Specify true to run fullscreen or false to run in a window\n"
              "\\param bitDepth [optional] The desired bit-depth. Defaults to the current setting. This parameter is ignored if you are running in a window.\n"
              "\\param refreshRate [optional] The desired refresh rate. Defaults to the current setting. This parameter is ignored if you are running in a window"
              "\\param antialiasLevel [optional] The level of anti-aliasing to apply 0 = none" )
{
    if (!object->getPlatformWindow())
        return;
    
//    if (Journal::IsRecording() || Journal::IsPlaying())
//        return;
    
    // Update the video mode and tell the window to reset.
    GFXVideoMode vm = object->getPlatformWindow()->getVideoMode();
    
    U32 width = dAtoi(argv[2]);
    U32 height = dAtoi(argv[3]);
    
    bool changed = false;
    if (width == 0 && height > 0)
    {
        // Our width is 0 but our height isn't...
        // Try to find a matching width
        for(S32 i=0; i<object->getPlatformWindow()->getGFXDevice()->getVideoModeList()->size(); i++)
        {
            const GFXVideoMode &newVm = (*(object->getPlatformWindow()->getGFXDevice()->getVideoModeList()))[i];
            
            if(newVm.resolution.y == height)
            {
                width = newVm.resolution.x;
                changed = true;
                break;
            }
        }
    }
    else if (height == 0 && width > 0)
    {
        // Our height is 0 but our width isn't...
        // Try to find a matching height
        for(S32 i=0; i<object->getPlatformWindow()->getGFXDevice()->getVideoModeList()->size(); i++)
        {
            const GFXVideoMode &newVm = (*(object->getPlatformWindow()->getGFXDevice()->getVideoModeList()))[i];
            
            if(newVm.resolution.x == width)
            {
                height = newVm.resolution.y;
                changed = true;
                break;
            }
        }
    }
    
    if (width == 0 || height == 0)
    {
        // Got a bad size for both of our dimensions or one of our dimensions and
        // didn't get a match for the other default back to our current resolution
        width  = vm.resolution.x;
        height = vm.resolution.y;
        
        changed = true;
    }
    
    if (changed)
        Con::errorf("GuiCanvas::setVideoMode(): Error - Invalid resolution of (%d, %d) - attempting (%d, %d)", dAtoi(argv[2]), dAtoi(argv[3]), width, height);
    
    vm.resolution  = Point2I(width, height);
    vm.fullScreen  = dAtob(argv[4]);
    
//    if (Platform::getWebDeployment())
//        vm.fullScreen  = false;
    
    // These optional params are set to default at construction of vm. If they
    // aren't specified, just leave them at whatever they were set to.
    if ((argc > 5) && (dStrlen(argv[5]) > 0))
    {
        vm.bitDepth = dAtoi(argv[5]);
    }
    if ((argc > 6) && (dStrlen(argv[6]) > 0))
    {
        vm.refreshRate = dAtoi(argv[6]);
    }
    
    if ((argc > 7) && (dStrlen(argv[7]) > 0))
    {
        vm.antialiasLevel = dAtoi(argv[7]);
    }
    
    object->getPlatformWindow()->setVideoMode(vm);
    object->paint();
    
    // Store the new mode into a pref.
    Con::setVariable( "$pref::Video::mode", vm.toString() );
}

ConsoleMethod( GuiCanvas, getVideoMode, const char*, 2, 2,
				   "@brief Gets the current screen mode as a string.\n\n"
                   
				   "The return string will contain 5 values (width, height, fullscreen, bitdepth, refreshRate). "
				   "You will need to parse out each one for individual use.\n\n"
                   
				   "@tsexample\n"
				   "%screenWidth = getWord(Canvas.getVideoMode(), 0);\n"
				   "%screenHeight = getWord(Canvas.getVideoMode(), 1);\n"
				   "%isFullscreen = getWord(Canvas.getVideoMode(), 2);\n"
				   "%bitdepth = getWord(Canvas.getVideoMode(), 3);\n"
				   "%refreshRate = getWord(Canvas.getVideoMode(), 4);\n"
				   "@endtsexample\n\n"
                   
				   "@return String formatted with screen width, screen height, screen mode, bit depth, and refresh rate.")
{
	// Grab the video mode.
    if (!object->getPlatformWindow())
        return "";
    
    GFXVideoMode vm = object->getPlatformWindow()->getVideoMode();
    char* buf = Con::getReturnBuffer(vm.toString());
    return buf;
}

ConsoleMethod( GuiCanvas, getModeCount, S32, 2, 2,
				   "@brief Gets the number of modes available on this device.\n\n"
                   
				   "@param param Description\n\n"
                   
				   "@tsexample\n"
				   "%modeCount = Canvas.getModeCount()\n"
				   "@endtsexample\n\n"
                   
				   "@return The number of video modes supported by the device")
{
	if (!object->getPlatformWindow())
        return 0;
    
    // Grab the available mode list from the device.
    const Vector<GFXVideoMode>* const modeList =
    object->getPlatformWindow()->getGFXDevice()->getVideoModeList();
    
    // Return the number of resolutions.
    return (S32)modeList->size();
}

ConsoleMethod( GuiCanvas, getMode, const char*, 3, 3,
				   "@brief Gets information on the specified mode of this device.\n\n"
				   "@param modeId Index of the mode to get data from.\n"
				   "@return A video mode string given an adapter and mode index.\n\n"
				   "@see GuiCanvas::getVideoMode()")
{
	if (!object->getPlatformWindow())
        return 0;
    
    // Grab the available mode list from the device.
    const Vector<GFXVideoMode>* const modeList =
    object->getPlatformWindow()->getGFXDevice()->getVideoModeList();
    
    // Get the desired index and confirm it's valid.
    S32 idx = dAtoi(argv[2]);
    if((idx < 0) || (idx >= modeList->size()))
    {
        Con::errorf("GuiCanvas::getResolution - You requested an out of range index of %d. Please specify an index in the range [0, %d).", idx, modeList->size());
        return "";
    }
    
    // Great - we got something valid, so convert the videomode into a
    // string and return to the user.
    GFXVideoMode vm = (*modeList)[idx];
    
    char *retString = Con::getReturnBuffer(vm.toString());
    return retString;
}

/*! Use the getContent method to get the ID of the control which is being used as the current canvas content.
    @return Returns the ID of the current canvas content (a control), or 0 meaning the canvas is empty
*/
ConsoleMethodWithDocs( GuiCanvas, getContent, ConsoleInt, 2, 2, ())
{
    GuiControl *ctrl = object->getContentControl();
    if(ctrl)
        return ctrl->getId();
    return -1;
}

/*! Use the setContent method to set the control identified by handle as the current canvas content.
    @param handle The numeric ID or name of the control to be made the canvas contents.
    @return No return value
*/
ConsoleMethodWithDocs( GuiCanvas, setContent, ConsoleVoid, 3, 3, ( handle ))
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
    object->setContentControl(gui);
}

/*! Use the pushDialog method to open a dialog on a specific canvas layer, or in the same layer the last openned dialog. Newly placed dialogs placed in a layer with another dialog(s) will overlap the prior dialog(s).
    @param handle The numeric ID or name of the dialog to be opened.
    @param layer A integer value in the range [ 0 , inf ) specifying the canvas layer to place the dialog in.
    @return No return value
*/
ConsoleMethodWithDocs( GuiCanvas, pushDialog, ConsoleVoid, 3, 4, ( handle [ , layer ] ))
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
    object->pushDialogControl(gui, layer);
}

/*! Use the popDialog method to remove a currently showing dialog. If no handle is provided, the top most dialog is popped.
    @param handle The ID or a previously pushed dialog.
    @return No return value.
    @sa pushDialog, popLayer
*/
ConsoleMethodWithDocs( GuiCanvas, popDialog, ConsoleVoid, 2, 3, ( handle ))
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
        object->popDialogControl(gui);
    else
        object->popDialogControl();
}

/*! Use the popLayer method to remove (close) all dialogs in the specified canvas ???layer???.
    @param layer A integer value in the range [ 0 , inf ) specifying the canvas layer to clear.
    @return No return value.
    @sa pushDialog, popDialog
*/
ConsoleMethodWithDocs( GuiCanvas, popLayer, ConsoleVoid, 2, 3, ( layer ))
{
    S32 layer = 0;
    if (argc == 3)
        layer = dAtoi(argv[2]);
    
    object->popDialogControl(layer);
}

/*! Use the cursorOn method to enable the cursor.
    @return No return value
*/
ConsoleMethodWithDocs(GuiCanvas, cursorOn, ConsoleVoid, 2, 2, ())
{
    object->setCursorON(true);
}

/*! Use the cursorOff method to disable the cursor.
    @return No return value
*/
ConsoleMethodWithDocs(GuiCanvas, cursorOff, ConsoleVoid, 2, 2, ())
{
    object->setCursorON(false);
}

/*! Use the setCursor method to select the current cursor.
    @param cursorHandle The ID of a previously defined GuiCursor object.
    @return No return value
*/
ConsoleMethodWithDocs( GuiCanvas, setCursor, ConsoleVoid, 3, 3, ( cursorHandle ))
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
    object->setCursor(curs);
}

/*! 
*/
ConsoleMethodWithDocs( GuiCanvas, renderFront, ConsoleVoid, 3, 3, (bool enable))
{
    object->setRenderFront(dAtob(argv[2]));
}

/*! 
*/
ConsoleMethodWithDocs( GuiCanvas, showCursor, ConsoleVoid, 2, 2, ())
{
    object->showCursor(true);
}

/*! 
*/
ConsoleMethodWithDocs( GuiCanvas, hideCursor, ConsoleVoid, 2, 2, ())
{
    object->showCursor(false);
}

/*! 
*/
ConsoleMethodWithDocs( GuiCanvas, isCursorOn, ConsoleBool, 2, 2, ())
{
    return object->isCursorON();
}

/*! 
*/
ConsoleMethodWithDocs( GuiCanvas, setDoubleClickDelay, ConsoleVoid, 3, 3, ())
{
    object->setDoubleClickTime(dAtoi(argv[2]));
}

/*! 
*/
ConsoleMethodWithDocs( GuiCanvas, setDoubleClickMoveBuffer, ConsoleVoid, 4, 4, ())
{
    object->setDoubleClickWidth(dAtoi(argv[2]));
    object->setDoubleClickHeight(dAtoi(argv[3]));
}

/*! Use the repaint method to force the canvas to redraw all elements.
    @return No return value
*/
ConsoleMethodWithDocs( GuiCanvas, repaint, ConsoleVoid, 2, 2, ())
{
    object->paint();
}

/*! Use the reset method to reset the current canvas update region.
    @return No return value
*/
ConsoleMethodWithDocs( GuiCanvas, reset, ConsoleVoid, 2, 2, ())
{
    object->resetUpdateRegions();
}

/*! Use the getCursorPos method to retrieve the current position of the mouse pointer.
    @return Returns a vector containing the ???x y??? coordinates of the cursor in the canvas
*/
ConsoleMethodWithDocs( GuiCanvas, getCursorPos, ConsoleString, 2, 2, ())
{
    Point2I pos = object->getCursorPos();
    char * ret = Con::getReturnBuffer(32);
    dSprintf(ret, 32, "%d %d", pos.x, pos.y);
    return(ret);
}

/*! Use the setCursorPos method to set the position of the cursor in the cavas.
    @param position An \x y\ position vector specifying the new location of the cursor.
    @return No return value
*/
ConsoleMethodWithDocs( GuiCanvas, setCursorPos, ConsoleVoid, 3, 4, ( ))
{
    Point2I pos(0,0);
    
    if(argc == 4)
        pos.set(dAtoi(argv[2]), dAtoi(argv[3]));
    else
        dSscanf(argv[2], "%d %d", &pos.x, &pos.y);
    
    object->setCursorPos(pos);
}

/*! Gets the gui control under the mouse.
*/
ConsoleMethodWithDocs( GuiCanvas, getMouseControl, ConsoleInt, 2, 2, ())
{
    GuiControl* control = object->getMouseControl();
    if (control)
        return control->getId();
    
    return NULL;
}

//-----------------------------------------------------------------------------

/*! Sets the background color for the canvas.
    @param red The red value.
    @param green The green value.
    @param blue The blue value.
    @param alpha The alpha value.
    @return No return Value.
*/
ConsoleMethodWithDocs(GuiCanvas, setBackgroundColor, ConsoleVoid, 3, 6, (float red, float green, float blue, [float alpha = 1.0]))
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

/*! Gets the background color for the canvas.
    @return (float red / float green / float blue / float alpha) The background color for the canvas.
*/
ConsoleMethodWithDocs(GuiCanvas, getBackgroundColor, ConsoleString, 2, 2, (...))
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

/*! Sets whether to use the canvas background color or not.
    @param useBackgroundColor Whether to use the canvas background color or not.
    @return No return value.
*/
ConsoleMethodWithDocs(GuiCanvas, setUseBackgroundColor, ConsoleVoid, 3, 3, (...))
{
    // Fetch flag.
    const bool useBackgroundColor = dAtob(argv[2]);
    
    // Set the flag.
    object->setUseBackgroundColor( useBackgroundColor );
}

//-----------------------------------------------------------------------------

/*! Gets whether the canvas background color is in use or not.
    @return Whether the canvas background color is in use or not.
*/
ConsoleMethodWithDocs(GuiCanvas, getUseBackgroundColor, ConsoleBool, 2, 2, (...))
{
    // Get the flag.
    return object->getUseBackgroundColor();
}

/*! Sets the title to the provided string 
    @param windowTitle The desired title
    @return No Return Value
*/
ConsoleMethodWithDocs(GuiCanvas, setCanvasTitle, ConsoleVoid, 3, 3, (...))
{
    return object->setWindowTitle( argv[2] );
}

ConsoleMethodGroupEndWithDocs(GuiCanvas)

#endif



