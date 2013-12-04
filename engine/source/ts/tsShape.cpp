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
#include "ts/tsShape.h"
#include "ts/tsShapeInstance.h"
#include "StreamFn.h"

//#include "ts/tsLastDetail.h"
#include "ts/tsMaterialList.h"
#include "string/stringTable.h"
#include "console/console.h"
//#include "collision/convex.h"
//#include "materials/matInstance.h"
//#include "materials/materialManager.h"
#include "math/mathIO.h"
#include "platform/platformEndian.h"
#include "console/compiler.h"

#ifdef TORQUE_COLLADA
extern TSShape* loadColladaShape(const StringTableEntry &path);
#endif

/// most recent version -- this is the version we write
S32 TSShape::smVersion = 26;
/// the version currently being read...valid only during a read
S32 TSShape::smReadVersion = -1;
const U32 TSShape::smMostRecentExporterVersion = DTS_EXPORTER_CURRENT_VERSION;

F32 TSShape::smAlphaOutLastDetail = -1.0f;
F32 TSShape::smAlphaInBillboard = 0.15f;
F32 TSShape::smAlphaOutBillboard = 0.15f;
F32 TSShape::smAlphaInDefault = -1.0f;
F32 TSShape::smAlphaOutDefault = -1.0f;

// don't bother even loading this many of the highest detail levels (but
// always load last renderable detail)
S32 TSShape::smNumSkipLoadDetails = 0;

bool TSShape::smInitOnRead = true;


ResourceInstance* constructShape(std::iostream& stream)
{
    TSShape *ret = new TSShape;
    
    if(!ret->read(stream))
    {
        SAFE_DELETE(ret);
    }
    
    return ret;
}

//template<> void *Resource<TSShape>::create(const StringTableEntry &path)
//{
//   // Execute the shape script if it exists
//   StringTableEntry scriptPath(path);
//   scriptPath.setExtension("cs");
//
//   // Don't execute the script if we're already doing so!
//   StringTableEntry currentScript = Platform::stripBasePath(CodeBlock::getCurrentCodeBlockFullPath());
//   if (!scriptPath.getFullPath().equal(currentScript))
//   {
//      StringTableEntry scriptPathDSO(scriptPath);
//      scriptPathDSO.setExtension("cs.dso");
//
//      if (Torque::FS::IsFile(scriptPathDSO) || Torque::FS::IsFile(scriptPath))
//      {
//         String evalCmd = "exec(\"" + scriptPath + "\");";
//
//         String instantGroup = Con::getVariable("InstantGroup");
//         Con::setIntVariable("InstantGroup", RootGroupId);
//         Con::evaluate((const char*)evalCmd.c_str(), false, scriptPath.getFullPath());
//         Con::setVariable("InstantGroup", instantGroup.c_str());
//      }
//   }
//
//   // Attempt to load the shape
//   TSShape * ret = 0;
//   bool readSuccess = false;
//   const String extension = path.getExtension();
//
//   if ( extension.equal( "dts", String::NoCase ) )
//   {
//      FileStream stream;
//      stream.open( path.getFullPath(), Torque::FS::File::Read );
//      if ( stream.getStatus() != Stream::Ok )
//      {
//         Con::errorf( "Resource<TSShape>::create - Could not open '%s'", path.getFullPath().c_str() );
//         return nullptr;
//      }
//
//      ret = new TSShape;
//      readSuccess = ret->read(&stream);
//   }
//   else if ( extension.equal( "dae", String::NoCase ) || extension.equal( "kmz", String::NoCase ) )
//   {
//#ifdef TORQUE_COLLADA
//      // Attempt to load the DAE file
//      ret = loadColladaShape(path);
//      readSuccess = (ret != nullptr);
//#else
//      // No COLLADA support => attempt to load the cached DTS file instead
//      StringTableEntry cachedPath = path;
//      cachedPath.setExtension("cached.dts");
//
//      FileStream stream;
//      stream.open( cachedPath.getFullPath(), Torque::FS::File::Read );
//      if ( stream.getStatus() != Stream::Ok )
//      {
//         Con::errorf( "Resource<TSShape>::create - Could not open '%s'", cachedPath.getFullPath().c_str() );
//         return nullptr;
//      }
//      ret = new TSShape;
//      readSuccess = ret->read(&stream);
//#endif
//   }
//   else
//   {
//      Con::errorf( "Resource<TSShape>::create - '%s' has an unknown file format", path.getFullPath().c_str() );
//      delete ret;
//      return nullptr;
//   }
//
//   if( !readSuccess )
//   {
//      Con::errorf( "Resource<TSShape>::create - Error reading '%s'", path.getFullPath().c_str() );
//      delete ret;
//      ret = nullptr;
//   }
//
//   return ret;
//}


TSShape::TSShape()
{
   materialList = nullptr;
   mReadVersion = -1; // -1 means constructed from scratch (e.g., in exporter or no read yet)
   mHasSkinMesh = false;
   mSequencesConstructed = false;
   mShapeData = nullptr;
   mShapeDataSize = 0;

   mUseDetailFromScreenError = false;

   mDetailLevelLookup.setSize( 1 );
   mDetailLevelLookup[0].set( -1, 0 );

   VECTOR_SET_ASSOCIATION(sequences);
   VECTOR_SET_ASSOCIATION(nodeRotations);
   VECTOR_SET_ASSOCIATION(nodeTranslations);
   VECTOR_SET_ASSOCIATION(nodeUniformScales);
   VECTOR_SET_ASSOCIATION(nodeAlignedScales);
   VECTOR_SET_ASSOCIATION(nodeArbitraryScaleRots);
   VECTOR_SET_ASSOCIATION(nodeArbitraryScaleFactors);
   VECTOR_SET_ASSOCIATION(groundRotations);
   VECTOR_SET_ASSOCIATION(groundTranslations);
   VECTOR_SET_ASSOCIATION(triggers);
//   VECTOR_SET_ASSOCIATION(billboardDetails);
   VECTOR_SET_ASSOCIATION(detailCollisionAccelerators);
   VECTOR_SET_ASSOCIATION(names);

   VECTOR_SET_ASSOCIATION( nodes );
   VECTOR_SET_ASSOCIATION( objects );
   VECTOR_SET_ASSOCIATION( objectStates );
   VECTOR_SET_ASSOCIATION( subShapeFirstNode );
   VECTOR_SET_ASSOCIATION( subShapeFirstObject );
   VECTOR_SET_ASSOCIATION( detailFirstSkin );
   VECTOR_SET_ASSOCIATION( subShapeNumNodes );
   VECTOR_SET_ASSOCIATION( subShapeNumObjects );
   VECTOR_SET_ASSOCIATION( details );
   VECTOR_SET_ASSOCIATION( defaultRotations );
   VECTOR_SET_ASSOCIATION( defaultTranslations );

   VECTOR_SET_ASSOCIATION( subShapeFirstTranslucentObject );
   VECTOR_SET_ASSOCIATION( meshes );

   VECTOR_SET_ASSOCIATION( alphaIn );
   VECTOR_SET_ASSOCIATION( alphaOut );
}

TSShape::~TSShape()
{
   delete materialList;

   S32 i;

   // everything left over here is a legit mesh
   for (i=0; i<meshes.size(); i++)
   {
      if (!meshes[i])
         continue;

      // Handle meshes that were either assembled with the shape or added later
      if (((S8*)meshes[i] >= mShapeData) && ((S8*)meshes[i] < (mShapeData + mShapeDataSize)))
         destructInPlace(meshes[i]);
      else
         delete meshes[i];
   }

//   for (i=0; i<billboardDetails.size(); i++)
//   {
//      delete billboardDetails[i];
//      billboardDetails[i] = nullptr;
//   }
//   billboardDetails.clear();

   // Delete any generated accelerators
   S32 dca;
   for (dca = 0; dca < detailCollisionAccelerators.size(); dca++)
   {
      ConvexHullAccelerator* accel = detailCollisionAccelerators[dca];
      if (accel != nullptr) {
         delete [] accel->vertexList;
         delete [] accel->normalList;
         for (S32 j = 0; j < accel->numVerts; j++)
            delete [] accel->emitStrings[j];
         delete [] accel->emitStrings;
         delete accel;
      }
   }
   for (dca = 0; dca < detailCollisionAccelerators.size(); dca++)
      detailCollisionAccelerators[dca] = nullptr;

   if( mShapeData )
      delete[] mShapeData;
}

const String& TSShape::getName( S32 nameIndex ) const
{
   AssertFatal(nameIndex>=0 && nameIndex<names.size(),"TSShape::getName");
   return names[nameIndex];
}

const String& TSShape::getMeshName( S32 meshIndex ) const
{
   S32 nameIndex = objects[meshIndex].nameIndex;
   if ( nameIndex < 0 )
      return String::EmptyString;

   return names[nameIndex];
}

const String& TSShape::getNodeName( S32 nodeIndex ) const
{   
   S32 nameIdx = nodes[nodeIndex].nameIndex;
   if ( nameIdx < 0 )
      return String::EmptyString;

   return names[nameIdx];
}

const String& TSShape::getSequenceName( S32 seqIndex ) const
{
   AssertFatal(seqIndex >= 0 && seqIndex<sequences.size(),"TSShape::getSequenceName index beyond range");

   S32 nameIdx = sequences[seqIndex].nameIndex;
   if ( nameIdx < 0 )
      return String::EmptyString;

   return names[nameIdx];
}

S32 TSShape::findName(const String &name) const
{
   for (S32 i=0; i<names.size(); i++)
   {
      if (names[i].equal( name, String::NoCase ))
         return i;
   }
  
   return -1;
}

const String& TSShape::getTargetName( S32 mapToNameIndex ) const
{
	size_t targetCount = materialList->getMaterialNameList().size();

	if(mapToNameIndex < 0 || mapToNameIndex >= targetCount)
		return String::EmptyString;

	return materialList->getMaterialNameList()[mapToNameIndex];
}

S32 TSShape::getTargetCount() const
{
	if(!this)
		return -1;

	return materialList->getMaterialNameList().size();

}

S32 TSShape::findNode(S32 nameIndex) const
{
   for (S32 i=0; i<nodes.size(); i++)
      if (nodes[i].nameIndex==nameIndex)
         return i;
   return -1;
}

S32 TSShape::findObject(S32 nameIndex) const
{
   for (S32 i=0; i<objects.size(); i++)
      if (objects[i].nameIndex==nameIndex)
         return i;
   return -1;
}

S32 TSShape::findDetail(S32 nameIndex) const
{
   for (S32 i=0; i<details.size(); i++)
      if (details[i].nameIndex==nameIndex)
         return i;
   return -1;
}

S32 TSShape::findDetailBySize(S32 size) const
{
   for (S32 i=0; i<details.size(); i++)
      if (details[i].size==size)
         return i;
   return -1;
}

S32 TSShape::findSequence(S32 nameIndex) const
{
   for (S32 i=0; i<sequences.size(); i++)
      if (sequences[i].nameIndex==nameIndex)
         return i;
   return -1;
}

bool TSShape::findMeshIndex(const String& meshName, S32& objIndex, S32& meshIndex)
{
   // Determine the object name and detail size from the mesh name
   S32 detailSize = 999;  
   objIndex = findObject(String::GetTrailingNumber(meshName, detailSize));
   if (objIndex < 0)
      return false;

   // Determine the subshape this object belongs to
   S32 subShapeIndex = getSubShapeForObject(objIndex);
   AssertFatal(subShapeIndex < subShapeFirstObject.size(), "Could not find subshape for object!");

   // Get the detail levels for the subshape
   Vector<S32> validDetails;
   getSubShapeDetails(subShapeIndex, validDetails);

   // Find the detail with the correct size
   for (meshIndex = 0; meshIndex < validDetails.size(); meshIndex++)
   {
      const TSShape::Detail& det = details[validDetails[meshIndex]];
      if (detailSize == det.size)
         return true;
   }

   return false;
}

