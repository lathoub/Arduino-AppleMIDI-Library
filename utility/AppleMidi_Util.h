/*!
 *  @file		AppleMIDI_Util.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		GPL
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

BEGIN_APPLEMIDI_NAMESPACE
	
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

	static uint8_t* toBuffer(uint8_t number)
	{
		uint8_t buffer[sizeof(uint8_t)];

		buffer[0] = (number >> 0) & 0xFF;

		return buffer;
	}

	static uint8_t* toBuffer(uint16_t number)
	{
		uint8_t buffer[sizeof(uint16_t)];

		buffer[0] = (number >>  8) & 0xFF;
		buffer[1] = (number >>  0) & 0xFF;

		return buffer;
	}

	static uint8_t* toBuffer(uint32_t number)
	{
		uint8_t buffer[sizeof(uint32_t)];

		buffer[0] = (number >> 24) & 0xFF;
		buffer[1] = (number >> 16) & 0xFF;
		buffer[2] = (number >>  8) & 0xFF;
		buffer[3] = (number >>  0) & 0xFF;

		return buffer;
	}

	static uint8_t* toBuffer(int64_t number)
	{
		uint8_t buffer[sizeof(int64_t)];

		buffer[0] = (number >> 56) & 0xFF;
		buffer[1] = (number >> 48) & 0xFF;
		buffer[2] = (number >> 40) & 0xFF;
		buffer[3] = (number >> 32) & 0xFF;
		buffer[4] = (number >> 24) & 0xFF;
		buffer[5] = (number >> 16) & 0xFF;
		buffer[6] = (number >>  8) & 0xFF;
		buffer[7] = (number >>  0) & 0xFF;

		return buffer;
	}

};

END_APPLEMIDI_NAMESPACE
