//
//  mDualQuat.h
//  Torque2D
//
//  Created by Paul L Jan on 2013-04-02.
//

#ifndef Torque2D_mDualQuat_h
#define Torque2D_mDualQuat_h

#include "math/mQuat.h"

class DualQuatF
{
public:
    QuatF m_real;
    QuatF m_dual;
    
    DualQuatF()
    {
        m_real = QuatF(0.0f, 0.0f, 0.0f, 1.0f);
        m_dual = QuatF(0.0f, 0.0f, 0.0f, 0.0f);
    }
    DualQuatF( QuatF r, QuatF d)
    {
        m_real = r.normalize();
        m_dual = d;
    }
    DualQuatF( QuatF rotation, Point3F translation) 
    {
        m_real = rotation.normalize();
        m_dual = (QuatF(translation, 0) * m_real) * 0.5f;
    }
    static float dot( DualQuatF a, DualQuatF b)
    {
        return QuatF::dot( a.m_real, b.m_real);
    }
    DualQuatF operator*= ( float scale)
    {
        m_real *= scale;
        m_dual *= scale;
        return *this;
    }
    inline DualQuatF& normalize()
    {
        float mag = QuatF::dot( m_real, m_real);
        AssertFatal( mag > POINT_EPSILON, "");
        m_real *= 1.0f/ mag;
        m_dual *= 1.0f/ mag;
        return *this;
    }
    DualQuatF& operator+= ( const DualQuatF& rhs)
    {
        m_real += rhs.m_real;
        m_dual += rhs.m_dual;
        return *this;
    }
    DualQuatF& operator*= ( const DualQuatF& rhs)
    {
        m_real = rhs.m_real * m_real;
        m_dual = rhs.m_dual * m_real + rhs.m_real*m_dual;
        return *this;
    }
    
    inline QuatF getRotation()
    {
        return m_real;
    }
    
    inline Point3F getTranslation()
    {
        QuatF t = (m_dual * 2.0f) * QuatF::conjugate( m_real );
        return Point3F( t.x, t.y, t.z );
    }

    static DualQuatF conjugate(const DualQuatF & q);
    static QuatF getRotation(const DualQuatF & q);
    static Point3F getTranslation(const DualQuatF & q);
    
    MatrixF toMatrix();
};

inline DualQuatF operator*( DualQuatF lhs, float scale)
{
    lhs *= scale;
    return lhs;
}

inline DualQuatF operator+( DualQuatF lhs, const DualQuatF & rhs)
{
    lhs += rhs;
    return lhs;
}

inline DualQuatF operator*( DualQuatF lhs, const DualQuatF & rhs)
{
    lhs *= rhs;
    return lhs;
}

inline DualQuatF DualQuatF::conjugate(const DualQuatF & q) 
{
    return DualQuatF( QuatF::conjugate(q.m_real), QuatF::conjugate(q.m_dual) );
}

inline QuatF DualQuatF::getRotation( const DualQuatF & q) 
{
    return q.m_real;
}

inline Point3F DualQuatF::getTranslation( const DualQuatF & q) 
{
    QuatF t = (q.m_dual * 2.0f) * QuatF::conjugate( q.m_real );
    return Point3F( t.x, t.y, t.z );
}

inline MatrixF DualQuatF::toMatrix()
{
    normalize();
    
    MatrixF m = MatrixF(true);
    F32 w = m_real.w;
    F32 x = m_real.x;
    F32 y = m_real.y;
    F32 z = m_real.z;
    
    // rotational information
    m[0] = w*w + x*x - y*y - z*z;
    m[4] = 2*x*y + 2*w*z;
    m[9] = 2*x*z - 2*w*y;

    m[1] = 2*x*y - 2*w*z;
    m[5] = w*w + y*y - x*x - z*z;
    m[10] = 2*y*z + 2*w*z;
    
    m[2] = 2*x*z + 2*w*y;
    m[6] = 2*y*z - 2*w*x;
    m[11] = w*w + z*z - x*x - y*y;
    
    // translation information
    QuatF t = (m_dual * 2.0f) * QuatF::conjugate(m_real);
    m[3] = t.x;
    m[7] = t.y;
    m[12] = t.z;
   return m;
}

#endif
