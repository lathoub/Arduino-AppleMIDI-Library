/*!
 *  @file		AppleMIDI_ReceiverFeedback.h
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
	
typedef struct AppleMIDI_ReceiverFeedback {

	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint16_t	sequenceNr;
	uint16_t	dummy;

	AppleMIDI_ReceiverFeedback()
	{
		init();
	}

//	AppleMIDI_ReceiverFeedback(unsigned char* buffer, int length)
//	{
//		init();
//
//		this->ssrc           = AppleMIDI_Util::readUInt32(buffer + offsetof(struct AppleMIDI_ReceiverFeedback, ssrc));
////		this->sequenceNr     = AppleMIDI_Util::readUInt16(buffer + offsetof(struct AppleMIDI_ReceiverFeedback, sequenceNr));
//		this->sequenceNr     = AppleMIDI_Util::readUInt16(buffer + 8);
//	}

	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amReceiverFeedback, sizeof(amReceiverFeedback));
	}

	//static int findEndOfReceiverFeedback(unsigned char* buffer, int length)
	//{
	//	// check for minimum length
	//	if (length < 2 + 2 + 4 + 2 + 2)
	//		return -1;

	//	return 2 + 2 + 4 + 2 + 2;
	//}

	void write(EthernetUDP* udp)
	{
	}

} ReceiverFeedback_t;

END_APPLEMIDI_NAMESPACE
