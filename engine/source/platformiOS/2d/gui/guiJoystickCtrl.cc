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

#include "guiJoystickCtrl.h"

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
#include "guiJoystickCtrl_ScriptBindings.h"
#include "input/actionMap.h"
#import "iOSInputManager.h"

extern CodeMapping gVirtualMap[];

//-----------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(GuiJoystickCtrl);

//-----------------------------------------------------------------------------

GuiJoystickCtrl::GuiJoystickCtrl() :
    mCircleAssetId( StringTable->EmptyString ),
    mStickAssetId( StringTable->EmptyString ),
    m_XeventCode(0),
    m_YeventCode(0),
    m_touchRadius(150),
    m_state(INACTIVE),
    circleframe(0),
    stickframe(0),
    mImageCircleAsset(NULL),
    mImageStickAsset(NULL)
{
    setExtent(140, 30);
}

//-----------------------------------------------------------------------------

void GuiJoystickCtrl::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addProtectedField("CircleImage", TypeAssetId, Offset(mCircleAssetId, GuiJoystickCtrl), &setCircleImage, &getCircleImage, "The image asset Id used for the normal button state.");
    addProtectedField("StickImage", TypeAssetId, Offset(mStickAssetId, GuiJoystickCtrl), &setStickImage, &getStickImage, "The image asset Id used for the hover button state.");
    addProtectedField("Xevent", TypeS32, Offset(m_XeventCode, GuiJoystickCtrl), &setXevent, &getXevent, "");
    addProtectedField("Yevent", TypeS32, Offset(m_YeventCode, GuiJoystickCtrl), &setYevent, &getYevent, "");
    addProtectedField("TouchRadius", TypeS32, Offset(m_touchRadius, GuiJoystickCtrl), &setTouchRadius, &defaultProtectedGetFn, &defaultProtectedWriteFn, "");
}

//-----------------------------------------------------------------------------

bool GuiJoystickCtrl::onWake()
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

void GuiJoystickCtrl::onSleep()
{
    // Clear assets.
    mImageCircleAsset.clear();
    mImageStickAsset.clear();

    // Call parent.
    Parent::onSleep();
}

//-----------------------------------------------------------------------------

void GuiJoystickCtrl::setCircleImage( const char* pImageAssetId )
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

void GuiJoystickCtrl::setStickImage( const char* pImageAssetId )
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

void GuiJoystickCtrl::setXevent( const char* pEvent)
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


StringTableEntry GuiJoystickCtrl::getXevent( void )
{
    for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
    {
        if (m_XeventCode == gVirtualMap[j].code)
            return gVirtualMap[j].pDescription;
    }
    return NULL;
}


//-----------------------------------------------------------------------------


void GuiJoystickCtrl::setYevent( const char* pEvent)
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


StringTableEntry GuiJoystickCtrl::getYevent( void )
{
    for (U32 j = 0; gVirtualMap[j].code != 0xFFFFFFFF; j++)
    {
        if (m_YeventCode == gVirtualMap[j].code)
            return gVirtualMap[j].pDescription;
    }
    return NULL;
}


//-----------------------------------------------------------------------------

void GuiJoystickCtrl::onTouchUp(const GuiEvent &event)
{
    if (m_eventid == event.eventID)
    {
        m_TouchDown.set(0, 0);
        m_LastTouch.set(0, 0);
        m_state = INACTIVE;
        m_eventid = -1;
    }
}


//-----------------------------------------------------------------------------

void GuiJoystickCtrl::onTouchDown(const GuiEvent &event)
{
    if (event.mousePoint.x >= getPosition().x+m_touchRadius && getPosition().y+event.mousePoint.y >= m_touchRadius && event.mousePoint.x < getPosition().x+getWidth()-m_touchRadius && event.mousePoint.y < getPosition().y+getHeight()-m_touchRadius)
    {
        m_TouchDown = event.mousePoint;
        m_LastTouch = event.mousePoint;
        m_state = ACTIVE;
        m_eventid = event.eventID;
    }
}


