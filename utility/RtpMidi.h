/*!
 *  @file		RtpMidi.h
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

#include "utility/AppleMidi_Util.h"

#define PAYLOADTYPE_RTPMIDI 97

BEGIN_APPLEMIDI_NAMESPACE
	
typedef struct RtpMidi {
	uint8_t		ddddd;
	uint8_t		playLoadType;
	uint16_t	sequenceNr;
	uint32_t	timestamp;
	uint32_t	ssrc;

	RtpMidi()
	{
		ddddd = 0b10000000; // TODO
		playLoadType = PAYLOADTYPE_RTPMIDI;
	}

	void beginWrite(EthernetUDP* udp)
	{
		udp->beginPacket(udp->remoteIP(), udp->remotePort());

		udp->write(&ddddd, sizeof(ddddd));
		udp->write(&playLoadType,   sizeof(playLoadType));
		udp->write(AppleMIDI_Util::toBuffer(sequenceNr), sizeof(sequenceNr));
		udp->write(AppleMIDI_Util::toBuffer(timestamp), sizeof(timestamp));
		udp->write(AppleMIDI_Util::toBuffer(ssrc), sizeof(ssrc));
	}

	void endWrite(EthernetUDP* udp)
	{
		udp->endPacket(); 
		udp->flush(); // Waits for the transmission of outgoing serial data to complete
	}

} RtpMidi_t;

END_APPLEMIDI_NAMESPACE