TSMesh* TSShape::findMesh(const String& meshName)
{
   S32 objIndex, meshIndex;
   if (!findMeshIndex(meshName, objIndex, meshIndex))
      return 0;
   return meshes[objects[objIndex].startMeshIndex + meshIndex];
}

S32 TSShape::getSubShapeForNode(S32 nodeIndex)
{
   for (S32 i = 0; i < subShapeFirstNode.size(); i++)
   {
      S32 start = subShapeFirstNode[i];
      S32 end = start + subShapeNumNodes[i];
      if ((nodeIndex >= start) && (nodeIndex < end))
         return i;;
   }
   return -1;
}

S32 TSShape::getSubShapeForObject(S32 objIndex)
{
   for (S32 i = 0; i < subShapeFirstObject.size(); i++)
   {
      S32 start = subShapeFirstObject[i];
      S32 end = start + subShapeNumObjects[i];
      if ((objIndex >= start) && (objIndex < end))
         return i;
   }
   return -1;
}

void TSShape::getSubShapeDetails(S32 subShapeIndex, Vector<S32>& validDetails)
{
   validDetails.clear();
   for (S32 i = 0; i < details.size(); i++)
   {
      if ((details[i].subShapeNum == subShapeIndex) ||
          (details[i].subShapeNum < 0))
          validDetails.push_back(i);
   }
}

void TSShape::getNodeWorldTransform(S32 nodeIndex, MatrixF* mat) const
{
   if ( nodeIndex == -1 )
   {
      mat->identity();
   }
   else
   {
      // Calculate the world transform of the given node
      defaultRotations[nodeIndex].getQuatF().setMatrix(mat);
      mat->setPosition(defaultTranslations[nodeIndex]);

      S32 parentIndex = nodes[nodeIndex].parentIndex;
      while (parentIndex != -1)
      {
         MatrixF mat2(*mat);
         defaultRotations[parentIndex].getQuatF().setMatrix(mat);
         mat->setPosition(defaultTranslations[parentIndex]);
         *mat*=mat2;

         parentIndex = nodes[parentIndex].parentIndex;
      }
   }
}

void TSShape::getNodeObjects(S32 nodeIndex, Vector<S32>& nodeObjects)
{
   for (S32 i = 0; i < objects.size(); i++)
   {
      if ((nodeIndex == -1) || (objects[i].nodeIndex == nodeIndex))
         nodeObjects.push_back(i);
   }
}

void TSShape::getNodeChildren(S32 nodeIndex, Vector<S32>& nodeChildren)
{
   for (S32 i = 0; i < nodes.size(); i++)
   {
      if (nodes[i].parentIndex == nodeIndex)
         nodeChildren.push_back(i);
   }
}

void TSShape::getObjectDetails(S32 objIndex, Vector<S32>& objDetails)
{
   // Get the detail levels for this subshape
   Vector<S32> validDetails;
   getSubShapeDetails(getSubShapeForObject(objIndex), validDetails);

   // Get the non-nullptr details for this object
   const TSShape::Object& obj = objects[objIndex];
   for (S32 i = 0; i < obj.numMeshes; i++)
   {
      if (meshes[obj.startMeshIndex + i])
         objDetails.push_back(validDetails[i]);
   }
}

void TSShape::init()
{
   S32 numSubShapes = subShapeFirstNode.size();
   AssertFatal(numSubShapes==subShapeFirstObject.size(),"TSShape::init");

   S32 i,j;

   // set up parent/child relationships on nodes and objects
   for (i=0; i<nodes.size(); i++)
      nodes[i].firstObject = nodes[i].firstChild = nodes[i].nextSibling = -1;
   for (i=0; i<nodes.size(); i++)
   {
      S32 parentIndex = nodes[i].parentIndex;
      if (parentIndex>=0)
      {
         if (nodes[parentIndex].firstChild<0)
            nodes[parentIndex].firstChild=i;
         else
         {
            S32 child = nodes[parentIndex].firstChild;
            while (nodes[child].nextSibling>=0)
               child = nodes[child].nextSibling;
            nodes[child].nextSibling = i;
         }
      }
   }
   for (i=0; i<objects.size(); i++)
   {
      objects[i].nextSibling = -1;

      S32 nodeIndex = objects[i].nodeIndex;
      if (nodeIndex>=0)
      {
         if (nodes[nodeIndex].firstObject<0)
            nodes[nodeIndex].firstObject = i;
         else
         {
            S32 objectIndex = nodes[nodeIndex].firstObject;
            while (objects[objectIndex].nextSibling>=0)
               objectIndex = objects[objectIndex].nextSibling;
            objects[objectIndex].nextSibling = i;
         }
      }
   }

   mFlags = 0;
   for (i=0; i<sequences.size(); i++)
   {
      if (!sequences[i].animatesScale())
         continue;

      std::bitset<32> curVal = std::bitset<32>(mFlags) & std::bitset<32>(AnyScale);
      std::bitset<32> newVal = sequences[i].flags & std::bitset<32>(AnyScale);
      mFlags &= ~(AnyScale);
      mFlags |= std::max(curVal.to_ulong(),newVal.to_ulong()); // take the larger value (can only convert upwards)
   }

   // set up alphaIn and alphaOut vectors...
   alphaIn.setSize(details.size());
   alphaOut.setSize(details.size());

   for (i=0; i<details.size(); i++)
   {
      if (details[i].size<0)
      {
         // we don't care...
         alphaIn[i]  = 0.0f;
         alphaOut[i] = 0.0f;
      }
      else if (i+1==details.size() || details[i+1].size<0)
      {
         alphaIn[i]  = 0.0f;
         alphaOut[i] = smAlphaOutLastDetail;
      }
      else
      {
         if (details[i+1].subShapeNum<0)
         {
            // following detail is a billboard detail...treat special...
            alphaIn[i]  = smAlphaInBillboard;
            alphaOut[i] = smAlphaOutBillboard;
         }
         else
         {
            // next detail is normal detail
            alphaIn[i] = smAlphaInDefault;
            alphaOut[i] = smAlphaOutDefault;
         }
      }
   }

   for (i=mSmallestVisibleDL-1; i>=0; i--)
   {
      if (i<smNumSkipLoadDetails)
      {
         // this detail level renders when pixel size
         // is larger than our cap...zap all the meshes and decals
         // associated with it and use the next detail level
         // instead...
         S32 ss    = details[i].subShapeNum;
         S32 od    = details[i].objectDetailNum;

         if (ss==details[i+1].subShapeNum && od==details[i+1].objectDetailNum)
            // doh! already done this one (init can be called multiple times on same shape due
            // to sequence importing).
            continue;
         details[i].subShapeNum = details[i+1].subShapeNum;
         details[i].objectDetailNum = details[i+1].objectDetailNum;
      }
   }

   for (i=0; i<details.size(); i++)
   {
      S32 count = 0;
      S32 ss = details[i].subShapeNum;
      S32 od = details[i].objectDetailNum;
      if (ss<0)
      {
         // billboard detail...
         details[i].polyCount = 2;
         continue;
      }
      S32 start = subShapeFirstObject[ss];
      S32 end   = start + subShapeNumObjects[ss];
      for (j=start; j<end; j++)
      {
         Object & obj = objects[j];
         if (od<obj.numMeshes)
         {
            TSMesh * mesh = meshes[obj.startMeshIndex+od];
            count += mesh ? mesh->getNumPolys() : 0;
         }
      }
      details[i].polyCount = count;
   }

   // Init the collision accelerator array.  Note that we don't compute the
   //  accelerators until the app requests them
   {
      S32 dca;
      for (dca = 0; dca < detailCollisionAccelerators.size(); dca++)
      {
         ConvexHullAccelerator* accel = detailCollisionAccelerators[dca];
         if (accel != nullptr) {
            delete [] accel->vertexList;
            delete [] accel->normalList;
            for (S32 j = 0; j < accel->numVerts; j++)
               delete [] accel->emitStrings[j];
            delete [] accel->emitStrings;
            delete accel;
         }
      }

      detailCollisionAccelerators.setSize(details.size());
      for (dca = 0; dca < detailCollisionAccelerators.size(); dca++)
         detailCollisionAccelerators[dca] = nullptr;
   }

   initVertexFeatures();
   initMaterialList();
}

void TSShape::initVertexFeatures()
{
   bool hasColors = false;
   bool hasTexcoord2 = false;

   Vector<TSMesh*>::iterator iter = meshes.begin();
   for ( ; iter != meshes.end(); iter++ )
   {
      TSMesh *mesh = *iter;
      if (  mesh &&
            (  mesh->getMeshType() == TSMesh::StandardMeshType ||
               mesh->getMeshType() == TSMesh::SkinMeshType ) )
      {
         if ( mesh->mVertexData.isReady() )
         {
            hasColors |= mesh->mHasColor;
            hasTexcoord2 |= mesh->mHasTVert2;
         }
         else
         {
            hasColors |= !mesh->colors.empty();
            hasTexcoord2 |= !mesh->tverts2.empty();
         }
      }
   }

   mVertSize = ( hasTexcoord2 || hasColors ) ? sizeof(TSMesh::__TSMeshVertex_3xUVColor) : sizeof(TSMesh::__TSMeshVertexBase);
   mVertexFormat.clear();
  
   mVertexFormat.addElement( GFXSemantic::POSITION, GFXDeclType_Float3 );
   mVertexFormat.addElement( GFXSemantic::TANGENTW, GFXDeclType_Float, 3 );
   mVertexFormat.addElement( GFXSemantic::NORMAL, GFXDeclType_Float3 );
   mVertexFormat.addElement( GFXSemantic::TANGENT, GFXDeclType_Float3 );

   mVertexFormat.addElement( GFXSemantic::TEXCOORD, GFXDeclType_Float2, 0 );

   if(hasTexcoord2 || hasColors)
   {
      mVertexFormat.addElement( GFXSemantic::TEXCOORD, GFXDeclType_Float2, 1 );
      mVertexFormat.addElement( GFXSemantic::COLOR, GFXDeclType_Color );
      mVertexFormat.addElement( GFXSemantic::TEXCOORD, GFXDeclType_Float, 2 );
   }

   // Go fix up meshes to include defaults for optional features
   // and initialize them if they're not a skin mesh.
   iter = meshes.begin();
   for ( ; iter != meshes.end(); iter++ )
   {
      TSMesh *mesh = *iter;
      if (  !mesh ||
            (  mesh->getMeshType() != TSMesh::StandardMeshType &&
               mesh->getMeshType() != TSMesh::SkinMeshType ) )
         continue;

      // Set the flags.
      mesh->mVertexFormat = &mVertexFormat;
      mesh->mVertSize = mVertSize;

      // Create and fill aligned data structure
      mesh->convertToAlignedMeshData();

      // Init the vertex buffer.
      if ( mesh->getMeshType() == TSMesh::StandardMeshType )
         mesh->createVBIB();
   }
}

void TSShape::setupBillboardDetails( const String &cachePath )
{
   // set up billboard details -- only do this once, meaning that
   // if we add a sequence to the shape we don't redo the billboard
   // details...
//   if ( !billboardDetails.empty() )
      return;

//   for ( U32 i=0; i < details.size(); i++ )
//   {
//      const Detail &det = details[i];
//
//      if ( det.subShapeNum >= 0 )
//         continue; // not a billboard detail
//
//      while (billboardDetails.size() <= i )
//         billboardDetails.push_back(nullptr);
//
//      billboardDetails[i] = new TSLastDetail(   this,
//                                                cachePath,
//                                                det.bbEquatorSteps,
//                                                det.bbPolarSteps,
//                                                det.bbPolarAngle,
//                                                det.bbIncludePoles,
//                                                det.bbDetailLevel,
//                                                det.bbDimension );
//
//      billboardDetails[i]->update();
//   }
}

