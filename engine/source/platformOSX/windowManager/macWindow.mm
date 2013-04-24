//-----------------------------------------------------------------------------
// Torque
// Copyright GarageGames, LLC 2011
//-----------------------------------------------------------------------------

#import <Cocoa/Cocoa.h>
#import "./macWindow.h"
#import "./osxTorqueView.h"

#import "console/console.h"
#import "AppDelegate.h"

MacWindow* MacWindow::sInstance = NULL;

MacWindow::MacWindow(U32 windowId, const char* windowText, Point2I clientExtent)
{
   mMouseLocked      = false;
   mShouldMouseLock  = false;
   mTitle            = NULL;
   mMouseCaptured    = false;
   
   mCocoaWindow      = NULL;
    _torqueView       = NULL;
   mCursorController = new MacCursorController( this );
   mOwningWindowManager = NULL;
   
   mFullscreen = false;
   mShouldFullscreen = false;
   mDefaultDisplayMode = NULL;
   
   mSkipMouseEvents = 0;
   
   mDisplay = kCGDirectMainDisplay;
   mMainDisplayBounds = mDisplayBounds = CGDisplayBounds(mDisplay);
   
   mWindowId = windowId;
   _initCocoaWindow(windowText, clientExtent);
   
   appEvent.notify(this, &MacWindow::_onAppEvent);
   
   sInstance = this;
}

MacWindow::~MacWindow()
{
   if(mFullscreen)
      _setFullscreen(false);

   appEvent.remove(this, &MacWindow::_onAppEvent);

   //ensure our view isn't the delegate
   [NSApp setDelegate:nil];
   
   if( mCocoaWindow )
   {
      NSWindow* window = mCocoaWindow;
      _disassociateCocoaWindow();
      
      [ window close ];
   }
   
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   sInstance = NULL;
}


void MacWindow::_initCocoaWindow(const char* windowText, Point2I clientExtent)
{
   // TODO: cascade windows on screen?
   
   // create the window
   U32 style;
   
    AppDelegate* appDelegate = [NSApp delegate];
    
  NSRect frame = NSMakeRect(0,0,clientExtent.x, clientExtent.y);
  
  style = NSTitledWindowMask | NSClosableWindowMask | NSMiniaturizableWindowMask | NSResizableWindowMask;

    mCocoaWindow = [[NSWindow alloc] initWithContentRect:frame
                                                styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask
                                                  backing:NSBackingStoreBuffered
                                                    defer:NO];

    appDelegate.window = mCocoaWindow;
    [mCocoaWindow setBackgroundColor:[NSColor blackColor]];

    // The full frame for a window must consider the title bar height as well
    // Thus, our NSWindow must be larger than the passed width and height
    frame = [NSWindow frameRectForContentRect:frame styleMask:NSTitledWindowMask | NSClosableWindowMask | NSResizableWindowMask];
    [mCocoaWindow setFrame:frame display:YES];

    if(windowText)
     [mCocoaWindow setTitle: [NSString stringWithUTF8String: windowText]];   

  // necessary to accept mouseMoved events
  [mCocoaWindow setAcceptsMouseMovedEvents:YES];
  
  // correctly position the window on screen
  [mCocoaWindow center];
   
   // create the opengl view. we don't care about its pixel format, because we
   // will be replacing its context with another one.
    _torqueView = [[OSXTorqueView alloc] initWithFrame:frame];
    [_torqueView initialize];    
    
   [_torqueView setTorqueWindow:this];
   [mCocoaWindow setContentView:_torqueView];
    mCocoaWindow.delegate = _torqueView;
    // Show the window and all its contents
    [mCocoaWindow makeKeyAndOrderFront:NSApp];
//    [mCocoaWindow setDelegate:torqueView];
    [mCocoaWindow center];
}

void MacWindow::_disassociateCocoaWindow()
{
   if( !mCocoaWindow )
      return;
      
   [mCocoaWindow setContentView:nil];
   [mCocoaWindow setDelegate:nil];   

   mCocoaWindow = NULL;
}

