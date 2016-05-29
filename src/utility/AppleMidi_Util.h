/*!
 *  @file		AppleMIDI_Util.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
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
	static void Assert(bool result, const char* message)
	{
		if (!result) Serial.print(message);
	}
	static void Write(const char* message)
	{
		Serial.print(message);
	}
	static void WriteLine(const char* message)
	{
		Serial.println(message);
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
		unsigned char tmpBuffer[sizeof(uint64_t)];
		for (int j = sizeof(uint64_t) - 1; j >= 0; j--)
			tmpBuffer[j] = buffer[sizeof(uint64_t) - 1 - j];
		return *((uint64_t*)&tmpBuffer[0]);
	}

	static uint32_t readUInt32(unsigned char* buffer)
	{
		unsigned char tmpBuffer[sizeof(uint32_t)];
		for (int j = sizeof(uint32_t) - 1; j >= 0; j--)
			tmpBuffer[j] = buffer[sizeof(uint32_t) - 1 - j];
		return *((uint32_t*)&tmpBuffer[0]);
	}

	static uint16_t readUInt16(unsigned char* buffer)
	{
		unsigned char tmpBuffer[sizeof(uint16_t)];
		for (int j = sizeof(uint16_t) - 1; j >= 0; j--)
			tmpBuffer[j] = buffer[sizeof(uint16_t) - 1 - j];
		return *((uint16_t*)&tmpBuffer[0]);
	}

	static uint8_t readUInt8(unsigned char* buffer)
	{
		return *((uint8_t*)&buffer[0]);
	}

	//static uint8_t* toBuffer(uint8_t number)
	//{
	//	uint8_t* buffer = (uint8_t*) malloc(4);

	//	buffer[0] = (number >> 0) & 0xFF;

	//	return buffer;
	//}

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
