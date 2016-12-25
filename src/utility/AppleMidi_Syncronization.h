/*!
 *  @file		AppleMIDI_Syncronization.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub, chris-zen
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

typedef struct __attribute__((packed)) AppleMIDI_Syncronization
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint8_t		count;
	uint8_t		padding[3];
	uint64_t	timestamps[3];

	AppleMIDI_Syncronization()
	{
		init();
	}

	AppleMIDI_Syncronization(uint32_t ssrc, uint8_t count, uint64_t* ts)
	{
		init();

		this->ssrc  = ssrc;
		this->count = count;
		this->timestamps[0] = ts[0];
		this->timestamps[1] = ts[1];
		this->timestamps[2] = ts[2];
	}

private:
	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amSyncronization, sizeof(amSyncronization));
		memset(padding, 0, sizeof(padding));
	}
} AppleMIDI_Syncronization_t;

END_APPLEMIDI_NAMESPACE

#ifdef __cpp
}
#endif