//-----------------------------------------------------------------------------

void GuiJoystickCtrl::onTouchDragged(const GuiEvent &event)
{
    if (m_state != ACTIVE || event.eventID != m_eventid) return;

    m_LastTouch = event.mousePoint;
    Point2I m_Vect = m_LastTouch - m_TouchDown;
    U32 vecLen = m_Vect.len();
    if ( vecLen > m_touchRadius)
    {
        m_Vect *= m_touchRadius;
        m_Vect /= vecLen;
        m_LastTouch = m_Vect + m_TouchDown;
    }

//    m_LastTouch.x = mClamp(m_LastTouch.x, m_TouchDown.x-m_touchRadius, m_TouchDown.x+m_touchRadius);
//    m_LastTouch.y = mClamp(m_LastTouch.y, m_TouchDown.y-m_touchRadius, m_TouchDown.y+m_touchRadius);
}

//-----------------------------------------------------------------------------

void GuiJoystickCtrl::process()
{
    Point2I offset = (m_LastTouch - m_TouchDown);

    if (m_XeventCode)
    {
        InputEventInfo inputEvent;

        inputEvent.deviceInst = 0;
        inputEvent.fValue = mClampF((F32)offset.x/(F32)m_touchRadius, -1.0f, 1.0f);
        inputEvent.deviceType = JoystickDeviceType;
        inputEvent.objType = m_XeventCode;
        inputEvent.objInst = SI_AXIS;
        inputEvent.action = SI_MOVE;
        inputEvent.modifier = 0;

        if (!ActionMap::handleEventGlobal(&inputEvent))
            ActionMap::handleEvent(&inputEvent);
    }

    if (m_YeventCode)
    {
        InputEventInfo inputEvent;

        inputEvent.deviceInst = 0;
        inputEvent.fValue = mClampF((F32)offset.y/(F32)m_touchRadius, -1.0f, 1.0f);
        inputEvent.deviceType = JoystickDeviceType;
        inputEvent.objType = m_YeventCode;
        inputEvent.objInst = SI_AXIS;
        inputEvent.action = SI_MOVE;
        inputEvent.modifier = 0;

        if (!ActionMap::handleEventGlobal(&inputEvent))
            ActionMap::handleEvent(&inputEvent);
    }
}




//-----------------------------------------------------------------------------

void GuiJoystickCtrl::onRender(Point2I offset, const RectI& updateRect)
{

    if (m_state == ACTIVE)
    {
        renderButtons( offset, updateRect);
    }
#ifdef TORQUE_DEBUG
    else
    {
        RectI ctrlRect(Point2I(offset.x+m_touchRadius, offset.y+m_touchRadius), Point2I(getExtent().x -m_touchRadius*2, getExtent().y -m_touchRadius*2));
        mProfile->mBorder = 1;
        renderBorder(ctrlRect, mProfile);
    }
#endif
}

//------------------------------------------------------------------------------

