/*!
 *  @file		AppleMIDI_AcceptInvitation.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.0
 *  @author		lathoub 
 *	@date		01/04/13
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

#include "utility/AppleMidi_Util.h"

BEGIN_APPLEMIDI_NAMESPACE
	
class AppleMIDI_InvitationRejected {
public:
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[16];

	AppleMIDI_InvitationRejected()
	{
		init();
	}

	inline AppleMIDI_InvitationRejected(uint32_t ssrc, uint32_t initiatorToken, char* sessionName)
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationRejected, sizeof(amInvitationRejected));
		version = 2;

		this->initiatorToken = initiatorToken;
		this->ssrc           = ssrc;
		strcpy(this->sessionName, static_cast<const char*>(sessionName));
	}

private:

	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationRejected, sizeof(amInvitationRejected));
		version = 2;
	}

};

END_APPLEMIDI_NAMESPACE
