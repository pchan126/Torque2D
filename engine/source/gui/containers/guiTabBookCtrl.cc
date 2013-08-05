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
#include "console/console.h"
#include "console/consoleTypes.h"
#include "graphics/gfxDevice.h"
#include "sim/simBase.h"
#include "gui/guiCanvas.h"
#include "gui/containers/guiTabBookCtrl.h"
#include "platform/event.h"
#include "io/fileStream.h"
#include "gui/containers/guiScrollCtrl.h"
#include "gui/editor/guiEditCtrl.h"
#include "gui/guiPopUpCtrl.h"
#include "gui/guiDefaultControlRender.h"
#include "graphics/gfxDrawUtil.h"


// So we can set tab alignment via gui editor
static EnumTable::Enums tabAlignEnums[] =
{
   { GuiTabBookCtrl::AlignTop,   "Top"    },
   { GuiTabBookCtrl::AlignBottom,"Bottom" },
};
static EnumTable gTabAlignEnums(4,&tabAlignEnums[0]);


IMPLEMENT_CONOBJECT(GuiTabBookCtrl);

GuiTabBookCtrl::GuiTabBookCtrl()
{
   VECTOR_SET_ASSOCIATION(mPages);
   mTabHeight = 24;
   mTabPosition = AlignTop;
   mActivePage = NULL;
   mHoverTab = NULL;
   mHasTexture = false;
   mBitmapBounds = NULL;
   setExtent( 400, 300 );
   mPageRect = RectI(0,0,0,0);
   mTabRect = RectI(0,0,0,0);
   mFrontTabPadding = 0;

   mPages.reserve(12);
   mTabMargin = 7;
   mMinTabWidth = 64;
   mIsContainer = true;
    mSelectedPageNum = -1;
    mDefaultPageNum = -1;
    
    mAllowReorder = false;
    mDraggingTab  = false;
    mDraggingTabRect = false;
    mIsFirstWake = true;}

void GuiTabBookCtrl::initPersistFields()
{
   Parent::initPersistFields();
//   addField("tabPosition",		TypeEnum,		Offset(mTabPosition,GuiTabBookCtrl), 1, &gTabAlignEnums );
    addField( "tabPosition",     TypeEnum, Offset( mTabPosition,    GuiTabBookCtrl ), 1, &gTabAlignEnums,
             "Where to place the tab page headers." );
    addField( "tabMargin",       TypeS32,  Offset( mTabMargin,      GuiTabBookCtrl ),
             "Spacing to put between individual tab page headers." );
    addField( "minTabWidth",     TypeS32,  Offset( mMinTabWidth,    GuiTabBookCtrl ),
             "Minimum width allocated to a tab page header." );
    addField( "tabHeight",       TypeS32,  Offset( mTabHeight,      GuiTabBookCtrl ),
             "Height of tab page headers." );
    addField( "allowReorder",    TypeBool, Offset( mAllowReorder,   GuiTabBookCtrl ),
             "Whether reordering tabs with the mouse is allowed." );
    addField( "defaultPage",     TypeS32,  Offset( mDefaultPageNum, GuiTabBookCtrl ),
             "Index of page to select on first onWake() call (-1 to disable)." );
    
//    addProtectedField( "selectedPage", TypeS32, Offset( mSelectedPageNum, GuiTabBookCtrl ),
//                      &_setSelectedPage, &defaultProtectedGetFn,
//                      "Index of currently selected page." );
    
    addField( "frontTabPadding", TypeS32, Offset( mFrontTabPadding, GuiTabBookCtrl ),
             "X offset of first tab page header." );
}


void GuiTabBookCtrl::onChildRemoved( GuiControl* child )
{
   for (S32 i = 0; i < mPages.size(); i++ )
   {
      GuiTabPageCtrl* tab = mPages[i].Page;
      if( tab == child )
      {
         if( tab == mActivePage )
            mActivePage = NULL;
         mPages.erase( i );
         break;
      }
   }

   if( mPages.empty() )
      mActivePage = NULL;
   else if (mActivePage == NULL )
      mActivePage = static_cast<GuiTabPageCtrl*>(mPages[0].Page);

}

