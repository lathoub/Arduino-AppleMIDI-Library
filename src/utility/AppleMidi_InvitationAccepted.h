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
	
class AppleMIDI_InvitationAccepted {
public:
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		name[16];

	AppleMIDI_InvitationAccepted()
	{
		init();
	}

	inline AppleMIDI_InvitationAccepted(uint32_t ssrc, uint32_t initiatorToken, char* name)
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationAccepted, sizeof(amInvitationAccepted));
		version = 2;

		this->initiatorToken = initiatorToken;
		this->ssrc           = ssrc;
		strcpy(this->name, static_cast<const char*>(name));
	}

private:
	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationAccepted, sizeof(amInvitationAccepted));
		version = 2;
	}

};

END_APPLEMIDI_NAMESPACE
