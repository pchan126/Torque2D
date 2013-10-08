#include "platform/platform.h"
#include <iostream>

namespace StreamFn {

    inline void writeString (std::iostream& stream, const char* string, S32 maxLen=255)
    {
        S32 len = string ? dStrlen(string) : 0;
        if(len > maxLen)
            len = maxLen;

        stream << (U8(len));
        if(len)
            stream.write(string, len);
    }

    inline void writeLongString(std::iostream& stream, U32 maxStringLen, const char *string)
    {
        U32 len = dStrlen(string);
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

    inline int getStreamSize(std::istream& is)
    {
        is.seekg (0, is.end);
        int length = is.tellg();
        is.seekg (0, is.beg);
        return length;
    }

    inline void writeLine(std::ostream& os, const char *line)
    {
        os.write(line, dStrlen(line));
        os.write("\r\n", 2);
    }

}