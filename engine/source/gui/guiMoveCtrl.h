//
// Created by Paul L Jan on 2013-07-29.
//



#ifndef __guiMoveCtrl_H_
#define __guiMoveCtrl_H_

#include "gui/guiControl.h"
#include "gui/guiTickCtrl.h"

class GuiMoveCtrl : public GuiTickCtrl
{
public:
    /// Camera Interpolation Mode.
    enum InterpolationMode
    {
        INVALID_INTERPOLATION_MODE,

        LINEAR,             ///< Standard Linear.
        SIGMOID             ///< Slow Start / Slow Stop.

    } mInterpolationMode;

    GuiMoveCtrl();
    virtual ~GuiMoveCtrl();
   
   bool onAdd(void);

    void setTargetPosition( const Point2I& position );
    void setTargetSize( const Point2I& targetSize );
    void setTargetArea( const RectI& targetArea );
    inline Point2I getTargetSize( void ) const                { return mTargetRect.extent; }
    inline Point2I getTargetPosition( void ) const                { return mTargetRect.point; }
    inline RectI   getTargetArea(void ) const                 { return mTargetRect; }

    static InterpolationMode getInterpolationModeEnum(const char* label);

    F32 getInterpolationTime (void) const {return mTransitionTime; }
    void setInterpolationTime( const F32 interpolationTime ) { mTransitionTime = interpolationTime;   }
    void setInterpolationMode( const InterpolationMode interpolationMode )   { mInterpolationMode = interpolationMode; }

    void zeroTime( void )  { mRenderTime = mPreTime = mPostTime = mCurrentTime = 0.0f;    }
    void updateTickTime( void );
    void resetTickTime( void )   { mRenderTime = mPreTime = mPostTime = mCurrentTime;   }
    void stopMove( void );
    void completeMove( void );
    void startMove( const F32 interpolationTime );
    void updateMove( void );

   void onRender(Point2I offset, const RectI &updateRect);

private:
    typedef GuiTickCtrl Parent;
    F32                 mPreTime;
    F32                 mPostTime;
    F32                 mRenderTime;
    F32                 mCurrentTime;
    bool mMoving;
    RectI mTargetRect;
    F32                 mTransitionTime;

    // So this can be instantiated and not be a pure virtual class
    virtual void interpolateTick( F32 delta );
    virtual void processTick();
    virtual void advanceTime( F32 timeDelta ) {};

    F32 interpolate( F32 from, F32 to, F32 delta );

public:
    DECLARE_CONOBJECT( GuiMoveCtrl );
};


#endif //__guiMoveCtrl_H_
