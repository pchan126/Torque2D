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

#import "platformOSX/platformOSX.h"
#include "platform/platform.h"
#include "string/stringTable.h"
#include "console/console.h"
#include "debug/profiler.h"
#include <stdlib.h>
#include <stdarg.h>
#include <string.h>
#include <ctype.h>
#include <stdio.h>
#include <stdlib.h>

typedef char nat_char;

//-----------------------------------------------------------------------------

char *dStrdup_r(const char *src, const char *file, dsize_t line)
{
    char *buffer = (char *) dMalloc_r(dStrlen(src) + 1, file, line);
    
    dStrcpy(buffer, src);
    return buffer;
}

//-----------------------------------------------------------------------------

char *dStrnew(const char *src)
{
    char *buffer = new char[dStrlen(src) + 1];
    dStrcpy(buffer, src);
    return buffer;
}

//-----------------------------------------------------------------------------

char* dStrcat(char *dst, const char *src)
{
    return strcat(dst,src);
}

//-----------------------------------------------------------------------------

char* dStrncat(char *dst, const char *src, dsize_t len)
{
    return strncat(dst,src,len);
}

//-----------------------------------------------------------------------------

// concatenates a list of src's onto the end of dst
// the list of src's MUST be terminated by a NULL parameter
// dStrcatl(dst, sizeof(dst), src1, src2, NULL);
char* dStrcatl(char *dst, dsize_t dstSize, ...)
{
    const char* src;
    char *p = dst;
    
    AssertFatal(dstSize > 0, "dStrcatl: destination size is set zero");
    dstSize--;  // leave room for string termination
    
    // find end of dst
    while (dstSize && *p)
    {
        p++;
        dstSize--;
    }
    
    va_list args;
    va_start(args, dstSize);
    
    // concatenate each src to end of dst
    while((src = va_arg(args, const char*)) != NULL)
    {
        while( dstSize && *src )
        {
            *p++ = *src++;
            dstSize--;
        }
    }
    
    va_end(args);
    
    // make sure the string is terminated
    *p = 0;
    
    return dst;
}

//-----------------------------------------------------------------------------

// copy a list of src's into dst
// the list of src's MUST be terminated by a NULL parameter
// dStrccpyl(dst, sizeof(dst), src1, src2, NULL);
char* dStrcpyl(char *dst, dsize_t dstSize, ...)
{
    const char* src;
    char *p = dst;
    
    AssertFatal(dstSize > 0, "dStrcpyl: destination size is set zero");
    dstSize--;  // leave room for string termination
    
    va_list args;
    va_start(args, dstSize);
    
    // concatenate each src to end of dst
    while((src = va_arg(args, const char*)) != NULL)
    {
        while( dstSize && *src )
        {
            *p++ = *src++;
            dstSize--;
        }
    }
    
    va_end(args);
    
    // make sure the string is terminated
    *p = 0;
    
    return dst;
}

//-----------------------------------------------------------------------------

int dStrcmp(const char *str1, const char *str2)
{
    return strcmp(str1, str2);
}

//-----------------------------------------------------------------------------

int dStrcmp( const UTF16 *str1, const UTF16 *str2)
{
    int ret;
    const UTF16 *a, *b;
    a = str1;
    b = str2;
    
    while(*a && *b && (ret = *a - *b) == 0)
    {
        a++, b++;
    }
    
    if ( *a == 0 && *b != 0 )
        return -1;

    if ( *b == 0 && *a != 0 )
        return 1;
   
    return ret;
}

//-----------------------------------------------------------------------------

int dStricmp(const char *str1, const char *str2)
{
    char c1, c2;
    
    while (1)
    {
        c1 = tolower(*str1++);
        c2 = tolower(*str2++);
        
        if (c1 < c2)
            return -1;
        
        if (c1 > c2)
            return 1;
        
        if (c1 == 0)
            return 0;
    }
}

//-----------------------------------------------------------------------------

int dStrncmp(const char *str1, const char *str2, dsize_t len)
{
    return strncmp(str1, str2, len);
}

//-----------------------------------------------------------------------------

int dStrnicmp(const char *str1, const char *str2, dsize_t len)
{
    S32 i;
    char c1, c2;
    
    for(i = 0; i < len; i++)
    {
        c1 = tolower(*str1++);
        c2 = tolower(*str2++);
        
        if (c1 < c2)
            return -1;
        
        if (c1 > c2)
            return 1;
        
        if (!c1)
            return 0;
    }
    return 0;
}


static inline int
nat_isdigit( nat_char a )
{
    return dIsdigit( a );
}


static inline int
nat_isspace( nat_char a )
{
    return dIsspace( a );
}


static inline nat_char
nat_toupper( nat_char a )
{
    return dToupper( a );
}


