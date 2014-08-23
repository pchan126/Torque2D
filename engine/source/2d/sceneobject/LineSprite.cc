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

#ifndef _SPRITE_H_
#include "Sprite.h"
#endif

#include "graphics/gfxDevice.h"

#ifndef _STRINGBUFFER_H_
#include "string/stringBuffer.h"
#endif

// Script bindings.
#include "Sprite_ScriptBinding.h"
#include "2d/scene/Layer.h"

//------------------------------------------------------------------------------

IMPLEMENT_CONOBJECT(Sprite);

//------------------------------------------------------------------------------

Sprite::Sprite() :
    mFlipX(false),
    mFlipY(false), mRows(1), mColumns(1), mSquishy(false)
{
}

//------------------------------------------------------------------------------

Sprite::~Sprite()
{
}

//------------------------------------------------------------------------------

void Sprite::copyTo(SimObject* object)
{
    // Call to parent.
    Parent::copyTo(object);

    // Cast to sprite.
    Sprite* pSprite = static_cast<Sprite*>(object);

    // Sanity!
    AssertFatal(pSprite != nullptr, "Sprite::copyTo() - Object is not the correct type.");
   assert(pSprite != nullptr);

    /// Render flipping.
    pSprite->setFlip( getFlipX(), getFlipY() );
    pSprite->mSquishy = mSquishy;
    pSprite->mNodes = mNodes;
}

//------------------------------------------------------------------------------

void Sprite::initPersistFields()
{
    // Call parent.
    Parent::initPersistFields();

    /// Render flipping.
    addField("FlipX", TypeBool, Offset(mFlipX, Sprite), &writeFlipX, "");
    addField("FlipY", TypeBool, Offset(mFlipY, Sprite), &writeFlipY, "");
    addField( "MeshRows", TypeS32, Offset(mRows, Sprite), &writeRows, "" );
    addField( "MeshColumns", TypeS32, Offset(mColumns, Sprite), &writeColumns, "" );
}

//------------------------------------------------------------------------------

void Sprite::setSize( const Vector2& size )
{
   SceneObject::setSize(size);
   
   setColumns(U32(mCeil(size.x/5)));
   setRows(U32(mCeil(size.y/5)));
}


void Sprite::sceneRender( const t2dSceneRenderState * pSceneRenderState, const SceneRenderRequest* pSceneRenderRequest, BatchRender* pBatchRenderer )
{
    if (mSquishy)
    {
        renderMesh(pSceneRenderState, pSceneRenderRequest, pBatchRenderer);
        return;
    }

    if (!mShaderAsset.isNull())
        pBatchRenderer->setShader(*mShaderAsset);

    // Let the parent render.
    ImageFrameProvider::render(
        getFlipX(), getFlipY(),
        mRenderOOBB[0],
        mRenderOOBB[1],
        mRenderOOBB[2],
        mRenderOOBB[3],
        pBatchRenderer,
        getBlendColor()*getSceneLayerObj()->getLight()
   , mRows, mColumns);

    if (!mShaderAsset.isNull())
        pBatchRenderer->clearShader();
}


void Sprite::renderMesh( const t2dSceneRenderState * pSceneRenderState, const SceneRenderRequest* pSceneRenderRequest, BatchRender* pBatchRenderer )
{
    if (!mShaderAsset.isNull())
        pBatchRenderer->setShader(*mShaderAsset);

    // Finish if we can't render.
    if ( validRender() )
    {
        // Fetch texel area.
        ImageAsset::FrameArea::TexelArea texelArea = getProviderImageFrameArea().mTexelArea;

        // Flip texture coordinates appropriately.
        texelArea.setFlip( getFlipX(), getFlipY() );

        // Fetch lower/upper texture coordinates.
        const Vector2& texLower = texelArea.mTexelLower;
        const Vector2& texUpper = texelArea.mTexelUpper;

        Vector<GFXVertexPCT> verts;

        Vector2 subVert0 = mRenderOOBB[0];
        Vector2 subVert3 = mRenderOOBB[3];
        for (int j = 1; j <= mRows; j++)
        {
            F32 texfactor = (F32)(j-1)/(F32)mRows;
            F32 factorY = (F32)j/(F32)mRows;
            Vector2 subVert1 = (mLerp(mRenderOOBB[0], mRenderOOBB[1], factorY));
            Vector2 subVert2 = (mLerp(mRenderOOBB[2], mRenderOOBB[3], factorY));

            verts.setSize(mColumns*2+2);
            for (int i = 0; i <= mColumns; i++)
            {
                F32 factor = (F32)i/(F32)mColumns;

                SceneObject *node1 = mNodes[(j-1)*(mColumns+1)+i];
                SceneObject *node2 = mNodes[(j)*(mColumns+1)+i];

                verts[i*2+0].point.set(node1->getRenderPosition().x, node1->getRenderPosition().y, 0.0);
                verts[i*2+1].point.set(node2->getRenderPosition().x, node2->getRenderPosition().y, 0.0);
                verts[i*2+0].texCoord.set(mLerp(texLower.x, texUpper.x, texfactor),
                        mLerp(texUpper.y, texLower.y, factor));
                verts[i*2+1].texCoord.set(mLerp(texLower.x, texUpper.x, factorY),
                        mLerp(texUpper.y, texLower.y, factor));
            }

            for (int i = 0; i < verts.size(); i++)
                verts[i].color = getBlendColor()*getSceneLayerObj()->getLight();

            pBatchRenderer->SubmitTriangleStrip(verts, getProviderTexture());
            subVert0 = subVert1;
            subVert3 = subVert2;
        }
    }

    if (!mShaderAsset.isNull())
        pBatchRenderer->clearShader();
}

void Sprite::setNode(SceneObject *obj, U32 row, U32 column) {
    if ((row * (mColumns+1) + column) > mNodes.size())
        return;

    mNodes[(row * (mColumns+1) + column)] = obj;
    mSquishy = true;
//    setBodyType( b2_staticBody );
}

void Sprite::clearNodes()
{
    mNodes.clear();
    mSquishy = false;
}