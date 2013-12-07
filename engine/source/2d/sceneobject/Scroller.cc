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

#include "Scroller.h"
#include "graphics/gfxDevice.h"
#include "math/mMathFn.h"
#include "console/consoleTypes.h"
#include "Scroller_ScriptBinding.h"
#include "2d/scene/Layer.h"

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(Scroller);

//------------------------------------------------------------------------------

Scroller::Scroller() :
    mRepeatX(1.0f),
    mRepeatY(1.0f),
    mScrollX(0.0f),
    mScrollY(0.0f),
    mTextureOffsetX(0.0f),
    mTextureOffsetY(0.0f),
    mRows(1), mColumns(1)
{
   // Use a static body by default.
   mBodyDefinition.type = b2_staticBody;

   // Use fixed rotation by default.
   mBodyDefinition.fixedRotation = true;
}

//------------------------------------------------------------------------------

Scroller::~Scroller()
{
}

//------------------------------------------------------------------------------

void Scroller::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    addProtectedField("repeatX", TypeF32, Offset(mRepeatX, Scroller), &setRepeatX, &defaultProtectedGetFn, &writeRepeatX, "");
    addProtectedField("repeatY", TypeF32, Offset(mRepeatY, Scroller), &setRepeatY, &defaultProtectedGetFn, &writeRepeatY, "");
    addField("scrollX", TypeF32, Offset(mScrollX, Scroller), &writeScrollX, "");
    addField("scrollY", TypeF32, Offset(mScrollY, Scroller), &writeScrollY, "");
    addField("scrollPositionX", TypeF32, Offset(mTextureOffsetX, Scroller), &writeScrollPositionX, "");
    addField("scrollPositionY", TypeF32, Offset(mTextureOffsetY, Scroller), &writeScrollPositionY, "");
   addField( "MeshRows", TypeS32, Offset(mRows, Scroller), &writeRows, "" );
   addField( "MeshColumns", TypeS32, Offset(mColumns, Scroller), &writeColumns, "" );
}

//------------------------------------------------------------------------------

void Scroller::copyTo(SimObject* object)
{
   Parent::copyTo(object);

   AssertFatal(dynamic_cast<Scroller*>(object), "Scroller::copyTo() - Object is not the correct type.");
   Scroller* scroller = static_cast<Scroller*>(object);

   scroller->setRepeat(getRepeatX(), getRepeatY());
   scroller->setScroll(getScrollX(), getScrollY());
   scroller->setScrollPosition(getScrollPositionX(), getScrollPositionY());
}

//------------------------------------------------------------------------------

bool Scroller::onAdd()
{
    // Call Parent.
    if(!Parent::onAdd())
        return false;

    // Reset Tick Scroll Positions.
    resetTickScrollPositions();

    // Return Okay.
    return true;
}

//------------------------------------------------------------------------------

void Scroller::onRemove()
{
    // Call Parent.
    Parent::onRemove();
}

//------------------------------------------------------------------------------