void GuiJoystickCtrl::renderButtons( Point2I &offset, const RectI& updateRect)
{
    // Ignore an invalid datablock.
    if ( mImageCircleAsset == NULL )
        return;

    // Is the asset valid and has the specified frame?
    if ( mImageCircleAsset->isAssetValid() && circleframe < mImageCircleAsset->getFrameCount() )
    {
        // Yes, so calculate the source region.
        const ImageAsset::FrameArea::PixelArea& pixelArea = mImageCircleAsset->getImageFrameArea( circleframe ).mPixelArea;
        RectI sourceRegion( pixelArea.mPixelOffset, Point2I(pixelArea.mPixelWidth, pixelArea.mPixelHeight) );

        // Calculate destination region.
        RectI destinationRegion(m_TouchDown-Point2I(m_touchRadius, m_touchRadius), Point2I(m_touchRadius*2, m_touchRadius*2));

        // Render image.
        GFX->getDrawUtil()->setBitmapModulation( ColorI(255, 255, 255) );
        GFX->getDrawUtil()->drawBitmapStretchSR( mImageCircleAsset->getImageTexture(), destinationRegion, sourceRegion );
        GFX->getDrawUtil()->clearBitmapModulation();
        renderChildControls( offset, updateRect);
    }
    else
    {
        // No, so fetch the 'cannot render' proxy.
        RenderProxy* pNoImageRenderProxy = Sim::findObject<RenderProxy>( CANNOT_RENDER_PROXY_NAME );

        // Finish if no render proxy available or it can't render.
        if ( pNoImageRenderProxy == NULL || !pNoImageRenderProxy->validRender() )
            return;

        // Render using render-proxy..
        pNoImageRenderProxy->renderGui( *this, offset, updateRect );
    }

    // Ignore an invalid datablock.
    if ( mImageStickAsset == NULL )
        return;

    // Is the asset valid and has the specified frame?
    if ( mImageStickAsset->isAssetValid() && stickframe < mImageStickAsset->getFrameCount() )
    {
        // Yes, so calculate the source region.
        const ImageAsset::FrameArea::PixelArea& pixelArea = mImageStickAsset->getImageFrameArea( stickframe ).mPixelArea;
        RectI sourceRegion( pixelArea.mPixelOffset, Point2I(pixelArea.mPixelWidth, pixelArea.mPixelHeight) );

        // Calculate destination region.
        RectI destinationRegion(m_LastTouch-Point2I(m_touchRadius/2, m_touchRadius/2), Point2I(m_touchRadius, m_touchRadius));

        // Render image.
        GFX->getDrawUtil()->setBitmapModulation( ColorI(255, 255, 255) );
        GFX->getDrawUtil()->drawBitmapStretchSR( mImageStickAsset->getImageTexture(), destinationRegion, sourceRegion );
        GFX->getDrawUtil()->clearBitmapModulation();
        renderChildControls( offset, updateRect);
    }
    else
    {
        // No, so fetch the 'cannot render' proxy.
        RenderProxy* pNoImageRenderProxy = Sim::findObject<RenderProxy>( CANNOT_RENDER_PROXY_NAME );

        // Finish if no render proxy available or it can't render.
        if ( pNoImageRenderProxy == NULL || !pNoImageRenderProxy->validRender() )
            return;

        // Render using render-proxy..
        pNoImageRenderProxy->renderGui( *this, offset, updateRect );
    }
}

//------------------------------------------------------------------------------


bool GuiJoystickCtrl::onAdd() {
    // Let Parent Do Work.
    if(!Parent::onAdd())
        return false;

    mCircle = new GuiSpriteCtrl();
    AssertFatal(mCircle, "Failed to create the GuiSpriteCtrl for the Circle");
    mCircle->registerObject();

    mStick = new GuiSpriteCtrl();
    AssertFatal(mStick, "Failed to create the GuiSpriteCtrl for the Stick");
    mStick->registerObject();

    iOSInputManager* inputManager = dynamic_cast<iOSInputManager*>(Input::getManager());
    if (inputManager)
        inputManager->addInput(this);

    return true;
}

void GuiJoystickCtrl::onRemove() {
    iOSInputManager* inputManager = dynamic_cast<iOSInputManager*>(Input::getManager());
    if (inputManager)
        inputManager->removeObject(this);

    Parent::onRemove();
}

bool GuiJoystickCtrl::pointInControl(const Point2I& parentCoordPoint) {
    S32 xt = parentCoordPoint.x - getPosition().x;
    S32 yt = parentCoordPoint.y - getPosition().y;

    return xt >= 0 && yt >= 0 && xt < getWidth() && yt < getHeight();
}

void GuiJoystickCtrl::onTouchLeave(const GuiEvent& event) {
    onTouchUp(event);
}

void GuiJoystickCtrl::onMouseLeave(const GuiEvent& event) {
    onTouchUp(event);
}
