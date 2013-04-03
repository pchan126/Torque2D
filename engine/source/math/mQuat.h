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

#ifndef _MQUAT_H_
#define _MQUAT_H_

#ifndef _MMATH_H_
#include "math/mMath.h"
#endif

#ifdef TORQUE_OS_IOS
#import <GLKit/GLKMath.h>
#endif


class MatrixF;
class QuatF;

inline F32 QuatIsEqual(F32 a,F32 b,F32 epsilon = POINT_EPSILON)
{
   return mFabs(a-b) < epsilon;
}

inline F32 QuatIsZero(F32 a,F32 epsilon = POINT_EPSILON)
{
   return mFabs(a) < epsilon;
}

//----------------------------------------------------------------------------
// unit quaternion class:

class QuatF
{
  public:
    union
    {
        F32 q[4];
        F32  x,y,z,w;
#ifdef __GLK_MATH_TYPES_H
        GLKQuaternion mGQ;
#endif
    } ;

   QuatF();
   QuatF( F32 _x, F32 _y, F32 _z, F32 w );
   QuatF( const MatrixF & m );
   QuatF( const Point3F & _axis, F32 _angle ); // axisVector, radian
   QuatF( const EulerF & e );
#ifdef __GLK_QUATERNION_H
   QuatF(const GLKQuaternion & q);
#endif

   QuatF& set( F32 _x, F32 _y, F32 _z, F32 _w );
   QuatF& set( const MatrixF & m );
   QuatF& set( const Point3F & _axis, F32 _angle );
   QuatF& set( const EulerF & e );
#ifdef __GLK_QUATERNION_H
    QuatF& set(const GLKQuaternion & q);
#endif
    

   int operator ==( const QuatF & c ) const;
   int operator !=( const QuatF & c ) const;
    
   QuatF& operator *=( const QuatF & b )
   {
        QuatF prod;
#ifdef __GLK_QUATERNION_H
        prod.mGQ = GLKQuaternionMultiply(mGQ, b.mGQ);
#else
        prod.w = w * b.w - x * b.x - y * b.y - z * b.z;
        prod.x = w * b.x + x * b.w + y * b.z - z * b.y;
        prod.y = w * b.y + y * b.w + z * b.x - x * b.z;
        prod.z = w * b.z + z * b.w + x * b.y - y * b.x;
#endif
        *this = prod;
        return (*this);
   }
    
    QuatF& operator /=( const QuatF & c )
    {
        QuatF temp = c;
        return ( (*this) *= temp.inverse() );
    }
    
    QuatF& operator +=( const QuatF & c )
    {
#ifdef __GLK_QUATERNION_H
        mGQ = GLKQuaternionAdd(mGQ, c.mGQ);
#else
        x += c.x;
        y += c.y;
        z += c.z;
        w += c.w;
#endif
        return *this;
    }

    QuatF& operator -=( const QuatF & c )
    {
#ifdef __GLK_QUATERNION_H
        mGQ = GLKQuaternionSubtract(mGQ, c.mGQ);
#else
        x -= c.x;
        y -= c.y;
        z -= c.z;
        w -= c.w;
#endif
        return *this;
    }
    
    QuatF& operator *=( F32 a )
    {
        x *= a;
        y *= a;
        z *= a;
        w *= a;
        return *this;
    }
    
    QuatF& operator /=( F32 a )
    {
        x /= a;
        y /= a;
        z /= a;
        w /= a;
        return *this;
    }
    
   QuatF& square()
   {
        F32 t = w*2.0f;
        w = (w*w) - (x*x + y*y + z*z);
        x *= t;
        y *= t;
        z *= t;
        return *this;
   }
    
    QuatF& neg();

   static F32 dot( const QuatF a, const QuatF b)
    {
        return (a.w*b.w + a.x*b.x + a.y*b.y + a.z*b.z);
    };

   MatrixF* setMatrix( MatrixF * mat ) const;

    QuatF& normalize()
    {
#ifdef __GLK_QUATERNION_H
        mGQ = GLKQuaternionNormalize(mGQ);
#else
        F32 l = mSqrt( x*x + y*y + z*z + w*w );
        if( l == F32(0.0) )
            identity();
        else
        {
            x /= l;
            y /= l;
            z /= l;
            w /= l;
        }
#endif
        return *this;
    }
    
