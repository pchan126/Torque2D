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

#include "string/str.h"
#include <algorithm>
#include <functional>
#include <cctype>
#include <locale>
#include "platform/platform.h"
#include "collection/hashTable.h"
#include "string/unicode.h"
#include <memory>
#include "console/console.h"

const size_t String::NPos = U32(~0);
const String String::EmptyString;

/// A delete policy for the AutoPtr class
struct DeleteString
{
   template<class T>
   static void destroy(T *ptr) { dFree(ptr); }
};


//-----------------------------------------------------------------------------

/// Search for a character.
/// Search for the position of the needle in the haystack.
/// Default mode is StrCase | StrLeft, mode also accepts StrNoCase and StrRight.
/// If pos is non-zero, then in mode StrLeft the search starts at (hay + pos) and
/// in mode StrRight the search starts at (hay + pos - 1)
/// @return Returns a pointer to the location of the character in the haystack or 0
static const char* StrFind(const char* hay, char needle, size_t pos, U32 mode)
{
   if (mode & String::Right)
   {
      // Go to the end first, then search backwards
      const char  *he = hay;

      if (pos)
      {
         he += pos - 1;
      }
      else
      {
         while (*he)
            he++;
      }

      if (mode & String::NoCase)
      {
         needle = dTolower(needle);

         for (; he >= hay; he--)
         {
            if (dTolower(*he) == needle)
               return he;
         }
      }
      else
      {
         for (; he >= hay; he--)
         {
            if (*he == needle)
               return he;
         }
      }
      return 0;
   }
   else
   {
      if (mode & String::NoCase)
      {
         needle = dTolower(needle);
         for (hay += pos; *hay && dTolower(*hay) != needle;)
            hay++;
      }
      else
      {
         for (hay += pos; *hay && *hay != needle;)
            hay++;
      }

      return *hay ? hay : 0;
   }
}

/// Search for a StringData.
/// Search for the position of the needle in the haystack.
/// Default mode is StrCase | StrLeft, mode also accepts StrNoCase and StrRight.
/// If pos is non-zero, then in mode StrLeft the search starts at (hay + pos) and
/// in mode StrRight the search starts at (hay + pos - 1)
/// @return Returns a pointer to the StringData in the haystack or 0
static const char* StrFind(const char* hay, const char* needle, S32 pos, U32 mode)
{
   if (mode & String::Right)
   {
      const char  *he = hay;

      if (pos)
      {
         he += pos - 1;
      }
      else
      {
         while (*he)
            he++;
      }

      if (mode & String::NoCase)
      {
          std::unique_ptr<char[]> ln(dStrlwr(dStrdup(needle)));
         for (; he >= hay; he--)
         {
            if (dTolower(*he) == *ln.get())
            {
               U32 i = 0;
               while (ln[i] && ln[i] == dTolower(he[i]))
                  i++;
               if (!ln[i])
                  return he;
               if (!hay[i])
                  return 0;
            }
         }
      }
      else
      {
         for (; he >= hay; he--)
         {
            if (*he == *needle)
            {
               U32 i = 0;
               while (needle[i] && needle[i] == he[i])
                  i++;
               if (!needle[i])
                  return he;
               if (!hay[i])
                  return 0;
            }
         }
      }
      return 0;
   }
   else
   {
      if (mode & String::NoCase)
      {
          std::unique_ptr<char[]> ln(dStrlwr(dStrdup(needle)));
         for (hay += pos; *hay; hay++)
         {
            if (dTolower(*hay) == *ln.get())
            {
               U32 i = 0;
               while (ln[i] && ln[i] == dTolower(hay[i]))
                  i++;
               if (!ln[i])
                  return hay;
               if (!hay[i])
                  return 0;
            }
         }
      }
      else
      {
         for (hay += pos; *hay; hay++)
         {
            if (*hay == *needle)
            {
               U32 i = 0;
               while (needle[i] && needle[i] == hay[i])
                  i++;
               if (!needle[i])
                  return hay;
               if (!hay[i])
                  return 0;
            }
         }
      }
   }

   return 0;
}



//-----------------------------------------------------------------------------

#ifdef TORQUE_DEBUG

/// Tracks the number of bytes allocated for strings.
/// @bug This currently does not include UTF16 allocations.
static U32 sgStringMemBytes;

/// Tracks the number of Strings which are currently instantiated.
static U32 sgStringInstances;

