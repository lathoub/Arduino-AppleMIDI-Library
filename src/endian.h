#pragma once
	
#include <inttypes.h>
#include <stdint.h>

namespace {

	class Endian
	{
	private:
		static constexpr uint32_t uint32_ = 0x01020304;
		static constexpr uint8_t magic_ = (const uint8_t&)uint32_;
	public:
		static constexpr bool little = magic_ == 0x04;
		static constexpr bool middle = magic_ == 0x02;
		static constexpr bool big = magic_ == 0x01;
		static_assert(little || middle || big, "Cannot determine endianness!");
	private:
		Endian() = delete;
	};

	/* a=target variable, b=bit number to act upon 0-n */
	#define BIT_SET(a,b) ((a) |= (1ULL<<(b)))
	#define BIT_CLEAR(a,b) ((a) &= ~(1ULL<<(b)))
	#define BIT_FLIP(a,b) ((a) ^= (1ULL<<(b)))
	#define BIT_CHECK(a,b) (!!((a) & (1ULL<<(b))))        // '!!' to make sure this returns 0 or 1

	/* x=target variable, y=mask */
	#define BITMASK_SET(x,y) ((x) |= (y))
	#define BITMASK_CLEAR(x,y) ((x) &= (~(y)))
	#define BITMASK_FLIP(x,y) ((x) ^= (y))
	#define BITMASK_CHECK_ALL(x,y) (((x) & (y)) == (y))   // warning: evaluates y twice
	#define BITMASK_CHECK_ANY(x,y) ((x) & (y))

	static uint64_t ntohll(byte a, byte b, byte c, byte d, byte e, byte f, byte g, byte h)
	{
if (Endian::little)
		return	(uint64_t)a << 56 |
				(uint64_t)b << 48 |
				(uint64_t)c << 40 |
				(uint64_t)d << 32 |
				(uint64_t)e << 24 |
				(uint64_t)f << 16 |
				(uint64_t)g << 8 |
				(uint64_t)h;
else
		return	(uint64_t)h << 56 |
				(uint64_t)g << 48 |
				(uint64_t)f << 40 |
				(uint64_t)e << 32 |
				(uint64_t)d << 24 |
				(uint64_t)c << 16 |
				(uint64_t)b << 8 |
				(uint64_t)a;
	}

	static uint32_t ntohl(byte a, byte b, byte c, byte d)
	{
if (Endian::little)
		return	(uint32_t)a << 24 |
				(uint32_t)b << 16 |
				(uint32_t)c << 8 |
				(uint32_t)d;
else
		return	(uint32_t)d << 24 |
				(uint32_t)c << 16 |
				(uint32_t)b << 8 |
				(uint32_t)a;
	}

	static uint16_t ntohs(byte a, byte b)
	{
if (Endian::little)
		return	(uint32_t)a << 8 |
			    (uint32_t)b;
else
		return	(uint32_t)b << 8 |
			    (uint32_t)a;
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

	static uint16_t htons(uint16_t number)
	{
		if (Endian::little)
			return swap_uint16(number);
		else
			return number;
	}

	static int16_t htons(int16_t number)
	{
		if (Endian::little)
			return swap_int16(number);
		else
			return number;
	}

	static uint32_t htonl(uint32_t number)
	{
		if (Endian::little)
			return swap_uint32(number);
		else
			return number;
	}

	static int32_t htonl(int32_t number)
	{
		if (Endian::little)
			return swap_int32(number);
		else
			return number;
	}

	static uint64_t htonll(uint64_t number)
	{
		if (Endian::little)
			return swap_uint64(number);
		else
			return number;
	}

	static int64_t htonll(int64_t number)
	{
		if (Endian::little)
			return swap_int64(number);
		else
			return number;
	}

}