void TSShape::initMaterialList()
{
    materialList = new TSMaterialList;
//   S32 numSubShapes = subShapeFirstObject.size();
//   #if defined(TORQUE_MAX_LIB)
//   subShapeFirstTranslucentObject.setSize(numSubShapes);
//   #endif
//
//   mHasSkinMesh = false;
//
//   S32 i,j,k;
//   // for each subshape, find the first translucent object
//   // also, while we're at it, set mHasTranslucency
//   for (S32 ss = 0; ss<numSubShapes; ss++)
//   {
//      S32 start = subShapeFirstObject[ss];
//      S32 end = subShapeNumObjects[ss];
//      subShapeFirstTranslucentObject[ss] = end;
//      for (i=start; i<end; i++)
//      {
//         // check to see if this object has translucency
//         Object & obj = objects[i];
//         for (j=0; j<obj.numMeshes; j++)
//         {
//            TSMesh * mesh = meshes[obj.startMeshIndex+j];
//            if (!mesh)
//               continue;
//
//            mHasSkinMesh |= mesh->getMeshType() == TSMesh::SkinMeshType;
//
//            for (k=0; k<mesh->primitives.size(); k++)
//            {
//               if (mesh->primitives[k].matIndex & TSDrawPrimitive::NoMaterial)
//                  continue;
//               S32 flags = materialList->getFlags(mesh->primitives[k].matIndex & TSDrawPrimitive::MaterialMask);
//               if (flags & TSMaterialList::AuxiliaryMap)
//                  continue;
//               if (flags & TSMaterialList::Translucent)
//               {
//                  mFlags |= HasTranslucency;
//                  subShapeFirstTranslucentObject[ss] = i;
//                  break;
//               }
//            }
//            if (k!=mesh->primitives.size())
//               break;
//         }
//         if (j!=obj.numMeshes)
//            break;
//      }
//      if (i!=end)
//         break;
//   }

}

bool TSShape::preloadMaterialList(const String &path)
{
//   if (materialList)
//      materialList->setTextureLookupPath(path.getPath());
   return true;
}

//bool TSShape::buildConvexHull(S32 dl) const
//{
//   AssertFatal(dl>=0 && dl<details.size(),"TSShape::buildConvexHull: detail out of range");
//
//   bool ok = true;
//
//   const Detail & detail = details[dl];
//   S32 ss = detail.subShapeNum;
//   S32 od = detail.objectDetailNum;
//
//   S32 start = subShapeFirstObject[ss];
//   S32 end   = subShapeNumObjects[ss];
//   for (S32 i=start; i<end; i++)
//   {
//      TSMesh * mesh = meshes[objects[i].startMeshIndex+od];
//      if (!mesh)
//         continue;
//      ok &= mesh->buildConvexHull();
//   }
//   return ok;
//}

Vector<MatrixF> gTempNodeTransforms(__FILE__, __LINE__);

//void TSShape::computeBounds(S32 dl, Box3F & bounds) const
//{
//   // if dl==-1, nothing to do
//   if (dl==-1)
//      return;
//
//   AssertFatal(dl>=0 && dl<details.size(),"TSShapeInstance::computeBounds");
//
//   // get subshape and object detail
//   const TSDetail * detail = &details[dl];
//   S32 ss = detail->subShapeNum;
//   S32 od = detail->objectDetailNum;
//
//   // If we have no subshapes then there is
//   // no valid bounds for this detail level.
//   if ( ss < 0 )
//      return;
//
//   // set up temporary storage for non-local transforms...
//   S32 i;
//   S32 start = subShapeFirstNode[ss];
//   S32 end   = subShapeNumNodes[ss] + start;
//   gTempNodeTransforms.setSize(end-start);
//   for (i=start; i<end; i++)
//   {
//      MatrixF mat;
//      QuatF q;
//      TSTransform::setMatrix(defaultRotations[i].getQuatF(&q),defaultTranslations[i],&mat);
//      if (nodes[i].parentIndex>=0)
//         gTempNodeTransforms[i-start].mul(gTempNodeTransforms[nodes[i].parentIndex-start],mat);
//      else
//         gTempNodeTransforms[i-start] = mat;
//   }
//
//   // run through objects and updating bounds as we go
//   bounds.minExtents.set( 10E30f, 10E30f, 10E30f);
//   bounds.maxExtents.set(-10E30f,-10E30f,-10E30f);
//   Box3F box;
//   start = subShapeFirstObject[ss];
//   end   = subShapeNumObjects[ss] + start;
//   for (i=start; i<end; i++)
//   {
//      const Object * object = &objects[i];
//      TSMesh * mesh = od<object->numMeshes ? meshes[object->startMeshIndex+od] : nullptr;
//      if (mesh)
//      {
//         static MatrixF idMat(true);
//         if (object->nodeIndex<0)
//            mesh->computeBounds(idMat,box);
//         else
//            mesh->computeBounds(gTempNodeTransforms[object->nodeIndex-start],box);
//         bounds.minExtents.setMin(box.minExtents);
//         bounds.maxExtents.setMax(box.maxExtents);
//      }
//   }
//}

TSShapeAlloc TSShape::smTSAlloc;

#define tsalloc TSShape::smTSAlloc


// messy stuff: check to see if we should "skip" meshNum
// this assumes that meshes for a given object are in a row
// skipDL is the lowest detail number we keep (i.e., the # of details we skip)
bool TSShape::checkSkip(S32 meshNum, S32 & curObject, S32 skipDL)
{
   if (skipDL==0)
      // easy out...
      return false;

   // skip detail level exists on this subShape
   S32 skipSS = details[skipDL].subShapeNum;

   if (curObject<objects.size())
   {
      S32 start = objects[curObject].startMeshIndex;
      if (meshNum>=start)
      {
         // we are either from this object, the next object, or a decal
         if (meshNum < start + objects[curObject].numMeshes)
         {
            // this object...
            if (subShapeFirstObject[skipSS]>curObject)
               // haven't reached this subshape yet
               return true;
            if (skipSS+1==subShapeFirstObject.size() || curObject<subShapeFirstObject[skipSS+1])
               // curObject is on subshape of skip detail...make sure it's after skipDL
               return (meshNum-start<details[skipDL].objectDetailNum);
            // if we get here, then curObject occurs on subShape after skip detail (so keep it)
            return false;
         }
         else
            // advance object, try again
            return checkSkip(meshNum,++curObject,skipDL);
      }
   }

   AssertFatal(0,"TSShape::checkSkip: assertion failed");
   return false;
}