   QuatF& inverse()
   {
#ifdef __GLK_QUATERNION_H
        mGQ = GLKQuaternionInvert(mGQ);
#else
        F32 invMagnitude = 1.0f/(w*w + x*x + y*y + z*z);
        w = w * invMagnitude;
        x = -x * invMagnitude;
        y = -y * invMagnitude;
        z = -z * invMagnitude;
#endif
        return *this;
    }
    
   QuatF&       identity();
   int          isIdentity() const;
   QuatF&       slerp( const QuatF & q, F32 t );
   QuatF&       extrapolate( const QuatF & q1, const QuatF & q2, F32 t );
   QuatF&       interpolate( const QuatF & q1, const QuatF & q2, F32 t );
   F32          getAngle() const;
   Point3F      getAxis() const;
   MatrixF      getMatrix() const;

   Point3F&     mulP(const Point3F& a, Point3F* b);   // r = p * this
    
    static QuatF conjugate( const QuatF& q )
    {
        QuatF ret;
        ret.x = -q.x;
        ret.y = -q.y;
        ret.z = -q.z;
        ret.w = q.w;
        return ret;
    };
};


//----------------------------------------------------------------------------
// quaternion implementation:

inline QuatF::QuatF()
{
}

inline QuatF::QuatF( F32 _x, F32 _y, F32 _z, F32 _w )
{
   set( _x, _y, _z, _w );
}

inline QuatF::QuatF( const Point3F & _axis, F32 _angle )
{
   set( _axis, _angle );
}

inline QuatF::QuatF( const EulerF & e )
{
   set(e);
}

#ifdef __GLK_QUATERNION_H
inline QuatF::QuatF( const GLKQuaternion & q )
{
    set(q);
}
#endif

inline QuatF& QuatF::set( F32 _x, F32 _y, F32 _z, F32 _w )
{
   x = _x;
   y = _y;
   z = _z;
   w = _w;
   return *this;
}

#ifdef __GLK_QUATERNION_H
inline QuatF& QuatF::set( const GLKQuaternion & q )
{
    mGQ = q;
    return *this;
}
#endif

inline int QuatF::operator ==( const QuatF & c ) const
{
   QuatF a = *this;
   QuatF b = c;
   a.normalize();
   b.normalize();
   b.inverse();
   a *= b;
   return a.isIdentity();
}

inline int QuatF::isIdentity() const
{
   return QuatIsZero( x ) && QuatIsZero( y ) && QuatIsZero( z );
}

inline QuatF& QuatF::identity()
{
   x = 0.0f;
   y = 0.0f;
   z = 0.0f;
   w = 1.0f;
   return *this;
}

inline int QuatF::operator !=( const QuatF & c ) const
{
   return ! operator==( c );
}

inline QuatF::QuatF( const MatrixF & m )
{
   set( m );
}


inline QuatF& QuatF::neg()
{
   x = -x;
   y = -y;
   z = -z;
   w = -w;
   return *this;
}

inline Point3F QuatF::getAxis() const
{
    F32 sinHalfAngle = mSqrt(1 - w * w);
    if (sinHalfAngle != 0)
        return Point3F( x / sinHalfAngle, y / sinHalfAngle, z / sinHalfAngle );
    else
        return Point3F(1,0,0);
}

inline F32 QuatF::getAngle() const
{
    return mAcos( w ) * 2;
}

inline QuatF operator+( QuatF a, const QuatF &b)
{
    a += b;
    return a;
}

inline QuatF operator-( QuatF a, const QuatF &b)
{
    a -= b;
    return a;
}

inline QuatF operator*(QuatF a, const QuatF &b)
{
    a *= b;
    return a;
}

inline QuatF operator/(QuatF a, const QuatF &b)
{
    a /= b;
    return a;
}

inline QuatF operator*( QuatF a, F32 b)
{
    a *= b;
    return a;
}

inline QuatF operator/( QuatF a, F32 b)
{
    a /= b;
    return a;
}

#endif
