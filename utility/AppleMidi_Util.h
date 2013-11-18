/*!
 *  @file		AppleMIDI_Util.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.0
 *  @author		lathoub  
 *	@date		01/04/13
 *  License		GPL
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

BEGIN_APPLEMIDI_NAMESPACE
	
class AppleMIDI_Util {
public:
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

	static Timeval_t readTimeval(unsigned char* buffer)
	{
		unsigned char tmpBuffer[sizeof(Timeval_t)];
		for (int j = sizeof(Timeval_t) - 1; j >= 0; j--)
			tmpBuffer[j] = buffer[7-j];

		Timeval_t timeval;
		timeval.tv_usec = *((uint32_t*)&tmpBuffer[0]);
		timeval.tv_sec  = *((uint32_t*)&tmpBuffer[4]);

		return timeval;
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

	static uint8_t* toBuffer(Timeval_t timeval)
	{
		uint8_t buffer[sizeof(Timeval_t)];

		buffer[0] = (timeval.tv_sec >> 24) & 0xFF;
		buffer[1] = (timeval.tv_sec >> 16) & 0xFF;
		buffer[2] = (timeval.tv_sec >>  8) & 0xFF;
		buffer[3] = (timeval.tv_sec >>  0) & 0xFF;

		buffer[4] = (timeval.tv_usec >> 24) & 0xFF;
		buffer[5] = (timeval.tv_usec >> 16) & 0xFF;
		buffer[6] = (timeval.tv_usec >>  8) & 0xFF;
		buffer[7] = (timeval.tv_usec >>  0) & 0xFF;

		return buffer;
	}

};

END_APPLEMIDI_NAMESPACE