ConsoleFunction( dumpStringMemStats, void, 1, 1, "()"
				"@brief Dumps information about String memory usage\n\n"
				"@ingroup Debugging\n"
				"@ingroup Strings\n")
{
   Con::printf( "String Data: %i instances, %i bytes", sgStringInstances, sgStringMemBytes );
}

#endif


String::String(const StringChar *str, size_t len):_intern(nullptr)
{
    _string = str;
    _string.resize(len);
}


String::String(const UTF16 *str):_intern(nullptr)
{
   PROFILE_SCOPE(String_UTF16_constructor);

   if( str && str[ 0 ] )
   {
      UTF8* utf8 = convertUTF16toUTF8( str );
      _string = utf8;
      delete [] utf8;
   }
   else
      _string.clear();
}

String::~String()
{
}

//-----------------------------------------------------------------------------

size_t String::find(const String &str, size_t pos, U32 mode) const
{
   return find(str._string.c_str(), pos, mode);
}

String& String::insert(size_t pos, const String &str)
{
   return insert(pos, str._string.c_str());
}

String& String::replace(size_t pos, size_t len, const String &str)
{
   return replace(pos, len, str._string.c_str());
}

//-----------------------------------------------------------------------------

String& String::operator=(StringChar c)
{
    _string = c;
   return *this;
}

String& String::operator+=(StringChar c)
{
    _string+=c;
    return *this;
}

//-----------------------------------------------------------------------------

String& String::operator=(const StringChar *str)
{
    _string = str;
   return *this;
}

String& String::operator=(const String &src)
{
    _string = src._string;
   return *this;
}

String& String::operator+=(const StringChar *src)
{
    _string += src;
   return *this;
}

String& String::operator+=(const String &src)
{
    _string += src._string;
   return *this;
}

//-----------------------------------------------------------------------------

String operator+(const String &a, const String &b)
{
   return String(a._string + b._string);
}

String operator+(const String &a, StringChar c)
{
   return String(a._string + &c);
}

String operator+(StringChar c, const String &a)
{
   String temp(&c);
   return String(temp+a._string);
}

String operator+(const String &a, const StringChar *b)
{
    std::string p1 = a._string;
    std::string p2(b);
   return String(p1 + p2);
}

String operator+(const StringChar *a, const String &b)
{
    std::string p1(a);
    std::string p2 = b._string;
   return String(p1 + p2);
}

bool String::operator==(const String &str) const
{
    if (isInterned() && str.isInterned())
        return (_intern == str._intern);

    return _string == str._string;
}

bool String::operator==( StringChar c ) const
{
   if( _string.size() != 1 )
      return false;
   else
      return ( _string.c_str()[ 0 ] == c );
}

bool String::operator<(const String &str) const
{
   return ( dStrnatcmp( _string.c_str(), str._string.c_str() ) < 0 );
}

bool String::operator>(const String &str) const
{
   return ( dStrnatcmp( _string.c_str(), str._string.c_str() ) > 0 );
}

bool String::operator<=(const String &str) const
{
   return ( dStrnatcmp( _string.c_str(), str._string.c_str() ) <= 0 );
}

bool String::operator>=(const String &str) const
{
   return ( dStrnatcmp( _string.c_str(), str._string.c_str() ) >= 0 );
}

//-----------------------------------------------------------------------------
// Base functions for string comparison

S32 String::compare(const StringChar *str, size_t len, U32 mode) const
{
    std::string p1 = _string;
    std::string p2(str);
    if (mode & String::NoCase)
    {
        std::transform(p1.begin(), p1.end(), p1.begin(), tolower);
        std::transform(p2.begin(), p2.end(), p2.begin(), tolower);
    }

    return ( p1.compare(0, len, p2.c_str()));

}

S32 String::compare(const String &str, size_t len, U32 mode) const
{
    std::string p1 = _string;
    std::string p2 = str._string;
    if (mode & String::NoCase)
    {
        std::transform(p1.begin(), p1.end(), p1.begin(), tolower);
        std::transform(p2.begin(), p2.end(), p2.begin(), tolower);
    }

    return ( p1.compare(0, len, p2.c_str()));
}

bool String::equal(const String &str, U32 mode) const
{
    std::string p1 = _string;
    std::string p2 = str._string;
    if (mode & String::NoCase)
    {
        std::transform(p1.begin(), p1.end(), p1.begin(), tolower);
        std::transform(p2.begin(), p2.end(), p2.begin(), tolower);
    }

     return ( p1.compare( p2.c_str() ) == 0 );
}

//-----------------------------------------------------------------------------