void TSShape::assembleShape()
{
   S32 i,j;

   // get counts...
   S32 numNodes = tsalloc.get32();
   S32 numObjects = tsalloc.get32();
   S32 numDecals = tsalloc.get32();
   S32 numSubShapes = tsalloc.get32();
   S32 numIflMaterials = tsalloc.get32();
   S32 numNodeRots;
   S32 numNodeTrans;
   S32 numNodeUniformScales;
   S32 numNodeAlignedScales;
   S32 numNodeArbitraryScales;
   if (smReadVersion<22)
   {
      numNodeRots = numNodeTrans = tsalloc.get32() - numNodes;
      numNodeUniformScales = numNodeAlignedScales = numNodeArbitraryScales = 0;
   }
   else
   {
      numNodeRots = tsalloc.get32();
      numNodeTrans = tsalloc.get32();
      numNodeUniformScales = tsalloc.get32();
      numNodeAlignedScales = tsalloc.get32();
      numNodeArbitraryScales = tsalloc.get32();
   }
   S32 numGroundFrames = 0;
   if (smReadVersion>23)
      numGroundFrames = tsalloc.get32();
   S32 numObjectStates = tsalloc.get32();
   S32 numDecalStates = tsalloc.get32();
   S32 numTriggers = tsalloc.get32();
   S32 numDetails = tsalloc.get32();
   S32 numMeshes = tsalloc.get32();
   S32 numSkins = 0;
   if (smReadVersion<23)
      // in later versions, skins are kept with other meshes
      numSkins = tsalloc.get32();
   S32 numNames = tsalloc.get32();

   // Note that we are recalculating these values later on for safety.
   mSmallestVisibleSize = (F32)tsalloc.get32();
   mSmallestVisibleDL   = tsalloc.get32();
   
   tsalloc.checkGuard();

   // get bounds...
   tsalloc.get32((S32*)&radius,1);
   tsalloc.get32((S32*)&tubeRadius,1);
   tsalloc.get32((S32*)&center,3);
   tsalloc.get32((S32*)&bounds,6);

   tsalloc.checkGuard();

   // copy various vectors...
   S32 * ptr32 = tsalloc.copyToShape32(numNodes*5);
   nodes.set((Node*)ptr32,numNodes);

   tsalloc.checkGuard();

   ptr32 = tsalloc.copyToShape32(numObjects*6,true);
   if (!ptr32)
      ptr32 = tsalloc.allocShape32(numSkins*6); // pre v23 shapes store skins and meshes separately...no longer
   else
      tsalloc.allocShape32(numSkins*6);
   objects.set((Object*)ptr32,numObjects);

   tsalloc.checkGuard();

   // DEPRECATED decals
   ptr32 = tsalloc.getPointer32(numDecals*5);

   tsalloc.checkGuard();

   // DEPRECATED ifl materials
   ptr32 = tsalloc.copyToShape32(numIflMaterials*5);

   tsalloc.checkGuard();

   ptr32 = tsalloc.copyToShape32(numSubShapes,true);
   subShapeFirstNode.set(ptr32,numSubShapes);
   ptr32 = tsalloc.copyToShape32(numSubShapes,true);
   subShapeFirstObject.set(ptr32,numSubShapes);
   // DEPRECATED subShapeFirstDecal
   ptr32 = tsalloc.getPointer32(numSubShapes);

   tsalloc.checkGuard();

   ptr32 = tsalloc.copyToShape32(numSubShapes);
   subShapeNumNodes.set(ptr32,numSubShapes);
   ptr32 = tsalloc.copyToShape32(numSubShapes);
   subShapeNumObjects.set(ptr32,numSubShapes);
   // DEPRECATED subShapeNumDecals
   ptr32 = tsalloc.getPointer32(numSubShapes);

   tsalloc.checkGuard();

   ptr32 = tsalloc.allocShape32(numSubShapes);
   subShapeFirstTranslucentObject.set(ptr32,numSubShapes);

   // get default translation and rotation
   S16 * ptr16 = tsalloc.allocShape16(0);
   for (i=0;i<numNodes;i++)
      tsalloc.copyToShape16(4);
   defaultRotations.set((Quat16*)ptr16,numNodes);
   tsalloc.align32();
   ptr32 = tsalloc.allocShape32(0);
   for (i=0;i<numNodes;i++)
   {
      tsalloc.copyToShape32(3);
      tsalloc.copyToShape32(sizeof(Point3F)-12); // handle alignment issues w/ point3f
   }
   defaultTranslations.set((Point3F*)ptr32,numNodes);

   // get any node sequence data stored in shape
   nodeTranslations.setSize(numNodeTrans);
   for (i=0;i<numNodeTrans;i++)
      tsalloc.get32((S32*)&nodeTranslations[i],3);
   nodeRotations.setSize(numNodeRots);
   for (i=0;i<numNodeRots;i++)
      tsalloc.get16((S16*)&nodeRotations[i],4);
   tsalloc.align32();

   tsalloc.checkGuard();

   if (smReadVersion>21)
   {
      // more node sequence data...scale
      nodeUniformScales.setSize(numNodeUniformScales);
      for (i=0;i<numNodeUniformScales;i++)
         tsalloc.get32((S32*)&nodeUniformScales[i],1);
      nodeAlignedScales.setSize(numNodeAlignedScales);
      for (i=0;i<numNodeAlignedScales;i++)
         tsalloc.get32((S32*)&nodeAlignedScales[i],3);
      nodeArbitraryScaleFactors.setSize(numNodeArbitraryScales);
      for (i=0;i<numNodeArbitraryScales;i++)
         tsalloc.get32((S32*)&nodeArbitraryScaleFactors[i],3);
      nodeArbitraryScaleRots.setSize(numNodeArbitraryScales);
      for (i=0;i<numNodeArbitraryScales;i++)
         tsalloc.get16((S16*)&nodeArbitraryScaleRots[i],4);
      tsalloc.align32();

      tsalloc.checkGuard();
   }

   // old shapes need ground transforms moved to ground arrays...but only do it once
   if (smReadVersion<22 && tsalloc.allocShape32(0))
   {
      for (i=0; i<sequences.size(); i++)
      {
         // move ground transform data to ground vectors
         Sequence & seq = sequences[i];
         S32 oldSz = groundTranslations.size();
         groundTranslations.setSize(oldSz+seq.numGroundFrames);
         groundRotations.setSize(oldSz+seq.numGroundFrames);
         for (S32 j=0;j<seq.numGroundFrames;j++)
         {
            groundTranslations[j+oldSz] = nodeTranslations[seq.firstGroundFrame+j-numNodes];
            groundRotations[j+oldSz] = nodeRotations[seq.firstGroundFrame+j-numNodes];
         }
         seq.firstGroundFrame = oldSz;
         seq.baseTranslation -= numNodes;
         seq.baseRotation -= numNodes;
         seq.baseScale = 0; // not used on older shapes...but keep it clean
      }
   }

   // version 22 & 23 shapes accidentally had no ground transforms, and ground for
   // earlier shapes is handled just above, so...
   if (smReadVersion>23)
   {
      groundTranslations.setSize(numGroundFrames);
      for (i=0;i<numGroundFrames;i++)
         tsalloc.get32((S32*)&groundTranslations[i],3);
      groundRotations.setSize(numGroundFrames);
      for (i=0;i<numGroundFrames;i++)
         tsalloc.get16((S16*)&groundRotations[i],4);
      tsalloc.align32();

      tsalloc.checkGuard();
   }

   // object states
   ptr32 = tsalloc.copyToShape32(numObjectStates*3);
   objectStates.set((ObjectState*)ptr32,numObjectStates);
   tsalloc.allocShape32(numSkins*3); // provide buffer after objectStates for older shapes

   tsalloc.checkGuard();

   // DEPRECATED decal states
   ptr32 = tsalloc.getPointer32(numDecalStates);

   tsalloc.checkGuard();

   // frame triggers
   ptr32 = tsalloc.getPointer32(numTriggers*2);
   triggers.setSize(numTriggers);
   dMemcpy(triggers.address(),ptr32,sizeof(S32)*numTriggers*2);

   tsalloc.checkGuard();

   // details
   if ( smReadVersion >= 26 )
   {
      U32 alignedSize32 = sizeof( Detail ) / 4;
      ptr32 = tsalloc.copyToShape32( numDetails * alignedSize32, true );
      details.set( (Detail*)ptr32, numDetails );
   }
   else
   {
      // Previous to version 26 the Detail structure
      // only contained the first 7 values...
      //
      //    struct Detail
      //    {
      //       S32 nameIndex;
      //       S32 subShapeNum;
      //       S32 objectDetailNum;
      //       F32 size;
      //       F32 averageError;
      //       F32 maxError;
      //       S32 polyCount;
      //    };
      //
      // In the code below we're reading just these 7 values and
      // copying them to the new larger structure.

      ptr32 = tsalloc.copyToShape32( numDetails * 7, true );

      details.setSize( numDetails );
      for ( U32 i = 0; i < details.size(); i++, ptr32 += 7 )
      {
         Detail *det = &(details[i]);

         // Clear the struct... we don't want to leave 
         // garbage in the parts that are unfilled.
         U32 alignedSize32 = sizeof( Detail );
         dMemset( det, 0, alignedSize32 );

         // Copy the old struct values over.
         dMemcpy( det, ptr32, 7 * 4 );

         // If this is an autobillboard then we need to
         // fill in the new part of the struct.
         if ( det->subShapeNum >= 0 )
            continue; 

         S32 lastDetailOpts = det->objectDetailNum;
         det->bbEquatorSteps = lastDetailOpts & 0x7F; // bits 0..6
         det->bbPolarSteps = (lastDetailOpts >> 7) & 0x3F; // bits 7..12
         det->bbPolarAngle = 0.5f * M_PI_F * (1.0f/64.0f) * (F32) (( lastDetailOpts >>13 ) & 0x3F); // bits 13..18
         det->bbDetailLevel = (lastDetailOpts >> 19) & 0x0F;  // 19..22
         det->bbDimension = (lastDetailOpts >> 23) & 0xFF; // 23..30
         det->bbIncludePoles = (lastDetailOpts & 0x80000000)!=0; // bit 31
      }
   }

   // Some DTS exporters (MAX - I'm looking at you!) write garbage into the
   // averageError and maxError values which stops LOD from working correctly.
   // Try to detect and fix it
   for ( U32 i = 0; i < details.size(); i++ )
   {
      if ( ( details[i].averageError == 0 ) || ( details[i].averageError > 10000 ) ||
           ( details[i].maxError == 0 ) || ( details[i].maxError > 10000 ) )
      {
         details[i].averageError = details[i].maxError = -1.0f;
      }
   }

   // We don't trust the value of mSmallestVisibleDL loaded from the dts
   // since some legacy meshes seem to have the wrong value. Recalculate it
   // now that we have the details loaded.
//   updateSmallestVisibleDL();

   S32 skipDL = std::min(mSmallestVisibleDL,smNumSkipLoadDetails);
   if (skipDL < 0)
      skipDL = 0;


   tsalloc.checkGuard();

   // about to read in the meshes...first must allocate some scratch space
   S32 scratchSize = std::max(numSkins,numMeshes);
   TSMesh::smVertsList.setSize(scratchSize);
   TSMesh::smTVertsList.setSize(scratchSize);

   if ( smReadVersion >= 26 )
   {
      TSMesh::smTVerts2List.setSize(scratchSize);
      TSMesh::smColorsList.setSize(scratchSize);
   }

   TSMesh::smNormsList.setSize(scratchSize);
   TSMesh::smEncodedNormsList.setSize(scratchSize);
   TSMesh::smDataCopied.resize(scratchSize);
   TSSkinMesh::smInitTransformList.setSize(scratchSize);
   TSSkinMesh::smVertexIndexList.setSize(scratchSize);
   TSSkinMesh::smBoneIndexList.setSize(scratchSize);
   TSSkinMesh::smWeightList.setSize(scratchSize);
   TSSkinMesh::smNodeIndexList.setSize(scratchSize);
   for (i=0; i<numMeshes; i++)
   {
      TSMesh::smVertsList[i]=nullptr;
      TSMesh::smTVertsList[i]=nullptr;
      
      if ( smReadVersion >= 26 )
      {
         TSMesh::smTVerts2List[i] = nullptr;
         TSMesh::smColorsList[i] = nullptr;
      }
      
      TSMesh::smNormsList[i]=nullptr;
      TSMesh::smEncodedNormsList[i]=nullptr;
      TSMesh::smDataCopied[i]=false;
      TSSkinMesh::smInitTransformList[i] = nullptr;
      TSSkinMesh::smVertexIndexList[i] = nullptr;
      TSSkinMesh::smBoneIndexList[i] = nullptr;
      TSSkinMesh::smWeightList[i] = nullptr;
      TSSkinMesh::smNodeIndexList[i] = nullptr;
   }

   // read in the meshes (sans skins)...straightforward read one at a time
   ptr32 = tsalloc.allocShape32(numMeshes + numSkins * numDetails * sizeof(PTR)/4); // leave room for skins on old shapes
   PTR* mesh_ptrs = (PTR*)ptr32;
   S32 curObject = 0; // for tracking skipped meshes
   for (i=0; i<numMeshes; i++)
   {
      bool skip = checkSkip(i,curObject,skipDL); // skip this mesh?
      S32 meshType = tsalloc.get32();
      if (meshType == TSMesh::DecalMeshType)
         // decal mesh deprecated
         skip = true;
      TSMesh * mesh = TSMesh::assembleMesh(meshType,skip);
      if (mesh_ptrs)
         mesh_ptrs[i] = skip ?  NULL : (PTR)mesh;

      // fill in location of verts, tverts, and normals for detail levels
      if (mesh && meshType!=TSMesh::DecalMeshType)
      {
         TSMesh::smVertsList[i]  = mesh->verts.address();
         TSMesh::smTVertsList[i] = mesh->tverts.address();
         TSMesh::smNormsList[i]  = mesh->norms.address();
         TSMesh::smEncodedNormsList[i] = mesh->encodedNorms.address();
         TSMesh::smDataCopied[i] = !skip; // as long as we didn't skip this mesh, the data should be in shape now
         if (meshType==TSMesh::SkinMeshType)
         {
            TSSkinMesh * skin = (TSSkinMesh*)mesh;
            TSMesh::smVertsList[i]  = skin->batchData.initialVerts.address();
            TSMesh::smNormsList[i]  = skin->batchData.initialNorms.address();
            TSSkinMesh::smInitTransformList[i] = skin->batchData.initialTransforms.address();
            TSSkinMesh::smVertexIndexList[i] = skin->vertexIndex.address();
            TSSkinMesh::smBoneIndexList[i] = skin->boneIndex.address();
            TSSkinMesh::smWeightList[i] = skin->weight.address();
            TSSkinMesh::smNodeIndexList[i] = skin->batchData.nodeIndex.address();
         }
      }
   }
   meshes.set((TSMesh * const *)mesh_ptrs,numMeshes);

   tsalloc.checkGuard();

   // names
   char * nameBufferStart = (char*)tsalloc.getPointer8(0);
   char * name = nameBufferStart;
   S32 nameBufferSize = 0;
   names.setSize(numNames);
   for (i=0; i<numNames; i++)
   {
      for (j=0; name[j]; j++)
         ;

      names[i] = name;
      nameBufferSize += j + 1;
      name += j + 1;
   }

   tsalloc.getPointer8(nameBufferSize);
   tsalloc.align32();

   tsalloc.checkGuard();

   if (smReadVersion<23)
   {
      // get detail information about skins...
      S32 * detailFirstSkin = tsalloc.getPointer32(numDetails);
//      S32 * detailNumSkins = tsalloc.getPointer32(numDetails);

      tsalloc.checkGuard();

      // about to read in skins...clear out scratch space...
      if (numSkins)
      {
         TSSkinMesh::smInitTransformList.setSize(numSkins);
         TSSkinMesh::smVertexIndexList.setSize(numSkins);
         TSSkinMesh::smBoneIndexList.setSize(numSkins);
         TSSkinMesh::smWeightList.setSize(numSkins);
         TSSkinMesh::smNodeIndexList.setSize(numSkins);
      }
      for (i=0; i<numSkins; i++)
      {
         TSMesh::smVertsList[i]=nullptr;
         TSMesh::smTVertsList[i]=nullptr;
         TSMesh::smNormsList[i]=nullptr;
         TSMesh::smEncodedNormsList[i]=nullptr;
         TSMesh::smDataCopied[i]=false;
         TSSkinMesh::smInitTransformList[i] = nullptr;
         TSSkinMesh::smVertexIndexList[i] = nullptr;
         TSSkinMesh::smBoneIndexList[i] = nullptr;
         TSSkinMesh::smWeightList[i] = nullptr;
         TSSkinMesh::smNodeIndexList[i] = nullptr;
      }

      // skins
      ptr32 = tsalloc.allocShape32(numSkins);
      for (i=0; i<numSkins; i++)
      {
         bool skip = i<detailFirstSkin[skipDL];
         TSSkinMesh * skin = (TSSkinMesh*)TSMesh::assembleMesh(TSMesh::SkinMeshType,skip);
         if (meshes.address())
         {
            // add pointer to skin in shapes list of meshes
            // we reserved room for this above...
            meshes.set(meshes.address(),meshes.size()+1);
            meshes[meshes.size()-1] = skip ? nullptr : skin;
         }

         // fill in location of verts, tverts, and normals for shared detail levels
         if (skin)
         {
            TSMesh::smVertsList[i]  = skin->batchData.initialVerts.address();
            TSMesh::smTVertsList[i] = skin->tverts.address();
            TSMesh::smNormsList[i]  = skin->batchData.initialNorms.address();
            TSMesh::smEncodedNormsList[i]  = skin->encodedNorms.address();
            TSMesh::smDataCopied[i] = !skip; // as long as we didn't skip this mesh, the data should be in shape now
            TSSkinMesh::smInitTransformList[i] = skin->batchData.initialTransforms.address();
            TSSkinMesh::smVertexIndexList[i] = skin->vertexIndex.address();
            TSSkinMesh::smBoneIndexList[i] = skin->boneIndex.address();
            TSSkinMesh::smWeightList[i] = skin->weight.address();
            TSSkinMesh::smNodeIndexList[i] = skin->batchData.nodeIndex.address();
         }
      }

      tsalloc.checkGuard();

      // we now have skins in mesh list...add skin objects to object list and patch things up
//      fixupOldSkins(numMeshes,numSkins,numDetails,detailFirstSkin,detailNumSkins);
   }

   // allocate storage space for some arrays (filled in during Shape::init)...
   ptr32 = tsalloc.allocShape32(numDetails);
   alphaIn.set((float*)ptr32,numDetails);
   ptr32 = tsalloc.allocShape32(numDetails);
   alphaOut.set((float*)ptr32,numDetails);
}

