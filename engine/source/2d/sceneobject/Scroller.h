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

#ifndef _SCROLLER_H_
#define _SCROLLER_H_

#ifndef _SPRITE_BASE_H_
#include "2d/core/SpriteBase.h"
#endif

#ifndef _TEXTURE_MANAGER_H_
#include "graphics/gfxTextureManager.h"
#endif

//-----------------------------------------------------------------------------

class Scroller : public SpriteBase
{
    typedef SpriteBase      Parent;

private:
    F32                     mRepeatX;
    F32                     mRepeatY;
    F32                     mScrollX;
    F32                     mScrollY;

    F32                     mTextureOffsetX;
    F32                     mTextureOffsetY;

    Vector2               mPreTickTextureOffset;
    Vector2               mPostTickTextureOffset;
    Vector2               mRenderTickTextureOffset;
    
    Vector<GFXVertexPCT> mVertexBuffer;
   U32   mRows;
   U32   mColumns;

public:
    Scroller();
    virtual ~Scroller();

    static void initPersistFields();

    inline void setRepeatX( const F32 repeatX ) { setRepeat(repeatX, getRepeatY()); };
    inline void setRepeatY( const F32 repeatY ) { setRepeat(getRepeatX(), repeatY); };
    void setRepeat( const F32 repeatX, const F32 repeatY );
    void setScrollX(F32 x) { setScroll(x, getScrollY()); };
    void setScrollY(F32 y) { setScroll(getScrollX(), y); };
    void setScroll( F32 scrollX = 0.0f, F32 scrollY = 0.0f );
    inline void setScrollPositionX( F32 scrollX ) { setScrollPosition(scrollX, getScrollPositionY()); };
    inline void setScrollPositionY( F32 scrollY ) { setScrollPosition(getScrollPositionX(), scrollY); };
    void setScrollPosition( F32 scrollX, F32 scrollY );

    inline F32 getRepeatX() { return mRepeatX; };
    inline F32 getRepeatY() { return mRepeatY; };
    inline F32 getScrollX() { return mScrollX; };
    inline F32 getScrollY() { return mScrollY; };
    inline F32 getScrollPositionX() { return mTextureOffsetX; };
    inline F32 getScrollPositionY() { return mTextureOffsetY; };

    /// Tick Processing.
    void resetTickScrollPositions( void );
    void updateTickScrollPosition( void );
    virtual void interpolateObject( const F32 timeDelta );

    virtual bool onAdd();
    virtual void onRemove();
    virtual void integrateObject( const F32 totalTime, const F32 elapsedTime, DebugStats* pDebugStats );
    virtual void sceneRender( const SceneRenderState* pSceneRenderState, const SceneRenderRequest* pSceneRenderRequest, BatchRender* pBatchRenderer );

    virtual void setAngle( const F32 radians ) { Parent::setAngle( 0.0f ); }; // Stop angle being changed.
    virtual void setFixedAngle( const bool fixed ) { Parent::setFixedAngle( true ); } // Always fixed angle.

    virtual void copyTo(SimObject* object);

   void setRows( const U32 rows )                       { mRows = mClamp(rows, 1, 10); }
   void setColumns( const U32 columns )                 { mColumns = mClamp(columns, (U32)1, 10); }
   inline U32 getRows( void ) const                     { return mRows; }
   inline U32 getColumns( void ) const                  { return mColumns; }

   /// Declare Console Object.
    DECLARE_CONOBJECT(Scroller);

protected:
    static bool setRepeatX(void* obj, const char* data)                { static_cast<Scroller*>(obj)->setRepeatX( dAtof(data) ); return false; }
    static bool writeRepeatX( void* obj, StringTableEntry pFieldName ) { return mNotEqual( static_cast<Scroller*>(obj)->mRepeatX, 1.0f); }
    static bool setRepeatY(void* obj, const char* data)                { static_cast<Scroller*>(obj)->setRepeatY( dAtof(data) ); return false; }
    static bool writeRepeatY( void* obj, StringTableEntry pFieldName ) { return mNotEqual( static_cast<Scroller*>(obj)->mRepeatY, 1.0f); }
    static bool writeScrollX( void* obj, StringTableEntry pFieldName ) { return mNotZero(static_cast<Scroller*>(obj)->mScrollX); }
    static bool writeScrollY( void* obj, StringTableEntry pFieldName ) { return mNotZero(static_cast<Scroller*>(obj)->mScrollY); }
    static bool writeScrollPositionX( void* obj, StringTableEntry pFieldName ) { return mNotZero(static_cast<Scroller*>(obj)->mTextureOffsetX); }
    static bool writeScrollPositionY( void* obj, StringTableEntry pFieldName ) { return mNotZero(static_cast<Scroller*>(obj)->mTextureOffsetY); }
   static bool writeRows( void* obj, StringTableEntry pFieldName ) { return static_cast<Scroller*>(obj)->getRows() > 0; }
   static bool writeColumns( void* obj, StringTableEntry pFieldName ) { return static_cast<Scroller*>(obj)->getColumns() > 0; }
};

#endif // _SCROLLER_H_
