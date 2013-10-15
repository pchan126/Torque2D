//-----------------------------------------------------------------------------
// Copyright (c) 2012 GarageGames, LLC
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
#include "graphics/sim/cubemapAsset.h"

#include "console/consoleTypes.h"
#include "graphics/gfxCubemap.h"
#include "graphics/gfxDevice.h"
//#include "graphics/gfxTransformSaver.h"
//#include "graphics/gfxDebugEvent.h"
//#include "scene/sceneManager.h"
//#include "console/engineAPI.h"
#include "math/mathUtils.h"

#ifndef _ASSET_PTR_H_
#include "assets/assetPtr.h"
#endif

IMPLEMENT_CONOBJECT( CubemapAsset );

//------------------------------------------------------------------------------

ConsoleType( CubemapAssetPtr, TypeCubemapAssetPtr, sizeof(AssetPtr<CubemapAsset>), ASSET_ID_FIELD_PREFIX )

//-----------------------------------------------------------------------------

ConsoleGetType( TypeCubemapAssetPtr )
{
   // Fetch asset Id.
   return (*((AssetPtr<CubemapAsset>*)dptr)).getAssetId();
}

//-----------------------------------------------------------------------------

ConsoleSetType( TypeCubemapAssetPtr )
{
   // Was a single argument specified?
   if( argc == 1 )
   {
      // Yes, so fetch field value.
      const char* pFieldValue = argv[0];
      
      // Fetch asset pointer.
      AssetPtr<CubemapAsset>* pAssetPtr = dynamic_cast<AssetPtr<CubemapAsset>*>((AssetPtrBase*)(dptr));
      
      // Is the asset pointer the correct type?
      if ( pAssetPtr == nullptr )
      {
         // No, so fail.
         Con::warnf( "(TypeCubemapAssetPtr) - Failed to set asset Id '%d'.", pFieldValue );
         return;
      }
      
      // Set asset.
      pAssetPtr->setAssetId( pFieldValue );
      
      return;
   }
   
   // Warn.
   Con::warnf( "(TypeCubemapAssetPtr) - Cannot set multiple args to a single asset." );
}


CubemapAsset::CubemapAsset()
{
   mCubemap = nullptr;
   mDynamic = false;
   mDynamicSize = 512;
   mDynamicNearDist = 0.1f;
   mDynamicFarDist = 100.0f;
   mDynamicObjectTypeMask = 0;
#ifdef INIT_HACK
   mInit = false;
#endif
}

CubemapAsset::~CubemapAsset()
{
   mCubemap = nullptr;
}

//ConsoleDocClass( CubemapAsset, 
//   "@brief Used to create static or dynamic cubemaps.\n\n"
//   "This object is used with Material, WaterObject, and other objects for cubemap reflections.\n\n"
//   "A simple declaration of a static cubemap:\n"
//   "@tsexample\n"
//   "singleton CubemapAsset( SkyboxCubemap )\n"
//   "{\n"
//   "   cubeFace[0] = \"./skybox_1\";\n"
//   "   cubeFace[1] = \"./skybox_2\";\n"
//   "   cubeFace[2] = \"./skybox_3\";\n"
//   "   cubeFace[3] = \"./skybox_4\";\n"
//   "   cubeFace[4] = \"./skybox_5\";\n"
//   "   cubeFace[5] = \"./skybox_6\";\n"
//   "};\n"
//   "@endtsexample\n"
//   "@note The dynamic cubemap functionality in CubemapAsset has been depreciated in favor of ReflectorDesc.\n"
//   "@see ReflectorDesc\n"
//   "@ingroup GFX\n" );

