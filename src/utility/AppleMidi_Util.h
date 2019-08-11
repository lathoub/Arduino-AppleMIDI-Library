/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

BEGIN_APPLEMIDI_NAMESPACE
	
#include <inttypes.h>
#include <stdint.h>

class Debug
{
public:
#if defined(DEBUG_BUILD)

	#define DEBUGSTREAM Serial

	static void Assert(bool result, const char* message)
	{
		if (!result) DEBUGSTREAM.print(message);
	}
	static void Write(const char* message)
	{
		DEBUGSTREAM.print(message);
	}
	static void WriteLine(const char* message)
	{
		DEBUGSTREAM.println(message);
	}
#else
	static void Assert(bool, const char*) {}
	static void Write(const char*) {}
	static void WriteLine(const char*) {}
#endif
};

class AppleMIDI_Util {
public:
	static uint64_t readUInt64(unsigned char* buffer)
	{
		return	(uint64_t)buffer[0] << 56 |
				(uint64_t)buffer[1] << 48 |
				(uint64_t)buffer[2] << 40 |
				(uint64_t)buffer[3] << 32 |
				(uint64_t)buffer[4] << 24 |
				(uint64_t)buffer[5] << 16 |
				(uint64_t)buffer[6] << 8  |
				(uint64_t)buffer[7];
	}

	static uint32_t readUInt32(unsigned char* buffer)
	{
		return	(uint32_t)buffer[0] << 24 |
				(uint32_t)buffer[1] << 16 |
				(uint32_t)buffer[2] << 8  |
				(uint32_t)buffer[3];
	}

	static uint16_t readUInt16(unsigned char* buffer)
	{
		return	(uint32_t)buffer[0] << 8 |
				(uint32_t)buffer[1];
	}

	static uint8_t readUInt8(unsigned char* buffer)
	{
		return *((uint8_t*)&buffer[0]);
	}

	static uint8_t toEndian(uint8_t number)
	{
		return number;
	}

	static int8_t toEndian(int8_t number)
	{
		return number;
	}

	static uint16_t toEndian(uint16_t number)
	{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swap_uint16(number);
#else
		return number;
#endif
	}

	static int16_t toEndian(int16_t number)
	{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swap_int16(number);
#else
		return number;
#endif
	}

	static uint32_t toEndian(uint32_t number)
	{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swap_uint32(number);
#else
		return number;
#endif
	}

	static int32_t toEndian(int32_t number)
	{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swap_int32(number);
#else
		return number;
#endif
	}

	static uint64_t toEndian(uint64_t number)
	{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swap_uint64(number);
#else
		return number;
#endif
	}

	static int64_t toEndian(int64_t number)
	{
#if ( SYSTEM_ENDIAN == _ENDIAN_LITTLE_ )
		return swap_int64(number);
#else
		return number;
#endif
	}

private:

	//! Byte swap unsigned short
	static uint16_t swap_uint16(uint16_t val)
	{
		return (val << 8) | (val >> 8);
	}

	//! Byte swap short
	static int16_t swap_int16(int16_t val)
	{
		return (val << 8) | ((val >> 8) & 0xFF);
	}

	//! Byte swap unsigned int
	static uint32_t swap_uint32(uint32_t val)
	{
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
		return (val << 16) | (val >> 16);
	}

	//! Byte swap int
	static int32_t swap_int32(int32_t val)
	{
		val = ((val << 8) & 0xFF00FF00) | ((val >> 8) & 0xFF00FF);
		return (val << 16) | ((val >> 16) & 0xFFFF);
	}

	static uint64_t swap_uint64(uint64_t val)
	{
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
		return (val << 32) | (val >> 32);
	}

	static int64_t swap_int64(int64_t val)
	{
		val = ((val << 8) & 0xFF00FF00FF00FF00ULL) | ((val >> 8) & 0x00FF00FF00FF00FFULL);
		val = ((val << 16) & 0xFFFF0000FFFF0000ULL) | ((val >> 16) & 0x0000FFFF0000FFFFULL);
		return (val << 32) | ((val >> 32) & 0xFFFFFFFFULL);
	}

};

END_APPLEMIDI_NAMESPACE
