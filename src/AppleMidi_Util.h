#pragma once
	
#include <inttypes.h>
#include <stdint.h>

namespace {

	static uint64_t readUInt64(byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h)
	{
		return	(uint64_t)a << 56 |
			(uint64_t)b << 48 |
			(uint64_t)c << 40 |
			(uint64_t)d << 32 |
			(uint64_t)e << 24 |
			(uint64_t)f << 16 |
			(uint64_t)g << 8 |
			(uint64_t)h;
	}

	static uint32_t readUInt32(byte a, byte b, byte c, byte d)
	{
		return	(uint32_t)a << 24 |
			(uint32_t)b << 16 |
			(uint32_t)c << 8 |
			(uint32_t)d;
	}

	static uint16_t readUInt16(byte a, byte b)
	{
		return	(uint32_t)a << 8 |
			(uint32_t)b;
	}

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

}