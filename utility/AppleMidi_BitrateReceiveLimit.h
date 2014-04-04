/*!
 *  @file		AppleMIDI_BitrateReceiveLimit.h
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
	
typedef struct AppleMIDI_BitrateReceiveLimit {

	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	bitratelimit;

	AppleMIDI_BitrateReceiveLimit()
	{
		init();
	}

	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amBitrateReceiveLimit, sizeof(amBitrateReceiveLimit));
	}

	void write(EthernetUDP* udp)
	{
	}

} BitrateReceiveLimit_t;

END_APPLEMIDI_NAMESPACE