void GuiTabBookCtrl::onChildAdded( GuiControl *child )
{
   GuiTabPageCtrl *page = dynamic_cast<GuiTabPageCtrl*>(child);
   if( !page )
   {
      Con::warnf("GuiTabBookCtrl::onChildAdded - attempting to add NON GuiTabPageCtrl as child page");
      SimObject *simObj = reinterpret_cast<SimObject*>(child);
      removeObject( simObj );
      if( mActivePage )
      {
         mActivePage->addObject( simObj );
      }
      else
      {
         Con::warnf("GuiTabBookCtrl::onChildAdded - unable to find active page to reassign ownership of new child control to, placing on parent");
         GuiControl *rent = getParent();
         if( rent )
            rent->addObject( simObj );
      }
      return;
   }


   TabHeaderInfo newPage;

   newPage.Page      = page;
   newPage.TabRow    = -1;
   newPage.TabColumn = -1;

   mPages.push_back( newPage );

   // Calculate Page Information
   calculatePageTabs();

   child->resize( mPageRect.point, mPageRect.extent );


   // Select this Page
   selectPage( page );
}


//-----------------------------------------------------------------------------

bool GuiTabBookCtrl::reOrder(SimObject* obj, SimObject* target)
{
    if ( !Parent::reOrder(obj, target) )
        return false;
    
    // Store the Selected Page.
    GuiTabPageCtrl *selectedPage = NULL;
    if ( mSelectedPageNum != -1 )
        selectedPage = mPages[mSelectedPageNum].Page;
    
    // Determine the Target Page Index.
    S32 targetIndex = -1;
    for( S32 i = 0; i < mPages.size(); i++ )
    {
        const TabHeaderInfo &info = mPages[i];
        if ( info.Page == target )
        {
            targetIndex = i;
            break;
        }
    }
    
    if ( targetIndex == -1 )
    {
        return false;
    }
    
    for( S32 i = 0; i < mPages.size(); i++ )
    {
        const TabHeaderInfo &info = mPages[i];
        if ( info.Page == obj )
        {
            // Store Info.
            TabHeaderInfo objPage = info;
            
            // Remove.
            mPages.erase( i );
            // Insert.
            mPages.insert( targetIndex, objPage );
            
            break;
        }
    }
    
    // Update Tabs.
    calculatePageTabs();
    
    // Reselect Page.
    selectPage( selectedPage );
    
    return true;
}

//-----------------------------------------------------------------------------

bool GuiTabBookCtrl::acceptsAsChild( SimObject* object ) const
{
    // Only accept tab pages.
    return ( dynamic_cast< GuiTabPageCtrl* >( object ) != NULL );
}

//-----------------------------------------------------------------------------

bool GuiTabBookCtrl::onWake()
{
   if (! Parent::onWake())
      return false;

   mHasTexture = mProfile->constructBitmapArray();
   if( mHasTexture )
      mBitmapBounds = mProfile->mBitmapArrayRects.address();

   return true;
}


void GuiTabBookCtrl::addNewPage()
{
   char textbuf[1024];

   GuiTabPageCtrl * page = new GuiTabPageCtrl();

   page->setField("profile", "GuiTabPageProfile");

   dSprintf(textbuf, sizeof(textbuf), "TabBookPage%d_%d", getId(), page->getId());
   page->registerObject(textbuf);

   this->addObject( page );
}

bool GuiTabBookCtrl::resize(const Point2I &newPosition, const Point2I &newExtent)
{
    bool result = Parent::resize( newPosition, newExtent );
    
    calculatePageTabs();
    
    return result;
}

