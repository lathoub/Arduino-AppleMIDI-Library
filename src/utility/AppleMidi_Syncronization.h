/*!
 *  @file		AppleMIDI_Syncronization.h
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
#include "utility/AppleMidi_Util.h"

BEGIN_APPLEMIDI_NAMESPACE
	
typedef struct AppleMIDI_Syncronization {

	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint8_t		count;
	uint8_t		padding[3];
	int64_t		timestamps[3];

	AppleMIDI_Syncronization()
	{
		init();
	}

	AppleMIDI_Syncronization(uint32_t ssrc, uint8_t count, int64_t* ts)
	{
		init();

		this->ssrc  = ssrc;
		this->count = count;
		this->timestamps[0] = ts[0];
		this->timestamps[1] = ts[1];
		this->timestamps[2] = ts[2];
	}

	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amSyncronization, sizeof(amSyncronization));
	}

	void write(EthernetUDP* udp)
	{
		udp->beginPacket(udp->remoteIP(), udp->remotePort());

		udp->write(signature, sizeof(signature));
		udp->write(command,   sizeof(command));
		
		// To appropriate endian conversion
		uint32_t _ssrc = AppleMIDI_Util::toEndian(ssrc);
		uint8_t _count = AppleMIDI_Util::toEndian(count);
		uint8_t _zero = 0;
		int64_t _ts0 = AppleMIDI_Util::toEndian(timestamps[0]);
		int64_t _ts1 = AppleMIDI_Util::toEndian(timestamps[1]);
		int64_t _ts2 = AppleMIDI_Util::toEndian(timestamps[2]);

		// write then out
		udp->write((uint8_t*) ((void*) (&_ssrc)), sizeof(_ssrc));
		udp->write((uint8_t*) ((void*) (&_count)), sizeof(_count));
		udp->write((uint8_t*) ((void*) (&_zero)), sizeof(_zero));
		udp->write((uint8_t*) ((void*) (&_zero)), sizeof(_zero));
		udp->write((uint8_t*) ((void*) (&_zero)), sizeof(_zero));
		udp->write((uint8_t*) ((void*) (&_ts0)), sizeof(_ts0));
		udp->write((uint8_t*) ((void*) (&_ts1)), sizeof(_ts1));
		udp->write((uint8_t*) ((void*) (&_ts2)), sizeof(_ts2));

		udp->endPacket(); 
		udp->flush(); // Waits for the transmission of outgoing serial data to complete
	}

} Syncronization_t;

END_APPLEMIDI_NAMESPACE
