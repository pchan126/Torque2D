//
//  guiCanvas_ScriptBinding.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-03-27.
//  Copyright (c) 2013 Michael Perry. All rights reserved.
//

#ifndef Torque2D_guiCanvas_ScriptBinding_h
#define Torque2D_guiCanvas_ScriptBinding_h

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
