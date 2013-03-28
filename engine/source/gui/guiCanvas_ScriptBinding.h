//
//  guiCanvas_ScriptBinding.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-03-27.
//  Copyright (c) 2013 Michael Perry. All rights reserved.
//

#ifndef Torque2D_guiCanvas_ScriptBinding_h
#define Torque2D_guiCanvas_ScriptBinding_h

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
    return modeList->size();
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
    object->setContentControl(gui);
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
    object->pushDialogControl(gui, layer);
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
        object->popDialogControl(gui);
    else
        object->popDialogControl();
}

ConsoleMethod( GuiCanvas, popLayer, void, 2, 3, "( layer ) Use the popLayer method to remove (close) all dialogs in the specified canvas �layer�.\n"
              "@param layer A integer value in the range [ 0 , inf ) specifying the canvas layer to clear.\n"
              "@return No return value.\n"
              "@sa pushDialog, popDialog")
{
    S32 layer = 0;
    if (argc == 3)
        layer = dAtoi(argv[2]);
    
    object->popDialogControl(layer);
}

ConsoleMethod(GuiCanvas, cursorOn, void, 2, 2, "() Use the cursorOn method to enable the cursor.\n"
              "@return No return value")
{
    object->setCursorON(true);
}

ConsoleMethod(GuiCanvas, cursorOff, void, 2, 2, "() Use the cursorOff method to disable the cursor.\n"
              "@return No return value")
{
    object->setCursorON(false);
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
    object->setCursor(curs);
}

ConsoleMethod( GuiCanvas, renderFront, void, 3, 3, "(bool enable)")
{
    object->setRenderFront(dAtob(argv[2]));
}

ConsoleMethod( GuiCanvas, showCursor, void, 2, 2, "")
{
    object->showCursor(true);
}

ConsoleMethod( GuiCanvas, hideCursor, void, 2, 2, "")
{
    object->showCursor(false);
}

ConsoleMethod( GuiCanvas, isCursorOn, bool, 2, 2, "")
{
    return object->isCursorON();
}

ConsoleMethod( GuiCanvas, setDoubleClickDelay, void, 3, 3, "")
{
    object->setDoubleClickTime(dAtoi(argv[2]));
}

ConsoleMethod( GuiCanvas, setDoubleClickMoveBuffer, void, 4, 4, "")
{
    object->setDoubleClickWidth(dAtoi(argv[2]));
    object->setDoubleClickHeight(dAtoi(argv[3]));
}

ConsoleMethod( GuiCanvas, repaint, void, 2, 2, "() Use the repaint method to force the canvas to redraw all elements.\n"
              "@return No return value")
{
    object->paint();
}

ConsoleMethod( GuiCanvas, reset, void, 2, 2, "() Use the reset method to reset the current canvas update region.\n"
              "@return No return value")
{
    object->resetUpdateRegions();
}

ConsoleMethod( GuiCanvas, getCursorPos, const char*, 2, 2, "() Use the getCursorPos method to retrieve the current position of the mouse pointer.\n"
              "@return Returns a vector containing the �x y� coordinates of the cursor in the canvas")
{
    Point2I pos = object->getCursorPos();
    char * ret = Con::getReturnBuffer(32);
    dSprintf(ret, 32, "%d %d", pos.x, pos.y);
    return(ret);
}

ConsoleMethod( GuiCanvas, setCursorPos, void, 3, 4, "( ) Use the setCursorPos method to set the position of the cursor in the canvas.\n"
              "@param position An \"x y\" position vector specifying the new location of the cursor.\n"
              "@return No return value")
{
    Point2I pos(0,0);
    
    if(argc == 4)
        pos.set(dAtoi(argv[2]), dAtoi(argv[3]));
    else
        dSscanf(argv[2], "%d %d", &pos.x, &pos.y);
    
    object->setCursorPos(pos);
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

#endif
