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

#include "platform/platform.h"
#include "graphics/gfxVertexFormat.h"

#include "debug/profiler.h"
//#include "core/util/hashFunction.h"
#include "graphics/gfxDevice.h"


namespace GFXSemantic
{
    const StringTableEntry POSITION = StringTable->insert( "POSITION" );
    const StringTableEntry NORMAL = StringTable->insert( "NORMAL" );
    const StringTableEntry BINORMAL = StringTable->insert( "BINORMAL" );
    const StringTableEntry TANGENT = StringTable->insert( "TANGENT" );
    const StringTableEntry TANGENTW = StringTable->insert( "TANGENTW" );
    const StringTableEntry COLOR = StringTable->insert( "COLOR" );
    const StringTableEntry TEXCOORD = StringTable->insert( "TEXCOORD" );
    const StringTableEntry SIZE = StringTable->insert( "SIZE" );
}


U32 GFXVertexElement::getSizeInBytes() const
{
   switch ( mType )
   {
      case GFXDeclType_Float:
         return 4;

      case GFXDeclType_Float2:
         return 8;

      case GFXDeclType_Float3:
         return 12;

      case GFXDeclType_Float4:
         return 16;

      case GFXDeclType_Color:
         return 4;
           
       case GFXDeclType_TexCoord:
           return 8;

      default:
         return 0;
   };
}


GFXVertexFormat::GFXVertexFormat()
   :  mDirty( true ),
      mHasColor( false ),
      mHasNormal( false ),
      mHasTangent( false ),
      mHasSize(false),
      mTexCoordCount( 0 ),
      mSizeInBytes( 0 ),
      mDecl( NULL )
{
   VECTOR_SET_ASSOCIATION( mElements );
}

void GFXVertexFormat::copy( const GFXVertexFormat &format )
{
   mDirty = format.mDirty;
   mHasNormal = format.mHasNormal;
   mHasTangent = format.mHasTangent;
   mHasColor = format.mHasColor;
    mHasSize = format.mHasSize;
   mTexCoordCount = format.mTexCoordCount;
   mSizeInBytes = format.mSizeInBytes;
   mDescription = format.mDescription;
   mElements = format.mElements;
   mDecl = format.mDecl;
}

void GFXVertexFormat::append( const GFXVertexFormat &format, U32 streamIndex )
{
   for ( U32 i=0; i < format.getElementCount(); i++ )
   {
      mElements.increment();
      mElements.back() = format.getElement( i );
      if ( streamIndex != -1 )
         mElements.back().mStreamIndex = streamIndex;
   }

   mDirty = true;
}

void GFXVertexFormat::clear()
{ 
   mDirty = true;
   mElements.clear(); 
   mDecl = NULL;
}

void GFXVertexFormat::addElement( const StringTableEntry semantic, GFXDeclType type, U32 index, U32 stream )
{ 
   mDirty = true;
   mElements.increment();
   mElements.back().mStreamIndex = stream;
   mElements.back().mSemantic = StringTable->insert(semantic);
   mElements.back().mSemanticIndex = index;
   mElements.back().mType = type;
}

const StringTableEntry& GFXVertexFormat::getDescription() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mDescription;
}

GFXVertexDecl* GFXVertexFormat::getDecl() const
{
   if ( !mDecl || mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDecl();

   return mDecl;
}

bool GFXVertexFormat::hasNormal() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasNormal;
}

bool GFXVertexFormat::hasTangent() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasTangent;
}

bool GFXVertexFormat::hasColor() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mHasColor;
}

bool GFXVertexFormat::hasSize() const
{
    if ( mDirty )
        const_cast<GFXVertexFormat*>(this)->_updateDirty();

    return mHasSize;
}

U32 GFXVertexFormat::getTexCoordCount() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mTexCoordCount;
}

U32 GFXVertexFormat::getSizeInBytes() const
{
   if ( mDirty )
      const_cast<GFXVertexFormat*>(this)->_updateDirty();

   return mSizeInBytes;
}

void GFXVertexFormat::_updateDirty()
{
   PROFILE_SCOPE( GFXVertexFormat_updateDirty );

   mTexCoordCount = 0;

   mHasColor = false;
   mHasNormal = false;
   mHasTangent = false;
   mSizeInBytes = 0;

   String desc;

   for ( U32 i=0; i < mElements.size(); i++ )
   {
      const GFXVertexElement &element = mElements[i];

      desc += String::ToString( "%d,%s,%d,%d\n",   element.mStreamIndex,
                                                   element.mSemantic,
                                                   element.mSemanticIndex, 
                                                   element.mType );

      if ( element.getSemantic() == GFXSemantic::NORMAL )
         mHasNormal = true;
      else if ( element.getSemantic() == GFXSemantic::TANGENT )
         mHasTangent = true;
      else if ( element.getSemantic() == GFXSemantic::COLOR )
         mHasColor = true;
      else if ( element.getSemantic() == GFXSemantic::TEXCOORD )
         ++mTexCoordCount;
      else if ( element.getSemantic() == GFXSemantic::SIZE )
          mHasSize = true;

      mSizeInBytes += element.getSizeInBytes();
   }

   // Intern the string for fast compares later.
   mDescription = desc;

   mDirty = false;
}

void GFXVertexFormat::_updateDecl()
{
   PROFILE_SCOPE( GFXVertexFormat_updateDecl );

   if ( mDirty )
      _updateDirty();

   mDecl = GFX->allocVertexDecl( this );
}
