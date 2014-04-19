//
//  gfxDevice.cc
//  iTorque2D
//
//  Created by Paul on 9/14/12.
//
//

#include "platform/platform.h"
#include "graphics/gfxDevice.h"

#include "graphics/gfxInit.h"
#include "graphics/primBuilder.h"
#include "graphics/gfxDrawUtil.h"
#include "graphics/gfxFontRenderBatcher.h"
#include "graphics/gfxShader.h"
#include "graphics/gfxStateBlock.h"
#include "graphics/gfxStringEnumTranslate.h"
#include "graphics/gfxTextureManager.h"

#include "memory/frameAllocator.h"
#include "string/unicode.h"
#include "delegates/process.h"
#include "memory/safeDelete.h"
#include "console/consoleTypes.h"
#include "console/console.h"
#include "game/version.h"
#include "io/StreamFn.h"

GFXDevice * GFXDevice::smGFXDevice = nullptr;
bool GFXDevice::smWireframe = false;
bool GFXDevice::smDisableVSync = true;
F32 GFXDevice::smForcedPixVersion = -1.0f;
bool GFXDevice::smDisableOcclusionQuery = false;
bool gDisassembleAllShaders = false;


void GFXDevice::initConsole()
{
   GFXStringEnumTranslate::init();
}

GFXDevice::DeviceEventSignal& GFXDevice::getDeviceEventSignal()
{
   static DeviceEventSignal theSignal;
   return theSignal;
}

