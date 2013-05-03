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

#include "console/consoleTypes.h"
#include "console/console.h"
#include "console/consoleInternal.h"
#include "console/codeBlock.h"
#include "platform/event.h"
#include "graphics/gBitmap.h"
#include "graphics/gfxDevice.h"
#include "input/actionMap.h"
#include "gui/guiCanvas.h"
#include "gui/guiControl.h"
#include "gui/guiControl_ScriptBinding.h"
#include "gui/guiDefaultControlRender.h"
#include "gui/editor/guiEditCtrl.h"
#include "string/unicode.h"
#include "collection/vector.h"

#ifndef _FRAMEALLOCATOR_H_
#include "memory/frameAllocator.h"
#endif

IMPLEMENT_CONOBJECT( GuiControl );

//------------------------------------------------------------------------------


//used to locate the next/prev responder when tab is pressed
GuiControl *GuiControl::smPrevResponder = NULL;
GuiControl *GuiControl::smCurResponder = NULL;

GuiEditCtrl *GuiControl::smEditorHandle = NULL;

bool GuiControl::smDesignTime = false;

GuiControl::GuiControl() : mAddGroup( NULL ),
                            mLayer(0),
                            mBounds(0,0,64,64),
                            mMinExtent(8,2),
                            mProfile(NULL),
                            mLangTable(NULL),
                            mFirstResponder(NULL),
                            mVisible(true),
                            mActive(true),
                            mAwake(false),
                            mHorizSizing(horizResizeRight),
                            mVertSizing(vertResizeBottom),
                            mTooltipProfile(NULL),
                            mTipHoverTime(1000),
                            mIsContainer(false),
                            mCanResize(true),
                            mCanHit( true )
{
    mConsoleVariable     = StringTable->EmptyString;
    mAcceleratorKey      = StringTable->EmptyString;
    mLangTableName       = StringTable->EmptyString;
    
    mTooltip = StringTable->EmptyString;
    mRenderTooltipDelegate.bind( this, &GuiControl::defaultTooltipRender );
    
    mCanSaveFieldDictionary = false;
    mNotifyChildrenResized = true;
}

GuiControl::~GuiControl()
{
}

bool GuiControl::onAdd()
{
   // Let Parent Do Work.
   if(!Parent::onAdd())
      return false;

   // Grab the classname of this object
   const char *cName = getClassName();

   // if we're a pure GuiControl, then we're a container by default.
   if(dStrcmp("GuiControl", cName) == 0)
      mIsContainer = true;

   // Clamp to minExtent
   setWidth( getMax( mMinExtent.x, getWidth() ));
   setHeight( getMax( mMinExtent.y, getHeight() ));


   // Add to root group.
   Sim::getGuiGroup()->addObject(this);

    // Notify Script.
    if( isMethod("onAdd") )
        Con::executef(this, 1, "onAdd");
    
    GFXStateBlockDesc d;
    
    d.cullDefined = true;
    d.cullMode = GFXCullNone;
    d.zDefined = true;
    d.zEnable = false;
    
    mDefaultGuiSB = GFX->createStateBlock( d );
    
   // Return Success.
   return true;
}

void GuiControl::onChildAdded( GuiControl *child )
{
   // Base class does not make use of this
}

static EnumTable::Enums horzEnums[] =
{
    { GuiControl::horizResizeRight,      "right"     },
    { GuiControl::horizResizeWidth,      "width"     },
    { GuiControl::horizResizeLeft,       "left"      },
   { GuiControl::horizResizeCenter,     "center"    },
   { GuiControl::horizResizeRelative,   "relative"  }
};
static EnumTable gHorizSizingTable(5, &horzEnums[0]);

static EnumTable::Enums vertEnums[] =
{
    { GuiControl::vertResizeBottom,      "bottom"     },
    { GuiControl::vertResizeHeight,      "height"     },
    { GuiControl::vertResizeTop,         "top"        },
   { GuiControl::vertResizeCenter,      "center"     },
   { GuiControl::vertResizeRelative,    "relative"   }
};
static EnumTable gVertSizingTable(5, &vertEnums[0]);

