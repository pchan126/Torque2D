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

#include "platformiOS/2d/gui/guiJoystickButtonCtrl.h"

#ifndef _RENDER_PROXY_H_
#include "2d/core/RenderProxy.h"
#endif

#include "graphics/gfxDrawUtil.h"

#ifndef _CONSOLE_H_
#include "console/console.h"
#endif

#ifndef _CONSOLETYPES_H_
#include "console/consoleTypes.h"
#endif

#ifndef _GUICANVAS_H_
#include "gui/guiCanvas.h"
#endif

#ifndef _H_GUIDEFAULTCONTROLRENDER_
#include "gui/guiDefaultControlRender.h"
#endif

/// Script bindings.
#include "guiJoystickButtonCtrl_ScriptBindings.h"
#include "input/actionMap.h"

extern CodeMapping gVirtualMap[];

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiJoystickButtonCtrl);

//-----------------------------------------------------------------------------

GuiJoystickButtonCtrl::GuiJoystickButtonCtrl() :
    mCircleAssetId( StringTable->EmptyString ),
    mStickAssetId( StringTable->EmptyString ),
    m_XeventCode(0),
    m_YeventCode(0),
    m_touchRadius(50),
    m_state(INACTIVE)
{
    setExtent(140, 30);
}

//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addProtectedField("CircleImage", TypeAssetId, Offset(mCircleAssetId, GuiJoystickButtonCtrl), &setCircleImage, &getCircleImage, "The image asset Id used for the normal button state.");
    addProtectedField("StickImage", TypeAssetId, Offset(mStickAssetId, GuiJoystickButtonCtrl), &setStickImage, &getStickImage, "The image asset Id used for the hover button state.");
    addProtectedField("Xevent", TypeS32, Offset(m_XeventCode, GuiJoystickButtonCtrl), &setXevent, &getXevent, "");
    addProtectedField("Yevent", TypeS32, Offset(m_YeventCode, GuiJoystickButtonCtrl), &setYevent, &getYevent, "");
    addProtectedField("TouchRadius", TypeS32, Offset(m_touchRadius, GuiJoystickButtonCtrl), &setTouchRadius, &defaultProtectedGetFn, &defaultProtectedWriteFn, "");
}

//-----------------------------------------------------------------------------

bool GuiJoystickButtonCtrl::onWake()
{
    // Call parent.
    if (!Parent::onWake())
        return false;

    // Is only the "normal" image specified?
    if (    mCircleAssetId != StringTable->EmptyString &&
            mStickAssetId == StringTable->EmptyString )
    {
        // Yes, so use it for all states.
        mImageCircleAsset = mCircleAssetId;
        mImageStickAsset = mCircleAssetId;
    }
    else
    {
        // No, so assign individual states.
        mImageCircleAsset = mCircleAssetId;
        mImageStickAsset = mStickAssetId;
    }
   
    return true;
}

//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::onSleep()
{
    // Clear assets.
    mImageCircleAsset.clear();
    mImageStickAsset.clear();

    // Call parent.
    Parent::onSleep();
}

//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::setCircleImage( const char* pImageAssetId )
{
    // Sanity!
    AssertFatal( pImageAssetId != NULL, "Cannot use a NULL asset Id." );

    // Fetch the asset Id.
    mCircleAssetId = StringTable->insert(pImageAssetId);

    // Assign asset if awake.
    if ( isAwake() )
        mImageCircleAsset = mCircleAssetId;

    // Update control.
    setUpdate();
}

//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::setStickImage( const char* pImageAssetId )
{
    // Sanity!
    AssertFatal( pImageAssetId != NULL, "Cannot use a NULL asset Id." );

    // Fetch the asset Id.
    mStickAssetId = StringTable->insert(pImageAssetId);

    // Assign asset if awake.
    if ( isAwake() )
        mImageStickAsset = mStickAssetId;

    // Update control.
    setUpdate();
}


//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::setXevent( const char* pEvent)
{
    for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
    {
        if (dStricmp(pEvent, gVirtualMap[j].pDescription) == 0)
        {
            m_XeventCode = gVirtualMap[j].code;
            return;
        }
    }
}


//-----------------------------------------------------------------------------


StringTableEntry GuiJoystickButtonCtrl::getXevent( void )
{
    for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
    {
        if (m_XeventCode == gVirtualMap[j].code)
            return gVirtualMap[j].pDescription;
    }
    return NULL;
}


//-----------------------------------------------------------------------------


