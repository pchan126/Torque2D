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

#include "./sphereMesh.h"

SphereMesh::SphereMesh(U32 baseType)
{
   VECTOR_SET_ASSOCIATION(mDetails);

   switch(baseType)
   {
      case Tetrahedron:
         mDetails.push_back(createTetrahedron());
         break;

      case Octahedron:
         mDetails.push_back(createOctahedron());
         break;

      case Icosahedron:
         mDetails.push_back(createIcosahedron());
         break;
   }
   calcNormals(mDetails[0]);
}

//------------------------------------------------------------------------------

SphereMesh::TriangleMesh * SphereMesh::createTetrahedron()
{
   const F32 sqrt3 = 0.5773502692f;

   static bool init = false;
   static TriangleMesh tetrahedronMesh;

   if (!init)
   {
       tetrahedronMesh.mVerts.push_back(Point3F( sqrt3, sqrt3, sqrt3 ));
       tetrahedronMesh.mVerts.push_back(Point3F(-sqrt3,-sqrt3, sqrt3 ));
       tetrahedronMesh.mVerts.push_back(Point3F(-sqrt3, sqrt3,-sqrt3 ));
       tetrahedronMesh.mVerts.push_back(Point3F( sqrt3,-sqrt3,-sqrt3 ));

       tetrahedronMesh.mTriangleIndex.push_back(TriangleIndices(0, 1, 2));
       tetrahedronMesh.mTriangleIndex.push_back(TriangleIndices(0, 3, 1));
       tetrahedronMesh.mTriangleIndex.push_back(TriangleIndices(2, 1, 3));
       tetrahedronMesh.mTriangleIndex.push_back(TriangleIndices(3, 0, 2));
       init = true;
   };

   return(&tetrahedronMesh);
}

//------------------------------------------------------------------------------

SphereMesh::TriangleMesh * SphereMesh::createOctahedron()
{
    static bool init = false;
    static TriangleMesh octahedronMesh;
   //
    if (!init)
    {
        octahedronMesh.mVerts.push_back(Point3F( 1, 0, 0 ));
        octahedronMesh.mVerts.push_back(Point3F( -1, 0, 0 ));
        octahedronMesh.mVerts.push_back(Point3F( 0, 1, 0 ));
        octahedronMesh.mVerts.push_back(Point3F( 0, -1, 0 ));
        octahedronMesh.mVerts.push_back(Point3F( 0, 0, 1 ));
        octahedronMesh.mVerts.push_back(Point3F( 0, 0, -1 ));

        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(0, 4, 2));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(2, 4, 1));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(1, 4, 3));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(3, 4, 0));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(0, 2, 5));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(2, 1, 5));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(1, 3, 5));
        octahedronMesh.mTriangleIndex.push_back(TriangleIndices(3, 0, 5));
        init = true;
    };

   return(&octahedronMesh);
}

SphereMesh::TriangleMesh * SphereMesh::createIcosahedron()
{
   const F32 tau = 0.8506508084f;
   const F32 one = 0.5257311121f;

    static bool init = false;
    static TriangleMesh icosahedronMesh;

    if (!init)
    {
        icosahedronMesh.mVerts.push_back(Point3F(  tau,  one,    0 ));
        icosahedronMesh.mVerts.push_back(Point3F( -tau,  one,    0 ));
        icosahedronMesh.mVerts.push_back(Point3F( -tau, -one,    0 ));
        icosahedronMesh.mVerts.push_back(Point3F(  tau, -one,    0 ));
        icosahedronMesh.mVerts.push_back(Point3F(  one,    0,  tau ));
        icosahedronMesh.mVerts.push_back(Point3F(  one,    0, -tau ));
        icosahedronMesh.mVerts.push_back(Point3F( -one,    0, -tau ));
        icosahedronMesh.mVerts.push_back(Point3F(    0,  tau,  one ));
        icosahedronMesh.mVerts.push_back(Point3F(    0, -tau,  one ));
        icosahedronMesh.mVerts.push_back(Point3F(    0, -tau, -one ));
        icosahedronMesh.mVerts.push_back(Point3F(    0,  tau, -one ));

        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 4, 8, 7 ));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 4, 7, 9 ));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 5, 6, 11 ));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 5, 10, 6));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 0, 4, 3));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 0, 3, 5));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 2, 7, 1));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 2, 1, 6));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 8, 0, 11));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 8, 11, 1 ));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 9, 10, 3));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 9, 2, 10));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 8, 4, 0));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 11, 0, 5));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 4, 9, 3));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 5, 3, 10));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 7, 8, 1));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 6, 1, 11));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 7, 2, 9));
        icosahedronMesh.mTriangleIndex.push_back(TriangleIndices( 6, 10, 2));
        init = true;
    };

   return(&icosahedronMesh);
}

//------------------------------------------------------------------------------

void SphereMesh::calcNormals(TriangleMesh * mesh)
{
   mesh->mNormals.clear();
   for(U32 i = 0; i < mesh->mTriangleIndex.size(); i++)
   {
      Point3F normal;
      TriangleIndices & tri = mesh->mTriangleIndex[i];
      mCross(mesh->mVerts[tri.index[1]] - mesh->mVerts[tri.index[0]], mesh->mVerts[tri.index[2]] - mesh->mVerts[tri.index[0]], &normal);
      mesh->mNormals.push_back(normal);
   }
}

//------------------------------------------------------------------------------

SphereMesh::~SphereMesh()
{
}

//------------------------------------------------------------------------------

const SphereMesh::TriangleMesh * SphereMesh::getMesh(U32 level)
{
   AssertFatal(mDetails.size(), "SphereMesh::getMesh: no details!");

   if(level > MaxLevel)
      level = MaxLevel;

   //
   while(mDetails.size() <= level)
      mDetails.push_back(subdivideMesh(mDetails.back()));

   return(mDetails[level]);
}

SphereMesh::TriangleMesh * SphereMesh::subdivideMesh(TriangleMesh * prevMesh)
{
   AssertFatal(prevMesh, "SphereMesh::subdivideMesh: invalid previous mesh level!");
   TriangleMesh * mesh = new TriangleMesh;
   mesh->mVerts = prevMesh->mVerts;

   for (auto itr:prevMesh->mTriangleIndex )
   {
       Point3F a = (prevMesh->mVerts[itr.index[0]] + prevMesh->mVerts[itr.index[2]]) / 2;
       Point3F b = (prevMesh->mVerts[itr.index[0]] + prevMesh->mVerts[itr.index[1]]) / 2;
       Point3F c = (prevMesh->mVerts[itr.index[1]] + prevMesh->mVerts[itr.index[2]]) / 2;

       // force the point onto the unit sphere surface
       a.normalize();
       b.normalize();
       c.normalize();

       U32 index1 = mesh->mVerts.push_back_unique(a);
       U32 index2 = mesh->mVerts.push_back_unique(b);
       U32 index3 = mesh->mVerts.push_back_unique(c);

       //
       mesh->mTriangleIndex.push_back(TriangleIndices(itr.index[0], index2, index1));
       mesh->mTriangleIndex.push_back(TriangleIndices(index2, itr.index[1], index3));
       mesh->mTriangleIndex.push_back(TriangleIndices(index1, index2, index3));
       mesh->mTriangleIndex.push_back(TriangleIndices(index1, index3, itr.index[2]));
   }
   calcNormals(mesh);
   return(mesh);
}
