//
//  guiControl_ScriptBinding.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-03-27.
//

#ifndef Torque2D_guiControl_ScriptBinding_h
#define Torque2D_guiControl_ScriptBinding_h

ConsoleMethod( GuiControl, addGuiControl, void, 3, 3, "(int controlId) Adds the gui control\n"
              "@param controlId integer ID of the control to add\n"
              "@return No Return value")
{
    
    GuiControl *ctrl = dynamic_cast<GuiControl *>(Sim::findObject(argv[2]));
    if(ctrl)
    {
        object->addObject(ctrl);
    }
    
}


ConsoleMethod(GuiControl, pointInControl, bool, 4,4,"(int x, int y) Check if point id in the control\n"
              "@param x Point x coordinate in parent coords\n"
              "@param y Point y coordinate in parent coords\n"
              "@return Returns true if the point is in the control, false otherwise")
{
    Point2I kPoint(dAtoi(argv[2]), dAtoi(argv[3]));
    return object->pointInControl(kPoint);
}


//-----------------------------------------------------------------------------

ConsoleMethod( GuiControl, getRoot, S32, 2, 2,
              "Get the canvas on which the control is placed.")
{
    GuiCanvas* pRoot		= object->getRoot();
    if(pRoot)
    {
        return pRoot->getId();
    }
    
    return 0;
}

//-----------------------------------------------------------------------------
//	Make Sure Child 1 is Ordered Just Under Child 2.
//-----------------------------------------------------------------------------
ConsoleMethod(GuiControl, reorderChild, void, 4,4," (child1, child2) uses simset reorder to push child 1 after child 2 - both must already be child controls of this control")
{
    GuiControl* pControl = dynamic_cast<GuiControl*>(Sim::findObject(dAtoi(argv[2])));
    GuiControl* pTarget	 = dynamic_cast<GuiControl*>(Sim::findObject(dAtoi(argv[3])));
    
    if(pControl && pTarget)
    {
        object->reOrder(pControl,pTarget);
    }
}

ConsoleMethod( GuiControl, getParent, S32, 2, 2, "() @return Returns the Id of the parent control")
{
    
    GuiControl* pParent		= object->getParent();
    if(pParent)
    {
        return pParent->getId();
    }
    
    return 0;
    
}

ConsoleMethod( GuiControl, setValue, void, 3, 3, "( value ) Use the setValue method to set the control specific value to value. Purpose and type varies by control type.\n"
              "@param value Some control specific value.\n"
              "@return No return value")
{
    object->setScriptValue(argv[2]);
}

ConsoleMethod( GuiControl, getValue, const char*, 2, 2, "() Use the getValue method to get the control-specific 'value' for this control.\n"
              "@return Returns a control-specific specific value. Varies by control")
{
    return object->getScriptValue();
}

ConsoleMethod( GuiControl, setActive, void, 3, 3, "( isActive ) Use the setActive method to (de)activate this control. Once active, a control can accept inputs. Controls automatically re-shade/skin themselves to reflect their active/inactive state.\n"
              "@param isActive A boolean value. f isActive is true, this control is activated, else it is set to inactive.\n"
              "@return No return value")
{
    object->setActive(dAtob(argv[2]));
}

ConsoleMethod( GuiControl, isActive, bool, 2, 2, "() Use the isActive method to determine if this control is active.\n"
              "An inactive control may visible, but will not accept inputs. It will also normally re-shade or re-skin itself to reflect its inactive state\n"
              "@return Returns true if this control is active.")
{
    return object->isActive();
}

ConsoleMethod( GuiControl, setVisible, void, 3, 3, "( isVisible ) Use the setVisible method to (un)hide this control.\n"
              "@param isVisible A boolean value. If true, the control will be made visible, otherwise the control will be hidden.\n"
              "@return No return value")
{
    object->setVisible(dAtob(argv[2]));
}

ConsoleMethod( GuiControl, makeFirstResponder, void, 3, 3, "( isFirst ) Use the makeFirstResponder method to force this control to become the first responder.\n"
              "@param isFirst A boolean value. If true, then this control become first reponder and at captures inputs before all other controls, excluding dialogs above this control.\n"
              "@return No return value")
{
    object->makeFirstResponder(dAtob(argv[2]));
}

ConsoleMethod( GuiControl, isVisible, bool, 2, 2, "() Use the isVisible method to determine if this control is visible.\n"
              "This can return true, even if the entire control covered by another. This merely means that the control will render if not covered\n"
              "@return Returns true if the control is visible.")
{
    return object->isVisible();
}

ConsoleMethod( GuiControl, isAwake, bool, 2, 2, "() Use the isAwake method to determine if this control is awake.\n"
              "@return Returns true if this control is awake and ready to display")
{
    return object->isAwake();
}

ConsoleMethod( GuiControl, setProfile, void, 3, 3, "(GuiControlProfile p) Sets the currently used from for the GuiControl\n"
              "@param p The profile you wish to set the control to use\n"
              "@return No return value")
{
    GuiControlProfile * profile;
    
    if(Sim::findObject(argv[2], profile))
        object->setControlProfile(profile);
}

ConsoleMethod( GuiControl, resize, void, 6, 6, "(int x, int y, int w, int h) Resizes the control to the given dimensions")
{
    Point2I newPos(dAtoi(argv[2]), dAtoi(argv[3]));
    Point2I newExt(dAtoi(argv[4]), dAtoi(argv[5]));
    object->resize(newPos, newExt);
}

