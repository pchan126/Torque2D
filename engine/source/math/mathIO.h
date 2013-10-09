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

#ifndef _MATHIO_H_
#define _MATHIO_H_

//Includes
#include "platform/platform.h"
#include "math/mMath.h"

//------------------------------------------------------------------------------
//-------------------------------------- READING
//
inline bool mathRead(std::istream &stream, Point2I *p)
{
   stream >> (p->x);
   stream >> (p->y);
   return stream.good();
}

inline bool mathRead(std::istream &stream, Point3I *p)
{
   stream >> (p->x);
   stream >> (p->y);
   stream >> (p->z);
   return stream.good();
}

inline bool mathRead(std::istream &stream, Point2F *p)
{
   stream >> (p->x);
   stream >> (p->y);
   return stream.good();
}

inline bool mathRead(std::istream &stream, Point3F *p)
{
   stream >> (p->x);
   stream >> (p->y);
   stream >> (p->z);
   return stream.good();
}

inline bool mathRead(std::istream &stream, Point4F *p)
{
   stream >> (p->x);
   stream >> (p->y);
   stream >> (p->z);
   stream >> (p->w);
   return stream.good();
}

inline bool mathRead(std::istream &stream, Point3D *p)
{
   stream >> (p->x);
   stream >> (p->y);
   stream >> (p->z);
   return stream.good();
}

inline bool mathRead(std::istream &stream, PlaneF *p)
{
   stream >> (p->x);
   stream >> (p->y);
   stream >> (p->z);
   stream >> (p->d);
   return stream.good();
}

inline bool mathRead(std::istream &stream, Box3F *b)
{
   bool success = mathRead(stream, &b->minExtents);
   success     &= mathRead(stream, &b->maxExtents);
   return success;
}

inline bool mathRead(std::istream &stream, SphereF *s)
{
   mathRead(stream, &s->center);
   stream >> s->radius;
   return stream.good();
}

inline bool mathRead(std::istream &stream, RectI *r)
{
   bool success = mathRead(stream, &r->point);
   success     &= mathRead(stream, &r->extent);
   return success;
}

inline bool mathRead(std::istream &stream, RectF *r)
{
   bool success = mathRead(stream, &r->point);
   success     &= mathRead(stream, &r->extent);
   return success;
}

inline bool mathRead(std::istream &stream, MatrixF *m)
{
   bool success = true;
   F32* pm    = *m;
   stream.read( pm, 16*sizeof(U32));
   return success;
}

inline bool mathRead(std::istream &stream, QuatF *q)
{
   stream >> (q->x);
   stream >> (q->y);
   stream >> (q->z);
   stream >> (q->w);
   return stream.good();
}

//------------------------------------------------------------------------------
//-------------------------------------- WRITING
//
inline bool mathWrite(std::ostream &stream, const Point2I& p)
{
   stream << (p.x);
   stream << (p.y);
   return stream.good();
}

inline bool mathWrite(std::ostream& stream, const Point3I& p)
{
   stream << (p.x);
   stream << (p.y);
   stream << (p.z);
   return stream.good();
}

inline bool mathWrite(std::iostream &stream, const Point2F& p)
{
   stream << (p.x);
   stream << (p.y);
   return stream.good();
}

inline bool mathWrite(std::iostream &stream, const Point3F& p)
{
   stream << (p.x);
   stream << (p.y);
   stream << (p.z);
   return stream.good();
}

inline bool mathWrite(std::iostream &stream, const Point4F& p)
{
   stream << (p.x);
   stream << (p.y);
   stream << (p.z);
   stream << (p.w);
   return stream.good();
}

inline bool mathWrite(std::ostream &stream, const Point3D& p)
{
   stream << (p.x);
   stream << (p.y);
   stream << (p.z);
   return stream.good();
}

inline bool mathWrite(std::ostream &stream, const PlaneF& p)
{
   stream << (p.x);
   stream << (p.y);
   stream << (p.z);
   stream << (p.d);
   return stream.good();
}

inline bool mathWrite(std::ostream& stream, const Box3F& b)
{
   bool success = mathWrite(stream, b.minExtents);
   success     &= mathWrite(stream, b.maxExtents);
   return stream.good();
}

inline bool mathWrite(std::ostream& stream, const SphereF& s)
{
   bool success = mathWrite(stream, s.center);
   stream << (s.radius);
    return stream.good();
}

inline bool mathWrite(std::ostream& stream, const RectI& r)
{
   bool success = mathWrite(stream, r.point);
   success     &= mathWrite(stream, r.extent);
    return stream.good();
}

inline bool mathWrite(std::ostream& stream, const RectF& r)
{
   bool success = mathWrite(stream, r.point);
   success     &= mathWrite(stream, r.extent);
    return stream.good();
}

inline bool mathWrite(std::ostream& stream, const MatrixF& m)
{
   bool success    = true;
   const F32* pm = m;
   stream.write(pm, sizeof(F32)*16);
   return stream.good();
}

inline bool mathWrite(std::ostream& stream, const QuatF& q)
{
   stream << (q.x);
   stream << (q.y);
   stream << (q.z);
   stream << (q.w);
   return stream.good();
}

#endif //_MATHIO_H_