void GuiJoystickButtonCtrl::setYevent( const char* pEvent)
{
    for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
    {
        if (dStricmp(pEvent, gVirtualMap[j].pDescription) == 0)
        {
            m_YeventCode = gVirtualMap[j].code;
            return;
        }
    }
}


//-----------------------------------------------------------------------------


StringTableEntry GuiJoystickButtonCtrl::getYevent( void )
{
    for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
    {
        if (m_YeventCode == gVirtualMap[j].code)
            return gVirtualMap[j].pDescription;
    }
    return NULL;
}


//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::onTouchUp(const GuiEvent &event)
{
    m_TouchDown.set(0, 0);
    m_LastTouch.set(0, 0);
    m_state = INACTIVE;
}


//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::onTouchDown(const GuiEvent &event)
{
    m_TouchDown = event.mousePoint;
    m_state = ACTIVE;
}


//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::onTouchDragged(const GuiEvent &event)
{
    m_LastTouch = event.mousePoint;
}

//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::poll()
{
    Point2I offset = m_LastTouch - m_TouchDown;

    {
        InputEventInfo inputEvent;

        inputEvent.deviceInst = 0;
        inputEvent.fValue = mClamp((F32)offset.x/(F32)m_touchRadius, -1.0, 1.0);
        inputEvent.deviceType = JoystickDeviceType;
        inputEvent.objType = m_XeventCode;
        inputEvent.objInst = SI_AXIS;
        inputEvent.action = SI_MOVE;
        inputEvent.modifier = 0;

        // Give the ActionMap first shot.
        if (ActionMap::handleEventGlobal(&inputEvent))
            return;

        // If we get here we failed to process it with anything prior... so let
        // the ActionMap handle it.
        ActionMap::handleEvent(&inputEvent);
    }

    {
        InputEventInfo inputEvent;

        inputEvent.deviceInst = 0;
        inputEvent.fValue = mClamp((F32)offset.y/(F32)m_touchRadius, -1.0, 1.0);
        inputEvent.deviceType = JoystickDeviceType;
        inputEvent.objType = m_YeventCode;
        inputEvent.objInst = SI_AXIS;
        inputEvent.action = SI_MOVE;
        inputEvent.modifier = 0;

        // Give the ActionMap first shot.
        if (ActionMap::handleEventGlobal(&inputEvent))
            return;

        // If we get here we failed to process it with anything prior... so let
        // the ActionMap handle it.
        ActionMap::handleEvent(&inputEvent);
    }
}




//-----------------------------------------------------------------------------

void GuiJoystickButtonCtrl::onRender(Point2I offset, const RectI& updateRect)
{
    if (m_state = ACTIVE)
    {

    }
    else
    {

    }
}

//------------------------------------------------------------------------------

void GuiJoystickButtonCtrl::renderButton( ImageAsset* pImageAsset, const U32 frame, Point2I &offset, const RectI& updateRect )
{
//    // Ignore an invalid datablock.
//    if ( pImageAsset == NULL )
//        return;
//
//    // Is the asset valid and has the specified frame?
//    if ( pImageAsset->isAssetValid() && frame < pImageAsset->getFrameCount() )
//    {
//        // Yes, so calculate the source region.
//        const ImageAsset::FrameArea::PixelArea& pixelArea = pImageAsset->getImageFrameArea( frame ).mPixelArea;
//        RectI sourceRegion( pixelArea.mPixelOffset, Point2I(pixelArea.mPixelWidth, pixelArea.mPixelHeight) );
//
//        // Calculate destination region.
//        RectI destinationRegion(offset, getExtent());
//
//        // Render image.
//        GFX->getDrawUtil()->setBitmapModulation( mProfile->mFillColor );
//        GFX->getDrawUtil()->drawBitmapStretchSR( pImageAsset->getImageTexture(), destinationRegion, sourceRegion );
//        GFX->getDrawUtil()->clearBitmapModulation();
//        renderChildControls( offset, updateRect);
//    }
//    else
//    {
//        // No, so fetch the 'cannot render' proxy.
//        RenderProxy* pNoImageRenderProxy = Sim::findObject<RenderProxy>( CANNOT_RENDER_PROXY_NAME );
//
//        // Finish if no render proxy available or it can't render.
//        if ( pNoImageRenderProxy == NULL || !pNoImageRenderProxy->validRender() )
//            return;
//
//        // Render using render-proxy..
//        pNoImageRenderProxy->renderGui( *this, offset, updateRect );
//    }
//
//    // Update the control.
//    setUpdate();
}
