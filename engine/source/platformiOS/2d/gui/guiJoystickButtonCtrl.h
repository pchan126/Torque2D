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

#ifndef _GUIJOYSTICKBUTTONCTRL_H_
#define _GUIJOYSTICKBUTTONCTRL_H_

#ifndef _GUIBUTTONCTRL_H_
#include "gui/buttons/guiButtonCtrl.h"
#endif

#include "graphics/gfxTextureManager.h"


#ifndef _IMAGE_ASSET_H_
#include "2d/assets/ImageAsset.h"
#include "types.h"

#endif

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

//-----------------------------------------------------------------------------

class GuiJoystickButtonCtrl : public GuiButtonCtrl
{
private:
   typedef GuiButtonCtrl Parent;

protected:
    enum ButtonState
    {
        ACTIVE,
        INACTIVE
    };

    StringTableEntry mCircleAssetId;
    StringTableEntry mStickAssetId;

    AssetPtr<ImageAsset> mImageCircleAsset;
    AssetPtr<ImageAsset> mImageStickAsset;

    U16 m_XeventCode;
    U16 m_YeventCode;
    Point2I m_TouchDown;
    Point2I m_LastTouch;
    U32 m_touchRadius;
    ButtonState m_state;

    void renderButton( ImageAsset* pImageAsset, const U32 frame, Point2I &offset, const RectI& updateRect);

public:
    GuiJoystickButtonCtrl();
   bool onWake();
   void onSleep();
   void onRender(Point2I offset, const RectI &updateRect);

   static void initPersistFields();

   void setCircleImage( const char* pImageAssetId );
   inline StringTableEntry getCircleImage( void ) const { return mCircleAssetId; }
   void setStickImage( const char* pImageAssetId );
   inline StringTableEntry getStickImage( void ) const { return mStickAssetId; }

   void setXevent( const char* pEvent);
   StringTableEntry getXevent( void );
   void setYevent( const char* pEvent);
   StringTableEntry getYevent( void );

    inline void setTouchRadius( const F32 radius) { m_touchRadius = radius;};
    inline F32 getTouchRadius( ) const { return m_touchRadius;};

    void poll();

    virtual void onTouchUp(const GuiEvent &event);
    virtual void onTouchDown(const GuiEvent &event);
    virtual void onTouchDragged(const GuiEvent &event);

   // Declare type.
   DECLARE_CONOBJECT(GuiJoystickButtonCtrl);

protected:
    static bool setCircleImage(void* obj, const char* data) { static_cast<GuiJoystickButtonCtrl*>(obj)->setCircleImage( data ); return false; }
    static const char*getCircleImage(void* obj, const char* data) { return static_cast<GuiJoystickButtonCtrl*>(obj)->getCircleImage(); }
    static bool setStickImage(void* obj, const char* data) { static_cast<GuiJoystickButtonCtrl*>(obj)->setStickImage( data ); return false; }
    static const char*getStickImage(void* obj, const char* data) { return static_cast<GuiJoystickButtonCtrl*>(obj)->getStickImage(); }

    static bool setXevent(void* obj, const char* data) { static_cast<GuiJoystickButtonCtrl*>(obj)->setXevent( data ); return false; }
    static const char*getXevent(void* obj, const char* data) { return static_cast<GuiJoystickButtonCtrl*>(obj)->getXevent(); }
    static bool setYevent(void* obj, const char* data) { static_cast<GuiJoystickButtonCtrl*>(obj)->setYevent( data ); return false; }
    static const char*getYevent(void* obj, const char* data) { return static_cast<GuiJoystickButtonCtrl*>(obj)->getYevent(); }

    static bool setTouchRadius(void* obj, const char* data) { static_cast<GuiJoystickButtonCtrl*>(obj)->setTouchRadius(dAtof(data)); return false; }
};

#endif //_GUIIMAGEBUTTON_H_
