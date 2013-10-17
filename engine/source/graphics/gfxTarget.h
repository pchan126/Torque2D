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

#ifndef _GFXTARGET_H_
#define _GFXTARGET_H_

class GFXTarget;
class GFXWindowTarget;
class GFXTextureTarget;

#include <memory>

typedef std::shared_ptr<GFXTarget> GFXTargetRef;
typedef std::shared_ptr<GFXWindowTarget> GFXWindowTargetRef;
typedef std::shared_ptr<GFXTextureTarget> GFXTextureTargetRef;

#include "graphics/gfxEnums.h"
#include "graphics/gfxResource.h"
#include "math/mPoint.h"
#include "windowManager/windowInputGenerator.h"
#include "gfxTextureObject.h"

class Point2I;
class GFXCubemap;

/// Base class for a target to which GFX can render.
///
/// Most modern graphics hardware supports selecting render targets. However,
/// there may be multiple types of render target, with wildly varying
/// device-level implementations, resource requirements, and so forth.
///
/// This base class is used to represent a render target; it might be a context
/// tied to a window, or a set of surfaces or textures.
class GFXTarget : public GFXResource
{
private:
   S32 mChangeToken;
   S32 mLastAppliedChange;

protected:

   /// Called whenever a change is made to this target.
   inline void invalidateState()
   {
      mChangeToken++;
   }

   /// Called when the device has applied pending state.
   inline void stateApplied()
   {
      mLastAppliedChange = mChangeToken;
   }
public:

   /// Constructor to initialize the state tracking logic.
   GFXTarget() : mChangeToken( 0 ),
                 mLastAppliedChange( 0 )
   {
   }
   virtual ~GFXTarget() {}

   /// Called to check if we have pending state for the device to apply.
   inline const bool isPendingState() const
   {
      return (mChangeToken != mLastAppliedChange);
   }

   /// Returns the size in pixels of the render target.
   virtual const Point2I getSize()=0;

   /// Returns the texture format of the render target.
   virtual GFXFormat getFormat()=0;

   // GFXResourceInterface
   /// The resource should put a description of itself (number of vertices, size/width of texture, etc.) in buffer
   virtual const String describeSelf() const;

   /// This is called to set the render target.
   virtual void activate() { }

   /// This is called when the target is not being used anymore.
   virtual void deactivate() { }

   /// This tells the target that the contents of this target should be restored
   /// when activate() is next called.
   virtual void preserve() { }
};

/// A render target associated with an OS window.
///
/// Various API/OS combinations will implement their own GFXTargets for
/// rendering to a window. However, they are all subclasses of GFXWindowTarget.
///
/// This allows platform-neutral code to safely distinguish between various
/// types of render targets (using dynamic_cast<>), as well as letting it
/// gain access to useful things like the corresponding PlatformWindow.
class GFXWindowTarget : public GFXTarget, public std::enable_shared_from_this<GFXWindowTarget>
{
protected:
   PlatformWindow *mWindow;
public:
   GFXWindowTarget() : mWindow(nullptr){};
   GFXWindowTarget( PlatformWindow *windowObject )
   {
      mWindow = windowObject;
   }
   virtual ~GFXWindowTarget() {}

   /// Returns a pointer to the window this target is bound to.
   inline PlatformWindow *getWindow() { return mWindow; };

   /// Present latest buffer, if buffer swapping is in effect.
   virtual bool present()=0;

   /// Notify the target that the video mode on the window has changed.
   virtual void resetMode()=0;
};

/// A render target associated with one or more textures.
///
/// Although some APIs allow directly selecting any texture or surfaces, in
/// some cases it is necessary to allocate helper resources to enable RTT
/// operations.
///
/// @note A GFXTextureTarget will retain references to textures that are 
/// attached to it, so be sure to clear them out when you're done!
///
/// @note Different APIs have different restrictions on what they can support
///       here. Be aware when mixing cubemaps vs. non-cubemaps, or targets of
///       different resolutions. The devices will attempt to limit behavior
///       to things that are safely portable, but they cannot catch every
///       possible situation for all drivers and API - so make sure to
///       actually test things!
class GFXTextureTarget : public GFXTarget
{
public:
   enum RenderSlot
   {
      DepthStencil,
      Color0,
      Color1,
      Color2,
      Color3,
      Color4,
      MaxRenderSlotId,
   };

   static GFXTextureObject *sDefaultDepthStencil;

   virtual ~GFXTextureTarget() {}

   /// Attach a surface to a given slot as part of this render target.
   ///
   /// @param slot What slot is used for multiple render target (MRT) effects.
   ///             Most of the time you'll use Color0.
   /// @param tex A texture and miplevel to bind for rendering, or else NULL/0
   ///            to clear a slot.
   /// @param mipLevel What level of this texture are we rendering to?
   /// @param zOffset  If this is a depth texture, what z level are we 
   ///                 rendering to?
    virtual void attachTexture(GFXTexHandle &tex, RenderSlot slot = Color0, U32 mipLevel = 0, U32 zOffset = 0) = 0;
    virtual void removeTexture( RenderSlot slot = Color0, U32 mipLevel = 0, U32 zOffset = 0) = 0;
   /// Support binding to cubemaps.
   ///
   /// @param slot What slot is used for multiple render target (MRT) effects.
   ///             Most of the time you'll use Color0.
   /// @param tex  What cubemap will we be rendering to?
   /// @param face A face identifier.
   /// @param mipLevel What level of this texture are we rendering to?
//   virtual void attachTexture(GFXCubemap *tex, U32 face, RenderSlot slot = Color0, U32 mipLevel=0) = 0;
};

#endif // _GFXTARGET_H_
