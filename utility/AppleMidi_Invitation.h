/*!
 *  @file		AppleMIDI_Invitation.h
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
	
typedef struct AppleMIDI_Invitation {

	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[16];

	AppleMIDI_Invitation()
	{
		init();
	}

	void init()
	{
		memcpy(signature, amSignature,  sizeof(amSignature));
		memcpy(command,   amInvitation, sizeof(amInvitation));
		version = 2;
	}

	void write(IPAddress ip, uint16_t port, EthernetUDP* udp)
	{
		udp->beginPacket(ip, port);

		udp->write(signature, sizeof(signature));
		udp->write(command,   sizeof(command));
		udp->write(AppleMIDI_Util::toBuffer(version), sizeof(version));
		udp->write(AppleMIDI_Util::toBuffer(initiatorToken), sizeof(initiatorToken));
		udp->write(AppleMIDI_Util::toBuffer(ssrc), sizeof(ssrc));
		udp->write((uint8_t*)sessionName, strlen(sessionName) + 1);

		udp->endPacket(); 
		udp->flush(); // Waits for the transmission of outgoing serial data to complete
	}

} Invitation_t;

END_APPLEMIDI_NAMESPACE