void CubemapAsset::initPersistFields()
{
   addField( "cubeFace", TypeAssetLooseFilePath, Offset(mCubeFaceFile, CubemapAsset), 6, nullptr,
      "@brief The 6 cubemap face textures for a static cubemap.\n\n"
      "They are in the following order:\n"
      "  - cubeFace[0] is -X\n"
      "  - cubeFace[1] is +X\n"
      "  - cubeFace[2] is -Z\n"
      "  - cubeFace[3] is +Z\n"
      "  - cubeFace[4] is -Y\n"
      "  - cubeFace[5] is +Y\n" );

   addField("dynamic", TypeBool, Offset(mDynamic, CubemapAsset),
      "Set to true if this is a dynamic cubemap.  The default is false." );

   addField("dynamicSize", TypeS32, Offset(mDynamicSize, CubemapAsset),
      "The size of each dynamic cubemap face in pixels." );

   addField("dynamicNearDist", TypeF32, Offset(mDynamicNearDist, CubemapAsset),
      "The near clip distance used when rendering to the dynamic cubemap." );

   addField("dynamicFarDist", TypeF32, Offset(mDynamicFarDist, CubemapAsset),
      "The far clip distance used when rendering to the dynamic cubemap." );

   addField("dynamicObjectTypeMask", TypeS32, Offset(mDynamicObjectTypeMask, CubemapAsset),
      "The typemask used to filter the objects rendered to the dynamic cubemap." );

   Parent::initPersistFields();
}

bool CubemapAsset::onAdd()
{
   if( !Parent::onAdd() )
      return false;

   // Do NOT call this here as it forces every single cubemap defined to load its images, immediately, without mercy
   // createMap();

   return true;
}

void CubemapAsset::createMap()
{
   if( !mCubemap )
   {
//      if( mDynamic )
//      {
//         mCubemap = GFX->createCubemap();
//         mCubemap->initDynamic( mDynamicSize );
//         mDepthBuff = std::shared_ptr<GFXTextureObject>( mDynamicSize, mDynamicSize, GFXFormatD24S8,
//            &GFXDefaultZTargetProfile, avar("%s() - mDepthBuff (line %d)", __FUNCTION__, __LINE__));
//         mRenderTarget = GFX->allocRenderToTextureTarget();
//      }
//      else
      {
         bool initSuccess = true;

         for( U32 i=0; i<6; i++ )
         {
            if( !mCubeFaceFile[i].isEmpty() )
            {
               if(!mCubeFace[i].set(mCubeFaceFile[i], &GFXDefaultStaticDiffuseProfile, avar("%s() - mCubeFace[%d] (line %d)", __FUNCTION__, i, __LINE__) ))
               {
                  Con::errorf("CubemapAsset::createMap - Failed to load texture '%s'", mCubeFaceFile[i].c_str());
                  initSuccess = false;
               }
            }
         }

         if( initSuccess )
         {
            mCubemap = GFX->createCubemap();
            mCubemap->initStatic( mCubeFace );
         }
      }
   }
}

