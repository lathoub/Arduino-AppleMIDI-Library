/*!
 *  @file		AppleMIDI_AcceptInvitation.h
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
	
typedef struct AppleMIDI_AcceptInvitation {

	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		name[16];

	inline AppleMIDI_AcceptInvitation(uint32_t ssrc, uint32_t initiatorToken, char* name)
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amAcceptInvitation, sizeof(amAcceptInvitation));
		version = 2;

		this->initiatorToken = initiatorToken;
		this->ssrc           = ssrc;
		strcpy(this->name, (const char*)name);
	}

	void write(EthernetUDP* udp)
	{
		udp->beginPacket(udp->remoteIP(), udp->remotePort());

		udp->write(signature, sizeof(signature));
		udp->write(command,   sizeof(command));
		udp->write(AppleMIDI_Util::toBuffer(version), sizeof(version));
		udp->write(AppleMIDI_Util::toBuffer(initiatorToken), sizeof(initiatorToken));
		udp->write(AppleMIDI_Util::toBuffer(ssrc), sizeof(ssrc));
		udp->write((uint8_t*)name, strlen(name) + 1);

		udp->endPacket(); 
		udp->flush(); // Waits for the transmission of outgoing serial data to complete

#if (APPLEMIDI_DEBUG)
		Serial.print  ("< Invitation Accepted: peer = \"");
		Serial.print  (name);
		Serial.println("\"");
#endif
	}

} AcceptInvitation_t;

END_APPLEMIDI_NAMESPACE