void Scroller::integrateObject( const F32 totalTime, const F32 elapsedTime, DebugStats* pDebugStats )
{
    // Call Parent.
    Parent::integrateObject( totalTime, elapsedTime, pDebugStats );

    // Calculate texel shift per world-unit.
    const F32 scrollTexelX = mRepeatX / getSize().x;
    const F32 scrollTexelY = mRepeatY / getSize().y;

    // Calculate Scrolling Offsets.
    const F32 scrollOffsetX = scrollTexelX * mScrollX * elapsedTime;
    const F32 scrollOffsetY = scrollTexelY * mScrollY * elapsedTime;

    // Calculate new offset.
    mTextureOffsetX += scrollOffsetX;
    mTextureOffsetY += scrollOffsetY;

    // Update Tick Scroll Position.
    // NOTE:-   We *must* do the tick update here!
    updateTickScrollPosition();

    // Make sure the offsets used don't under/overflow.
    // NOTE-    We could simply use 'mFmod' on the offsets but unfortunately
    //          we need to ensure that we can do a modulo simultaneously on both
    //          the pre/post ticks values otherwise the pre/post interpolation
    //          won't worked correctly resulting in a nasty wrap 'hitch'.

    // Calculate Renormalized Offsets.
    const F32 renormalizedPreOffsetX = mFmod( mPreTickTextureOffset.x, 1.0f );
    const F32 renormalizedPreOffsetY = mFmod( mPreTickTextureOffset.y, 1.0f );
    const F32 renormalizedPostOffsetX = mFmod( mPostTickTextureOffset.x, 1.0f );
    const F32 renormalizedPostOffsetY = mFmod( mPostTickTextureOffset.y, 1.0f );

    // Scrolling X Positive?
    if ( mGreaterThanZero(scrollOffsetX) )
    {
        // Yes, so old/new normalised simultaneously?
        if ( mLessThan(renormalizedPreOffsetX, renormalizedPostOffsetX) )
        {
            // Yes, so normalised offset.
            mTextureOffsetX = renormalizedPostOffsetX;
            // Normalise Pre/Post Ticks.
            mPreTickTextureOffset.x = renormalizedPreOffsetX;
            mPostTickTextureOffset.x = renormalizedPostOffsetX;
        }        
    }
    else
    {
        // No, so old/new normalised simultaneously?
        if ( mGreaterThan(renormalizedPreOffsetX, renormalizedPostOffsetX) )
        {
            // Yes, so normalised offset.
            mTextureOffsetX = renormalizedPostOffsetX;
            // Normalise Pre/Post Ticks.
            mPreTickTextureOffset.x = renormalizedPreOffsetX;
            mPostTickTextureOffset.x = renormalizedPostOffsetX;
        }        
    }

    // Scrolling Y Positive?
    if ( mGreaterThanZero(scrollOffsetY) )
    {
        // Yes, so old/new normalised proportionally?
        if ( mLessThan(renormalizedPreOffsetY, renormalizedPostOffsetY) )
        {
            // Yes, so normalised offset.
            mTextureOffsetY = renormalizedPostOffsetY;
            // Normalise Pre/Post Ticks.
            mPreTickTextureOffset.y = renormalizedPreOffsetY;
            mPostTickTextureOffset.y = renormalizedPostOffsetY;
        }        
    }
    else
    {
        // No, so old/new normalised proportionally?
        if ( mGreaterThan(renormalizedPreOffsetY, renormalizedPostOffsetY) )
        {
            // Yes, so normalised offset.
            mTextureOffsetY = renormalizedPostOffsetY;
            // Normalise Pre/Post Ticks.
            mPreTickTextureOffset.y = renormalizedPreOffsetY;
            mPostTickTextureOffset.y = renormalizedPostOffsetY;
        }        
    }
}

//------------------------------------------------------------------------------

void Scroller::interpolateObject( const F32 timeDelta )
{
    // Base object interpolation.
    Parent::interpolateObject( timeDelta );

    // Calculate Render Tick Position.
    mRenderTickTextureOffset = mLerp(mPreTickTextureOffset, mPostTickTextureOffset, timeDelta);
}

//------------------------------------------------------------------------------

void Scroller::resetTickScrollPositions( void )
{
    // Reset Scroll Positions.
    mRenderTickTextureOffset.Set( mTextureOffsetX, mTextureOffsetY );
    mPreTickTextureOffset = mPostTickTextureOffset = mRenderTickTextureOffset;
}

//------------------------------------------------------------------------------

void Scroller::updateTickScrollPosition( void )
{
    // Store Pre Tick Scroll Position.
    mPreTickTextureOffset = mPostTickTextureOffset;

    // Store Current Tick Scroll Position.
    mPostTickTextureOffset.Set( mTextureOffsetX, mTextureOffsetY );

    // Render Tick Position is at Pre-Tick Scroll Position.
    mRenderTickTextureOffset = mPreTickTextureOffset;
};