void MacWindow::setVideoMode(const GFXVideoMode &mode)
{
   mCurrentMode = mode;
   setSize(mCurrentMode.resolution);
   
   if(mTarget.isValid())
      mTarget->resetMode();

   _setFullscreen(mCurrentMode.fullScreen);
}

void MacWindow::_onAppEvent(WindowId, S32 evt)
{
   if(evt == LoseFocus && isFullscreen())
   {
      mShouldFullscreen = true;
      GFXVideoMode mode = mCurrentMode;
      mode.fullScreen = false;
      setVideoMode(mode);
   }
   
   if(evt == GainFocus && !isFullscreen() && mShouldFullscreen)
   {
      mShouldFullscreen = false;
      GFXVideoMode mode = mCurrentMode;
      mode.fullScreen = true;
      setVideoMode(mode);
   }
}

void MacWindow::_setFullscreen(bool fullScreen)
{
   if(mFullscreen == fullScreen)
      return;
   
   mFullscreen = fullScreen;
   
   if(mFullscreen)
   {
      Con::printf("Capturing display %x", mDisplay);
      CGDisplayCapture(mDisplay);
      [mCocoaWindow setAlphaValue:0.0f];
   }
   else
   {
      if(mDefaultDisplayMode)
      {
//         Con::printf("Restoring default display mode... width: %i height: %i bpp: %i", [[mDefaultDisplayMode valueForKey:@"Width"] intValue], 
//               [[mDefaultDisplayMode valueForKey:@"Height"] intValue], [[mDefaultDisplayMode valueForKey:@"BitsPerPixel"] intValue]);
//         CGDisplaySwitchToMode(mDisplay, (CFDictionaryRef)mDefaultDisplayMode);

          CGDisplaySetDisplayMode(mDisplay, mDefaultDisplayMode, nil);
          
          
         mDisplayBounds = CGDisplayBounds(mDisplay);
         if(mDisplay == kCGDirectMainDisplay)
            mMainDisplayBounds = mDisplayBounds;
      }
      
      Con::printf("Releasing display %x", mDisplay);
      CGDisplayRelease(mDisplay);
      [mCocoaWindow setAlphaValue:1.0f];
      mDefaultDisplayMode = NULL;
   }
}

void* MacWindow::getPlatformDrawable() const
{
   return [mCocoaWindow contentView];
}

void MacWindow::show()
{
   [mCocoaWindow makeKeyAndOrderFront:nil];
   [mCocoaWindow makeFirstResponder:[mCocoaWindow contentView]];
   appEvent.trigger(getWindowId(), WindowShown);
   appEvent.trigger(getWindowId(), GainFocus);
}

void MacWindow::close()
{
   [mCocoaWindow close];
   appEvent.trigger(mWindowId, LoseFocus);
   appEvent.trigger(mWindowId, WindowDestroy);
   
   mOwningWindowManager->_removeWindow(this);
   
   delete this;
}

void MacWindow::hide()
{
   [mCocoaWindow orderOut:nil];
   appEvent.trigger(getWindowId(), WindowHidden);
}

void MacWindow::setDisplay(CGDirectDisplayID display)
{
   mDisplay = display;
   mDisplayBounds = CGDisplayBounds(mDisplay);
}

PlatformWindow* MacWindow::getNextWindow() const
{
   return mNextWindow;
}