void TSShape::disassembleShape()
{
   S32 i;

   // set counts...
   S32 numNodes = tsalloc.set32(nodes.size());
   S32 numObjects = tsalloc.set32(objects.size());
   tsalloc.set32(0); // DEPRECATED decals
   S32 numSubShapes = tsalloc.set32(subShapeFirstNode.size());
   tsalloc.set32(0); // DEPRECATED ifl materials
   S32 numNodeRotations = tsalloc.set32(nodeRotations.size());
   S32 numNodeTranslations = tsalloc.set32(nodeTranslations.size());
   S32 numNodeUniformScales = tsalloc.set32(nodeUniformScales.size());
   S32 numNodeAlignedScales = tsalloc.set32(nodeAlignedScales.size());
   S32 numNodeArbitraryScales = tsalloc.set32(nodeArbitraryScaleFactors.size());
   S32 numGroundFrames = tsalloc.set32(groundTranslations.size());
   S32 numObjectStates = tsalloc.set32(objectStates.size());
   tsalloc.set32(0); // DEPRECATED decals
   S32 numTriggers = tsalloc.set32(triggers.size());
   S32 numDetails = tsalloc.set32(details.size());
   S32 numMeshes = tsalloc.set32(meshes.size());
   S32 numNames = tsalloc.set32(names.size());
   tsalloc.set32((S32)mSmallestVisibleSize);
   tsalloc.set32(mSmallestVisibleDL);

   tsalloc.setGuard();

   // get bounds...
   tsalloc.copyToBuffer32((S32*)&radius,1);
   tsalloc.copyToBuffer32((S32*)&tubeRadius,1);
   tsalloc.copyToBuffer32((S32*)&center,3);
   tsalloc.copyToBuffer32((S32*)&bounds,6);

   tsalloc.setGuard();

   // copy various vectors...
   tsalloc.copyToBuffer32((S32*)nodes.address(),numNodes*5);
   tsalloc.setGuard();
   tsalloc.copyToBuffer32((S32*)objects.address(),numObjects*6);
   tsalloc.setGuard();
   // DEPRECATED: no copy decals
   tsalloc.setGuard();
   tsalloc.copyToBuffer32(0,0); // DEPRECATED: ifl materials!
   tsalloc.setGuard();
   tsalloc.copyToBuffer32((S32*)subShapeFirstNode.address(),numSubShapes);
   tsalloc.copyToBuffer32((S32*)subShapeFirstObject.address(),numSubShapes);
   tsalloc.copyToBuffer32(0, numSubShapes); // DEPRECATED: no copy subShapeFirstDecal
   tsalloc.setGuard();
   tsalloc.copyToBuffer32((S32*)subShapeNumNodes.address(),numSubShapes);
   tsalloc.copyToBuffer32((S32*)subShapeNumObjects.address(),numSubShapes);
   tsalloc.copyToBuffer32(0, numSubShapes); // DEPRECATED: no copy subShapeNumDecals
   tsalloc.setGuard();

   // default transforms...
   tsalloc.copyToBuffer16((S16*)defaultRotations.address(),numNodes*4);
   tsalloc.copyToBuffer32((S32*)defaultTranslations.address(),numNodes*3);

   // animated transforms...
   tsalloc.copyToBuffer16((S16*)nodeRotations.address(),numNodeRotations*4);
   tsalloc.copyToBuffer32((S32*)nodeTranslations.address(),numNodeTranslations*3);

   tsalloc.setGuard();

   // ...with scale
   tsalloc.copyToBuffer32((S32*)nodeUniformScales.address(),numNodeUniformScales);
   tsalloc.copyToBuffer32((S32*)nodeAlignedScales.address(),numNodeAlignedScales*3);
   tsalloc.copyToBuffer32((S32*)nodeArbitraryScaleFactors.address(),numNodeArbitraryScales*3);
   tsalloc.copyToBuffer16((S16*)nodeArbitraryScaleRots.address(),numNodeArbitraryScales*4);

   tsalloc.setGuard();

   tsalloc.copyToBuffer32((S32*)groundTranslations.address(),3*numGroundFrames);
   tsalloc.copyToBuffer16((S16*)groundRotations.address(),4*numGroundFrames);

   tsalloc.setGuard();

   // object states..
   tsalloc.copyToBuffer32((S32*)objectStates.address(),numObjectStates*3);
   tsalloc.setGuard();

   // decal states...
   // DEPRECATED (numDecalStates = 0)
   tsalloc.setGuard();

   // frame triggers
   tsalloc.copyToBuffer32((S32*)triggers.address(),numTriggers*2);
   tsalloc.setGuard();

   // details
   if (TSShape::smVersion > 25)
   {
      U32 alignedSize32 = sizeof( Detail ) / 4;
      tsalloc.copyToBuffer32((S32*)details.address(),numDetails * alignedSize32 );
   }
   else
   {
      // Legacy details => no explicit autobillboard parameters
      U32 legacyDetailSize32 = 7;   // only store the first 7 4-byte values of each detail
      for ( S32 i = 0; i < details.size(); i++ )
         tsalloc.copyToBuffer32( (S32*)&details[i], legacyDetailSize32 );
   }
   tsalloc.setGuard();

   // read in the meshes (sans skins)...
   bool * isMesh = new bool[numMeshes]; // funny business because decals are pretend meshes (legacy issue)
   for (i=0;i<numMeshes;i++)
      isMesh[i]=false;
   for (i=0; i<objects.size(); i++)
   {
      for (S32 j=0; j<objects[i].numMeshes; j++)
         // even if an empty mesh, it's a mesh...
         isMesh[objects[i].startMeshIndex+j]=true;
   }
   for (i=0; i<numMeshes; i++)
   {
      TSMesh * mesh = nullptr;
      // decal mesh deprecated
      if (isMesh[i])
         mesh = meshes[i];
      tsalloc.set32( (mesh && mesh->getMeshType() != TSMesh::DecalMeshType) ? mesh->getMeshType() : TSMesh::NullMeshType);
      if (mesh)
         mesh->disassemble();
   }
   delete [] isMesh;
   tsalloc.setGuard();

   // names
   for (i=0; i<numNames; i++)
      tsalloc.copyToBuffer8((S8 *)(names[i].c_str()),names[i].length()+1);

   tsalloc.setGuard();
}

//-------------------------------------------------
// write whole shape
//-------------------------------------------------
/** Determine whether we can write this shape in TSTPRO compatible format */
bool TSShape::canWriteOldFormat() const
{
   // Cannot use old format if using autobillboard details
   for (S32 i = 0; i < details.size(); i++)
   {
      if (details[i].subShapeNum < 0)
         return false;
   }

   for (S32 i = 0; i < meshes.size(); i++)
   {
      if (!meshes[i])
         continue;

      // Cannot use old format if using the new functionality (COLORs, 2nd UV set)
      if (meshes[i]->tverts2.size() || meshes[i]->colors.size())
         return false;

      // Cannot use old format if any primitive has too many triangles
      // (ie. cannot fit in a S16)
      for (S32 j = 0; j < meshes[i]->primitives.size(); j++)
      {
         if ((meshes[i]->primitives[j].start +
               meshes[i]->primitives[j].numElements) >= (1 << 15))
         {
            return false;
         }
      }
   }

   return true;
}

void TSShape::write(std::iostream s, bool saveOldFormat)
{
   S32 currentVersion = smVersion;
   if (saveOldFormat)
      smVersion = 24;

   // write version
   s << (smVersion | (mExporterVersion<<16));

   tsalloc.setWrite();
   disassembleShape();

   S32     * buffer32 = tsalloc.getBuffer32();
   S16     * buffer16 = tsalloc.getBuffer16();
   S8      * buffer8  = tsalloc.getBuffer8();

   S32 size32 = tsalloc.getBufferSize32();
   S32 size16 = tsalloc.getBufferSize16();
   S32 size8  = tsalloc.getBufferSize8();

   // convert sizes to dwords...
   if (size16 & 1)
      size16 += 2;
   size16 >>= 1;
   if (size8 & 3)
      size8 += 4;
   size8 >>= 2;

   S32 sizeMemBuffer, start16, start8;
   sizeMemBuffer = size32 + size16 + size8;
   start16 = size32;
   start8 = start16+size16;

   // in dwords -- write will properly endian-flip.
   s << (sizeMemBuffer);
   s << (start16);
   s << (start8);

	// endian-flip the entire write buffers.
   fixEndian(buffer32,buffer16,buffer8,size32,size16,size8);

   // now write buffers
   s.write((char*)buffer32, size32*4);
   s.write((char*)buffer16, size16*4);
   s.write((char*)buffer8, size8*4);

   // write sequences - write will properly endian-flip.
   s << (sequences.size());
   for (S32 i=0; i<sequences.size(); i++)
      sequences[i].write(s);

   // write material list - write will properly endian-flip.
   materialList->write(s);

   delete [] buffer32;
   delete [] buffer16;
   delete [] buffer8;

   smVersion = currentVersion;
}

//-------------------------------------------------
// read whole shape
//-------------------------------------------------

