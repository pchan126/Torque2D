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

#include "math/mMathFn.h"

#ifndef _VECTOR_H_
#define _VECTOR_H_

//Includes
#ifndef _PLATFORM_H_
#include "platform/platform.h"
#endif

#include <vector>
#include <algorithm>

//-----------------------------------------------------------------------------

/// Use the following macro to bind a vector to a particular line
///  of the owning class for memory tracking purposes
#ifdef TORQUE_DEBUG
#define VECTOR_SET_ASSOCIATION(x) x.setFileAssociation(__FILE__, __LINE__)
#else
#define VECTOR_SET_ASSOCIATION(x)
#endif

//-----------------------------------------------------------------------------
/// A dynamic array class.
///
/// The vector grows as you insert or append
/// elements.  Insertion is fastest at the end of the array.  Resizing
/// of the array can be avoided by pre-allocating space using the
/// reserve() method.
///
/// <b>***WARNING***</b>
///
/// This template does not initialize, construct or destruct any of
/// it's elements.  This means don't use this template for elements
/// (classes) that need these operations.  This template is intended
/// to be used for simple structures that have no constructors or
/// destructors.
///
/// @nosubgrouping
template<class T>
class Vector
{
  protected:
    std::vector<T> _vector;

#ifdef TORQUE_DEBUG
   const char* mFileAssociation;
   U32         mLineAssociation;
#endif

    // resizes, but does no construction/destruction
   bool  resize(U32 i)                      { _vector.resize(i);};
   void  construct(U32 start, U32 end); ///< Constructs elements from <i>start</i> to <i>end-1</i>

  public:
   Vector(const U32 initialSize = 0);
   Vector(const U32 initialSize, const char* fileName, const U32 lineNum);
   Vector(const char* fileName, const U32 lineNum);
   Vector(const Vector&);
   ~Vector();

#ifdef TORQUE_DEBUG
   void setFileAssociation(const char* file, const U32 line);
#endif

   /// @name STL interface
   /// @{

   typedef T        value_type;
   typedef T&       reference;
   typedef const T& const_reference;

   typedef typename std::vector<T>::iterator               iterator;
   typedef typename std::vector<T>::const_iterator         const_iterator;
   typedef typename std::vector<T>::reverse_iterator       reverse_iterator;
   typedef typename std::vector<T>::const_reverse_iterator const_reverse_iterator;
   typedef S32    difference_type;

   typedef difference_type (QSORT_CALLBACK *compare_func)(const T *a, const T *b);

   Vector<T>& operator=(const Vector<T>& p);

   iterator       begin()             { return _vector.begin(); };
   const_iterator begin() const       { return _vector.begin(); };
   iterator       end()               { return _vector.end(); };
   const_iterator end() const         { return _vector.end(); };

   reverse_iterator       rbegin()             { return _vector.rbegin(); };
   const_reverse_iterator rbegin() const       { return _vector.rbegin(); };
   reverse_iterator       rend()               { return _vector.rend(); };
   const_reverse_iterator rend() const         { return _vector.rend(); };

   SizeType size() const                   { return _vector.size(); };
   bool empty() const                      { return _vector.empty(); };
   bool contains(const T& x) const         { return std::count(_vector.begin(), _vector.end(), x) > 0; };

   void insert(iterator itr, const T& x)    { _vector.insert(itr, x); };
   void erase(iterator itr)                 { _vector.erase(itr); };
   void erase(const_iterator itr)           { _vector.erase(itr); };

   T&       front()                         { return _vector.front(); };
   const T& front() const                   { return _vector.front(); };
   T&       back()                          { return _vector.back(); };
   const T& back() const                    { return _vector.back(); };

    void push_back(const T x)          { _vector.push_back(x); };
    U32 push_back_unique(const T&);
    U32 push_back_unique(T&&);

    iterator       find( T x)                { return std::find(_vector.begin(), _vector.end(), x); }
    const_iterator find( T x) const          { return std::find(_vector.begin(), _vector.end(), x); }

    S32 find_next_index( const T&, U32 start = 0 );
    S32 find_next_index( T&&, U32 start = 0 );

   void pop_back();

   T& operator[](SizeType i)              { return _vector[i]; }
   const T& operator[](SizeType i ) const { return _vector[i]; }

   T& at(SizeType);
   const T& at(SizeType) const;

   void reserve(SizeType i)                 { return _vector.reserve(i);  };
   SizeType capacity() const              { return _vector.capacity(); };

   /// @}

   /// @name Extended interface
   /// @{

   U32  memSize() const       { return capacity() * sizeof(T); };
   T*   address() {     return _vector.data();   };
   const T*   address() const {     return _vector.data();   };
   SizeType  setSize(SizeType size)     { _vector.resize(size); return _vector.size(); };
   SizeType  setSize(SizeType size, const T& val)     { _vector.resize(size, val); return _vector.size(); };
   void increment( U32 = 1);
   void decrement(U32 = 1);
   void insert(SizeType index)             { _vector.insert(_vector.begin()+index, T()); };
   void insert(SizeType index, const T& x) { _vector.insert(_vector.begin()+index, x); };
   void erase(U32 index)      { _vector.erase(_vector.begin()+index); };
   void clear()               { return _vector.clear(); };
   void compact()             { return _vector.shrink_to_fit(); };

