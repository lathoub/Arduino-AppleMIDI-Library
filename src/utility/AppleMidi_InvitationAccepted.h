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

typedef struct __attribute__((packed)) AppleMIDI_InvitationAccepted
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

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
		strncpy(this->sessionName, static_cast<const char*>(name), SESSION_NAME_MAX_LEN);
	}

	inline uint8_t getLength()
	{
		return sizeof(AppleMIDI_InvitationAccepted) - (SESSION_NAME_MAX_LEN) + strlen(sessionName);
	}

private:
	void init()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationAccepted, sizeof(amInvitationAccepted));
		version = 2;
	}

} AppleMIDI_InvitationAccepted_t;

END_APPLEMIDI_NAMESPACE

#ifdef __cpp
}
#endif