bool TSShape::read(std::iostream &stream)
{
   // read version - read handles endian-flip
    stream >> smReadVersion;
    mExporterVersion = smReadVersion >> 16;
    smReadVersion &= 0xFF;
    if (smReadVersion>smVersion)
    {
      // error -- don't support future versions yet :>
      Con::errorf(ConsoleLogEntry::General,
                  "Error: attempt to load a version %i dts-shape, can currently only load version %i and before.",
                   smReadVersion,smVersion);
      return false;
   }
   mReadVersion = smReadVersion;

   S32 * memBuffer32;
   S16 * memBuffer16;
   S8 * memBuffer8;
   S32 count32, count16, count8;
   if (mReadVersion<19)
   {
      Con::errorf("... Shape with old version.");
      return false;
   }
   else
   {
      S32 i;
      U32 sizeMemBuffer, startU16, startU8;

      // in dwords. - read handles endian-flip
       stream >> sizeMemBuffer;
       stream >> startU16;
       stream >> startU8;

      if (!stream.good())
      {
         Con::errorf(ConsoleLogEntry::General, "Error: bad shape file.");
         return false;
      }

       S32 * tmp = new S32[sizeMemBuffer];
       stream.read((char*)tmp, sizeof(S32)*sizeMemBuffer);

      memBuffer32 = tmp;
      memBuffer16 = (S16*)(tmp+startU16);
      memBuffer8  = (S8*)(tmp+startU8);

      count32 = startU16;
      count16 = startU8-startU16;
      count8  = sizeMemBuffer-startU8;

      // read sequences
      S32 numSequences;
       stream >> numSequences;
      sequences.setSize(numSequences);
      for (i=0; i<numSequences; i++)
      {
         sequences[i].read(stream);

         // Store initial (empty) source data
         sequences[i].sourceData.total = sequences[i].numKeyframes;
         sequences[i].sourceData.end = sequences[i].sourceData.total - 1;
      }

      // read material list
      delete materialList; // just in case...
      materialList = new TSMaterialList;
      materialList->read(stream);
   }

	// since we read in the buffers, we need to endian-flip their entire contents...
   fixEndian(memBuffer32,memBuffer16,memBuffer8,count32,count16,count8);

   tsalloc.setRead(memBuffer32,memBuffer16,memBuffer8,true);
   assembleShape(); // determine size of buffer needed
   mShapeDataSize = tsalloc.getSize();
   tsalloc.doAlloc();
   mShapeData = tsalloc.getBuffer();
   tsalloc.setRead(memBuffer32,memBuffer16,memBuffer8,false);
   assembleShape(); // copy to buffer
   AssertFatal(tsalloc.getSize()==mShapeDataSize,"TSShape::read: shape data buffer size mis-calculated");

   delete [] memBuffer32;

   if (smInitOnRead)
      init();

//if (names.size() == 3 && dStricmp(names[2], "Box") == 0)
//{
//   Con::errorf("\nnodes.set(dMalloc(%d * sizeof(Node)), %d);", nodes.size(), nodes.size());
//   for (U32 i = 0; i < nodes.size(); i++)
//   {
//      Node& obj = nodes[i];
//
//      Con::errorf("   nodes[%d].nameIndex = %d;", i, obj.nameIndex);
//      Con::errorf("   nodes[%d].parentIndex = %d;", i, obj.parentIndex);
//      Con::errorf("   nodes[%d].firstObject = %d;", i, obj.firstObject);
//      Con::errorf("   nodes[%d].firstChild = %d;", i, obj.firstChild);
//      Con::errorf("   nodes[%d].nextSibling = %d;", i, obj.nextSibling);
//   }
//
//   Con::errorf("\nobjects.set(dMalloc(%d * sizeof(Object)), %d);", objects.size(), objects.size());
//   for (U32 i = 0; i < objects.size(); i++)
//   {
//      Object& obj = objects[i];
//
//      Con::errorf("   objects[%d].nameIndex = %d;", i, obj.nameIndex);
//      Con::errorf("   objects[%d].numMeshes = %d;", i, obj.numMeshes);
//      Con::errorf("   objects[%d].startMeshIndex = %d;", i, obj.startMeshIndex);
//      Con::errorf("   objects[%d].nodeIndex = %d;", i, obj.nodeIndex);
//      Con::errorf("   objects[%d].nextSibling = %d;", i, obj.nextSibling);
//      Con::errorf("   objects[%d].firstDecal = %d;", i, obj.firstDecal);
//   }
//
//   Con::errorf("\nobjectStates.set(dMalloc(%d * sizeof(ObjectState)), %d);", objectStates.size(), objectStates.size());
//   for (U32 i = 0; i < objectStates.size(); i++)
//   {
//      ObjectState& obj = objectStates[i];
//
//      Con::errorf("   objectStates[%d].vis = %g;", i, obj.vis);
//      Con::errorf("   objectStates[%d].frameIndex = %d;", i, obj.frameIndex);
//      Con::errorf("   objectStates[%d].matFrameIndex = %d;", i, obj.matFrameIndex);
//   }
//   Con::errorf("\nsubShapeFirstNode.set(dMalloc(%d * sizeof(S32)), %d);", subShapeFirstNode.size(), subShapeFirstNode.size());
//   for (U32 i = 0; i < subShapeFirstNode.size(); i++)
//      Con::errorf("   subShapeFirstNode[%d] = %d;", i, subShapeFirstNode[i]);
//
//   Con::errorf("\nsubShapeFirstObject.set(dMalloc(%d * sizeof(S32)), %d);", subShapeFirstObject.size(), subShapeFirstObject.size());
//   for (U32 i = 0; i < subShapeFirstObject.size(); i++)
//      Con::errorf("   subShapeFirstObject[%d] = %d;", i, subShapeFirstObject[i]);
//
//   //Con::errorf("numDetailFirstSkins = %d", detailFirstSkin.size());
//   Con::errorf("\nsubShapeNumNodes.set(dMalloc(%d * sizeof(S32)), %d);", subShapeNumNodes.size(), subShapeNumNodes.size());
//   for (U32 i = 0; i < subShapeNumNodes.size(); i++)
//      Con::errorf("   subShapeNumNodes[%d] = %d;", i, subShapeNumNodes[i]);
//
//   Con::errorf("\nsubShapeNumObjects.set(dMalloc(%d * sizeof(S32)), %d);", subShapeNumObjects.size(), subShapeNumObjects.size());
//   for (U32 i = 0; i < subShapeNumObjects.size(); i++)
//      Con::errorf("   subShapeNumObjects[%d] = %d;", i, subShapeNumObjects[i]);
//
//   Con::errorf("\ndetails.set(dMalloc(%d * sizeof(Detail)), %d);", details.size(), details.size());
//   for (U32 i = 0; i < details.size(); i++)
//   {
//      Detail& obj = details[i];
//
//      Con::errorf("   details[%d].nameIndex = %d;", i, obj.nameIndex);
//      Con::errorf("   details[%d].subShapeNum = %d;", i, obj.subShapeNum);
//      Con::errorf("   details[%d].objectDetailNum = %d;", i, obj.objectDetailNum);
//      Con::errorf("   details[%d].size = %g;", i, obj.size);
//      Con::errorf("   details[%d].averageError = %g;", i, obj.averageError);
//      Con::errorf("   details[%d].maxError = %g;", i, obj.maxError);
//      Con::errorf("   details[%d].polyCount = %d;", i, obj.polyCount);
//   }
//
//   Con::errorf("\ndefaultRotations.set(dMalloc(%d * sizeof(Quat16)), %d);", defaultRotations.size(), defaultRotations.size());
//   for (U32 i = 0; i < defaultRotations.size(); i++)
//   {
//      Con::errorf("   defaultRotations[%d].x = %g;", i, defaultRotations[i].x);
//      Con::errorf("   defaultRotations[%d].y = %g;", i, defaultRotations[i].y);
//      Con::errorf("   defaultRotations[%d].z = %g;", i, defaultRotations[i].z);
//      Con::errorf("   defaultRotations[%d].w = %g;", i, defaultRotations[i].w);
//   }
//
//   Con::errorf("\ndefaultTranslations.set(dMalloc(%d * sizeof(Point3F)), %d);", defaultTranslations.size(), defaultTranslations.size());
//   for (U32 i = 0; i < defaultTranslations.size(); i++)
//      Con::errorf("   defaultTranslations[%d].set(%g, %g, %g);", i, defaultTranslations[i].x, defaultTranslations[i].y, defaultTranslations[i].z);
//
//   Con::errorf("\nsubShapeFirstTranslucentObject.set(dMalloc(%d * sizeof(S32)), %d);", subShapeFirstTranslucentObject.size(), subShapeFirstTranslucentObject.size());
//   for (U32 i = 0; i < subShapeFirstTranslucentObject.size(); i++)
//      Con::errorf("   subShapeFirstTranslucentObject[%d] = %d;", i, subShapeFirstTranslucentObject[i]);
//
//   Con::errorf("\nmeshes.set(dMalloc(%d * sizeof(TSMesh)), %d);", meshes.size(), meshes.size());
//   for (U32 i = 0; i < meshes.size(); i++)
//   {
//      TSMesh* obj = meshes[i];
//
//      if (obj)
//      {
//         Con::errorf("   meshes[%d]->meshType = %d;", i, obj->meshType);
//         Con::errorf("   meshes[%d]->mBounds.minExtents.set(%g, %g, %g);", i, obj->mBounds.minExtents.x, obj->mBounds.minExtents.y, obj->mBounds.minExtents.z);
//         Con::errorf("   meshes[%d]->mBounds.maxExtents.set(%g, %g, %g);", i, obj->mBounds.maxExtents.x, obj->mBounds.maxExtents.y, obj->mBounds.maxExtents.z);
//         Con::errorf("   meshes[%d]->mCenter.set(%g, %g, %g);", i, obj->mCenter.x, obj->mCenter.y, obj->mCenter.z);
//         Con::errorf("   meshes[%d]->mRadius = %g;", i, obj->mRadius);
//         Con::errorf("   meshes[%d]->mVisibility = %g;", i, obj->mVisibility);
//         Con::errorf("   meshes[%d]->mDynamic = %d;", i, obj->mDynamic);
//         Con::errorf("   meshes[%d]->parentMesh = %d;", i, obj->parentMesh);
//         Con::errorf("   meshes[%d]->numFrames = %d;", i, obj->numFrames);
//         Con::errorf("   meshes[%d]->numMatFrames = %d;", i, obj->numMatFrames);
//         Con::errorf("   meshes[%d]->vertsPerFrame = %d;", i, obj->vertsPerFrame);
//
//         Con::errorf("\n   meshes[%d]->verts.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->verts.size(), obj->verts.size());
//         for (U32 j = 0; j < obj->verts.size(); j++)
//            Con::errorf("   meshes[%d]->verts[%d].set(%g, %g, %g);", i, j, obj->verts[j].x, obj->verts[j].y, obj->verts[j].z);
//
//         Con::errorf("\n   meshes[%d]->norms.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->norms.size(), obj->norms.size());
//         for (U32 j = 0; j < obj->norms.size(); j++)
//            Con::errorf("   meshes[%d]->norms[%d].set(%g, %g, %g);", i, j, obj->norms[j].x, obj->norms[j].y, obj->norms[j].z);
//
//         Con::errorf("\n   meshes[%d]->tverts.set(dMalloc(%d * sizeof(Point2F)), %d);", obj->tverts.size(), obj->tverts.size());
//         for (U32 j = 0; j < obj->tverts.size(); j++)
//            Con::errorf("   meshes[%d]->tverts[%d].set(%g, %g);", i, j, obj->tverts[j].x, obj->tverts[j].y);
//
//         Con::errorf("\n   meshes[%d]->primitives.set(dMalloc(%d * sizeof(TSDrawPrimitive)), %d);", obj->primitives.size(), obj->primitives.size());
//         for (U32 j = 0; j < obj->primitives.size(); j++)
//         {
//            TSDrawPrimitive& prim = obj->primitives[j];
//
//            Con::errorf("   meshes[%d]->primitives[%d].start = %d;", i, j, prim.start);
//            Con::errorf("   meshes[%d]->primitives[%d].numElements = %d;", i, j, prim.numElements);
//            Con::errorf("   meshes[%d]->primitives[%d].matIndex = %d;", i, j, prim.matIndex);
//         }
//
//         Con::errorf("\n   meshes[%d]->encodedNorms.set(dMalloc(%d * sizeof(U8)), %d);", obj->encodedNorms.size(), obj->encodedNorms.size());
//         for (U32 j = 0; j < obj->encodedNorms.size(); j++)
//            Con::errorf("   meshes[%d]->encodedNorms[%d] = %c;", i, j, obj->encodedNorms[j]);
//
//         Con::errorf("\n   meshes[%d]->indices.set(dMalloc(%d * sizeof(U16)), %d);", obj->indices.size(), obj->indices.size());
//         for (U32 j = 0; j < obj->indices.size(); j++)
//            Con::errorf("   meshes[%d]->indices[%d] = %d;", i, j, obj->indices[j]);
//
//         Con::errorf("\n   meshes[%d]->tangents.set(dMalloc(%d * sizeof(Point4F)), %d);", obj->tangents.size(), obj->tangents.size());
//         for (U32 j = 0; j < obj->tangents.size(); j++)
//            Con::errorf("   meshes[%d]->tangents[%d].set(%g, %g, %g, %g);", i, j, obj->tangents[j].x, obj->tangents[j].y, obj->tangents[j].z, obj->tangents[j].w);
//
//         Con::errorf("   meshes[%d]->billboardAxis.set(%g, %g, %g);", i, obj->billboardAxis.x, obj->billboardAxis.y, obj->billboardAxis.z);
//
//         Con::errorf("\n   meshes[%d]->planeNormals.set(dMalloc(%d * sizeof(Point3F)), %d);", obj->planeNormals.size(), obj->planeNormals.size());
//         for (U32 j = 0; j < obj->planeNormals.size(); j++)
//            Con::errorf("   meshes[%d]->planeNormals[%d].set(%g, %g, %g);", i, j, obj->planeNormals[j].x, obj->planeNormals[j].y, obj->planeNormals[j].z);
//
//         Con::errorf("\n   meshes[%d]->planeConstants.set(dMalloc(%d * sizeof(F32)), %d);", obj->planeConstants.size(), obj->planeConstants.size());
//         for (U32 j = 0; j < obj->planeConstants.size(); j++)
//            Con::errorf("   meshes[%d]->planeConstants[%d] = %g;", i, j, obj->planeConstants[j]);
//
//         Con::errorf("\n   meshes[%d]->planeMaterials.set(dMalloc(%d * sizeof(U32)), %d);", obj->planeMaterials.size(), obj->planeMaterials.size());
//         for (U32 j = 0; j < obj->planeMaterials.size(); j++)
//            Con::errorf("   meshes[%d]->planeMaterials[%d] = %d;", i, j, obj->planeMaterials[j]);
//
//         Con::errorf("   meshes[%d]->planesPerFrame = %d;", i, obj->planesPerFrame);
//         Con::errorf("   meshes[%d]->mergeBufferStart = %d;", i, obj->mergeBufferStart);
//      }
//   }
//
//   Con::errorf("\nalphaIn.set(dMalloc(%d * sizeof(F32)), %d);", alphaIn.size(), alphaIn.size());
//   for (U32 i = 0; i < alphaIn.size(); i++)
//      Con::errorf("   alphaIn[%d] = %g;", i, alphaIn[i]);
//
//   Con::errorf("\nalphaOut.set(dMalloc(%d * sizeof(F32)), %d);", alphaOut.size(), alphaOut.size());
//   for (U32 i = 0; i < alphaOut.size(); i++)
//      Con::errorf("   alphaOut[%d] = %g;", i, alphaOut[i]);
//
//   Con::errorf("numSequences = %d", sequences.size());
//   Con::errorf("numNodeRotations = %d", nodeRotations.size());
//   Con::errorf("numNodeTranslations = %d", nodeTranslations.size());
//   Con::errorf("numNodeUniformScales = %d", nodeUniformScales.size());
//   Con::errorf("numNodeAlignedScales = %d", nodeAlignedScales.size());
//   Con::errorf("numNodeArbitraryScaleRots = %d", nodeArbitraryScaleRots.size());
//   Con::errorf("numNodeArbitraryScaleFactors = %d", nodeArbitraryScaleFactors.size());
//   Con::errorf("numGroundRotations = %d", groundRotations.size());
//   Con::errorf("numGroundTranslations = %d", groundTranslations.size());
//   Con::errorf("numTriggers = %d", triggers.size());
//   Con::errorf("numBillboardDetails = %d", billboardDetails.size());

   //Con::errorf("\nnumDetailCollisionAccelerators = %d", detailCollisionAccelerators.size());
   //for (U32 i = 0; i < detailCollisionAccelerators.size(); i++)
   //{
   //   ConvexHullAccelerator* obj = detailCollisionAccelerators[i];

   //   if (obj)
   //   {
   //      Con::errorf("   detailCollisionAccelerators[%d].numVerts = %d", i, obj->numVerts);

   //      for (U32 j = 0; j < obj->numVerts; j++)
   //      {
   //         Con::errorf("      verts[%d](%g, %g, %g)", j, obj->vertexList[j].x, obj->vertexList[j].y, obj->vertexList[j].z);
   //         Con::errorf("      norms[%d](%g, %g, %g)", j, obj->normalList[j].x, obj->normalList[j].y, obj->normalList[j].z);
   //         //U8**     emitStrings;
   //      }
   //   }
   //}

//   Con::errorf("\nnames.setSize(%d);", names.size());
//   for (U32 i = 0; i < names.size(); i++)
//      Con::errorf("   names[%d] = StringTable->insert(\"%s\");", i, names[i].c_str());
//
//   TSMaterialList * materialList;
//
//   Con::errorf("\nradius = %g;", radius);
//   Con::errorf("tubeRadius = %g;", tubeRadius);
//   Con::errorf("center.set(%g, %g, %g);", center.x, center.y, center.z);
//   Con::errorf("bounds.minExtents.set(%g, %g, %g);", bounds.minExtents.x, bounds.minExtents.y, bounds.minExtents.z);
//   Con::errorf("bounds.maxExtents.set(%g, %g, %g);", bounds.maxExtents.x, bounds.maxExtents.y, bounds.maxExtents.z);
//
//   Con::errorf("\nmExporterVersion = %d;", mExporterVersion);
//   Con::errorf("mSmallestVisibleSize = %g;", mSmallestVisibleSize);
//   Con::errorf("mSmallestVisibleDL = %d;", mSmallestVisibleDL);
//   Con::errorf("mReadVersion = %d;", mReadVersion);
//   Con::errorf("mFlags = %d;", mFlags);
//   //Con::errorf("data = %d", data);
//   Con::errorf("mSequencesConstructed = %d;", mSequencesConstructed);
//}

   return true;
}