bool MacWindow::setSize(const Point2I &newSize)
{
    // Get the window's current frame
    NSRect frame = NSMakeRect([mCocoaWindow frame].origin.x, [mCocoaWindow frame].origin.y, newSize.x, newSize.y);
    
    // Get the starting position of the bar height
    F32 barOffset = frame.size.height;
    
    // If we are not going to full screen mode, get a new frame offset that accounts
    // for the title bar height
    if (!mFullscreen)
    {
        frame = [NSWindow frameRectForContentRect:frame styleMask:NSTitledWindowMask];
        
        // Set the new window frame
        [mCocoaWindow setFrame:frame display:YES];
        
        // Get the new position of the title bar
        barOffset -= frame.size.height;
    }
    else
    {
        // Otherwise, just go straight full screen
        [mCocoaWindow toggleFullScreen:nil];
    }
    
    // Update the frame of the torqueView to match the window
    frame = NSMakeRect([mCocoaWindow frame].origin.x, [mCocoaWindow frame].origin.y, newSize.x, newSize.y);
    NSRect viewFrame = NSMakeRect(0, 0, frame.size.width, frame.size.height);
    
    [_torqueView setFrame:viewFrame];
    
    [_torqueView updateContext];
    
    [mCocoaWindow makeKeyAndOrderFront:NSApp];
    [mCocoaWindow makeFirstResponder:_torqueView];
    
   return true;
}

void MacWindow::setClientExtent( const Point2I newExtent )
{
   if(!mFullscreen)
   {
      // Set the Client Area Extent (Resolution) of this window
      NSSize newSize = {static_cast<CGFloat>(newExtent.x), static_cast<CGFloat>(newExtent.y)};
      [mCocoaWindow setContentSize:newSize];
   }
   else
   {
      // In fullscreen we have to resize the monitor (it'll be good to change it back too...)
      if(!mDefaultDisplayMode)
         mDefaultDisplayMode = CGDisplayCopyDisplayMode(mDisplay);
      
       CGDisplayModeRef newMode;
//       = (NSDictionary*)CGDisplayBestModeForParameters(mDisplay, 32, newExtent.x, newExtent.y, NULL);
       
       
//      Con::printf("Switching to new display mode... width: %i height: %i bpp: %i",
//                  [[newMode valueForKey:@"Width"] intValue], [[newMode valueForKey:@"Height"] intValue], [[newMode valueForKey:@"BitsPerPixel"] intValue]); 

//       CGDisplaySwitchToMode(mDisplay, (CFDictionaryRef)newMode);
      CGDisplaySetDisplayMode(mDisplay, newMode, nil);
      mDisplayBounds = CGDisplayBounds(mDisplay);
      if(mDisplay == kCGDirectMainDisplay)
         mMainDisplayBounds = mDisplayBounds;
   }
}

const Point2I MacWindow::getClientExtent()
{
   if(!mFullscreen)
   {
      // Get the Client Area Extent (Resolution) of this window
      NSSize size = [[mCocoaWindow contentView] frame].size;
      return Point2I(size.width, size.height);
   }
   else
   {
      return Point2I(mDisplayBounds.size.width, mDisplayBounds.size.height);
   }
}

void MacWindow::setBounds( const RectI &newBounds )
{
   NSRect newFrame = NSMakeRect(newBounds.point.x, newBounds.point.y, newBounds.extent.x, newBounds.extent.y);
   [mCocoaWindow setFrame:newFrame display:YES];
}

const RectI MacWindow::getBounds() const
{
   if(!mFullscreen)
   {
      // Get the position and size (fullscreen windows are always at (0,0)).
      NSRect frame = [mCocoaWindow frame];
      return RectI(frame.origin.x, frame.origin.y, frame.size.width, frame.size.height);
   }
   else
   {
      return RectI(0, 0, mDisplayBounds.size.width, mDisplayBounds.size.height);
   }
}

void MacWindow::setPosition( const Point2I newPosition )
{
   NSScreen *screen = [mCocoaWindow screen];
   NSRect screenFrame = [screen frame];

   NSPoint pos = {static_cast<CGFloat>(newPosition.x), newPosition.y + screenFrame.size.height};
   [mCocoaWindow setFrameTopLeftPoint: pos];
}

const Point2I MacWindow::getPosition()
{
   NSScreen *screen = [mCocoaWindow screen];
   NSRect screenFrame = [screen frame];
   NSRect frame = [mCocoaWindow frame];

   return Point2I(frame.origin.x, screenFrame.size.height - (frame.origin.y + frame.size.height));
}

void MacWindow::centerWindow()
{
   [mCocoaWindow center];
}