size_t String::find(StringChar c, size_t pos, U32 mode) const
{
   const StringChar* ptr = StrFind(_string.c_str(),c,pos,mode);

   return ptr? size_t(ptr - _string.c_str()): NPos;
}

size_t String::find(const StringChar *str, size_t pos, U32 mode)  const
{
   AssertFatal(str,"String:: Invalid null ptr argument");

   const StringChar* ptr = StrFind(_string.c_str(),str,(S32)pos,mode);

   return ptr? size_t(ptr - _string.c_str()): NPos;
}


//-----------------------------------------------------------------------------

String& String::insert(size_t pos, const StringChar *str)
{
   AssertFatal(str,"String:: Invalid null ptr argument");

   return insert(pos,str,dStrlen(str));
}

String& String::insert(size_t pos, const StringChar *str, size_t len)
{
   _string.insert(pos, str, len);
   return *this;
}

String& String::erase(size_t pos, size_t len)
{
    _string.erase(pos, len);
    return *this;
}

String& String::replace(size_t pos, size_t len, const StringChar *str)
{
    _string.replace(pos, len, str);
    return *this;
}

String& String::replace( StringChar c1, StringChar c2 )
{
   if( isEmpty() )
      return *this;

   _string.replace(_string.find(c1), _string.length(), 1, c2);
   return *this;
}

String &String::replace(const String &s1, const String &s2)
{
   _string.replace(_string.find(s1._string), _string.length(), s2._string);
   return *this;
}



//-----------------------------------------------------------------------------

String String::trim() const
{
   if( isEmpty() )
      return *this;

    std::string sub = _string;
    sub.erase(std::find_if(sub.rbegin(), sub.rend(), std::not1(std::ptr_fun<int, int>(std::isspace))).base(), sub.end());

   return String(sub);
}

//-----------------------------------------------------------------------------

String String::expandEscapes() const
{
   char* tmp = ( char* ) dMalloc( length() * 2 + 1 ); // worst-case situation.
   expandEscape( tmp, c_str() );
   String str( tmp );
   dFree( tmp );
   return str;
}

//-----------------------------------------------------------------------------

String String::collapseEscapes() const
{
   char* tmp = dStrdup( c_str() );
   collapseEscape( tmp );
   String str( tmp );
   dFree( tmp );
   return str;
}

//-----------------------------------------------------------------------------

void String::split( const char* delimiter, Vector< String >& outElements ) const
{
   const char* ptr = _string.c_str();
   
   const char* start = ptr;
   while( *ptr )
   {
      // Search for start of delimiter.
      
      if( *ptr != delimiter[ 0 ] )
         ptr ++;
      else
      {
         // Skip delimiter.
         
         const char* end = ptr;
         const char* del = delimiter;
         while( *del && *del == *ptr )
         {
            ptr ++;
            del ++;
         }
         
         // If we didn't match all of delimiter,
         // continue with search.
         
         if( *del != '\0' )
            continue;
            
         // Extract component.
         
         outElements.push_back( String( start, end - start ) );
         start = ptr;
      }
   }
   
   // Add rest of string if there is any.
   
   if( start != ptr )
      outElements.push_back( start );
}

//-----------------------------------------------------------------------------

bool String::startsWith( const char* text ) const
{
   return dStrStartsWith( _string.c_str(), text );
}

//-----------------------------------------------------------------------------

bool String::endsWith( const char* text ) const
{
   return dStrEndsWith( _string.c_str(), text );
}


//-----------------------------------------------------------------------------

#if defined(TORQUE_OS_WIN32) || defined(TORQUE_OS_XBOX) || defined(TORQUE_OS_XENON)
// This standard function is not defined when compiling with VC7...
#define vsnprintf	_vsnprintf
#endif

String::StrFormat::~StrFormat()
{
   if( _dynamicBuffer )
      dFree( _dynamicBuffer );
}

S32 String::StrFormat::format( const char *format, va_list args )
{
   _len=0;
   return formatAppend(format,args);
}

