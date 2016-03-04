#pragma once

#ifndef __streamfn_h__
#define __streamfn_h__

#include "platform/platform.h"
#include "console/consoleObject.h"
#include <iostream>
#include <bitset>
#include <vector>

namespace StreamFn {

	inline bool readFlag(std::istream& is)
	{
		bool val;
		is >> val;
		return val;
	}

	inline bool writeFlag(std::ostream& os, bool val)
	{
		os << val;
		return val;
	}

	inline void readString(std::istream& is, char buf[256])
	{
		U8 len = 0;
		is >> len;
		is.read(buf, len);
		buf[len] = 0;
	}

	inline U32 getReadByteSize(std::istream& is)
	{
		is.seekg(0, is.end);
		U32 ret = (U32)is.tellg();
		is.seekg(0, is.beg);
		return ret;
	}

	inline void writeString(std::ostream& stream, const char* string, U32 maxLen = 255)
	{
		size_t len = string ? dStrlen(string) : 0;
		if (len > maxLen)
			len = maxLen;

		stream << (U8(len));
		if (len)
			stream.write(string, len);
	}


	inline void readLongString(std::istream& stream, size_t maxStringLen, char *stringBuf)
	{
		U32 len;
		stream >> len;
		if (len > maxStringLen)
		{
			return;
		}
		stream.read(stringBuf, len);
		stringBuf[len] = 0;
	}

	inline void writeLongString(std::ostream& stream, size_t maxStringLen, const char *string)
	{
		size_t len = dStrlen(string);
		if (len > maxStringLen)
			len = maxStringLen;
		stream << (len);
		stream.write(string, len);
	}

	/// Write a number of tabs to this stream
	inline void writeTabs(std::ostream& stream, U32 count)
	{
		char tab[] = "   ";
		while (count--)
			stream.write(tab, 3);
	}

	inline size_t getStreamSize(std::istream& is)
	{
		auto temp = is.tellg();
		is.seekg(0, is.end);
		size_t length = (size_t)is.tellg();
		is.seekg(0, is.beg + temp);
		return length;
	}

	inline void writeLine(std::ostream& os, const char *line)
	{
		os.write(line, dStrlen(line));
		os.write("\r\n", 2);
	}

	inline U32 readInt(std::istream& is, dsize_t size)
	{
		std::bitset<32> bitset;
		for (U32 i = 0; i < size; i++)
		{
			bool bit;
			is >> bit;
			bitset[i] = bit;
		}
		return (U32)bitset.to_ulong();
	}

	inline void writeInt(std::ostream& os, U32 val, dsize_t bitcount)
	{
		std::bitset<32> bitset(val);
		for (U32 i = 0; i < bitcount; i++)
			os << bitset[i];
	}

	inline U32 readRangedU32(std::istream& is, U32 rangeStart, U32 rangeEnd)
	{
		AssertFatal(rangeEnd >= rangeStart, "error, end of range less than start");

		U32 rangeSize = rangeEnd - rangeStart + 1;
		U32 rangeBits = getBinLog2(getNextPow2(rangeSize));

		U32 val = U32(readInt(is, rangeBits));
		return val + rangeStart;
	}

	inline void writeRangedU32(std::ostream &os, U32 value, size_t rangeStart, size_t rangeEnd)
	{
		AssertFatal(value >= rangeStart && value <= rangeEnd, "Out of bounds value!");
		AssertFatal(rangeEnd >= rangeStart, "error, end of range less than start");

		U32 rangeSize = (U32)(rangeEnd - rangeStart + 1);
		U32 rangeBits = getBinLog2(getNextPow2(rangeSize));

		writeInt(os, (U32)(value - rangeStart), rangeBits);
	}

	inline S32 readClassId(std::istream& is, U32 classType, U32 classGroup)
	{
		AssertFatal(classType < NetClassTypesCount, "Out of range class type.");
		S32 ret = readInt(is, AbstractClassRep::NetClassBitSize[classGroup][classType]);
		if (ret > AbstractClassRep::NetClassCount[classGroup][classType])
			return -1;
		return ret;
	}

	inline void writeClassId(std::ostream& os, U32 classId, U32 classType, U32 classGroup)
	{
		AssertFatal(classType < NetClassTypesCount, "Out of range class type.");
		AssertFatal(classId < AbstractClassRep::NetClassCount[classGroup][classType], "Out of range class id.");
		writeInt(os, classId, AbstractClassRep::NetClassBitSize[classGroup][classType]);
	}

	inline bool streamCopy(std::iostream& toStream, std::iostream& fromStream)
	{
		size_t buffSize = getStreamSize(fromStream);
		std::vector<char> buffer(buffSize);
		fromStream.read(buffer.data(), buffSize);
		toStream.write(buffer.data(), buffSize);
		return (toStream.good());
	}

	StringTableEntry readSTString(std::iostream& iostream, bool casesens)
	{
		char buf[256];
		readString(iostream, buf);
		return StringTable->insert(buf, casesens);
	}

	inline U32 getPosition(std::iostream& iostream)
	{
		return (U32)iostream.tellg();
	}

	inline bool setPosition(std::iostream& iostream, U32 pos)
	{
		iostream.seekg(iostream.beg + pos);
		iostream.seekp(iostream.beg + pos);
		return iostream.good();
	}


}

#endif