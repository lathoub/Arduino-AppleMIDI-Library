/*!
 *  @file		AppleMIDI_ReceiverFeedback.h
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
#include "utility/AppleMidi_Util.h"

BEGIN_APPLEMIDI_NAMESPACE
	
class AppleMIDI_ReceiverFeedback {
public:
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint16_t	sequenceNr;
	uint16_t	dummy;

	AppleMIDI_ReceiverFeedback()
	{
		init();
	}

private:
	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amReceiverFeedback, sizeof(amReceiverFeedback));
	}

};

END_APPLEMIDI_NAMESPACE