//------------------------------------------------------------------------------
void Scroller::sceneRender( const t2dSceneRenderState * pSceneRenderState, const SceneRenderRequest* pSceneRenderRequest, BatchRender* pBatchRenderer )
{
    // Finish if we can't render.
    if ( !ImageFrameProvider::validRender() )
        return;
    
    // Fetch texture and texture area.
    const ImageAsset::FrameArea::TexelArea& frameTexelArea = getProviderImageFrameArea().mTexelArea;
    GFXTexHandle& texture = getProviderTexture();
    
    // Calculate render offset.
    F32 renderOffsetX = mFmod( mRenderTickTextureOffset.x, 1.0f );
    F32 renderOffsetY = mFmod( mRenderTickTextureOffset.y, 1.0f );
    if ( renderOffsetX < 0.0f ) renderOffsetX += 1.0f;
    if ( renderOffsetY < 0.0f ) renderOffsetY += 1.0f;
   
//   Con::printf("repeatX: %f repeatY: %f", mRepeatX, mRepeatY);
//   Con::printf("renderOffsetX: %.2f renderOffsetY: %.2f", renderOffsetX, renderOffsetY);
   
    // Calculate region dimensions.
    const F32 regionWidth = (mRenderOOBB[1].x - mRenderOOBB[0].x) / mRepeatX;
    const F32 regionHeight = (mRenderOOBB[3].y - mRenderOOBB[0].y) / mRepeatY;
   
    // Flush any existing batches.
    pBatchRenderer->flush();
   
    Vector<F32> xDivisions;
    Vector<F32> yDivisions;

    xDivisions.push_back(0.0f);
    while(xDivisions.back() <= (F32)mRepeatX)
    {
       xDivisions.push_back(xDivisions.back()+(1.0f-renderOffsetX));
       if (xDivisions.back() >= (F32)mRepeatX)
          break;
       xDivisions.push_back(xDivisions.back()+renderOffsetX);
    }
   xDivisions.pop_back();

   yDivisions.push_back(0.0f);
   while(yDivisions.back() < (F32)mRepeatY)
   {
      yDivisions.push_back(yDivisions.back()+(1.0f-renderOffsetY));
      if (yDivisions.back() >= (F32)mRepeatY)
         break;
      yDivisions.push_back(yDivisions.back()+renderOffsetY);
   }
   yDivisions.pop_back();


//   std::string tempy = "";
//   {
//      for (F32 i:yDivisions) {
//         char buff[100];
//         sprintf(buff, "%.2f ", i);
//         tempy += buff;
//      }
//   }
//   std::string tempx = "";
//   {
//      for (F32 i:xDivisions) {
//         char buff[100];
//         sprintf(buff, "%.2f ", i);
//         tempx += buff;
//      }
//   }
   
//   Con::printf("xsize: %d, ysize: %d", xDivisions.size(), yDivisions.size());
//   Con::printf("xdiv %s", tempx.c_str());
//   Con::printf("ydiv %s", tempy.c_str());
   
   if (yDivisions.size() > 6)
   {
      int ek = 90;
   }

   F32 baseX = mRenderOOBB[0].x;
    F32 baseY = mRenderOOBB[0].y;
    F32 nextX;
    F32 nextY;
    
    F32 texX1;
    F32 texY1 = frameTexelArea.mTexelUpper.y - (frameTexelArea.mTexelHeight*renderOffsetY);
    F32 texX2;
    F32 texY2;
    
   Vector<GFXVertexPCT> verts;
   Vector<U16> index;
   
   U32 count = 0;
   U32 indexCount = 0;
   U32 yc = 0;
   std::string textempy = "";
   for ( auto yitr = yDivisions.begin(); yitr != yDivisions.end(); yitr++)
   {
//      Con::printf("start ydiv %d", yc);
      yc++;
      baseY = mRenderOOBB[0].y + (*yitr * regionHeight);
      F32 a1 = frameTexelArea.mTexelUpper.y;
      F32 a2 = frameTexelArea.mTexelHeight*(F32)mFmod((*yitr+renderOffsetY), 1.0f);

      if (yitr+1 == yDivisions.end())
      {
         nextY = mRenderOOBB[2].y;
         texY2 = texY1 - frameTexelArea.mTexelHeight*(F32)mFmod(((mRepeatY)-(*yitr)), 1.0f);
      }
      else
      {
         nextY = mRenderOOBB[0].y + (*(yitr+1) * regionHeight);
         texY2 = texY1 - frameTexelArea.mTexelHeight*(F32)mFmod((*(yitr+1)-(*yitr)), 1.0f);
      }
      
      
      
//      if (texY2 < 0.0f && !(texY1 > 0.0f))
//      {
//         texY2 += 1.0;
//         texY1 += 1.0;
//      }
      
      if (texY2 == texY1)
      {
         texY2 -= frameTexelArea.mTexelHeight;
         if (texY2 < 0.0 || texY2 > 1.0)
            texY2 = mClampF(texY2, 0.0, 1.0);
      }
//      char tempbuf[255];
//      sprintf(tempbuf, "%.2f %.2f ", texY1, texY2);
//      textempy += tempbuf;
      
      U32 xc = 0;
//      std::string textempx = "";
      texX1 = frameTexelArea.mTexelLower.x + (frameTexelArea.mTexelWidth*renderOffsetX);
      for ( auto xitr = xDivisions.begin(); xitr != xDivisions.end(); xitr++)
      {
//         Con::printf("start xdiv %d", xc);
         xc++;
         baseX = mRenderOOBB[0].x + (*xitr * regionWidth);
//         texX1 = frameTexelArea.mTexelLower.x + frameTexelArea.mTexelWidth*(F32)mFmod((*xitr+renderOffsetX), 1.0f);

         if (xitr+1 == xDivisions.end())
         {
            nextX = mRenderOOBB[2].x;
            texX2 = texX1 + frameTexelArea.mTexelWidth*(F32)mFmod(((mRepeatX)-(*xitr)), 1.0f);
         }
         else
         {
            nextX = mRenderOOBB[0].x + (*(xitr+1) * regionWidth);
            texX2 = texX1 + frameTexelArea.mTexelWidth*(F32)mFmod((*(xitr+1)-(*xitr)), 1.0f);
         }
         if (texX2 == texX1)
         {
            texX2 += frameTexelArea.mTexelWidth;
            if (texX2 < 0.0 || texX2 > 1.0)
               texX2 = mClampF(texX2, 0.0, 1.0);
         }

         verts.setSize((mRows+1)*(mColumns+1)*xDivisions.size()*yDivisions.size());
         index.setSize(((mRows*mColumns*4) + mRows*2)*xDivisions.size()*yDivisions.size());
         
         U16 vert_offset = count;
         
         for (U32 j = 0; j <= mRows; j++)
         {
            for (U32 i = 0; i <= mColumns; i++ )
            {
               F32 qwX2 = mLerp(baseX, nextX, ((F32)i)/(F32)mColumns);
               F32 qwY2 = mLerp(baseY, nextY, ((F32)j)/(F32)mRows);
               F32 qtX2 = mLerp(texX1, texX2, ((F32)i)/(F32)mColumns);
               F32 qtY2 = mLerp(texY1, texY2, ((F32)j)/(F32)mRows);

               verts[count].point.set(qwX2, qwY2, 0.0);
               verts[count].color.set(mBlendColor*getSceneLayerObj()->getLight());
               verts[count].texCoord.set(qtX2, qtY2);
//               Con::printf("point %d = (%d, %d)", count, i, j);
               count++;
            }
         }

//         for (U16 j = 0; j < 1; j++)
//         {
//            for (U16 i = 0; i <= 1; i++ )
//            {
//               index[indexCount]   = vert_offset+((U16)((j*(1+1))+i) );
//               index[indexCount+1] = vert_offset+((U16)(((j+1)*(1+1))+i) );
//               indexCount += 2;
//            }
//         }

         for (U16 j = 0; j < mRows; j++)
         {
            for (U16 i = 0; i <= mColumns; i++ )
            {
               index[indexCount]   = vert_offset+((U16)((j*(mColumns+1))+i) );
               index[indexCount+1] = vert_offset+((U16)(((j+1)*(mColumns+1))+i) );
               indexCount += 2;
            }

            if (j+1 != mRows )  // degenerate triangles between rows.
            {
               index[indexCount] = index[indexCount-1];
               index[indexCount+1] = vert_offset+ ((U16)(j+1)*(mColumns+1));
               indexCount += 2;
            }
         }
         
         if (xitr+1 != xDivisions.end())
         {
            index.setSize(index.size()+2);
            index[indexCount] = index[indexCount-1];
            index[indexCount+1] = count;
            indexCount += 2;
         }

//         char tempbuf[255];
//         sprintf(tempbuf, "%.2f %.2f ", texX1, texX2);
//         textempx += tempbuf;

         texX1 = texX2;
         if (texX1 > 0.999)
            texX1 -= 1.00f;

      }
//      Con::printf("xTex %s", textempx.c_str());

      if (yitr+1 != yDivisions.end())
      {
         index.setSize(index.size()+2);
         index[indexCount] = index[indexCount-1];
         index[indexCount+1] = count;
         indexCount += 2;
      }
      
      texY1 = texY2;
      if (texY1 < 0.001)
         texY1 += 1.00f;
   }
//   Con::printf("yTex %s", textempy.c_str());

   if (yDivisions.size() > 6)
   {
      int ek = 90;
   }

//   for (int i = 0; i < indexCount; i++)
//   {
//      Con::printf("indexsize: %d: count %d, ", index.size(), indexCount);
//   }
   
   index.setSize(indexCount);
    pBatchRenderer->SubmitIndexedTriangleStrip(verts, texture, index);
    // Flush the scroller batches.
    pBatchRenderer->flush();
    
}