void GuiControl::initPersistFields()
{
   Parent::initPersistFields();


   // Things relevant only to the editor.
   addGroup("Gui Editing");
   addField("isContainer",       TypeBool,      Offset(mIsContainer, GuiControl));
   endGroup("Gui Editing");

   // Parent Group.
   addGroup("GuiControl");

   addField("Profile",           TypeGuiProfile,	Offset(mProfile, GuiControl));
   addField("HorizSizing",       TypeEnum,			Offset(mHorizSizing, GuiControl), 1, &gHorizSizingTable);
   addField("VertSizing",        TypeEnum,			Offset(mVertSizing, GuiControl), 1, &gVertSizingTable);

   addField("Position",          TypePoint2I,		Offset(mBounds.point, GuiControl));
   addField("Extent",            TypePoint2I,		Offset(mBounds.extent, GuiControl));
   addField("MinExtent",         TypePoint2I,		Offset(mMinExtent, GuiControl));
   addField("Visible",           TypeBool,			Offset(mVisible, GuiControl));
   addDepricatedField("Modal");
   addDepricatedField("SetFirstResponder");

   addField("Variable",          TypeString,		Offset(mConsoleVariable, GuiControl));
   addField("Command",           TypeRealString,		Offset(mConsoleCommand, GuiControl));
   addField("AltCommand",        TypeRealString,		Offset(mAltConsoleCommand, GuiControl));
   addField("Accelerator",       TypeString,		Offset(mAcceleratorKey, GuiControl));
   addField("Active",			 TypeBool,			Offset(mActive, GuiControl));
   endGroup("GuiControl");	

   addGroup("ToolTip");
   addField("tooltipprofile",    TypeGuiProfile,	Offset(mTooltipProfile, GuiControl));
   addField("tooltip",           TypeRealString,		Offset(mTooltip, GuiControl));
   addField("tooltipWidth",      TypeS32,			Offset(mTooltipWidth, GuiControl));
   addField("hovertime",         TypeS32,			Offset(mTipHoverTime, GuiControl));
   endGroup("ToolTip");


   addGroup("Localization");
   addField("langTableMod",      TypeString,		Offset(mLangTableName, GuiControl));
   endGroup("Localization");
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

LangTable * GuiControl::getGUILangTable()
{
    if(mLangTable)
        return mLangTable;

    if(mLangTableName && *mLangTableName)
    {
        mLangTable = (LangTable *)getModLangTable((const UTF8*)mLangTableName);
        return mLangTable;
    }

    GuiControl *parent = getParent();
    if(parent)
        return parent->getGUILangTable();

    return NULL;
}

const UTF8 * GuiControl::getGUIString(S32 id)
{
    LangTable *lt = getGUILangTable();
    if(lt)
        return lt->getString(id);

    return NULL;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //


void GuiControl::addObject(SimObject *object)
{
   GuiControl *ctrl = dynamic_cast<GuiControl *>(object);
   if(!ctrl)
   {
      AssertWarn(0, "GuiControl::addObject: attempted to add NON GuiControl to set");
      return;
   }

   if(object->getGroup() == this)
      return;

    Parent::addObject(object);

   AssertFatal(!ctrl->isAwake(), "GuiControl::addObject: object is already awake before add");
   if(mAwake)
      ctrl->awaken();

  // If we are a child, notify our parent that we've been removed
  GuiControl *parent = ctrl->getParent();
  if( parent )
     parent->onChildAdded( ctrl );


}

void GuiControl::removeObject(SimObject *object)
{
   AssertFatal(mAwake == static_cast<GuiControl*>(object)->isAwake(), "GuiControl::removeObject: child control wake state is bad");
   if (mAwake)
      static_cast<GuiControl*>(object)->sleep();
    Parent::removeObject(object);
}

GuiControl *GuiControl::getParent()
{
    SimObject *obj = getGroup();
    if (GuiControl* gui = dynamic_cast<GuiControl*>(obj))
      return gui;
   return 0;
}

GuiCanvas *GuiControl::getRoot()
{
   GuiControl *root = NULL;
    GuiControl *parent = getParent();
   while (parent)
   {
      root = parent;
      parent = parent->getParent();
   }
   if (root)
      return dynamic_cast<GuiCanvas*>(root);
   else
      return NULL;
}

void GuiControl::inspectPreApply()
{
   if(smDesignTime && smEditorHandle)
      smEditorHandle->controlInspectPreApply(this);
   
   // The canvas never sleeps
   if(mAwake && dynamic_cast<GuiCanvas*>(this) == NULL )
   {
      onSleep(); // release all our resources.
      mAwake = true;
   }
}

void GuiControl::inspectPostApply()
{
   // Shhhhhhh, you don't want to wake the canvas!
   if(mAwake && dynamic_cast<GuiCanvas*>(this) == NULL )
   {
      mAwake = false;
      onWake();
   }
   
   if(smDesignTime && smEditorHandle)
      smEditorHandle->controlInspectPostApply(this);
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

Point2I GuiControl::localToGlobalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret += getPosition();
   GuiControl *walk = getParent();
   while(walk)
   {
      ret += walk->getPosition();
      walk = walk->getParent();
   }
   return ret;
}

Point2I GuiControl::globalToLocalCoord(const Point2I &src)
{
   Point2I ret = src;
   ret -= getPosition();
   GuiControl *walk = getParent();
   while(walk)
   {
      ret -= walk->getPosition();
      walk = walk->getParent();
   }
   return ret;
}

//----------------------------------------------------------------
bool GuiControl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
    const Point2I minExtent = getMinExtent();
    Point2I actualNewExtent = Point2I(getMax(minExtent.x, newExtent.x),
                                      getMax(minExtent.y, newExtent.y));
    
    // only do the child control resizing stuff if you really need to.
    const RectI bounds = getBounds();
    
    // If we didn't size anything, return false to indicate such
    bool extentChanged = (actualNewExtent != bounds.extent);
    bool positionChanged = (newPosition != bounds.point);
    if (!extentChanged && !positionChanged )
        return false;
    
    // Update Position
    if ( positionChanged )
        mBounds.point = newPosition;
    
    // Update Extent
    if( extentChanged )
    {
        //call set update both before and after
        setUpdate();
        
        mBounds.extent = actualNewExtent;
        
        // Obey the flag!
        // Could be set if we are resizing in response to a child resizing!
        if ( mNotifyChildrenResized )
        {
            iterator i;
            for(i = begin(); i != end(); i++)
            {
                GuiControl *ctrl = static_cast<GuiControl *>(*i);
                ctrl->parentResized(RectI(bounds.point, bounds.extent), RectI(newPosition,actualNewExtent));
            }
        }
        
        GuiControl *parent = getParent();
        if (parent)
            parent->childResized(this);
        setUpdate();
    }

    return extentChanged;
}
//-----------------------------------------------------------------------------

bool GuiControl::setPosition( const Point2I &newPosition )
{
    return resize( newPosition, mBounds.extent );
}

//-----------------------------------------------------------------------------

bool GuiControl::setExtent( const Point2I &newExtent )
{
    return resize( mBounds.point, newExtent );
}

//-----------------------------------------------------------------------------

bool GuiControl::setBounds( const RectI &newBounds )
{
    return resize( newBounds.point, newBounds.extent );
}

void GuiControl::setLeft( S32 newLeft )
{
   resize( Point2I( newLeft, getPosition().y), getExtent() );
}

void GuiControl::setTop( S32 newTop )
{
   resize( Point2I( getPosition().x, newTop ), getExtent() );
}

void GuiControl::setWidth( S32 newWidth )
{
   resize( getPosition(), Point2I( newWidth, getHeight() ) );
}

void GuiControl::setHeight( S32 newHeight )
{
   resize( getPosition(), Point2I( getWidth(), newHeight ) );
}

void GuiControl::childResized(GuiControl *child)
{
   // default to do nothing...
}

void GuiControl::parentResized(const RectI &oldParentRect, const RectI &newParentRect)
{
    Point2I newPosition = getPosition();
    Point2I newExtent = getExtent();
    
	S32 deltaX = newParentRect.extent.x - oldParentRect.extent.x;
 	S32 deltaY = newParentRect.extent.y - oldParentRect.extent.y;
    
	if (mHorizSizing == horizResizeCenter)
        newPosition.x = (newParentRect.extent.x - getWidth()) >> 1;
	else if (mHorizSizing == horizResizeWidth)
		newExtent.x += deltaX;
	else if (mHorizSizing == horizResizeLeft)
        newPosition.x += deltaX;
    else if (mHorizSizing == horizResizeRelative && oldParentRect.extent.x != 0)
    {
        S32 newLeft = mRoundToNearest( ( F32( newPosition.x ) / F32( oldParentRect.extent.x ) ) * F32( newParentRect.extent.x ) );
        S32 newWidth = mRoundToNearest( ( F32( newExtent.x ) / F32( oldParentRect.extent.x ) ) * F32( newParentRect.extent.x ) );
        
        newPosition.x = newLeft;
        newExtent.x = newWidth;
    }
    
	if (mVertSizing == vertResizeCenter)
        newPosition.y = (newParentRect.extent.y - getHeight()) >> 1;
	else if (mVertSizing == vertResizeHeight)
		newExtent.y += deltaY;
	else if (mVertSizing == vertResizeTop)
        newPosition.y += deltaY;
    else if(mVertSizing == vertResizeRelative && oldParentRect.extent.y != 0)
    {
        S32 newTop = mRoundToNearest( ( F32( newPosition.y ) / F32( oldParentRect.extent.y ) ) * F32( newParentRect.extent.y ) );
        S32 newHeight = mRoundToNearest( ( F32( newExtent.y ) / F32( oldParentRect.extent.y ) ) * F32( newParentRect.extent.y ) );
        
        newPosition.y = newTop;
        newExtent.y = newHeight;
    }
    
    // Resizing Re factor [9/18/2006]
    // Only resize if our minExtent is satisfied with it.
    Point2I minExtent = getMinExtent();
    if( newExtent.x >= minExtent.x && newExtent.y >= minExtent.y )
        resize(newPosition, newExtent);
}
//----------------------------------------------------------------

void GuiControl::onRender(Point2I offset, const RectI &updateRect)
{
    RectI ctrlRect(offset, getExtent());

    GFX->getDrawUtil()->setBitmapModulation( mProfile->mFontColor );
    //if opaque, fill the update rect with the fill color
    if (mProfile->mOpaque)
        GFX->getDrawUtil()->drawRectFill( ctrlRect, mProfile->mFillColor );

    //if there's a border, draw the border
    if (mProfile->mBorder)
        renderBorder(ctrlRect, mProfile);

    renderChildControls(offset, updateRect);
}

bool GuiControl::defaultTooltipRender( const Point2I &hoverPos, const Point2I &cursorPos, const char* tipText )
{
#ifndef TORQUE_OS_IOS
    // Short Circuit.
    if (!mAwake)
        return false;
    
    if ( dStrlen( mTooltip ) == 0 && ( tipText == NULL || dStrlen( tipText ) == 0 ) )
        return false;
    
    String renderTip( mTooltip );
    if ( tipText != NULL )
        renderTip = tipText;
    
    // Need to have root.
    GuiCanvas *root = getRoot();
    if ( !root )
        return false;
    
    GFont *font = mTooltipProfile->mFont;
    
    // Support for multi-line tooltip text...
    
    Vector<U32> startLineOffsets, lineLengths;
    
    font->wrapString( renderTip, U32_MAX, startLineOffsets, lineLengths );
    
    // The width is the longest line.
    U32 tipWidth = 0;
    for ( U32 i = 0; i < lineLengths.size(); i++ )
    {
        U32 width = font->getStrNWidth( renderTip.c_str() + startLineOffsets[i], lineLengths[i] );
        
        if ( width > tipWidth )
            tipWidth = width;
    }
    
    // The height is the number of lines times the height of the font.
    U32 tipHeight = lineLengths.size() * font->getHeight();
    
    // Vars used:
    // Screensize (for position check)
    // Offset to get position of cursor
    // textBounds for text extent.
    Point2I screensize = getRoot()->getWindowSize();
    Point2I offset = hoverPos;
    Point2I textBounds;
    
    // Offset below cursor image
    offset.y += 20; // TODO: Attempt to fix?: root->getCursorExtent().y;
    
    // Create text bounds...
    
    // Pixels above/below the text
    const U32 vMargin = 2;
    // Pixels left/right of the text
    const U32 hMargin = 4;
    
    textBounds.x = tipWidth + hMargin * 2;
    textBounds.y = tipHeight + vMargin * 2;
    
    // Check position/width to make sure all of the tooltip will be rendered
    // 5 is given as a buffer against the edge
    if ( screensize.x < offset.x + textBounds.x + 5 )
        offset.x = screensize.x - textBounds.x - 5;
    
    // And ditto for the height
    if ( screensize.y < offset.y + textBounds.y + 5 )
        offset.y = hoverPos.y - textBounds.y - 5;
    
    RectI oldClip = GFX->getClipRect();
    
    // Set rectangle for the box, and set the clip rectangle.
    RectI rect( offset, textBounds );
    GFX->setClipRect( rect );
    
    // Draw Filler bit, then border on top of that
    GFX->getDrawUtil()->drawRectFill( rect, mTooltipProfile->mFillColor );
    GFX->getDrawUtil()->drawRect( rect, mTooltipProfile->mBorderColor );
    
    // Draw the text centered in the tool tip box...
    
    GFX->getDrawUtil()->setBitmapModulation( mTooltipProfile->mFontColor );
    
    for ( U32 i = 0; i < lineLengths.size(); i++ )
    {
        Point2I start( hMargin, vMargin + i * font->getHeight() );
        const UTF8 *line = renderTip.c_str() + startLineOffsets[i];
        U32 lineLen = lineLengths[i];
        
        GFX->getDrawUtil()->drawTextN( font, start + offset, line, lineLen, mProfile->mFontColors );
    }
    
    GFX->setClipRect( oldClip );
#endif
    return true;
}

void GuiControl::renderChildControls(Point2I offset, const RectI &updateRect)
{
   // offset is the upper-left corner of this control in screen coordinates
   // updateRect is the intersection rectangle in screen coords of the control
   // hierarchy.  This can be set as the clip rectangle in most cases.
   RectI clipRect = updateRect;

   S32 size = objectList.size();
   S32 size_cpy = size;
    //-Mat look through our vector all normal-like, trying to use an iterator sometimes gives us
   //bad cast on good objects
   for( S32 count = 0; count < objectList.size(); count++ )
   {
      GuiControl *ctrl = (GuiControl *)objectList[count];
      if( ctrl == NULL ) {
          Con::errorf( "GuiControl::renderChildControls() object %i is NULL", count );
        continue;
      }
      if (ctrl->mVisible)
      {
         Point2I childPosition = offset + ctrl->getPosition();
         RectI childClip(childPosition, ctrl->getExtent());

         if (childClip.intersect(clipRect))
         {
            GFX->setClipRect(childClip);
            ctrl->onRender(childPosition, childClip);
         }
      }
      size_cpy = objectList.size(); //	CHRIS: i know its wierd but the size of the list changes sometimes during execution of this loop
      if(size != size_cpy)
      {
          size = size_cpy;
          count--;	//	CHRIS: just to make sure one wasnt skipped.
      }
   }
}

void GuiControl::setUpdateRegion(Point2I pos, Point2I ext)
{
   Point2I upos = localToGlobalCoord(pos);
   GuiCanvas *root = getRoot();
   if (root)
   {
      root->addUpdateRegion(upos, ext);
   }
}

void GuiControl::setUpdate()
{
   setUpdateRegion(Point2I(0,0), getExtent());
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::awaken()
{
   AssertFatal(!mAwake, "GuiControl::awaken: control is already awake");
   if(mAwake)
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);

      AssertFatal(!ctrl->isAwake(), "GuiControl::awaken: child control is already awake");
      if(!ctrl->isAwake())
         ctrl->awaken();
   }

   AssertFatal(!mAwake, "GuiControl::awaken: should not be awake here");
   if(!mAwake)
   {
      if(!onWake())
      {
         Con::errorf(ConsoleLogEntry::General, "GuiControl::awaken: failed onWake for obj: %s", getName());
         AssertFatal(0, "GuiControl::awaken: failed onWake");
         deleteObject();
      }
   }
}