ConsoleMethod( GuiControl, getPosition, const char*, 2, 2, "() @return A string set up as \"<pos.x> <pos.y>\"")
{
    char *retBuffer = Con::getReturnBuffer(64);
    const Point2I &pos = object->getPosition();
    dSprintf(retBuffer, 64, "%d %d", pos.x, pos.y);
    return retBuffer;
}
ConsoleMethod( GuiControl, getCenter, const char*, 2, 2, "() @return Returns center of control, as space seperated ints")
{
    char *retBuffer = Con::getReturnBuffer(64);
    const Point2I pos = object->getPosition();
    const Point2I ext = object->getExtent();
    Point2I center(pos.x + ext.x/2, pos.y + ext.y/2);
    dSprintf(retBuffer, 64, "%d %d", center.x, center.y);
    return retBuffer;
}

ConsoleMethod( GuiControl, setCenter, void, 4, 4, "(int x, int y) Sets control position, by center - coords are local not global\n"
              "@return No Return value.")
{
    const Point2I ext = object->getExtent();
    Point2I newpos(dAtoi(argv[2])-ext.x/2, dAtoi(argv[3])-ext.y/2);
    object->setPosition(newpos);
}

ConsoleMethod( GuiControl, getGlobalCenter, const char*, 2, 2, "@return Returns center of control, as space seperated ints")
{
    char *retBuffer = Con::getReturnBuffer(64);
    const Point2I tl(0,0);
    Point2I pos		 =	object->localToGlobalCoord(tl);
    const Point2I ext = object->getExtent();
    Point2I center(pos.x + ext.x/2, pos.y + ext.y/2);
    dSprintf(retBuffer, 64, "%d %d", center.x, center.y);
    return retBuffer;
}

ConsoleMethod( GuiControl, getGlobalPosition, const char*, 2, 2, "() @return Returns the control's position converted to global coordinates (position as space-separted integers)")
{
    char *retBuffer = Con::getReturnBuffer(64);
    const Point2I pos(0,0);
    Point2I gPos	=	object->localToGlobalCoord(pos);
    
    dSprintf(retBuffer, 64, "%d %d", gPos.x, gPos.y);
    return retBuffer;
}

ConsoleMethod( GuiControl, setPositionGlobal, void, 4, 4, "(int x, int y) Sets the control's position in global space\n"
              "@return No return value")
{
    //see if we can turn the x/y into ints directly,
    Point2I gPos(dAtoi(argv[2]), dAtoi(argv[3]));
    Point2I lPosOffset	=	object->globalToLocalCoord( gPos);
    
    lPosOffset += object->getPosition();
    
    object->setPosition( lPosOffset );
}

ConsoleMethod( GuiControl, setPosition, void, 4, 4, "(int x, int y) Sets the current control position in local space\n"
              "@return No Return Value.")
{
    object->setPosition( Point2I(dAtoi(argv[2]), dAtoi(argv[3])) );
}

ConsoleMethod( GuiControl, getExtent, const char*, 2, 2, "Get the width and height of the control.\n"
              "@return The height and width as a string with space-separated integers")
{
    char *retBuffer = Con::getReturnBuffer(64);
    const Point2I &ext = object->getExtent();
    dSprintf(retBuffer, 64, "%d %d", ext.x, ext.y);
    return retBuffer;
}

ConsoleMethod( GuiControl, setExtent, void, 4, 4, "(int width, int height) Sets the width & height of the control.\n"
              "@return No Return Value.")
{
    Point2I kExt(dAtoi(argv[2]), dAtoi(argv[3]));
    object->setExtent(kExt);
}

ConsoleMethod( GuiControl, getMinExtent, const char*, 2, 2, "() Get the minimum allowed size of the control.\n"
              "@return Returns the minimum extent as a string with space separated point values <width> <height>")
{
    char *retBuffer = Con::getReturnBuffer(64);
    const Point2I &minExt = object->getMinExtent();
    dSprintf(retBuffer, 64, "%d %d", minExt.x, minExt.y);
    return retBuffer;
}

ConsoleMethod( GuiControl, findHitControl, S32, 4, 4, "(int x, int y) Searches for the control at the given point\n"
              "@return Returns the Id of the control at the point")
{
    Point2I pos(dAtoi(argv[2]), dAtoi(argv[3]));
    GuiControl *hit = object->findHitControl(pos);
    return hit ? hit->getId() : 0;
}

ConsoleMethod( GuiControl, findHitControls, const char*, 6, 6, "(int x, int y, int width, int height) Find all visible child controls that intersect with the given rectangle\n"
              "@return Returns the Id of the control at the point")
{
    // Find hit controls.
    
    RectI bounds(dAtoi(argv[2]), dAtoi(argv[3]), dAtoi(argv[4]), dAtoi(argv[5]) );
    Vector< GuiControl* > controls;
    
    if( !object->findHitControls( bounds, controls ) )
        return "";
    
    // Create vector string.
    
    bool isFirst = true;
    StringBuilder str;
    for( U32 i = 0, num = controls.size(); i < num; ++ i )
    {
        if( !isFirst )
            str.append( ' ' );
        
        str.append( controls[ i ]->getIdString() );
        isFirst = false;
    }
    String s = str.end();
    
    // Return result.
    
    if ( s.compare( object->getIdString() ) == 0 )
        return "";
    
    char* buffer = Con::getReturnBuffer( s.size() );
    dStrcpy( buffer, s.c_str() );
    
    return buffer;
}


ConsoleMethod(GuiControl, setFirstResponder, void , 2, 2, "Sets this control as the first responder")
{
    object->setFirstResponder();
}

//------------------------------------------------------------------------------

ConsoleMethod( GuiControl, setProcessTicks, void, 2, 3, "( [tick = true] ) - This will set this object to either be processing ticks or not\n"
        "@return No return value." )
{
    if( argc == 3 )
        object->setProcessTicks( dAtob( argv[2] ) );
    else
        object->setProcessTicks();
}

#endif
