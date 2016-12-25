/*!
 *  @file		AppleMIDI_BitrateReceiveLimit.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub, hackmancoltaire
 *	@date		04/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"
#include "utility/AppleMidi_Util.h"

#ifdef __cpp
extern "C" {
#endif

BEGIN_APPLEMIDI_NAMESPACE

typedef struct __attribute__((packed)) AppleMIDI_BitrateReceiveLimit
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint32_t	bitratelimit;

	AppleMIDI_BitrateReceiveLimit()
	{
		init();
	}

private:
	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amBitrateReceiveLimit, sizeof(amBitrateReceiveLimit));
		bitratelimit = PACKET_MAX_SIZE;
	}
} AppleMIDI_BitrateReceiveLimit_t;

END_APPLEMIDI_NAMESPACE

#ifdef __cpp
}
#endif