void TSShape::createEmptyShape()
{
    Node node;
    node.nameIndex = 1;
    node.parentIndex = -1;
    node.firstObject = 0;
    node.firstChild = -1;
    node.nextSibling = -1;
    nodes.push_back(node);

    Object object;
    object.nameIndex = 2;
    object.numMeshes = 1;
    object.startMeshIndex = 0;
    object.nodeIndex = 0;
    object.nextSibling = -1;
    object.firstDecal = -1;
    objects.push_back(object);

    ObjectState objectState;
    objectState.vis = 1;
    objectState.frameIndex = 0;
    objectState.matFrameIndex = 0;
    objectStates.push_back(objectState);

    subShapeFirstNode.push_back(0);
    subShapeFirstObject.push_back(0);

    detailFirstSkin.push_back(0);

   subShapeNumNodes.push_back(1);

   subShapeNumObjects.push_back(1);

    Detail detail;
      detail.nameIndex = 0;
      detail.subShapeNum = 0;
      detail.objectDetailNum = 0;
      detail.size = 2.0f;
      detail.averageError = -1.0f;
      detail.maxError = -1.0f;
      detail.polyCount = 0;
    details.push_back(detail);

    Quat16 defaultRotation;
      defaultRotation.x = 0.0f;
      defaultRotation.y = 0.0f;
      defaultRotation.z = 0.0f;
      defaultRotation.w = 0.0f;
    defaultRotations.push_back(defaultRotation);

    defaultTranslations.push_back(Point3F(0.0f, 0.0f, 0.0f));

    subShapeFirstTranslucentObject.push_back(1);

    alphaIn.push_back(0);

    alphaOut.push_back(-1);

//   sequences.set(nullptr, 0);
//   nodeRotations.set(nullptr, 0);
//   nodeTranslations.set(nullptr, 0);
//   nodeUniformScales.set(nullptr, 0);
//   nodeAlignedScales.set(nullptr, 0);
//   nodeArbitraryScaleRots.set(nullptr, 0);
//   nodeArbitraryScaleFactors.set(nullptr, 0);
//   groundRotations.set(nullptr, 0);
//   groundTranslations.set(nullptr, 0);
//   triggers.set(nullptr, 0);
//   billboardDetails.set(nullptr, 0);

   names.setSize(3);
      names[0] = StringTable->insert("Detail2");
      names[1] = StringTable->insert("Mesh2");
      names[2] = StringTable->insert("Mesh");

   radius = 0.866025f;
   tubeRadius = 0.707107f;
   center.set(0.0f, 0.5f, 0.0f);
   bounds.minExtents.set(-0.5f, 0.0f, -0.5f);
   bounds.maxExtents.set(0.5f, 1.0f, 0.5f);

   mExporterVersion = 124;
   mSmallestVisibleSize = 2;
   mSmallestVisibleDL = 0;
   mReadVersion = 24;
   mFlags = 0;
   mSequencesConstructed = 0;

   mUseDetailFromScreenError = false;

   mDetailLevelLookup.setSize( 1 );
   mDetailLevelLookup[0].set( -1, 0 );

   // Init the collision accelerator array.  Note that we don't compute the
   //  accelerators until the app requests them
   detailCollisionAccelerators.setSize(details.size());
   for (U32 i = 0; i < detailCollisionAccelerators.size(); i++)
      detailCollisionAccelerators[i] = nullptr;
}

void TSShape::fixEndian(S32 * buff32, S16 * buff16, S8 *, S32 count32, S32 count16, S32)
{
	// if endian-ness isn't the same, need to flip the buffer contents.
   if (0x12345678!=convertLEndianToHost(0x12345678))
   {
      for (S32 i=0; i<count32; i++)
         buff32[i]=convertLEndianToHost(buff32[i]);
      for (S32 i=0; i<count16*2; i++)
         buff16[i]=convertLEndianToHost(buff16[i]);
   }
}