void GuiTabBookCtrl::childResized(GuiControl *child)
{
   //Parent::childResized( child );

   child->resize( mPageRect.point, mPageRect.extent );
}

void GuiTabBookCtrl::onMouseDown(const GuiEvent &event)
{
    Point2I localMouse = globalToLocalCoord( event.mousePoint );
    if( mTabRect.pointInRect( localMouse ) )
    {
        GuiTabPageCtrl *tab = findHitTab( localMouse );
        if( tab != NULL && tab->isActive() )
            selectPage( tab );
    }
}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::onMouseUp(const GuiEvent &event)
{
    Parent::onMouseUp( event );
    
    mDraggingTab = false;
    mDraggingTabRect = false;
}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::onMouseDragged(const GuiEvent &event)
{
    Parent::onMouseDragged( event );
    
    if ( !mDraggingTab )
        return;
    
    GuiTabPageCtrl *selectedPage = NULL;
    if ( mSelectedPageNum != -1 )
        selectedPage = mPages[mSelectedPageNum].Page;
    
    if ( !selectedPage )
        return;
    
    Point2I localMouse = globalToLocalCoord( event.mousePoint );
    if( mTabRect.pointInRect( localMouse ) )
    {
        GuiTabPageCtrl *tab = findHitTab( localMouse );
        if( tab != NULL && tab != selectedPage )
        {
            S32 targetIndex = -1;
            for( S32 i = 0; i < mPages.size(); i++ )
            {
                if( mPages[i].Page == tab )
                {
                    targetIndex = i;
                    break;
                }
            }
            
            if ( targetIndex > mSelectedPageNum )
            {
                reOrder( tab, selectedPage );
            }
            else
            {
                reOrder( selectedPage, tab );
            }
        }
    }
}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::onMouseMove(const GuiEvent &event)
{

   Point2I localMouse = globalToLocalCoord( event.mousePoint );
   if( mTabRect.pointInRect( localMouse ) )
   {

      GuiTabPageCtrl *tab = findHitTab( localMouse );
      if( tab != NULL && mHoverTab != tab )
         mHoverTab = tab;
      else if ( !tab )
         mHoverTab = NULL;
   }
   Parent::onMouseMove( event );
}

void GuiTabBookCtrl::onMouseLeave( const GuiEvent &event )
{
   mHoverTab = NULL;
}

bool GuiTabBookCtrl::onMouseDownEditor(const GuiEvent &event, Point2I offset)
{
   bool handled = false;
   Point2I localMouse = globalToLocalCoord( event.mousePoint );

   if( mTabRect.pointInRect( localMouse ) )
   {
      GuiTabPageCtrl *tab = findHitTab( localMouse );
      if( tab != NULL )
      {
         selectPage( tab );
         handled = true;
      }
   }

   // This shouldn't be called if it's not design time, but check just incase
   if ( GuiControl::smDesignTime )
   {
      // If we clicked in the editor and our addset is the tab book
      // ctrl, select the child ctrl so we can edit it's properties
      GuiEditCtrl* edit = GuiControl::smEditorHandle;
      if( edit  && ( edit->getAddSet() == this ) && mActivePage != NULL )
         edit->select( mActivePage );
   }

   // Return whether we handled this or not.
   return handled;

}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::onRightMouseUp( const GuiEvent& event )
{
    Point2I localMouse = globalToLocalCoord( event.mousePoint );
    if( mTabRect.pointInRect( localMouse ) )
    {
        GuiTabPageCtrl* tab = findHitTab( localMouse );
        if( tab )
            if (isMethod("onTabRightClick"))
                Con::executef(4, this, "onTabRightClick", tab->getText(), getPageNum( tab ) );
    }
}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::onRender(Point2I offset, const RectI &updateRect)
{
    RectI tabRect = mTabRect;
    tabRect.point += offset;
    RectI pageRect = mPageRect;
    pageRect.point += offset;
    
    // We're so nice we'll store the old modulation before we clear it for our rendering! :)
    ColorI oldModulation;
    GFX->getDrawUtil()->getBitmapModulation( &oldModulation );
    
    // Wipe it out
    GFX->getDrawUtil()->clearBitmapModulation();
    
    Parent::onRender(offset, updateRect);
    
    // Clip to tab area
    RectI savedClipRect = GFX->getClipRect();
    GFX->setClipRect( tabRect );
    
    // Render our tabs
    renderTabs( offset, tabRect );
    
    // Restore Rect.
    GFX->setClipRect( savedClipRect );
    
    // Restore old modulation
    GFX->getDrawUtil()->setBitmapModulation( oldModulation );
}