GFXDevice::GFXDevice():
    mViewport(0, 0, 0, 0),
    mNextViewport(0, 0, 0, 0)
{
   VECTOR_SET_ASSOCIATION( mVideoModes );
   VECTOR_SET_ASSOCIATION( mRTStack );

    mStateDirty = false;

    AssertFatal(smGFXDevice == nullptr, "Already a GFXDevice created! Bad!");
    smGFXDevice = this;

      
   // Vertex buffer cache
   mCurrVertexDecl = nullptr;
   mVertexDeclDirty = false;
   for ( U32 i=0; i < VERTEX_STREAM_COUNT; i++ )
   {
      mVertexBufferDirty[i] = false;
      mVertexBufferFrequency[i] = 0;
      mVertexBufferFrequencyDirty[i] = false;
   }

   mTexturesDirty = false;
   
   // Use of TEXTURE_STAGE_COUNT in initialization is okay [7/2/2007 Pat]
   for(U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
   {
      mTextureDirty[i] = false;
      mCurrentTexture[i] = nullptr;
      mNewTexture[i] = nullptr;
      mCurrentCubemap[i] = nullptr;
      mNewCubemap[i] = nullptr;
      mTexType[i] = GFXTDT_Normal;

//      mTextureMatrix[i].identity();
//      mTextureMatrixDirty[i] = false;
   }

   mLightsDirty = false;
   for(U32 i = 0; i < LIGHT_STAGE_COUNT; i++)
   {
      mLightDirty[i] = false;
      mCurrentLightEnable[i] = false;
   }

   mGlobalAmbientColorDirty = false;
   mGlobalAmbientColor = ColorF(0.0f, 0.0f, 0.0f, 1.0f);

   mLightMaterialDirty = false;
   dMemset(&mCurrentLightMaterial, 0, sizeof(GFXLightMaterial));

   // State block 
   mStateBlockDirty = false;
   mCurrentStateBlock = nullptr;
   mNewStateBlock = nullptr;

   mCurrentShaderConstBuffer = nullptr;

   // misc
   mAllowRender = true;
   mCanCurrentlyRender = false;
   mInitialized = false;

   mRTDirty = false;
   mCurrentRT = nullptr;

    mResourceListHead = nullptr;

   mCardProfiler = nullptr;

   // Initialize our drawing utility.
   mDrawer = nullptr;

//   // Add a few system wide shader macros.
//   GFXShader::addGlobalMacro( "TORQUE", "1" );
//   GFXShader::addGlobalMacro( "TORQUE_VERSION", String::ToString(getVersionNumber()) );
//   #if defined TORQUE_OS_WIN32
//      GFXShader::addGlobalMacro( "TORQUE_OS_WIN32" );
//   #elif defined TORQUE_OS_MAC
//      GFXShader::addGlobalMacro( "TORQUE_OS_MAC" );
//   #elif defined TORQUE_OS_LINUX
//      GFXShader::addGlobalMacro( "TORQUE_OS_LINUX" );      
//   #elif defined TORQUE_OS_XENON
//      GFXShader::addGlobalMacro( "TORQUE_OS_XENON" );
//   #elif defined TORQUE_OS_PS3
//      GFXShader::addGlobalMacro( "TORQUE_OS_PS3" );            
//   #endif
}

GFXDrawUtil* GFXDevice::getDrawUtil()
{
   if (!mDrawer)
   {
      mDrawer = new GFXDrawUtil(this);
   }
   return mDrawer;
}

void GFXDevice::deviceInited()
{
   getDeviceEventSignal().trigger(deInit);
   mDeviceStatistics.setPrefix("$GFXDeviceStatistics::");
//
//   // Initialize the static helper textures.
//   GBitmap temp( 2, 2, false, GFXFormatR8G8B8A8 );
//   temp.fill( ColorI( 255, 255, 255, 255) );
//   GFXTextureObject::ONE.set( &temp, &GFXDefaultStaticDiffuseProfile, false, "GFXTextureObject::ONE" ); 
//   temp.fill( ColorI( 0, 0, 0, 0) );
//   GFXTextureObject::ZERO.set( &temp, &GFXDefaultStaticDiffuseProfile, false, "GFXTextureObject::ZERO" ); 
//   temp.fill( ColorI( 128, 128, 255 ) );
//   GFXTextureObject::ZUP.set( &temp, &GFXDefaultStaticNormalMapProfile, false, "GFXTextureObject::ZUP" ); 
}

bool GFXDevice::destroy()
{
//   // Cleanup the static helper textures.
//   GFXTextureObject::ONE.free();
//   GFXTextureObject::ZERO.free();
//   GFXTextureObject::ZUP.free();
//
   // Make this release its buffer.
   PrimBuild::shutdown();

//   // Let people know we are shutting down
//   getDeviceEventSignal().trigger(deDestroy);

   if(smGFXDevice)
      smGFXDevice->preDestroy();
   SAFE_DELETE(smGFXDevice);

   return true;
}

void GFXDevice::preDestroy()
{
   // Delete draw util
   SAFE_DELETE( mDrawer );
}

GFXDevice::~GFXDevice()
{ 
   smGFXDevice = nullptr;

   // Clear out our current texture references
   for (U32 i = 0; i < TEXTURE_STAGE_COUNT; i++)
   {
      mCurrentTexture[i] = nullptr;
      mNewTexture[i] = nullptr;
//      mCurrentCubemap[i] = nullptr;
//      mNewCubemap[i] = nullptr;
   }

   // Release all the unreferenced textures in the cache.
   mTextureManager->cleanupCache();

//   // Check for resource leaks
//#ifdef TORQUE_DEBUG
//   AssertFatal( GFXTextureObject::dumpActiveTOs() == 0, "There is a texture object leak, check the log for more details." );
//   GFXPrimitiveBuffer::dumpActivePBs();
//#endif
//
//   SAFE_DELETE( mTextureManager );
//
//   // Clear out our state block references
//   mCurrentStateBlocks.clear();
//   mNewStateBlock = nullptr;
//   mCurrentStateBlock = nullptr;
//
   mCurrentShaderConstBuffer = nullptr;
   /// End Block above BTR

   // -- Clear out resource list
   // Note: our derived class destructor will have already released resources.
   // Clearing this list saves us from having our resources (which are not deleted
   // just released) turn around and try to remove themselves from this list.
   while (mResourceListHead)
   {
      GFXResource * head = mResourceListHead;
      mResourceListHead = head->mNextResource;
      
      head->mPrevResource = nullptr;
      head->mNextResource = nullptr;
      head->mOwningDevice = nullptr;
   }
}

GFXStateBlockRef GFXDevice::createStateBlock(const GFXStateBlockDesc& desc)
{
   PROFILE_SCOPE( GFXDevice_CreateStateBlock );

   size_t hashValue = desc.getHashValue();
   if (mCurrentStateBlocks[hashValue])
      return mCurrentStateBlocks[hashValue];

   GFXStateBlockRef result = createStateBlockInternal(desc);
   result->registerResourceWithDevice(this);   
   mCurrentStateBlocks[hashValue] = result;
   return result;
}

void GFXDevice::setStateBlock(GFXStateBlockRef block)
{
   AssertFatal(block, "nullptr state block!");
   AssertFatal(block->getOwningDevice() == this, "This state doesn't apply to this device!");

   if (block != mCurrentStateBlock)
   {
      mStateDirty = true;
      mStateBlockDirty = true;
      mNewStateBlock = block;
   } else {
      mStateBlockDirty = false;
      mNewStateBlock = mCurrentStateBlock;
   }
}

void GFXDevice::setStateBlockByDesc( const GFXStateBlockDesc &desc )
{
   PROFILE_SCOPE( GFXDevice_SetStateBlockByDesc );
   GFXStateBlockRef block = createStateBlock( desc );
   setStateBlock( block );
}

void GFXDevice::setShaderConstBuffer(GFXShaderConstBuffer* buffer)
{
   mCurrentShaderConstBuffer = buffer;
}


void GFXDevice::drawPrimitive( const GFXPrimitive &prim )
{
   // Do NOT add index buffer offset to this call, it will be added by drawIndexedPrimitive
   drawIndexedPrimitive(   prim.type, 
                           prim.startVertex,
                           prim.minIndex, 
                           prim.numVertices, 
                           prim.startIndex, 
                           prim.numPrimitives );
}



//-----------------------------------------------------------------------------
// Set Light
//-----------------------------------------------------------------------------
void GFXDevice::setLight(U32 stage, GFXLightInfo* light)
{
//   AssertFatal(stage < LIGHT_STAGE_COUNT, "GFXDevice::setLight - out of range stage!");
//
//   if(!mLightDirty[stage])
//   {
//      mStateDirty = true;
//      mLightsDirty = true;
//      mLightDirty[stage] = true;
//   }
//   mCurrentLightEnable[stage] = (light != nullptr);
//   if(mCurrentLightEnable[stage])
//      mCurrentLight[stage] = *light;
}

//-----------------------------------------------------------------------------
// Set Light Material
//-----------------------------------------------------------------------------
void GFXDevice::setLightMaterial(GFXLightMaterial mat)
{
   mCurrentLightMaterial = mat;
   mLightMaterialDirty = true;
   mStateDirty = true;
}

void GFXDevice::setGlobalAmbientColor(ColorF color)
{
   if(mGlobalAmbientColor != color)
   {
      mGlobalAmbientColor = color;
      mGlobalAmbientColorDirty = true;
   }
}

//-----------------------------------------------------------------------------
// Set texture
//-----------------------------------------------------------------------------
void GFXDevice::setTexture( U32 stage, GFXTextureObject *texture )
{
   AssertFatal(stage < getNumSamplers(), "GFXDevice::setTexture - out of range stage!");

   if (  mTexType[stage] == GFXTDT_Normal )
   {
      if ( mTextureDirty[stage] && mNewTexture[stage].getPointer() == texture )
          return;
       
      if ( !mTextureDirty[stage] && mCurrentTexture[stage].getPointer() == texture )
          return;
   }

   mStateDirty = true;
   mTexturesDirty = true;
   mTextureDirty[stage] = true;

   mNewTexture[stage] = texture;
   mTexType[stage] = GFXTDT_Normal;

//   // Clear out the cubemaps
//   mNewCubemap[stage] = nullptr;
//   mCurrentCubemap[stage] = nullptr;
}

//-----------------------------------------------------------------------------
// Set cube texture
//-----------------------------------------------------------------------------
void GFXDevice::setCubeTexture( U32 stage, GFXCubemap *texture )
{
   AssertFatal(stage < getNumSamplers(), "GFXDevice::setTexture - out of range stage!");

   if (  mTexType[stage] == GFXTDT_Cube &&
         (  ( mTextureDirty[stage] && mNewCubemap[stage].getPointer() == texture ) ||
            ( !mTextureDirty[stage] && mCurrentCubemap[stage].getPointer() == texture ) ) )
      return;

   mStateDirty = true;
   mTexturesDirty = true;
   mTextureDirty[stage] = true;

   mNewCubemap[stage] = texture;
   mTexType[stage] = GFXTDT_Cube;

   // Clear out the normal textures
   mNewTexture[stage] = nullptr;
   mCurrentTexture[stage] = nullptr;
}

inline bool GFXDevice::beginScene()
{
   AssertFatal( mCanCurrentlyRender == false, "GFXDevice::beginScene() - The scene has already begun!" );

   mDeviceStatistics.clear();

   // Send the start of frame signal.
   getDeviceEventSignal().trigger( GFXDevice::deStartOfFrame );

   return beginSceneInternal();
}

//------------------------------------------------------------------------------

inline void GFXDevice::endScene()
{
   AssertFatal( mCanCurrentlyRender == true, "GFXDevice::endScene() - The scene has already ended!" );
   
   // End frame signal
   getDeviceEventSignal().trigger( GFXDevice::deEndOfFrame );

   endSceneInternal();
   mDeviceStatistics.exportToConsole();
}

void GFXDevice::setViewport( const RectI &inRect )
{
   // Clip the rect against the renderable size.
   Point2I size = mCurrentRT->getSize();
   RectI maxRect(Point2I(0,0), size);
   RectI rect = inRect;
   rect.intersect(maxRect);
   
   mNextViewport = rect;
}

void GFXDevice::pushActiveRenderTarget()
{
   // Push the current target on to the stack.
   mRTStack.push_back( mCurrentRT );
}

void GFXDevice::popActiveRenderTarget()
{
   AssertFatal( !mRTStack.empty(), "GFXDevice::popActiveRenderTarget() - stack is empty!" );
   
   // Restore the last item on the stack and pop.
   setActiveRenderTarget( mRTStack.back() );
   mRTStack.pop_back();
}

void GFXDevice::setActiveRenderTarget( GFXTarget *target )
{
   AssertFatal( target,
               "GFXDevice::setActiveRenderTarget - must specify a render target!" );
   
   if ( target == mCurrentRT )
      return;
   
   // If we're not dirty then store the
   // current RT for deactivation later.
   if ( !mRTDirty )
   {
      // Deactivate the target queued for deactivation
      if(mRTDeactivate)
         mRTDeactivate->deactivate();
      
      mRTDeactivate = mCurrentRT;
   }
   
   mRTDirty = true;
   mCurrentRT = target;

   // When a target changes we also change the viewport
   // to match it.  This causes problems when the viewport
   // has been modified for clipping to a GUI bounds.
   //
   // We should consider removing this and making it the
   // responsibility of the caller to set a proper viewport
   // when the target is changed.
   setViewport( RectI( Point2I(0,0), mCurrentRT->getSize() ) );
}

/// Helper class for GFXDevice::describeResources.
class DescriptionOutputter
{
   /// Are we writing to a file?
   bool mWriteToFile;

   /// File if we are writing to a file
   std::fstream mFile;
public:
   DescriptionOutputter(const char* file)
   {
      mWriteToFile = false;
      // If we've been given what could be a valid file path, open it.
      if(file && file[0] != '\0')
      {
         mFile.open(file, std::fstream::out);
         mWriteToFile = mFile ? true : false;

         // Note that it is safe to retry.  If this is hit, we'll just write to the console instead of to the file.
         AssertFatal(mWriteToFile, avar("DescriptionOutputter::DescriptionOutputter - could not open file %s", file));
      }
   }

   ~DescriptionOutputter()
   {
      // Close the file
      if(mWriteToFile)
         mFile.close();
   }

   /// Writes line to the file or to the console, depending on what we want.
   void write(const char* line)
   {
      if(mWriteToFile)
          StreamFn::writeLine(mFile, line);
      else
         Con::printf(line);
   }
};


void GFXDevice::listResources(bool unflaggedOnly)
{
   U32 numTextures = 0, numShaders = 0, numRenderToTextureTargs = 0, numWindowTargs = 0;
   U32 numCubemaps = 0, numVertexBuffers = 0, numPrimitiveBuffers = 0, numFences = 0;
   U32 numStateBlocks = 0;

   GFXResource* walk = mResourceListHead;
   while(walk)
   {
      if(unflaggedOnly && walk->isFlagged())
      {
         walk = walk->getNextResource();
         continue;
      }

      if(dynamic_cast<GFXTextureObject*>(walk))
         numTextures++;
      else if(dynamic_cast<GFXShader*>(walk))
         numShaders++;
      else if(dynamic_cast<GFXTextureTarget*>(walk))
         numRenderToTextureTargs++;
      else if(dynamic_cast<GFXWindowTarget*>(walk))
         numWindowTargs++;
//      else if(dynamic_cast<GFXCubemap*>(walk))
//         numCubemaps++;
      else if(dynamic_cast<GFXVertexBuffer*>(walk))
         numVertexBuffers++;
//      else if(dynamic_cast<GFXPrimitiveBuffer*>(walk))
//         numPrimitiveBuffers++;
//      else if(dynamic_cast<GFXFence*>(walk))
//         numFences++;
      else if (dynamic_cast<GFXStateBlock*>(walk))
         numStateBlocks++;
      else
         Con::warnf("Unknown resource: %x", walk);

      walk = walk->getNextResource();
   }
   const char* flag = unflaggedOnly ? "unflagged" : "allocated";

   Con::printf("GFX currently has:");
   Con::printf("   %i %s textures", numTextures, flag);
   Con::printf("   %i %s shaders", numShaders, flag);
   Con::printf("   %i %s texture targets", numRenderToTextureTargs, flag);
   Con::printf("   %i %s window targets", numWindowTargs, flag);
   Con::printf("   %i %s cubemaps", numCubemaps, flag);
   Con::printf("   %i %s vertex buffers", numVertexBuffers, flag);
   Con::printf("   %i %s primitive buffers", numPrimitiveBuffers, flag);
   Con::printf("   %i %s fences", numFences, flag);
   Con::printf("   %i %s state blocks", numStateBlocks, flag);
}

void GFXDevice::fillResourceVectors(const char* resNames, bool unflaggedOnly, Vector<GFXResource*> &textureObjects,
                                 Vector<GFXResource*> &textureTargets, Vector<GFXResource*> &windowTargets, Vector<GFXResource*> &vertexBuffers, 
                                 Vector<GFXResource*> &primitiveBuffers, Vector<GFXResource*> &fences, Vector<GFXResource*> &cubemaps, 
                                 Vector<GFXResource*> &shaders, Vector<GFXResource*> &stateblocks)
{
//   bool describeTexture = true, describeTextureTarget = true, describeWindowTarget = true, describeVertexBuffer = true, 
//      describePrimitiveBuffer = true, describeFence = true, describeCubemap = true, describeShader = true,
//      describeStateBlock = true;
//
//   // If we didn't specify a string of names, we'll print all of them
//   if(resNames && resNames[0] != '\0')
//   {
//      // If we did specify a string of names, determine which names
//      describeTexture =          (dStrstr(resNames, "GFXTextureObject")    != nullptr);
//      describeTextureTarget =    (dStrstr(resNames, "GFXTextureTarget")    != nullptr);
//      describeWindowTarget =     (dStrstr(resNames, "GFXWindowTarget")     != nullptr);
//      describeVertexBuffer =     (dStrstr(resNames, "GFXVertexBuffer")     != nullptr);
//      describePrimitiveBuffer =  (dStrstr(resNames, "GFXPrimitiveBuffer")   != nullptr);
//      describeFence =            (dStrstr(resNames, "GFXFence")            != nullptr);
////      describeCubemap =          (dStrstr(resNames, "GFXCubemap")          != nullptr);
//      describeShader =           (dStrstr(resNames, "GFXShader")           != nullptr);
//      describeStateBlock =       (dStrstr(resNames, "GFXStateBlock")           != nullptr);
//   }
//
//   // Start going through the list
//   GFXResource* walk = mResourceListHead;
//   while(walk)
//   {
//      // If we only want unflagged resources, skip all flagged resources
//      if(unflaggedOnly && walk->isFlagged())
//      {
//         walk = walk->getNextResource();
//         continue;
//      }
//
//      // All of the following checks go through the same logic.
//      // if(describingThisResource) 
//      // {
//      //    ResourceType* type = dynamic_cast<ResourceType*>(walk)
//      //    if(type)
//      //    {
//      //       typeVector.push_back(type);
//      //       walk = walk->getNextResource();
//      //       continue;
//      //    }
//      // }
//
//      if(describeTexture)
//      {
//         GFXTextureObject* tex = dynamic_cast<GFXTextureObject*>(walk);
//         {
//            if(tex)
//            {
//               textureObjects.push_back(tex);
//               walk = walk->getNextResource();
//               continue;
//            }
//         }
//      }
//      if(describeShader)
//      {
//         GFXShader* shd = dynamic_cast<GFXShader*>(walk);
//         if(shd)
//         {
//            shaders.push_back(shd);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
//      if(describeVertexBuffer)
//      {
//         GFXVertexBuffer* buf = dynamic_cast<GFXVertexBuffer*>(walk);
//         if(buf)
//         {
//            vertexBuffers.push_back(buf);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
//      if(describePrimitiveBuffer)
//      {
//         GFXPrimitiveBuffer* buf = dynamic_cast<GFXPrimitiveBuffer*>(walk);
//         if(buf)
//         {
//            primitiveBuffers.push_back(buf);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
//      if(describeTextureTarget)
//      {
//         GFXTextureTarget* targ = dynamic_cast<GFXTextureTarget*>(walk);
//         if(targ)
//         {
//            textureTargets.push_back(targ);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
//      if(describeWindowTarget)
//      {
//         GFXWindowTarget* targ = dynamic_cast<GFXWindowTarget*>(walk);
//         if(targ)
//         {
//            windowTargets.push_back(targ);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
////      if(describeCubemap)
////      {
////         GFXCubemap* cube = dynamic_cast<GFXCubemap*>(walk);
////         if(cube)
////         {
////            cubemaps.push_back(cube);
////            walk = walk->getNextResource();
////            continue;
////         }
////      }
//      if(describeFence)
//      {
//         GFXFence* fence = dynamic_cast<GFXFence*>(walk);
//         if(fence)
//         {
//            fences.push_back(fence);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
//      if (describeStateBlock)
//      {
//         GFXStateBlock* sb = dynamic_cast<GFXStateBlock*>(walk);
//         if (sb)
//         {
//            stateblocks.push_back(sb);
//            walk = walk->getNextResource();
//            continue;
//         }
//      }
//      // Wasn't something we were looking for
//      walk = walk->getNextResource();
//   }
}

void GFXDevice::describeResources(const char* resNames, const char* filePath, bool unflaggedOnly)
{
//   const U32 numResourceTypes = 9;
//   Vector<GFXResource*> resVectors[numResourceTypes];
//   const char* reslabels[numResourceTypes] = { "texture", "texture target", "window target", "vertex buffers", "primitive buffers", "fences", "cubemaps", "shaders", "stateblocks" };   
//
//   // Fill the vectors with the right resources
//   fillResourceVectors(resNames, unflaggedOnly, resVectors[0], resVectors[1], resVectors[2], resVectors[3], 
//      resVectors[4], resVectors[5], resVectors[6], resVectors[7], resVectors[8]);
//
//   // Helper object
//   DescriptionOutputter output(filePath);
//
//   // Print the info to the file
//   // Note that we check if we have any objects of that type.
//   for (U32 i = 0; i < numResourceTypes; i++)
//   {
//      if (resVectors[i].size())
//      {
//         // Header
//         String header = String::ToString("--------Dumping GFX %s descriptions...----------", reslabels[i]);
//         output.write(header);
//         // Data
//         for (U32 j = 0; j < resVectors[i].size(); j++)
//         {
//            GFXResource* resource = resVectors[i][j];
//            String dataline = String::ToString("Addr: %x %s", resource, resource->describeSelf().c_str());
//            output.write(dataline.c_str());
//         }
//         // Footer
//         output.write("--------------------Done---------------------");
//         output.write("");
//      }
//   }
}

void GFXDevice::flagCurrentResources()
{
//   GFXResource* walk = mResourceListHead;
//   while(walk)
//   {
//      walk->setFlag();
//      walk = walk->getNextResource();
//   }
}

void GFXDevice::clearResourceFlags()
{
//   GFXResource* walk = mResourceListHead;
//   while(walk)
//   {
//      walk->clearFlag();
//      walk = walk->getNextResource();
//   }
}

//ConsoleFunction( listGFXResources, void,  2, 3,
//   "Returns a list of the unflagged GFX resources. See flagCurrentGFXResources for usage details.\n"
//   "@ingroup GFX\n"
//   "@see flagCurrentGFXResources, clearGFXResourceFlags, describeGFXResources" )
//{
//    bool unflaggedOnly = false;
//    if (argc == 3)
//        unflaggedOnly = dAtob(argv[2]);
//        
//   GFX->listResources(unflaggedOnly);
//}
//
//ConsoleFunction( flagCurrentGFXResources, void, 2, 2,
//   "@brief Flags all currently allocated GFX resources.\n"
//   "Used for resource allocation and leak tracking by flagging "
//   "current resources then dumping a list of unflagged resources "
//   "at some later point in execution.\n"
//   "@ingroup GFX\n"
//   "@see listGFXResources, clearGFXResourceFlags, describeGFXResources" )
//{
//   GFX->flagCurrentResources();
//}
//
//ConsoleFunction( clearGFXResourceFlags, void, 2, 2,
//   "Clears the flagged state on all allocated GFX resources. "
//   "See flagCurrentGFXResources for usage details.\n"
//   "@ingroup GFX\n"
//   "@see flagCurrentGFXResources, listGFXResources, describeGFXResources" )
//{
//   GFX->clearResourceFlags();
//}
//
//ConsoleFunction( describeGFXResources, void, 4, 5,
//   "@brief Dumps a description of GFX resources to a file or the console.\n"
//   "@param resourceTypes A space seperated list of resource types or an empty string for all resources.\n"
//   "@param filePath A file to dump the list to or an empty string to write to the console.\n"
//   "@param unflaggedOnly If true only unflagged resources are dumped. See flagCurrentGFXResources.\n"
//   "@note The resource types can be one or more of the following:\n\n"
//   "  - texture\n"
//   "  - texture target\n"
//   "  - window target\n"
//   "  - vertex buffers\n"
//   "  - primitive buffers\n"
//   "  - fences\n"
//   "  - cubemaps\n"
//   "  - shaders\n"
//   "  - stateblocks\n\n"
//   "@ingroup GFX\n" )
//{
//    bool unflaggedOnly = false;
//    if (argc == 5)
//        unflaggedOnly = dAtob(argv[4]);
//
////    ( const char *resourceTypes, const char *filePath, bool unflaggedOnly ), ( false ),
//   GFX->describeResources( argv[2], argv[3], unflaggedOnly );
//}
//
////ConsoleMethod( describeGFXStateBlocks, void, ( const char *filePath ),,
////   "Dumps a description of all state blocks.\n"     
////   "@param filePath A file to dump the state blocks to or an empty string to write to the console.\n"
////   "@ingroup GFX\n" )
////{
////   GFX->dumpStates( filePath );   
////}
//
//ConsoleFunction( getPixelShaderVersion, F32, 2, 2,
//   "Returns the pixel shader version for the active device.\n"
//   "@ingroup GFX\n" )
//{
//   return GFX->getPixelShaderVersion();
//}   
//
//ConsoleFunction( setPixelShaderVersion, void, 3, 3,
//   "@brief Sets the pixel shader version for the active device.\n"
//   "This can be used to force a lower pixel shader version than is supported by "
//   "the device for testing or performance optimization.\n"
//   "@param version The floating point shader version number.\n"
//   "@note This will only affect shaders/materials created after the call "
//   "and should be used before the game begins.\n"
//   "@see $pref::Video::forcedPixVersion\n"
//   "@ingroup GFX\n" )
//{
//   GFX->setPixelShaderVersion( atof(argv[2]) );
//}
//
//ConsoleFunction( getDisplayDeviceInformation, const char*, 2, 2,
//   "Get the string describing the active GFX device.\n"
//   "@ingroup GFX\n" )
//{
//   if (!GFX->devicePresent())
//      return "(no device)";
//
//   const GFXAdapter& adapter = GFX->getAdapter();
//   return adapter.getName();
//}
//
////ConsoleFunction( getBestHDRFormat, GFXFormat, 2, 2,
////   "Returns the best texture format for storage of HDR data for the active device.\n"
////   "@ingroup GFX\n" )
////{
////   // TODO: Maybe expose GFX::selectSupportedFormat() so that this
////   // specialized method can be moved to script.
////
////   // Figure out the best HDR format.  This is the smallest
////   // format which supports blending and filtering.
////   Vector<GFXFormat> formats;
////   formats.push_back( GFXFormatR10G10B10A2 );
////   formats.push_back( GFXFormatR16G16B16A16F );
////   formats.push_back( GFXFormatR16G16B16A16 );    
////   GFXFormat format = GFX->selectSupportedFormat(  &GFXDefaultRenderTargetProfile,
////                                                   formats, 
////                                                   true,
////                                                   true,
////                                                   true );
////
////   return format;
////}


////------------------------------------------------------------------------------
//void GFXDevice::init()
//{
//    smCurrentRes = GFXVideoMode();
//}
//
//
////------------------------------------------------------------------------------
//// This function returns a string containing all of the available resolutions for this device
//// in the format "<bit depth> <width> <height>", separated by tabs.
////
//const char* GFXDevice::getResolutionList()
//{
//    if (Con::getBoolVariable("$pref::Video::clipHigh", false))
//        for (S32 i = mVideoModes.size()-1; i >= 0; --i)
//            if (mVideoModes[i].resolution.x > 1152 || mVideoModes[i].resolution.y > 864)
//                mVideoModes.erase(i);
//    
//    if (Con::getBoolVariable("$pref::Video::only16", false))
//        for (S32 i = mVideoModes.size()-1; i >= 0; --i)
//            if (mVideoModes[i].bitDepth == 32)
//                mVideoModes.erase(i);
//    
//    U32 resCount = mVideoModes.size();
//    if ( resCount > 0 )
//    {
//        char* tempBuffer = new char[resCount * 15];
//        tempBuffer[0] = 0;
//        for ( U32 i = 0; i < resCount; i++ )
//        {
//            char newString[15];
//            dSprintf( newString, sizeof( newString ), "%d %d %d\t", mVideoModes[i].resolution.x, mVideoModes[i].resolution.y, mVideoModes[i].bitDepth );
//            dStrcat( tempBuffer, newString );
//        }
//        tempBuffer[dStrlen( tempBuffer ) - 1] = 0;
//        
//        char* returnString = Con::getReturnBuffer( dStrlen( tempBuffer ) + 1 );
//        dStrcpy( returnString, tempBuffer );
//        delete [] tempBuffer;
//        
//        return returnString;
//    }
//    
//    return nullptr;
//}