//template<> ResourceBase::Signature  Resource<TSShape>::signature()
//{
//   return MakeFourCC('t','s','s','h');
//}
//
//TSShape::ConvexHullAccelerator* TSShape::getAccelerator(S32 dl)
//{
//   AssertFatal(dl < details.size(), "Error, bad detail level!");
//   if (dl == -1)
//      return nullptr;
//
//   AssertFatal( detailCollisionAccelerators.size() == details.size(), 
//      "TSShape::getAccelerator() - mismatched array sizes!" );
//
//   if (detailCollisionAccelerators[dl] == nullptr)
//      computeAccelerator(dl);
//
//   AssertFatal(detailCollisionAccelerators[dl] != nullptr, "This should be non-nullptr after computing it!");
//   return detailCollisionAccelerators[dl];
//}
//
//
//void TSShape::computeAccelerator(S32 dl)
//{
//   AssertFatal(dl < details.size(), "Error, bad detail level!");
//
//   // Have we already computed this?
//   if (detailCollisionAccelerators[dl] != nullptr)
//      return;
//
//   // Create a bogus features list...
//   ConvexFeature cf;
//   MatrixF mat(true);
//   Point3F n(0, 0, 1);
//
//   const TSDetail* detail = &details[dl];
//   S32 ss = detail->subShapeNum;
//   S32 od = detail->objectDetailNum;
//
//   S32 start = subShapeFirstObject[ss];
//   S32 end   = subShapeNumObjects[ss] + start;
//   if (start < end)
//   {
//      // run through objects and collide
//      // DMMNOTE: This assumes that the transform of the collision hulls is
//      //  identity...
//      U32 surfaceKey = 0;
//      for (S32 i = start; i < end; i++)
//      {
//         const TSObject* obj = &objects[i];
//
//         if (obj->numMeshes && od < obj->numMeshes) {
//            TSMesh* mesh = meshes[obj->startMeshIndex + od];
//            if (mesh)
//               mesh->getFeatures(0, mat, n, &cf, surfaceKey);
//         }
//      }
//   }
//
//   Vector<Point3F> fixedVerts;
//   VECTOR_SET_ASSOCIATION(fixedVerts);
//   S32 i;
//   for (i = 0; i < cf.mVertexList.size(); i++) {
//      S32 j;
//      bool found = false;
//      for (j = 0; j < cf.mFaceList.size(); j++) {
//         if (cf.mFaceList[j].vertex[0] == i ||
//             cf.mFaceList[j].vertex[1] == i ||
//             cf.mFaceList[j].vertex[2] == i) {
//            found = true;
//            break;
//         }
//      }
//      if (!found)
//         continue;
//
//      found = false;
//      for (j = 0; j < fixedVerts.size(); j++) {
//         if (fixedVerts[j] == cf.mVertexList[i]) {
//            found = true;
//            break;
//         }
//      }
//      if (found == true) {
//         // Ok, need to replace any references to vertex i in the facelists with
//         //  a reference to vertex j in the fixed list
//         for (S32 k = 0; k < cf.mFaceList.size(); k++) {
//            for (S32 l = 0; l < 3; l++) {
//               if (cf.mFaceList[k].vertex[l] == i)
//                  cf.mFaceList[k].vertex[l] = j;
//            }
//         }
//      } else {
//         for (S32 k = 0; k < cf.mFaceList.size(); k++) {
//            for (S32 l = 0; l < 3; l++) {
//               if (cf.mFaceList[k].vertex[l] == i)
//                  cf.mFaceList[k].vertex[l] = fixedVerts.size();
//            }
//         }
//         fixedVerts.push_back(cf.mVertexList[i]);
//      }
//   }
//   cf.mVertexList.setSize(0);
//   cf.mVertexList = fixedVerts;
//
//   // Ok, so now we have a vertex list.  Lets copy that out...
//   ConvexHullAccelerator* accel = new ConvexHullAccelerator;
//   detailCollisionAccelerators[dl] = accel;
//   accel->numVerts    = cf.mVertexList.size();
//   accel->vertexList  = new Point3F[accel->numVerts];
//   dMemcpy(accel->vertexList, cf.mVertexList.address(), sizeof(Point3F) * accel->numVerts);
//
//   accel->normalList = new Point3F[cf.mFaceList.size()];
//   for (i = 0; i < cf.mFaceList.size(); i++)
//      accel->normalList[i] = cf.mFaceList[i].normal;
//
//   accel->emitStrings = new U8*[accel->numVerts];
//   dMemset(accel->emitStrings, 0, sizeof(U8*) * accel->numVerts);
//
//   for (i = 0; i < accel->numVerts; i++) {
//      S32 j;
//
//      Vector<U32> faces;
//      VECTOR_SET_ASSOCIATION(faces);
//      for (j = 0; j < cf.mFaceList.size(); j++) {
//         if (cf.mFaceList[j].vertex[0] == i ||
//             cf.mFaceList[j].vertex[1] == i ||
//             cf.mFaceList[j].vertex[2] == i) {
//            faces.push_back(j);
//         }
//      }
//      AssertFatal(faces.size() != 0, "Huh?  Vertex unreferenced by any faces");
//
//      // Insert all faces that didn't make the first cut, but share a plane with
//      //  a face that's on the short list.
//      for (j = 0; j < cf.mFaceList.size(); j++) {
//         bool found = false;
//         S32 k;
//         for (k = 0; k < faces.size(); k++) {
//            if (faces[k] == j)
//               found = true;
//         }
//         if (found)
//            continue;
//
//         found = false;
//         for (k = 0; k < faces.size(); k++) {
//            if (mDot(accel->normalList[faces[k]], accel->normalList[j]) > 0.999) {
//               found = true;
//               break;
//            }
//         }
//         if (found)
//            faces.push_back(j);
//      }
//
//      Vector<U32> vertRemaps;
//      VECTOR_SET_ASSOCIATION(vertRemaps);
//      for (j = 0; j < faces.size(); j++) {
//         for (U32 k = 0; k < 3; k++) {
//            U32 insert = cf.mFaceList[faces[j]].vertex[k];
//            bool found = false;
//            for (S32 l = 0; l < vertRemaps.size(); l++) {
//               if (insert == vertRemaps[l]) {
//                  found = true;
//                  break;
//               }
//            }
//            if (!found)
//               vertRemaps.push_back(insert);
//         }
//      }
//
//      Vector<Point2I> edges;
//      VECTOR_SET_ASSOCIATION(edges);
//      for (j = 0; j < faces.size(); j++) {
//         for (U32 k = 0; k < 3; k++) {
//            U32 edgeStart = cf.mFaceList[faces[j]].vertex[(k + 0) % 3];
//            U32 edgeEnd   = cf.mFaceList[faces[j]].vertex[(k + 1) % 3];
//
//            U32 e0 = getMin(edgeStart, edgeEnd);
//            U32 e1 = getMax(edgeStart, edgeEnd);
//
//            bool found = false;
//            for (S32 l = 0; l < edges.size(); l++) {
//               if (edges[l].x == e0 && edges[l].y == e1) {
//                  found = true;
//                  break;
//               }
//            }
//            if (!found)
//               edges.push_back(Point2I(e0, e1));
//         }
//      }
//
//      //AssertFatal(vertRemaps.size() < 256 && faces.size() < 256 && edges.size() < 256,
//      //            "Error, ran over the shapebase assumptions about convex hulls.");
//
//      U32 emitStringLen = 1 + vertRemaps.size()  +
//                          1 + (edges.size() * 2) +
//                          1 + (faces.size() * 4);
//      accel->emitStrings[i] = new U8[emitStringLen];
//
//      U32 currPos = 0;
//
//      accel->emitStrings[i][currPos++] = vertRemaps.size();
//      for (j = 0; j < vertRemaps.size(); j++)
//         accel->emitStrings[i][currPos++] = vertRemaps[j];
//
//      accel->emitStrings[i][currPos++] = edges.size();
//      for (j = 0; j < edges.size(); j++) {
//         S32 l;
//         U32 old = edges[j].x;
//         bool found = false;
//         for (l = 0; l < vertRemaps.size(); l++) {
//            if (vertRemaps[l] == old) {
//               found = true;
//               accel->emitStrings[i][currPos++] = l;
//               break;
//            }
//         }
//         AssertFatal(found, "Error, couldn't find the remap!");
//
//         old = edges[j].y;
//         found = false;
//         for (l = 0; l < vertRemaps.size(); l++) {
//            if (vertRemaps[l] == old) {
//               found = true;
//               accel->emitStrings[i][currPos++] = l;
//               break;
//            }
//         }
//         AssertFatal(found, "Error, couldn't find the remap!");
//      }
//
//      accel->emitStrings[i][currPos++] = faces.size();
//      for (j = 0; j < faces.size(); j++) {
//         accel->emitStrings[i][currPos++] = faces[j];
//         for (U32 k = 0; k < 3; k++) {
//            U32 old = cf.mFaceList[faces[j]].vertex[k];
//            bool found = false;
//            for (S32 l = 0; l < vertRemaps.size(); l++) {
//               if (vertRemaps[l] == old) {
//                  found = true;
//                  accel->emitStrings[i][currPos++] = l;
//                  break;
//               }
//            }
//            AssertFatal(found, "Error, couldn't find the remap!");
//         }
//      }
//      AssertFatal(currPos == emitStringLen, "Error, over/underflowed the emission string!");
//   }
//}

//-------------------------------------------------
// read/write sequence
//-------------------------------------------------
void TSShape::Sequence::read(std::iostream& s, bool readNameIndex)
{
    AssertISV(smReadVersion>=19,"Reading old sequence");
    
    if (readNameIndex)
        s >> nameIndex;
    flags = 0;
    if (TSShape::smReadVersion>21)
        s >> flags;
    else
        flags=0;
    
    s >> numKeyframes;
    s >> duration;
    
    if (TSShape::smReadVersion<22)
    {
        bool tmp = false;
        s >> tmp;
        if (tmp)
            flags |= Blend;
        s >> tmp;
        if (tmp)
            flags |= Cyclic;
        s >> tmp;
        if (tmp)
            flags |= MakePath;
    }
    
    s >> priority;
    s >> firstGroundFrame;
    s >> numGroundFrames;
    if (TSShape::smReadVersion>21)
    {
        s >> baseRotation;
        s >> baseTranslation;
        s >> baseScale;
        s >> baseObjectState;
        s >> baseDecalState;
    }
    else
    {
        s >> baseRotation;
        baseTranslation=baseRotation;
        s >> baseObjectState;
        s >> baseDecalState;
    }
    
    s >> firstTrigger;
    s >> numTriggers;
    s >> toolBegin;
    
    // now the membership sets:
    rotationMatters.read(s);
    if (TSShape::smReadVersion<22)
        translationMatters=rotationMatters;
    else
    {
        translationMatters.read(s);
        scaleMatters.read(s);
    }
    
    TSIntegerSet dummy;
    dummy.read(s); // DEPRECIATED: Decals
    dummy.read(s); // DEPRECIATED: Ifl materials
    
    visMatters.read(s);
    frameMatters.read(s);
    matFrameMatters.read(s);
    
    dirtyFlags = 0;
    if (rotationMatters.testAll() || translationMatters.testAll() || scaleMatters.testAll())
        dirtyFlags |= TSShapeInstance::TransformDirty;
    if (visMatters.testAll())
        dirtyFlags |= TSShapeInstance::VisDirty;
    if (frameMatters.testAll())
        dirtyFlags |= TSShapeInstance::FrameDirty;
    if (matFrameMatters.testAll())
        dirtyFlags |= TSShapeInstance::MatFrameDirty;
}

void TSShape::Sequence::write(std::iostream& stream, bool writeNameIndex) const
{
    if (writeNameIndex)
        stream << nameIndex;
    stream << flags;
    stream << numKeyframes;
    stream << duration;
    stream << priority;
    stream << firstGroundFrame;
    stream << numGroundFrames;
    stream << baseRotation;
    stream << baseTranslation;
    stream << baseScale;
    stream << baseObjectState;
    stream << baseDecalState;
    stream << firstTrigger;
    stream << numTriggers;
    stream << toolBegin;
    
    // now the membership sets:
    rotationMatters.write(stream);
    translationMatters.write(stream);
    scaleMatters.write(stream);
    
    TSIntegerSet dummy;
    dummy.write(stream); // DEPRECIATED: Decals
    dummy.write(stream); // DEPRECIATED: Ifl materials

    visMatters.write(stream);
    frameMatters.write(stream);
    matFrameMatters.write(stream);
}