//------------------------------------------------------------------------------

void Scroller::setRepeat( const F32 repeatX, const F32 repeatY )
{
    // Warn.
    if ( repeatX <= 0.0f || repeatY <= 0.0f )
    {
        Con::warnf("Scroller::setRepeat() - Repeats must be greater than zero!");
        return;
    }

    // Set Repeat X/Y.
    mRepeatX = repeatX;
    mRepeatY = repeatY;
}

//------------------------------------------------------------------------------

void Scroller::setScroll( F32 scrollX, F32 scrollY )
{
    // Set Scroll X/Y.
    mScrollX = scrollX;
    mScrollY = scrollY;

    // Reset Tick Scroll Positions.
    resetTickScrollPositions();
}

//------------------------------------------------------------------------------

void Scroller::setScrollPosition( F32 scrollX, F32 scrollY )
{
    // Yes, so calculate texel shift per world-unit.
    const F32 scrollTexelX = mRepeatX / getSize().x;
    const F32 scrollTexelY = mRepeatY / getSize().y;

    // Calculate new offset and clamp.
    mTextureOffsetX = mFmod( scrollTexelX * scrollX, 1.0f );
    mTextureOffsetY = mFmod( scrollTexelY * scrollY, 1.0f );

    // Reset Tick Scroll Positions.
    resetTickScrollPositions();
}