Point2I MacWindow::clientToScreen( const Point2I& pos )
{
   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
   
   p = [ mCocoaWindow convertBaseToScreen: p ];
   return Point2I( p.x, p.y );
}

Point2I MacWindow::screenToClient( const Point2I& pos )
{
   NSPoint p = { static_cast<CGFloat>(pos.x), static_cast<CGFloat>(pos.y) };
   
   p = [ mCocoaWindow convertScreenToBase: p ];
   return Point2I( p.x, p.y );
}

bool MacWindow::isFocused()
{
   return [mCocoaWindow isKeyWindow];
}

bool MacWindow::isOpen()
{
   // Maybe check if _window != NULL ?
   return true;
}

bool MacWindow::isVisible()
{
   return !isMinimized() && ([mCocoaWindow isVisible] == YES);
}
   
void MacWindow::setFocus()
{
   [mCocoaWindow makeKeyAndOrderFront:nil];
}

void MacWindow::signalGainFocus()
{
   if(isFocused())
      [[mCocoaWindow delegate] performSelector:@selector(signalGainFocus)];
}

void MacWindow::minimize()
{
   if(!isVisible())
      return;
      
   [mCocoaWindow miniaturize:nil];
   appEvent.trigger(getWindowId(), WindowHidden);

//    //-----------------------------------------------------------------------------
//    // Simulates the user clicking the minimize button by momentarily highlighting
//    // the button, then minimizing the window.
//    void Platform::minimizeWindow()
//    {
//        NSApplication *application = [NSApplication sharedApplication];
//        NSWindow *keyWindow = [application keyWindow];
//        [keyWindow miniaturize:keyWindow];
//    }

}

void MacWindow::maximize()
{
   if(!isVisible())
      return;
   
   // GFX2_RENDER_MERGE 
   //[mCocoaWindow miniaturize:nil];
   //appEvent.trigger(getWindowId(), WindowHidden);
}

void MacWindow::restore()
{
   if(!isMinimized())
      return;
   
   [mCocoaWindow deminiaturize:nil];
   appEvent.trigger(getWindowId(), WindowShown);
}

bool MacWindow::isMinimized()
{
    return false;
//   return [mCocoaWindow isMiniaturized] == YES;
}

bool MacWindow::isMaximized()
{
   return false;
}

void MacWindow::clearFocus()
{
   // Clear the focus state for this Window.  
   // If the Window does not have focus, nothing happens.
   // If the Window has focus, it relinquishes it's focus to the Operating System
   
   // TODO: find out if we can do anything correct here. We are instructed *not* to call [NSWindow resignKeyWindow], and we don't necessarily have another window to assign as key.
}

bool MacWindow::setCaption(const char* windowText)
{
   mTitle = windowText;
   [mCocoaWindow setTitle:[NSString stringWithUTF8String:mTitle]];
   return true;
}

void MacWindow::_doMouseLockNow()
{
   if(!isVisible())
      return;
      
   if(mShouldMouseLock == mMouseLocked && mMouseLocked != isCursorVisible())
      return;
   
   if(mShouldMouseLock)
      _dissociateMouse();
   else
      _associateMouse();
   
   // hide the cursor if we're locking, show it if we're unlocking
   setCursorVisible(!shouldLockMouse());

   mMouseLocked = mShouldMouseLock;

   return;
}

void MacWindow::_associateMouse()
{
   CGAssociateMouseAndMouseCursorPosition(true);
}

void MacWindow::_dissociateMouse()
{
   _centerMouse();
   CGAssociateMouseAndMouseCursorPosition(false);
}

void MacWindow::_centerMouse()
{
   NSRect frame = [mCocoaWindow frame];
   
   // Deal with the y flip (really fun when more than one monitor is involved)
   F32 offsetY = mMainDisplayBounds.size.height - mDisplayBounds.size.height;
   frame.origin.y = (mDisplayBounds.size.height + offsetY) - (S32)frame.origin.y - (S32)frame.size.height;
   mCursorController->setCursorPosition(frame.origin.x + frame.size.width / 2, frame.origin.y + frame.size.height / 2);
}