static int
compare_right(const nat_char* a, const nat_char* b)
{
    int bias = 0;
    
    /* The longest run of digits wins.  That aside, the greatest
     value wins, but we can't know that it will until we've scanned
     both numbers to know that they have the same magnitude, so we
     remember it in BIAS. */
    for (;; a++, b++) {
        if (!nat_isdigit(*a)  &&  !nat_isdigit(*b))
            return bias;
        else if (!nat_isdigit(*a))
            return -1;
        else if (!nat_isdigit(*b))
            return +1;
        else if (*a < *b) {
            if (!bias)
                bias = -1;
        } else if (*a > *b) {
            if (!bias)
                bias = +1;
        } else if (!*a  &&  !*b)
            return bias;
    }
    
    return 0;
}


static int
compare_left(const nat_char* a, const nat_char* b)
{
    /* Compare two left-aligned numbers: the first to have a
     different value wins. */
    for (;; a++, b++) {
        if (!nat_isdigit(*a)  &&  !nat_isdigit(*b))
            return 0;
        else if (!nat_isdigit(*a))
            return -1;
        else if (!nat_isdigit(*b))
            return +1;
        else if (*a < *b)
            return -1;
        else if (*a > *b)
            return +1;
    }
    
    return 0;
}


//-----------------------------------------------------------------------------

static int strnatcmp0(const nat_char* a, const nat_char* b, int fold_case)
{
    int ai, bi;
    nat_char ca, cb;
    int fractional, result;
    
    ai = bi = 0;
    while (1) {
        ca = a[ai]; cb = b[bi];
        
        /* skip over leading spaces or zeros */
        while (nat_isspace(ca))
            ca = a[++ai];
        
        while (nat_isspace(cb))
            cb = b[++bi];
        
        /* process run of digits */
        if (nat_isdigit(ca)  &&  nat_isdigit(cb)) {
            fractional = (ca == '0' || cb == '0');
            
            if (fractional) {
                if ((result = compare_left(a+ai, b+bi)) != 0)
                    return result;
            } else {
                if ((result = compare_right(a+ai, b+bi)) != 0)
                    return result;
            }
        }
        
        if (!ca && !cb) {
            /* The strings compare the same.  Perhaps the caller
             will want to call strcmp to break the tie. */
            return 0;
        }
        
        if (fold_case) {
            ca = nat_toupper(ca);
            cb = nat_toupper(cb);
        }
        
        if (ca < cb)
            return -1;
        else if (ca > cb)
            return +1;
        
        ++ai; ++bi;
    }
}

//-----------------------------------------------------------------------------

int dStrnatcmp(const nat_char* a, const nat_char* b) {
    return strnatcmp0(a, b, 0);
}


//-----------------------------------------------------------------------------

/* Compare, recognizing numeric string and ignoring case. */
int dStrnatcasecmp(const nat_char* a, const nat_char* b) {
    return strnatcmp0(a, b, 1);
}


/// Check if one string starts with another
bool dStrStartsWith(const char* str1, const char* str2)
{
    return !dStrnicmp(str1, str2, dStrlen(str2));
}

/// Check if one string ends with another
bool dStrEndsWith(const char* str1, const char* str2)
{
    const char *p = str1 + dStrlen(str1) - dStrlen(str2);
    return ((p >= str1) && !dStricmp(p, str2));
}


//-----------------------------------------------------------------------------

char* dStrcpy(char *dst, const char *src)
{
    AssertFatal(dst && src, "bad strings passed to dStrcpy()");
    return strcpy(dst, src);
}

//-----------------------------------------------------------------------------

char* dStrncpy(char *dst, const char *src, dsize_t len)
{
    return strncpy(dst, src, len);
}

//-----------------------------------------------------------------------------

dsize_t dStrlen(const char *str)
{
    return str ? strlen(str) : 0;
}

//-----------------------------------------------------------------------------

char* dStrupr(char *str)
{
    char* saveStr = str;
    
    while (*str)
    {
        *str = toupper(*str);
        str++;
    }
    
    return saveStr;
}

//-----------------------------------------------------------------------------

char* dStrlwr(char *str)
{
    char* saveStr = str;
    
    while (*str)
    {
        *str = tolower(*str);
        str++;
    }
    
    return saveStr;
}

//-----------------------------------------------------------------------------

char* dStrchr(char *str, int c)
{
    return strchr(str, c);
}

//-----------------------------------------------------------------------------

const char* dStrchr(const char *str, int c)
{
    return strchr(str, c);
}

//-----------------------------------------------------------------------------

const char* dStrrchr(const char *str, int c)
{
    return strrchr(str, c);
}

//-----------------------------------------------------------------------------