void GuiControl::sleep()
{
   AssertFatal(mAwake, "GuiControl::sleep: control is not awake");
   if(!mAwake)
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);

      AssertFatal(ctrl->isAwake(), "GuiControl::sleep: child control is already asleep");
      if(ctrl->isAwake())
         ctrl->sleep();
   }

   AssertFatal(mAwake, "GuiControl::sleep: should not be asleep here");
   if(mAwake)
      onSleep();
}

void GuiControl::preRender()
{
   AssertFatal(mAwake, "GuiControl::preRender: control is not awake");
   if(!mAwake)
      return;

   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->preRender();
   }
   onPreRender();
}

//-----------------------------------------------------------------------------

void GuiControl::onDeleteNotify(SimObject *object)
{
    if( object == mProfile )
    {
        GuiControlProfile* profile;
        Sim::findObject( "GuiDefaultProfile", profile );
        
        if ( profile == mProfile )
            mProfile = NULL;
        else
            setControlProfile( profile );
    }
    if (object == mTooltipProfile)
    {
        GuiControlProfile* profile;
        Sim::findObject( "GuiDefaultProfile", profile );
        
        if ( profile == mTooltipProfile )
            mTooltipProfile = NULL;
        else
            setTooltipProfile( profile );
    }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiControl::onWake()
{
   AssertFatal( !mAwake, "GuiControl::onWake: control is already awake" );
   if( mAwake )
      return false;

   // [tom, 4/18/2005] Cause mLangTable to be refreshed in case it was changed
   mLangTable = NULL;

   // Grab the classname of this object
   const char *cName = getClassName();

   //make sure we have a profile
   if( !mProfile )
   {
      // Ensure the classname is a valid name...
      if( cName && cName[0] )
      {
         S32 pos = 0;

         for( pos = 0; pos <= (S32)dStrlen( cName ); pos++ )
            if( !dStrncmp( cName + pos, "Ctrl", 4 ) )
               break;

         if( pos != 0 ) {
            char buff[255];
            dStrncpy( buff, cName, pos );
            buff[pos] = '\0';
            dStrcat( buff, "Profile\0" );

            SimObject *obj = Sim::findObject( buff );

            if( obj )
               mProfile = dynamic_cast<GuiControlProfile*>( obj );
         }
      }

      // Ok lets check to see if that worked
      if( !mProfile ) {
         SimObject *obj = Sim::findObject( "GuiDefaultProfile" );

         if( obj )
            mProfile = dynamic_cast<GuiControlProfile*>(obj);
      }

      AssertFatal( mProfile, avar( "GuiControl: %s created with no profile.", getName() ) );
   }

   //set the flag
   mAwake = true;

   //set the layer
   GuiCanvas *root = getRoot();
   AssertFatal(root, "Unable to get the root Canvas.");
   GuiControl *parent = getParent();
   if (parent && parent != root)
      mLayer = parent->mLayer;

   //make sure the first responder exists
   if (! mFirstResponder)
      mFirstResponder = findFirstTabable();

   //see if we should force this control to be the first responder
   //if (mProfile->mTabable && mProfile->mCanKeyFocus)
   //   setFirstResponder();

   //increment the profile
   mProfile->incRefCount();

   // Only invoke script callbacks if we have a namespace in which to do so
   // This will suppress warnings
   if( isMethod("onWake") )
      Con::executef(this, 1, "onWake");

   return true;
}

void GuiControl::onSleep()
{
   AssertFatal(mAwake, "GuiControl::onSleep: control is not awake");
   if(!mAwake)
      return;

   //decrement the profile referrence
   if( mProfile != NULL )
      mProfile->decRefCount();
   clearFirstResponder();
   mouseUnlock();

   // Only invoke script callbacks if we have a namespace in which to do so
   // This will suppress warnings
   if( isMethod("onSleep") )
      Con::executef(this, 1, "onSleep");

   // Set Flag
   mAwake = false;
}

void GuiControl::setControlProfile(GuiControlProfile *prof)
{
   AssertFatal(prof, "GuiControl::setControlProfile: invalid profile");
   if(prof == mProfile)
      return;
   if(mAwake)
      mProfile->decRefCount();
   mProfile = prof;
   if(mAwake)
      mProfile->incRefCount();

}

void GuiControl::onPreRender()
{
   // do nothing.
}
//-----------------------------------------------------------------------------
//	checks up the parent hierarchy - if anyone above us is not savable returns false
//	otherwise, returns true.
//-----------------------------------------------------------------------------
bool	GuiControl::getCanSaveParent()
{
   GuiControl *walk = this;
   while(walk)
   {
      if(!walk->getCanSave())
         return false;

      walk = walk->getParent();
   }

   return true;
}


////////////////////////////////////////////////////////////////////////////////////////////////////
//	checks out mCanSave flag, if true just passes along to our parent, 
//	if false, then we return without writing.  Note, also, that 
//	if our parent is not writeable, then we should not be writable...
////////////////////////////////////////////////////////////////////////////////////////////////////
void GuiControl::write(Stream &stream, U32 tabStop, U32 flags)
{
   //note: this will return false if either we, or any of our parents, are non-save controls
   bool bCanSave	=	getCanSaveParent();
   if(bCanSave)
   {
      Parent::write(stream, tabStop, flags);
   }

}


void GuiControl::onRemove()
{
   clearFirstResponder();

   Parent::onRemove();

  // If we are a child, notify our parent that we've been removed
  GuiControl *parent = getParent();
  if( parent )
     parent->onChildRemoved( this );
}

void GuiControl::onChildRemoved( GuiControl *child )
{
   // Base does nothing with this
}

//-----------------------------------------------------------------------------

void GuiControl::onGroupRemove()
{
    // If we have a first responder in our hierarchy,
    // make sure to kill it off.
    
    if( mFirstResponder )
        mFirstResponder->clearFirstResponder();
    else
    {
        GuiCanvas* root = getRoot();
        if( root )
        {
            GuiControl* firstResponder = root->getFirstResponder();
            if( firstResponder && firstResponder->isChildOfGroup( this ) )
                firstResponder->clearFirstResponder();
        }
    }
    
    // If we are awake, put us to sleep.
    
    if( isAwake() )
        sleep();
}
// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

const char *GuiControl::getScriptValue()
{
   return NULL;
}

void GuiControl::setScriptValue(const char *value)
{
}

void GuiControl::setConsoleVariable(const char *variable)
{
   if (variable)
   {
      mConsoleVariable = StringTable->insert(variable);
   }
   else
   {
      mConsoleVariable = StringTable->EmptyString;
   }
}
  
//-----------------------------------------------------------------------------
//	finds and returns the first immediate child of ours whose
//	internal name matches the passed String. returns Null if not found.
//-----------------------------------------------------------------------------
void GuiControl::setConsoleCommand( const String& newCmd )
{
    Con::printf("GuiControl::setConsoleCommand %s", newCmd.c_str());
    mConsoleCommand = newCmd;
}

const char * GuiControl::getConsoleCommand()
{
    return mConsoleCommand;
}

void GuiControl::setSizing(S32 horz, S32 vert)
{
    mHorizSizing = horz;
    mVertSizing = vert;
}


void GuiControl::setVariable(const char *value)
{
   if (mConsoleVariable[0])
      Con::setVariable(mConsoleVariable, value);
}

void GuiControl::setIntVariable(S32 value)
{
   if (mConsoleVariable[0])
      Con::setIntVariable(mConsoleVariable, value);
}

void GuiControl::setFloatVariable(F32 value)
{
   if (mConsoleVariable[0])
      Con::setFloatVariable(mConsoleVariable, value);
}

const char * GuiControl::getVariable()
{
   if (mConsoleVariable[0])
      return Con::getVariable(mConsoleVariable);
   else return NULL;
}

S32 GuiControl::getIntVariable()
{
   if (mConsoleVariable[0])
      return Con::getIntVariable(mConsoleVariable);
   else return 0;
}

F32 GuiControl::getFloatVariable()
{
   if (mConsoleVariable[0])
      return Con::getFloatVariable(mConsoleVariable);
   else return 0.0f;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiControl::cursorInControl()
{
   GuiCanvas *root = getRoot();
   if (! root) return false;

   Point2I pt = root->getCursorPos();
   Point2I offset = localToGlobalCoord(Point2I(0, 0));
   if (pt.x >= offset.x && pt.y >= offset.y &&
      pt.x < offset.x + getWidth() && pt.y < offset.y + getHeight())
   {
      return true;
   }
   else
   {
      return false;
   }
}

bool GuiControl::pointInControl(const Point2I& parentCoordPoint)
{
   S32 xt = parentCoordPoint.x - getPosition().x;
   S32 yt = parentCoordPoint.y - getPosition().y;
   return xt >= 0 && yt >= 0 && xt < getWidth() && yt < getHeight();
}

//=============================================================================
//    Properties.
//=============================================================================
// MARK: ---- Properties ----

//-----------------------------------------------------------------------------

void GuiControl::setTooltipProfile( GuiControlProfile *prof )
{
    AssertFatal( prof, "GuiControl::setTooltipProfile: invalid profile" );
    
    if ( prof == mTooltipProfile )
        return;
    
    bool skipAwaken = false;
    
    if ( mTooltipProfile == NULL )
        skipAwaken = true;
    
//    if( mTooltipProfile )
//        mTooltipProfile->decUseCount();
    
    if ( mAwake && mTooltipProfile )
        mTooltipProfile->decRefCount();
    
    // Clear the delete notification we previously set up
    if ( mTooltipProfile )
        clearNotify( mTooltipProfile );
    
    mTooltipProfile = prof;
//    mTooltipProfile->incUseCount();
    if ( mAwake )
        mTooltipProfile->incRefCount();
    
    // Make sure that the new profile will notify us when it is deleted
    if ( mTooltipProfile )
        deleteNotify( mTooltipProfile );
    
//    // force an update when the profile is changed
//    if ( mAwake && !skipAwaken )
//    {
//        sleep();
//        
//        if( !Sim::isShuttingDown() )
//            awaken();
//    }
}




// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

bool GuiControl::isMouseLocked()
{
   GuiCanvas *root = getRoot();
   return root ? root->getMouseLockedControl() == this : false;
}

void GuiControl::mouseLock(GuiControl *lockingControl)
{
   GuiCanvas *root = getRoot();
   if (root)
      root->mouseLock(lockingControl);
}

void GuiControl::mouseLock()
{
   GuiCanvas *root = getRoot();
   if (root)
      root->mouseLock(this);
}

void GuiControl::mouseUnlock()
{
   GuiCanvas *root = getRoot();
   if (root)
      root->mouseUnlock(this);
}

bool GuiControl::onInputEvent(const InputEventInfo &event)
{
    // Do nothing by default...
   return( false );
}

void GuiControl::onMouseUp(const GuiEvent &event)
{
}

void GuiControl::onMouseDown(const GuiEvent &event)
{
}

void GuiControl::onMouseMove(const GuiEvent &event)
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      parent->onMouseMove( event );
}

void GuiControl::onMouseDragged(const GuiEvent &event)
{
}

void GuiControl::onMouseEnter(const GuiEvent &)
{
}

void GuiControl::onMouseLeave(const GuiEvent &)
{
}

bool GuiControl::onMouseWheelUp( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelUp( event );
   else
      return false;
}

bool GuiControl::onMouseWheelDown( const GuiEvent &event )
{
   //if this control is a dead end, make sure the event stops here
   if ( !mVisible || !mAwake )
      return true;

   //pass the event to the parent
   GuiControl *parent = getParent();
   if ( parent )
      return parent->onMouseWheelDown( event );
   else
      return false;
}

void GuiControl::onRightMouseDown(const GuiEvent &)
{
}

void GuiControl::onRightMouseUp(const GuiEvent &)
{
}

void GuiControl::onRightMouseDragged(const GuiEvent &)
{
}

void GuiControl::onMiddleMouseDown(const GuiEvent &)
{
}

void GuiControl::onMiddleMouseUp(const GuiEvent &)
{
}

void GuiControl::onMiddleMouseDragged(const GuiEvent &)
{
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadButtonDown(const GuiEvent &event)
{
    return onKeyDown(event);
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadButtonUp(const GuiEvent &event)
{
    return onKeyUp(event);
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisUp(const GuiEvent &event)
{
    //pass the event to the parent
    GuiControl *parent = getParent();
    if (parent)
    {
        return parent->onGamepadAxisUp(event);
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisDown(const GuiEvent &event)
{
    //pass the event to the parent
    GuiControl *parent = getParent();
    if (parent)
    {
        return parent->onGamepadAxisDown(event);
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisLeft(const GuiEvent &event)
{
    //pass the event to the parent
    GuiControl *parent = getParent();
    if (parent)
    {
        return parent->onGamepadAxisLeft(event);
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadAxisRight(const GuiEvent &event)
{
    //pass the event to the parent
    GuiControl *parent = getParent();
    if (parent)
    {
        return parent->onGamepadAxisRight(event);
    }
    else
    {
        return false;
    }
}

//-----------------------------------------------------------------------------

bool GuiControl::onGamepadTrigger(const GuiEvent &event)
{
    //pass the event to the parent
    GuiControl *parent = getParent();
    if (parent)
    {
        return parent->onGamepadTrigger(event);
    }
    else
    {
        return false;
    }
}


//-----------------------------------------------------------------------------

bool GuiControl::acceptsAsChild( SimObject* object ) const
{
    return ( dynamic_cast< GuiControl* >( object ) != NULL );
}

//-----------------------------------------------------------------------------

GuiControl* GuiControl::findHitControl(const Point2I &pt, S32 initialLayer)
{
    iterator i = end(); // find in z order (last to first)
    
    while (i != begin())
    {
        i--;
        GuiControl *ctrl = static_cast<GuiControl *>(*i);
        if (initialLayer >= 0 && ctrl->mLayer > initialLayer)
        {
            continue;
        }
        
        else if (ctrl->mVisible && ctrl->mCanHit && ctrl->pointInControl(pt))
        {
            Point2I ptemp = pt - ctrl->getPosition();
            GuiControl *hitCtrl = ctrl->findHitControl(ptemp);
            
            if ( hitCtrl->mProfile->mModal )
                return hitCtrl;
        }
    }
    
    if( mCanHit )
        return this;
    return NULL;
}

//-----------------------------------------------------------------------------

bool GuiControl::findHitControls( const RectI& rect, Vector< GuiControl* >& outResult, U32 flags, S32 initialLayer, U32 depth )
{
    if( !mVisible )
        return false;
    else if( !mCanHit && flags & HIT_NoCanHitNoRecurse )
        return false;
    
    // Check for hit.  If not full-box, always counts.
    
    bool isHit = mVisible;
    if( flags & HIT_FullBoxOnly )
    {
        RectI rectInParentSpace = rect;
        rectInParentSpace.point += getPosition();
        
        isHit &= rectInParentSpace.contains( getBounds() );
    }
    else
        isHit &= mCanHit;
    
    // If we have a hit and should not recurse into children,
    // return us.
    
    if( isHit && flags & HIT_ParentPreventsChildHit && depth > 0 )
    {
        outResult.push_back( this );
        return true;
    }
    
    // Check child controls.
    
    bool haveFoundChild = false;
    iterator i = end();
    
    while( i != begin() )
    {
        i --;
        
        GuiControl* ctrl = static_cast< GuiControl* >( *i );
        if( initialLayer >= 0 && ctrl->mLayer > initialLayer )
            continue;
        
        if( ctrl->getBounds().overlaps( rect ) )
        {
            RectI transposedRect = rect;
            transposedRect.point -= ctrl->getPosition();
            
            if( ctrl->findHitControls( transposedRect, outResult, flags, -1, depth + 1 ) )
                haveFoundChild = true;
        }
    }
    
    if( ( !haveFoundChild || flags & HIT_AddParentHits ) && isHit )
    {
        outResult.push_back( this );
        return true;
    }
    
    return haveFoundChild;
}

//-----------------------------------------------------------------------------


GuiControl* GuiControl::findFirstTabable()
{
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findFirstTabable();
      if (tabCtrl)
      {
         mFirstResponder = tabCtrl;
         return tabCtrl;
      }
   }

   //nothing was found, therefore, see if this ctrl is tabable
   return ( mProfile != NULL ) ? ( ( mProfile->mTabable && mAwake && mVisible ) ? this : NULL ) : NULL;
}

GuiControl* GuiControl::findLastTabable(bool firstCall)
{
   //if this is the first call, clear the global
   if (firstCall)
      smPrevResponder = NULL;

   //if this control is tabable, set the global
   if (mProfile->mTabable)
      smPrevResponder = this;

   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->findLastTabable(false);
   }

   //after the entire tree has been traversed, return the last responder found
   mFirstResponder = smPrevResponder;
   return smPrevResponder;
}

GuiControl *GuiControl::findNextTabable(GuiControl *curResponder, bool firstCall)
{
   //if this is the first call, clear the global
   if (firstCall)
      smCurResponder = NULL;

   //first find the current responder
   if (curResponder == this)
      smCurResponder = this;

   //if the first responder has been found, return the very next *tabable* control
   else if ( smCurResponder && mProfile->mTabable && mAwake && mVisible && mActive )
      return( this );

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findNextTabable(curResponder, false);
      if (tabCtrl) break;
   }
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

GuiControl *GuiControl::findPrevTabable(GuiControl *curResponder, bool firstCall)
{
   if (firstCall)
      smPrevResponder = NULL;

   //if this is the current reponder, return the previous one
   if (curResponder == this)
      return smPrevResponder;

   //else if this is a responder, store it in case the next found is the current responder
   else if ( mProfile->mTabable && mAwake && mVisible && mActive )
      smPrevResponder = this;

   //loop through, checking each child to see if it is the one that follows the firstResponder
   GuiControl *tabCtrl = NULL;
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      tabCtrl = ctrl->findPrevTabable(curResponder, false);
      if (tabCtrl) break;
   }
   mFirstResponder = tabCtrl;
   return tabCtrl;
}

//-----------------------------------------------------------------------------

void GuiControl::onLoseFirstResponder()
{
    // Since many controls have visual cues when they are the firstResponder...
    setUpdate();

    // Notify Script.
    if( isMethod("onLoseFirstResponder") )
        Con::executef(this, 1, "onLoseFirstResponder");
}

//-----------------------------------------------------------------------------

void GuiControl::onGainFirstResponder()
{
    // Since many controls have visual cues when they are the firstResponder...
    this->setUpdate();

    // Notify Script.
    if( isMethod("onGainFirstResponder") )
        Con::executef(this, 1, "onGainFirstResponder");
}

//-----------------------------------------------------------------------------

bool GuiControl::ControlIsChild(GuiControl *child)
{
   //function returns true if this control, or one of it's children is the child control
   if (child == this)
      return true;

   //loop through, checking each child to see if it is ,or contains, the firstResponder
   iterator i;
   for (i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      if (ctrl->ControlIsChild(child)) return true;
   }

   //not found, therefore false
   return false;
}

bool GuiControl::isFirstResponder()
{
   GuiCanvas *root = getRoot();
   return root && root->getFirstResponder() == this;
}

void GuiControl::setFirstResponder( GuiControl* firstResponder )
{
   if ( firstResponder && firstResponder->mProfile->mCanKeyFocus )
      mFirstResponder = firstResponder;

   GuiControl *parent = getParent();
   if ( parent )
      parent->setFirstResponder( firstResponder );
}

void GuiControl::setFirstResponder()
{
    if ( mAwake && mVisible )
    {
       GuiControl *parent = getParent();
       if (mProfile->mCanKeyFocus == true && parent != NULL )
      {
         parent->setFirstResponder(this);

         // Since many controls have visual cues when they are the firstResponder...
         this->setUpdate();	
      }
    }
}


void GuiControl::clearFirstResponder()
{
   GuiControl *parent = this;
   while((parent = parent->getParent()) != NULL)
   {
      if(parent->mFirstResponder == this)
         parent->mFirstResponder = NULL;
      else
         break;
   }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::buildAcceleratorMap()
{
   //add my own accel key
   addAcceleratorKey();

   //add all my childrens keys
   iterator i;
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->buildAcceleratorMap();
   }
}

void GuiControl::addAcceleratorKey()
{
   //see if we have an accelerator
   if (mAcceleratorKey == StringTable->EmptyString)
      return;

   EventDescriptor accelEvent;
   ActionMap::createEventDescriptor(mAcceleratorKey, &accelEvent);

   //now we have a modifier, and a key, add them to the canvas
   GuiCanvas *root = getRoot();
   if (root)
      root->addAcceleratorKey(this, 0, accelEvent.eventCode, accelEvent.flags);
}

void GuiControl::acceleratorKeyPress(U32 index)
{
   onAction();
}

void GuiControl::acceleratorKeyRelease(U32 index)
{
   //do nothing
}

bool GuiControl::onKeyDown(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
      return parent->onKeyDown(event);
   else
      return false;
}

bool GuiControl::onKeyRepeat(const GuiEvent &event)
{
   // default to just another key down.
   return onKeyDown(event);
}

bool GuiControl::onKeyUp(const GuiEvent &event)
{
   //pass the event to the parent
   GuiControl *parent = getParent();
   if (parent)
      return parent->onKeyUp(event);
   else
      return false;
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::onAction()
{
   if (! mActive)
      return;

   //execute the console command
   if (mConsoleCommand && mConsoleCommand[0])
   {
      execConsoleCallback();
   }
   else
      Con::executef(this, 1, "onAction");
}

void GuiControl::onMessage(GuiControl *sender, S32 msg)
{
}

void GuiControl::messageSiblings(S32 message)
{
   GuiControl *parent = getParent();
   if (! parent) return;
   GuiControl::iterator i;
   for(i = parent->begin(); i != parent->end(); i++)
   {
      GuiControl *ctrl = dynamic_cast<GuiControl *>(*i);
      if (ctrl != this)
         ctrl->onMessage(this, message);
   }
}

// -=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=-=- //

void GuiControl::onDialogPush()
{
   // Notify Script.
   if( isMethod("onDialogPush") )
      Con::executef(this, 1, "onDialogPush");

}

void GuiControl::onDialogPop()
{
   // Notify Script.
   if( isMethod("onDialogPop") )
      Con::executef(this, 1, "onDialogPop");
}

//------------------------------------------------------------------------------
void GuiControl::setVisible(bool value)
{
    mVisible = value;
   iterator i;
   setUpdate();
   for(i = begin(); i != end(); i++)
   {
      GuiControl *ctrl = static_cast<GuiControl *>(*i);
      ctrl->clearFirstResponder();
    }

    GuiControl *parent = getParent();
    if (parent)
       parent->childResized(this);
}


void GuiControl::makeFirstResponder(bool value)
{
   if ( value )
      //setFirstResponder(this);
      setFirstResponder();
   else
      clearFirstResponder();
}

void GuiControl::setActive( bool value )
{
   mActive = value;

   if ( !mActive )
      clearFirstResponder();

   if ( mVisible && mAwake )
      setUpdate();
}

void GuiControl::getScrollLineSizes(U32 *rowHeight, U32 *columnWidth)
{
    // default to 10 pixels in y, 30 pixels in x
    *columnWidth = 30;
    *rowHeight = 30;
}

void GuiControl::renderJustifiedText(Point2I offset, Point2I extent, const char *text)
{
   GFont *font = mProfile->mFont;
   S32 textWidth = font->getStrWidth((const UTF8*)text);
   Point2I start;

   // align the horizontal
   switch( mProfile->mAlignment )
   {
      case GuiControlProfile::RightJustify:
         start.set( extent.x - textWidth, 0 );
         break;
      case GuiControlProfile::CenterJustify:
         start.set( ( extent.x - textWidth) / 2, 0 );
         break;
      default:
         // GuiControlProfile::LeftJustify
         start.set( 0, 0 );
         break;
   }

   // If the text is longer then the box size, (it'll get clipped) so
   // force Left Justify

   if( textWidth > extent.x )
      start.set( 0, 0 );

   // center the vertical
   if(font->getHeight() > (U32)extent.y)
      start.y = 0 - ((font->getHeight() - extent.y) / 2) ;
   else
      start.y = ( extent.y - font->getHeight() ) / 2;

   GFX->getDrawUtil()->drawText( font, start + offset, text, mProfile->mFontColors );
}

void GuiControl::getCursor(GuiCursor *&cursor, bool &showCursor, const GuiEvent &lastGuiEvent)
{
    if( !getRoot() )
        return;
    
    if(getRoot()->mCursorChanged != -1 && !isMouseLocked())
    {
        // We've already changed the cursor,
        // so set it back before we change it again.
        
        PlatformWindow *pWindow = static_cast<GuiCanvas*>(getRoot())->getPlatformWindow();
        AssertFatal(pWindow != NULL,"GuiControl without owning platform window!  This should not be possible.");
        PlatformCursorController *pController = pWindow->getCursorController();
        AssertFatal(pController != NULL,"PlatformWindow without an owned CursorController!");
        
        pController->popCursor();
        
        // We haven't changed it
        getRoot()->mCursorChanged = -1;
    }
}

const char* GuiControl::execConsoleCallback()
{
   if (mConsoleCommand && mConsoleCommand[0])
   {
      Con::setVariable("$ThisControl", avar("%d",getId()));
      return Con::evaluate(mConsoleCommand, false);
   }
   return "";
}

const char* GuiControl::execAltConsoleCallback()
{
   if(mAltConsoleCommand && mAltConsoleCommand[0])
   {
      Con::setVariable("$ThisControl", avar("%d",getId()));
      return Con::evaluate(mAltConsoleCommand, false);
   }
   return "";
}