//-----------------------------------------------------------------------------

void GuiTabBookCtrl::renderTabs( const Point2I &offset, const RectI &tabRect )
{
    // If the tab size is zero, don't render tabs,
    //  assuming it's a tab-less book
    if( mPages.empty() || mTabHeight <= 0 )
        return;
    
    for( S32 i = 0; i < mPages.size(); i++ )
    {
        const TabHeaderInfo &currentTabInfo = mPages[i];
        RectI tabBounds = mPages[i].TabRect;
        tabBounds.point += offset;
        GuiTabPageCtrl *tab = mPages[i].Page;
        if( tab != NULL )
            renderTab( tabBounds, tab );
        
        // If we're on the last tab, draw the nice end piece
        if( i + 1 == mPages.size() )
        {
            Point2I tabEndPoint = Point2I(currentTabInfo.TabRect.point.x + currentTabInfo.TabRect.extent.x + offset.x, currentTabInfo.TabRect.point.y + offset.y);
            Point2I tabEndExtent = Point2I((tabRect.point.x + tabRect.extent.x) - tabEndPoint.x, currentTabInfo.TabRect.extent.y);
            RectI tabEndRect = RectI(tabEndPoint,tabEndExtent);
            
            GFX->setClipRect( tabEndRect );
            
            // As it turns out the last tab can be outside the viewport in which
            // case trying to render causes a DX assert. Could be better if
            // setClipRect returned a bool.
            if ( GFX->getViewport().isValidRect() )
                renderFixedBitmapBordersFilled( tabEndRect, TabEnds + 1, mProfile );
        }
    }
}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::renderTab( RectI tabRect, GuiTabPageCtrl *tab )
{
    StringTableEntry text = tab->getText();
    ColorI oldColor;
    
    GFX->getDrawUtil()->getBitmapModulation( &oldColor );
    
    // Is this a skinned control?
    if( mHasTexture && mProfile->mBitmapArrayRects.size() >= 9 )
    {
        S32 indexMultiplier = 1;
        switch( mTabPosition )
        {
            case AlignTop:
            case AlignBottom:
                
                if ( mActivePage == tab )
                    indexMultiplier += TabSelected;
                else if( mHoverTab == tab )
                    indexMultiplier += TabHover;
                else
                    indexMultiplier += TabNormal;
                break;
        }
        
        renderFixedBitmapBordersFilled( tabRect, indexMultiplier, mProfile );
    }
    else
    {
        // If this isn't a skinned control or the bitmap is simply missing, handle it WELL
        if ( mActivePage == tab )
            GFX->getDrawUtil()->drawRectFill(tabRect, mProfile->mFillColor);
        else if( mHoverTab == tab )
            GFX->getDrawUtil()->drawRectFill(tabRect, mProfile->mFillColorHL);
        else
            GFX->getDrawUtil()->drawRectFill(tabRect, mProfile->mFillColorNA);
        
    }
    
    
    GFX->getDrawUtil()->setBitmapModulation(mProfile->mFontColor);
    
    switch( mTabPosition )
    {
        case AlignTop:
        case AlignBottom:
            renderJustifiedText( tabRect.point, tabRect.extent, text);
            break;
    }
    
    GFX->getDrawUtil()->setBitmapModulation( oldColor);
    
}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::setUpdate()
{
    Parent::setUpdate();
    
    setUpdateRegion(Point2I(0,0), getExtent());
    
    calculatePageTabs();
}