char* dStrrchr(char *str, int c)
{
    return strrchr(str, c);
}

//-----------------------------------------------------------------------------

dsize_t dStrspn(const char *str, const char *set)
{
    return(strspn(str, set));
}

//-----------------------------------------------------------------------------

dsize_t dStrcspn(const char *str, const char *set)
{
    return strcspn(str, set);
}

//-----------------------------------------------------------------------------

char* dStrstr(char *str1, char *str2)
{
    return strstr(str1, str2);
}

//-----------------------------------------------------------------------------

char* dStrstr(const char *str1, const char *str2)
{
    return strstr(str1, str2);
}

//-----------------------------------------------------------------------------

char* dStrtok(char *str, const char *sep)
{
    return strtok(str, sep);
}

//-----------------------------------------------------------------------------

int dAtoi(const char *str)
{
    if(!str)
        return 0;
    
    return atoi(str);
}

//-----------------------------------------------------------------------------

float dAtof(const char *str)
{
    if(!str)
        return 0;
    
    return atof(str);
}

//-----------------------------------------------------------------------------

bool dAtob(const char *str)
{
    return !dStricmp(str, "true") || !dStricmp(str, "1") || (0!=dAtoi(str));
}

//-----------------------------------------------------------------------------

bool dIsalnum(const char c)
{
    return isalnum(c);
}

//-----------------------------------------------------------------------------

bool dIsalpha(const char c)
{
    return(isalpha(c));
}

//-----------------------------------------------------------------------------

bool dIsspace(const char c)
{
    return(isspace(c));
}

//-----------------------------------------------------------------------------

bool dIsdigit(const char c)
{
    return(isdigit(c));
}

//-----------------------------------------------------------------------------

void dPrintf(const char *format, ...)
{
    va_list args;
    va_start(args, format);
    vprintf(format, args);
}

//-----------------------------------------------------------------------------

int dVprintf(const char *format, va_list arglist)
{
    S32 len = vprintf(format, arglist);
    
    return (len);
}

//-----------------------------------------------------------------------------

int dSprintf(char *buffer, dsize_t bufferSize, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    S32 len = vsprintf(buffer, format, args);
    
    // Sanity!
    AssertFatal(len <= bufferSize, "dSprintf - String format exceeded buffer size.  This will cause corruption.");
    
    return (len);
}

//-----------------------------------------------------------------------------

int dVsprintf(char *buffer, dsize_t bufferSize, const char *format, va_list arglist)
{
	S32 len = vsprintf(buffer, format, arglist);

    // Sanity!
    AssertFatal(len <= bufferSize, "dSprintf - String format exceeded buffer size.  This will cause corruption.");
    
    return (len);
}

//-----------------------------------------------------------------------------

int dStrrev(char* str)
{
    // Get the string length
	int l = dStrlen(str) - 1;
    
	for(int x=0;x < l;x++,l--)
	{
        // triple XOR Trick
		str[x]^=str[l];
        
        // for not using a temp
 		str[l]^=str[x];
  		str[x]^=str[l];
	}
    
	return l;
}

//-----------------------------------------------------------------------------

int dItoa(int n, char s[])
{
	int i, sign;
	
    // Record sign
	if ((sign = n) < 0)  
	{
        // Make n positive
        n = -n;
    }
    
    i = 0;
    
    // Generate digits in reverse order
	do
    {
        // Get next digit
        s[i++] = n % 10 + '0';   
	}
    while ((n /= 10) > 0);
    
    // Delete it
	if (sign < 0)
		s[i++] = '-';
    
	s[i] = '\0';
    
    // Reverse string.
	dStrrev(s);
    
    // Return length.
    return dStrlen(s);
}

//-----------------------------------------------------------------------------

int dSscanf(const char *buffer, const char *format, ...)
{
    va_list args;
    va_start(args, format);
    return vsscanf(buffer, format, args);
}

//-----------------------------------------------------------------------------

int dFflushStdout()
{
    return fflush(stdout);
}

//-----------------------------------------------------------------------------

int dFflushStderr()
{
    return fflush(stderr);
}

//-----------------------------------------------------------------------------

void dQsort(void *base, SizeType nelem, SizeType width, int (QSORT_CALLBACK *fcmp)(const void *, const void *))
{
    qsort(base, nelem, width, fcmp);
}

//-----------------------------------------------------------------------------

StringTableEntry Platform::createUUID( void )
{
    CFUUIDRef ref = CFUUIDCreate(nil);
    NSString* uuid = (__bridge_transfer NSString *)CFUUIDCreateString(nil,ref);
    CFRelease(ref);
    
    StringTableEntry uuidString = StringTable->insert([uuid UTF8String]);
    return uuidString;
}
