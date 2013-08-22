//
// Created by Paul L Jan on 2013-08-22.
//



#ifndef __tsMesh_H_
#define __tsMesh_H_

#include "collection/vector.h"
#include "graphics/gfxVertexBuffer.h"
#include "graphics/gfxDevice.h"

struct TSDrawPrimitive
{
    enum
    {
        Triangles    = 0 << 30, ///< bits 30 and 31 index element type
        Strip        = 1 << 30, ///< bits 30 and 31 index element type
        Fan          = 2 << 30, ///< bits 30 and 31 index element type
        Indexed      = BIT(29), ///< use glDrawElements if indexed, glDrawArrays o.w.
        NoMaterial   = BIT(28), ///< set if no material (i.e., texture missing)
        MaterialMask = ~(Strip|Fan|Triangles|Indexed|NoMaterial),
        TypeMask     = Strip|Fan|Triangles
    };

    S32 start;
    S32 numElements;
    S32 matIndex;    ///< holds material index & element type (see above enum)
};

typedef GFXVertexBufferDataHandle TSVertexBufferHandle;

class TSMesh {

public:
    TSVertexBufferHandle mVB;


    /// @name Vertex data
    /// @{

    template<class T>
    class FreeableVector : public Vector<T>
    {
    public:
        void free_memory() { Vector<T> temp; Vector<T>::swap(temp); }

        FreeableVector<T>& operator=(const Vector<T>& p) { Vector<T>::operator=(p); return *this; }
        FreeableVector<T>& operator=(const FreeableVector<T>& p) { Vector<T>::operator=(p); return *this; }
    };

    FreeableVector<Point3F> verts;
    FreeableVector<Point3F> norms;
    FreeableVector<Point2F> tverts;
    FreeableVector<Point4F> tangents;

    // Optional second texture uvs.
    FreeableVector<Point2F> tverts2;

    // Optional vertex colors data.
    FreeableVector<ColorI> colors;
    /// @}

    Vector<TSDrawPrimitive> primitives;
    Vector<U8> encodedNorms;
    Vector<U32> indices;

    /// @name Render Methods
    /// @{
    void innerRender( TSVertexBufferHandle &vb);
    /// @}

    /// persist methods...
    virtual void assemble( bool skip );
    static TSMesh* assembleMesh( U32 meshType, bool skip );
    virtual void disassemble();

    void createVBIB();
    void createTangents(const Vector<Point3F> &_verts, const Vector<Point3F> &_norms);
    void findTangent( U32 index1,
            U32 index2,
            U32 index3,
            Point3F *tan0,
            Point3F *tan1,
            const Vector<Point3F> &_verts);
};


#endif //__tsMesh_H_
