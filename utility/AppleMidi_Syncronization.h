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
		udp->write(AppleMIDI_Util::toBuffer(ssrc), sizeof(ssrc));
		udp->write(AppleMIDI_Util::toBuffer(count), sizeof(count));
		udp->write(AppleMIDI_Util::toBuffer((uint8_t)0), sizeof(uint8_t));
		udp->write(AppleMIDI_Util::toBuffer((uint8_t)0), sizeof(uint8_t));
		udp->write(AppleMIDI_Util::toBuffer((uint8_t)0), sizeof(uint8_t));
		udp->write(AppleMIDI_Util::toBuffer(timestamps[0]), sizeof(int64_t));
		udp->write(AppleMIDI_Util::toBuffer(timestamps[1]), sizeof(int64_t));
		udp->write(AppleMIDI_Util::toBuffer(timestamps[2]), sizeof(int64_t));

		udp->endPacket(); 
		udp->flush(); // Waits for the transmission of outgoing serial data to complete
	}

} Syncronization_t;

END_APPLEMIDI_NAMESPACE