//void CubemapAsset::updateDynamic(SceneManager* sm, const Point3F& pos)
//{
//   AssertFatal(mDynamic, "This is not a dynamic cubemap!");
//
//   GFXDEBUGEVENT_SCOPE( CubemapAsset_updateDynamic, ColorI::WHITE );
//
//#ifdef INIT_HACK
//   if( mInit ) return;
//   mInit = true;
//#endif
//
//   GFX->pushActiveRenderTarget();
//   mRenderTarget->attachTexture(GFXTextureTarget::DepthStencil, mDepthBuff );
//
//   // store current matrices
//   GFXTransformSaver saver;
//   F32 oldVisibleDist = sm->getVisibleDistance();
//
//   // set projection to 90 degrees vertical and horizontal
//   {
//      F32 left, right, top, bottom;
//      MathUtils::makeFrustum( &left, &right, &top, &bottom, M_HALFPI_F, 1.0f, mDynamicNearDist );
//      GFX->setFrustum( left, right, bottom, top, mDynamicNearDist, mDynamicFarDist );
//   }
//   sm->setVisibleDistance(mDynamicFarDist);
//
//   // We don't use a special clipping projection, but still need to initialize 
//   // this for objects like SkyBox which will use it during a reflect pass.
//   gClientSceneGraph->setNonClipProjection( (MatrixF&) GFX->getProjectionMatrix() );
//
//   // Loop through the six faces of the cube map.
//   for(U32 i=0; i<6; i++)
//   {      
//      // Standard view that will be overridden below.
//      VectorF vLookatPt(0.0f, 0.0f, 0.0f), vUpVec(0.0f, 0.0f, 0.0f), vRight(0.0f, 0.0f, 0.0f);
//
//      switch( i )
//      {
//      case 0 : // D3DCUBEMAP_FACE_POSITIVE_X:
//         vLookatPt = VectorF( 1.0f, 0.0f, 0.0f );
//         vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
//         break;
//      case 1 : // D3DCUBEMAP_FACE_NEGATIVE_X:
//         vLookatPt = VectorF( -1.0f, 0.0f, 0.0f );
//         vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
//         break;
//      case 2 : // D3DCUBEMAP_FACE_POSITIVE_Y:
//         vLookatPt = VectorF( 0.0f, 1.0f, 0.0f );
//         vUpVec    = VectorF( 0.0f, 0.0f,-1.0f );
//         break;
//      case 3 : // D3DCUBEMAP_FACE_NEGATIVE_Y:
//         vLookatPt = VectorF( 0.0f, -1.0f, 0.0f );
//         vUpVec    = VectorF( 0.0f, 0.0f, 1.0f );
//         break;
//      case 4 : // D3DCUBEMAP_FACE_POSITIVE_Z:
//         vLookatPt = VectorF( 0.0f, 0.0f, 1.0f );
//         vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
//         break;
//      case 5: // D3DCUBEMAP_FACE_NEGATIVE_Z:
//         vLookatPt = VectorF( 0.0f, 0.0f, -1.0f );
//         vUpVec    = VectorF( 0.0f, 1.0f, 0.0f );
//         break;
//      }
//
//      // create camera matrix
//      VectorF cross = mCross( vUpVec, vLookatPt );
//      cross.normalizeSafe();
//
//      MatrixF matView(true);
//      matView.setColumn( 0, cross );
//      matView.setColumn( 1, vLookatPt );
//      matView.setColumn( 2, vUpVec );
//      matView.setPosition( pos );
//      matView.inverse();
//
//      GFX->pushWorldMatrix();
//      GFX->setWorldMatrix(matView);
//
//      mRenderTarget->attachTexture( GFXTextureTarget::Color0, mCubemap, i );
//      GFX->setActiveRenderTarget( mRenderTarget );
//      GFX->clear( GFXClearStencil | GFXClearTarget | GFXClearZBuffer, ColorI( 64, 64, 64 ), 1.f, 0 );
//
//      // render scene
//      sm->renderScene( SPT_Reflect, mDynamicObjectTypeMask );
//
//      // Resolve render target for each face
//      mRenderTarget->resolve();
//      GFX->popWorldMatrix();
//   }
//
//   // restore render surface and depth buffer
//   GFX->popActiveRenderTarget();
//
//   mRenderTarget->attachTexture(GFXTextureTarget::Color0, nullptr);
//   sm->setVisibleDistance(oldVisibleDist);
//}

void CubemapAsset::updateFaces()
{
	bool initSuccess = true;

	for( U32 i=0; i<6; i++ )
   {
      if( !mCubeFaceFile[i].isEmpty() )
      {
         if(!mCubeFace[i].set(mCubeFaceFile[i], &GFXDefaultStaticDiffuseProfile, avar("%s() - mCubeFace[%d] (line %d)", __FUNCTION__, i, __LINE__) ))
         {
				initSuccess = false;
            Con::errorf("CubemapAsset::createMap - Failed to load texture '%s'", mCubeFaceFile[i].c_str());
         }
      }
   }

	if( initSuccess )
	{
		mCubemap = nullptr;
		mCubemap = GFX->createCubemap();

		mCubemap->initStatic( mCubeFace );
	}
}

//DefineEngineMethod( CubemapAsset, updateFaces, void, (),,
//   "Update the assigned cubemaps faces." )
//{
//	object->updateFaces();
//}
//
//DefineEngineMethod( CubemapAsset, getFilename, const char*, (),,
//   "Returns the script filename of where the CubemapAsset object was "
//   "defined.  This is used by the material editor." )
//{
//   return object->getFilename();
//}