//-----------------------------------------------------------------------------

S32 GuiTabBookCtrl::calculatePageTabWidth( GuiTabPageCtrl *page )
{
    if( !page )
        return mMinTabWidth;
    
    const char* text = page->getText();
    
    if( !text || dStrlen(text) == 0 || mProfile->mFont.isNull() )
        return mMinTabWidth;
    
    GFont *font = mProfile->mFont;
    
    return font->getStrNWidth( text, dStrlen(text) );

}

//-----------------------------------------------------------------------------

const RectI GuiTabBookCtrl::getClientRect()
{
    
    if( !mProfile || mProfile->mBitmapArrayRects.size() < NumBitmaps )
        return Parent::getClientRect();
    
    return mPageRect;
}


void GuiTabBookCtrl::calculatePageTabs()
{
    // Short Circuit.
    //
    // If the tab size is zero, don't render tabs,
    //  assuming it's a tab-less book
    if( mPages.empty() || mTabHeight <= 0 )
    {
        mPageRect.point.x = 0;
        mPageRect.point.y = 0;
        mPageRect.extent.x = getWidth();
        mPageRect.extent.y = getHeight();
        return;
    }
    
    S32 currRow    = 0;
    S32 currColumn = 0;
    S32 currX      = mFrontTabPadding;
    S32 maxWidth   = 0;
    
    for( S32 i = 0; i < mPages.size(); i++ )
    {
        // Fetch Tab Width
        S32 tabWidth = calculatePageTabWidth( mPages[i].Page ) + ( mTabMargin * 2 );
        tabWidth = getMax( tabWidth, mMinTabWidth );
        TabHeaderInfo &info = mPages[i];
        switch( mTabPosition )
        {
            case AlignTop:
            case AlignBottom:
                // If we're going to go outside our bounds
                // with this tab move it down a row
                if( currX + tabWidth > getWidth() )
                {
                    // Calculate and Advance State.
                    maxWidth = getMax( tabWidth, maxWidth );
                    balanceRow( currRow, currX );
                    info.TabRow = ++currRow;
                    // Reset Necessaries
                    info.TabColumn = currColumn = maxWidth = currX = 0;
                }
                else
                {
                    info.TabRow = currRow;
                    info.TabColumn = currColumn++;
                }
                
                // Calculate Tabs Bounding Rect
                info.TabRect.point.x  = currX;
                info.TabRect.extent.x = tabWidth;
                info.TabRect.extent.y = mTabHeight;
                
                // Adjust Y Point based on alignment
                if( mTabPosition == AlignTop )
                    info.TabRect.point.y  = ( info.TabRow * mTabHeight );
                else
                    info.TabRect.point.y  = getHeight() - ( ( 1 + info.TabRow ) * mTabHeight );
                
                currX += tabWidth;
                break;
        };
    }
    
    currRow++;
    currColumn++;
    
    Point2I localPoint = getExtent();
    
    // Calculate
    switch( mTabPosition )
    {
        case AlignTop:
            
            localPoint.y -= getTop();
            
            mTabRect.point.x = 0;
            mTabRect.extent.x = localPoint.x;
            mTabRect.point.y = 0;
            mTabRect.extent.y = currRow * mTabHeight;
            
            mPageRect.point.x = 0;
            mPageRect.point.y = mTabRect.extent.y;
            mPageRect.extent.x = mTabRect.extent.x;
            mPageRect.extent.y = getHeight() - mTabRect.extent.y;
            
            break;
        case AlignBottom:
            mTabRect.point.x = 0;
            mTabRect.extent.x = localPoint.x;
            mTabRect.extent.y = currRow * mTabHeight;
            mTabRect.point.y = getHeight() - mTabRect.extent.y;
            
            mPageRect.point.x = 0;
            mPageRect.point.y = 0;
            mPageRect.extent.x = mTabRect.extent.x;
            mPageRect.extent.y = localPoint.y - mTabRect.extent.y;
            
            break;
    }
}