   void emplace_back() { _vector.emplace_back(); };
   void sort(compare_func f);

   /// Finds the front matching element and erases it.
   /// @return Returns true if a match is found.
   bool remove( const T& );
   void merge(const Vector& p)     { _vector.insert(_vector.end(), p._vector.begin(), p._vector.end()); };
//   void merge(const )

   template<class InputIterator>
   void merge(InputIterator first, InputIterator last) { _vector.insert(_vector.end(), first, last); };
};

template<class T> inline Vector<T>::~Vector()
{
   clear();
}

template<class T> inline Vector<T>::Vector(const U32 initialSize)
{
#ifdef TORQUE_DEBUG
   mFileAssociation = NULL;
   mLineAssociation = 0;
#endif

   if(initialSize)
      reserve(initialSize);
}



template<class T> inline Vector<T>::Vector(const U32 initialSize,
                                           const char* fileName,
                                           const U32   lineNum)
{
#ifdef TORQUE_DEBUG
   mFileAssociation = fileName;
   mLineAssociation = lineNum;
#else
   TORQUE_UNUSED( fileName );
   TORQUE_UNUSED( lineNum );
#endif

   if(initialSize)
      reserve(initialSize);
}

template<class T> inline Vector<T>::Vector(const char* fileName,
                                           const U32   lineNum)
{
#ifdef TORQUE_DEBUG
   mFileAssociation = fileName;
   mLineAssociation = lineNum;
#else
   TORQUE_UNUSED( fileName );
   TORQUE_UNUSED( lineNum );
#endif
}

template<class T> inline Vector<T>::Vector(const Vector& p)
{
#ifdef TORQUE_DEBUG
   mFileAssociation = p.mFileAssociation;
   mLineAssociation = p.mLineAssociation;
#endif

    _vector = p._vector;
}


#ifdef TORQUE_DEBUG
template<class T> inline void Vector<T>::setFileAssociation(const char* file,
                                                            const U32   line)
{
   mFileAssociation = file;
   mLineAssociation = line;
}
#endif


template<class T> inline void  Vector<T>::construct(U32 start, U32 end) // destroys from start to end-1
{
   _vector.insert(_vector.begin()+start, (end-start), T());
}


template<class T> inline void Vector<T>::increment( U32 delta)
{
    _vector.insert(_vector.end(), delta, T());
}


template<class T> inline void Vector<T>::decrement(U32 delta)
{
   AssertFatal(_vector.size() != 0, "Vector<T>::decrement - cannot decrement zero-length vector.");

   for (U32 i = 0; i < delta; i++)
       _vector.pop_back();
}


template<class T> inline bool Vector<T>::remove( const T& x )
{
    iterator itr = std::find(_vector.begin(), _vector.end(), x);
    if (itr == _vector.end())
        return false;

    _vector.erase(itr);
    return true;
}

typedef int (QSORT_CALLBACK *qsort_compare_func)(const void *, const void *);

template<class T> inline void Vector<T>::sort(compare_func f)
{
    qsort(address(), size(), sizeof(T), (qsort_compare_func) f);
//    std::sort(_vector.begin(), _vector.end(), f);
}


//-----------------------------------------------------------------------------

template<class T> inline Vector<T>& Vector<T>::operator=(const Vector<T>& p)
{
    _vector = p._vector;
   return *this;
}

template<class T> inline U32 Vector<T>::push_back_unique(const T& x)
{
   S32 index = find_next_index(x);

   if (index == -1)
   {
      _vector.push_back(x);
      index = (U32)_vector.size() - 1;
   }

   return (U32)index;
}


template<class T> inline U32 Vector<T>::push_back_unique(T&& x)
{
    S32 index = find_next_index(x);

    if (index == -1)
    {
        _vector.push_back(x);
        index = _vector.size() - 1;
    }

    return (U32)index;
}

template<class T> inline S32 Vector<T>::find_next_index( const T& x, U32 start )
{
    typename std::vector<T>::iterator temp = std::find(_vector.begin()+start, _vector.end(), x);

    S32 index = -1;
    if (temp != _vector.end())
        index = std::distance(_vector.begin(), temp);

   return index;
}

template<class T> inline S32 Vector<T>::find_next_index( T&& x, U32 start )
{
    typename std::vector<T>::iterator temp = std::find(_vector.begin()+start, _vector.end(), x);

    S32 index = -1;
    if (temp != _vector.end())
        index = std::distance(_vector.begin(), temp);

    return index;
}

template<class T> inline void Vector<T>::pop_back()
{
   AssertFatal(_vector.size() != 0, "Vector<T>::pop_back - cannot pop the back of a zero-length vector.");
   _vector.pop_back();
}

template<class T> inline T& Vector<T>::at(SizeType index)
{
   AssertFatal(index < _vector.size(), "Vector<T>::at - out of bounds array access!");
   return _vector[index];
}

template<class T> inline const T& Vector<T>::at(SizeType index) const
{
   AssertFatal(index < _vector.size(), "Vector<T>::at - out of bounds array access!");
   return _vector[index];
}

//-----------------------------------------------------------------------------


#endif //_VECTOR_H_

