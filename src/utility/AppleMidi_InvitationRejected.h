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

#ifdef __cpp
extern "C" {
#endif

BEGIN_APPLEMIDI_NAMESPACE
	
typedef struct __attribute__((packed)) AppleMIDI_InvitationRejected
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

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
		strncpy(this->sessionName, static_cast<const char*>(sessionName), SESSION_NAME_MAX_LEN);
	}

private:

	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationRejected, sizeof(amInvitationRejected));
		version = 2;
	}

} AppleMIDI_InvitationRejected_t;

END_APPLEMIDI_NAMESPACE

#ifdef __cpp
}
#endif