void GuiTabBookCtrl::balanceColumn( S32 column , S32 totalTabWidth )
{
   // Short Circuit.
   //
   // If the tab size is zero, don't render tabs,
   //  and assume it's a tab-less tab-book - JDD
   if( mPages.empty() || mTabHeight <= 0 )
      return;

   Vector<TabHeaderInfo*> rowTemp;
   rowTemp.clear();

   for( S32 i = 0; i < mPages.size(); i++ )
   {
      TabHeaderInfo &info = mPages[i];

      if(info.TabColumn == column )
         rowTemp.push_back( &mPages[i] );
   }

   if( rowTemp.empty() )
      return;

   // Balance the tabs across the remaining space
   S32 spaceToDivide = getHeight() - totalTabWidth;
   S32 pointDelta    = 0;
   for( S32 i = 0; i < rowTemp.size(); i++ )
   {
      TabHeaderInfo &info = *rowTemp[i];
      S32 extraSpace = (S32)( spaceToDivide / rowTemp.size() );
      info.TabRect.extent.y += extraSpace;
      info.TabRect.point.y += pointDelta;
      pointDelta += extraSpace;
   }

}
void GuiTabBookCtrl::balanceRow( S32 row, S32 totalTabWidth )
{
   // Short Circuit.
   //
   // If the tab size is zero, don't render tabs,
   //  and assume it's a tab-less tab-book - JDD
   if( mPages.empty() || mTabHeight <= 0 )
      return;

   Vector<TabHeaderInfo*> rowTemp;
   rowTemp.clear();

   for( S32 i = 0; i < mPages.size(); i++ )
   {
      TabHeaderInfo &info = mPages[i];

      if(info.TabRow == row )
         rowTemp.push_back( &mPages[i] );
   }

   if( rowTemp.empty() )
      return;

   // Balance the tabs across the remaining space
   S32 spaceToDivide = getWidth() - totalTabWidth;
   S32 pointDelta    = 0;
   for( S32 i = 0; i < rowTemp.size(); i++ )
   {
      TabHeaderInfo &info = *rowTemp[i];
      S32 extraSpace = (S32)spaceToDivide / ( rowTemp.size() );
      info.TabRect.extent.x += extraSpace;
      info.TabRect.point.x += pointDelta;
      pointDelta += extraSpace;
   }
}


GuiTabPageCtrl *GuiTabBookCtrl::findHitTab( const GuiEvent &event )
{
   return findHitTab( event.mousePoint );
}

GuiTabPageCtrl *GuiTabBookCtrl::findHitTab( Point2I hitPoint )
{
   // Short Circuit.
   //
   // If the tab size is zero, don't render tabs,
   //  and assume it's a tab-less tab-book - JDD
   if( mPages.empty() || mTabHeight <= 0 )
      return NULL;

   for( S32 i = 0; i < mPages.size(); i++ )
   {
      if( mPages[i].TabRect.pointInRect( hitPoint ) )
         return mPages[i].Page;
   }
   return NULL;
}

void GuiTabBookCtrl::selectPage( S32 index )
{
   if( mPages.size() < index )
      return;

   // Select the page
   selectPage( mPages[ index ].Page );
}


void GuiTabBookCtrl::selectPage( GuiTabPageCtrl *page )
{
   for( auto i:mPages )
   {
      GuiTabPageCtrl *tab = reinterpret_cast<GuiTabPageCtrl*>(i.Page);
      if( page == tab )
      {
         mActivePage = tab;
         tab->setVisible( true );

         // Notify User
         char *retBuffer = Con::getReturnBuffer( 512 );
         dStrcpy( retBuffer, tab->getText() );
         Con::executef( this, 2, "onTabSelected",  retBuffer );

      }
      else
         tab->setVisible( false );
   }
}

