/*!
 *  @file		AppleMIDI_Syncronization.h
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

BEGIN_APPLEMIDI_NAMESPACE
	
typedef struct AppleMIDI_Syncronization {

	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint8_t		count;
	uint8_t		padding[3];
	Timeval_t	timestamps[3];

	AppleMIDI_Syncronization()
	{
		init();
	}

	AppleMIDI_Syncronization(uint32_t ssrc, uint8_t count, Timeval_t* ts)
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
		udp->write(AppleMIDI_Util::toBuffer(timestamps[0]), sizeof(Timeval_t));
		udp->write(AppleMIDI_Util::toBuffer(timestamps[1]), sizeof(Timeval_t));
		udp->write(AppleMIDI_Util::toBuffer(timestamps[2]), sizeof(Timeval_t));

		udp->endPacket(); 
		udp->flush(); // Waits for the transmission of outgoing serial data to complete
/*
#if (APPLEMIDI_DEBUG)
		Serial.print  ("< Syncronization: count = ");
		Serial.print  (count);
		Serial.print  (" Timestamps = ");
		Serial.print  (timestamps[0].tv_sec, HEX);
		Serial.print  (" ");
		Serial.print  (timestamps[0].tv_usec, HEX);
		Serial.print  (" ");
		Serial.print  (timestamps[1].tv_sec, HEX);
		Serial.print  (" ");
		Serial.print  (timestamps[1].tv_usec, HEX);
		Serial.print  (" ");
		Serial.print  (timestamps[2].tv_sec, HEX);
		Serial.print  (" ");
		Serial.print  (timestamps[2].tv_usec, HEX);
		Serial.println("");
#endif
*/
	}

} Syncronization_t;

END_APPLEMIDI_NAMESPACE
