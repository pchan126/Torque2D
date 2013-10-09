#include "platform/platform.h"
#include <iostream>

namespace StreamFn {

   inline bool readFlag(std::istream& is)
   {
       bool val;
       is >> val;
       return val;
   }

   inline void readString(std::istream& is, char buf[256])
   {
      U8 len;
      is >> len;
      is.read(buf, len);
      buf[len] = 0;
   }
   
    inline void writeString (std::iostream& stream, const char* string, S32 maxLen=255)
    {
        size_t len = string ? dStrlen(string) : 0;
        if(len > maxLen)
            len = maxLen;

        stream << (U8(len));
        if(len)
            stream.write(string, len);
    }

    inline void writeLongString(std::iostream& stream, size_t maxStringLen, const char *string)
    {
        size_t len = dStrlen(string);
        if(len > maxStringLen)
            len = maxStringLen;
        stream << (len);
        stream.write(string, len);
    }

    /// Write a number of tabs to this stream
    inline void writeTabs(std::iostream& stream, U32 count)
    {
        char tab[] = "   ";
        while(count--)
            stream.write(tab, 3);
    }

    inline size_t getStreamSize(std::istream& is)
    {
        is.seekg (0, is.end);
        size_t length = is.tellg();
        is.seekg (0, is.beg);
        return length;
    }

    inline void writeLine(std::ostream& os, const char *line)
    {
        os.write(line, dStrlen(line));
        os.write("\r\n", 2);
    }

   inline U32 readRangedU32(std::istream& is, U32 min, U32 max)
   {
      U32 val;
      is >> val;
      return ( std::min(std::max(val, min), max) );
   }
}