void GuiTabBookCtrl::selectPage( const char* pageName )
{
   for( auto i:mPages )
   {
      GuiTabPageCtrl *tab = reinterpret_cast<GuiTabPageCtrl*>(i.Page);
      if( dStricmp( pageName, tab->getText() ) == 0 )
      {
         mActivePage = tab;
         tab->setVisible( true );

         // Notify User
         char *retBuffer = Con::getReturnBuffer( 512 );
         dStrcpy( retBuffer, tab->getText() );
         Con::executef( this, 2, "onTabSelected",  retBuffer );

      }
      else
         tab->setVisible( false );
   }
}


bool GuiTabBookCtrl::onKeyDown(const GuiEvent &event)
{
   // Tab      = Next Page
   // Ctrl-Tab = Previous Page
   if( 0 && event.keyCode == KEY_TAB )
   {
      if( event.modifier & SI_CTRL )
         selectPrevPage();
      else 
         selectNextPage();

      return true;
   }

   return Parent::onKeyDown( event );
}

void GuiTabBookCtrl::selectNextPage()
{
   if( mPages.empty() )
      return;

   if( mActivePage == NULL )
      mActivePage = mPages[0].Page;

   S32 nI = 0;
   for( ; nI < mPages.size(); nI++ )
   {
      GuiTabPageCtrl *tab = mPages[ nI ].Page;
      if( tab == mActivePage )
      {
         if( nI == ( mPages.size() - 1 ) )
            selectPage( 0 );
         else if ( nI + 1 <= ( mPages.size() - 1 ) ) 
            selectPage( nI + 1 );
         else
            selectPage( 0 );

         // Notify User
         if( isMethod( "onTabSelected" ) )
         {
            char *retBuffer = Con::getReturnBuffer( 512 );
            dStrcpy( retBuffer, tab->getText() );
            Con::executef( this, 2, "onTabSelected",  retBuffer );
         }

         return;
      }
   }
}

void GuiTabBookCtrl::selectPrevPage()
{
   if( mPages.empty() )
      return;

   if( mActivePage == NULL )
      mActivePage = mPages[0].Page;

   S32 nI = 0;
   for( ; nI < mPages.size(); nI++ )
   {
      GuiTabPageCtrl *tab = mPages[ nI ].Page;
      if( tab == mActivePage )
      {
         if( nI == 0 )
            selectPage( mPages.size() - 1 );
         else
            selectPage( nI - 1 );

         // Notify User
         if( isMethod( "onTabSelected" ) )
         {
            char *retBuffer = Con::getReturnBuffer( 512 );
            dStrcpy( retBuffer, tab->getText() );
            Con::executef( this, 2, "onTabSelected",  retBuffer );
         }

         return;
      }
   }

}

//-----------------------------------------------------------------------------

void GuiTabBookCtrl::fitPage( GuiTabPageCtrl* page )
{
    page->resize( mPageRect.point, mPageRect.extent );
}

//-----------------------------------------------------------------------------

S32 GuiTabBookCtrl::getPageNum( GuiTabPageCtrl* page ) const
{
    const U32 numPages = mPages.size();
    for( U32 i = 0; i < numPages; ++ i )
        if( mPages[ i ].Page == page )
            return i;
    
    return -1;
}

// Empty for now, will implement for handling design time context menu for manipulating pages
ConsoleMethod( GuiTabBookCtrl, addPage, void, 2, 2, "() Empty")
{
    object->addNewPage();
}

ConsoleMethod( GuiTabBookCtrl, selectPage, void, 3, 3, "(int pageIndex)")
{
   S32 pageIndex = dAtoi(argv[2]);

   object->selectPage(pageIndex);
}

ConsoleMethod( GuiTabBookCtrl, selectPageName, void, 3, 3, "(pageName)")
{
   object->selectPage(argv[2]);
}