S32 String::StrFormat::formatAppend( const char *format, va_list args )
{
   // Format into the fixed buffer first.
   S32 startLen = _len;
   if (_dynamicBuffer == nullptr)
   {
      _len += vsnprintf(_fixedBuffer + _len, sizeof(_fixedBuffer) - _len, format, args);
      if ( _len < sizeof(_fixedBuffer))
         return _len;

      // Start off the dynamic buffer at twice fixed buffer size
      _len = startLen;
      _dynamicSize = sizeof(_fixedBuffer) * 2;
      _dynamicBuffer = (char*)dMalloc(_dynamicSize);
      dMemcpy(_dynamicBuffer, _fixedBuffer, _len + 1);
   }

   // Format into the dynamic buffer, if the buffer is not large enough, then
   // keep doubling it's size until it is.  The buffer is not reallocated
   // using reallocate() to avoid unnecessary buffer copying.
   _len += vsnprintf(_dynamicBuffer + _len, _dynamicSize - _len, format, *(va_list*)args);
   while ( _len >= _dynamicSize)
   {
      _len = startLen;
      _dynamicBuffer = (char*)dRealloc(_dynamicBuffer, _dynamicSize *= 2);
      _len += vsnprintf(_dynamicBuffer + _len, _dynamicSize - _len, format, *(va_list*)args);
   }

   return _len;
}

S32 String::StrFormat::append(const char * str, S32 len)
{
   if (_dynamicBuffer == nullptr)
   {
      if ( _len+len < sizeof(_fixedBuffer))
      {
         dMemcpy(_fixedBuffer + _len, str, len);
         _len += len;
         _fixedBuffer[_len] = '\0';
         return _len;
      }

      _dynamicSize = sizeof(_fixedBuffer) * 2;
      _dynamicBuffer = (char*)dMalloc(_dynamicSize);
      dMemcpy(_dynamicBuffer, _fixedBuffer, _len + 1);
   }

   S32 newSize = _dynamicSize;
   while (newSize < _len+len)
      newSize *= 2;
   if (newSize != _dynamicSize)
      _dynamicBuffer = (char*) dRealloc(_dynamicBuffer, newSize);
   _dynamicSize = newSize;
   dMemcpy(_dynamicBuffer + _len, str, len);
   _len += len;
   _dynamicBuffer[_len] = '\0';
   return _len;
}

S32 String::StrFormat::append(const char * str)
{
   return append(str, (S32)dStrlen(str));
}

char* String::StrFormat::copy( char *buffer ) const
{
   dMemcpy(buffer, _dynamicBuffer? _dynamicBuffer: _fixedBuffer, _len+1);
   return buffer;
}

//-----------------------------------------------------------------------------

String String::ToString( bool value )
{
   static String sTrue = "true";
   static String sFalse = "false";
   
   if( value )
      return sTrue;
   return sFalse;
}

String String::ToString(const char *str, ...)
{
   AssertFatal(str,"String:: Invalid null ptr argument");

   // Use the format object
   va_list args;
   va_start(args, str);
   String ret = VToString(str, args);
   va_end(args);
   return ret;
}

String String::VToString(const char* str, va_list args)
{
   StrFormat format(str, args);
    std::string sub = format.c_str();

   return String(sub);
}

String   String::SpanToString(const char *start, const char *end)
{
   if ( end == start )
      return String();
   
   AssertFatal( end > start, "Invalid arguments to String::SpanToString - end is before start" );

   return String(std::string( start, 0, end - start ));
}

String String::ToLower(const String &string)
{
   if ( string.isEmpty() )
      return String();

    std::string sub = string._string;
    std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
   return String(sub);
}

String String::ToUpper(const String &string)
{
    std::string s = string._string;
    std::transform(s.begin(), s.end(), s.begin(), toupper);
    return String(s);
}

String String::GetTrailingNumber(const char* str, S32& number)
{
   // Check for trivial strings
   if (!str || !str[0])
      return String::EmptyString;

   // Find the number at the end of the string
   String base(str);
   const char* p = base.c_str() + base.length() - 1;

   // Ignore trailing whitespace
   while ((p != base.c_str()) && dIsspace(*p))
      p--;

   // Need at least one digit!
   if (!dIsdigit(*p))
      return base;

   // Back up to the first non-digit character
   while ((p != base.c_str()) && dIsdigit(*p))
      p--;

   // Convert number => allow negative numbers, treat '_' as '-' for Maya
   if ((*p == '-') || (*p == '_'))
      number = -dAtoi(p + 1);
   else
      number = ((p == base.c_str()) ? dAtoi(p) : dAtoi(++p));

   // Remove space between the name and the number
   while ((p > base.c_str()) && dIsspace(*(p-1)))
      p--;

   return base.substr(0, p - base.c_str());
}

void String::intern() {
    _intern = StringTable->insert(_string.c_str());
}

U32 String::getHashCaseInsensitive() const {
    std::string sub = _string;
    std::transform(sub.begin(), sub.end(), sub.begin(), ::tolower);
    return (U32)std::hash<std::string>()(sub);
}

U32 String::getHashCaseSensitive() const {
    return (U32)std::hash<std::string>()(_string);
}

