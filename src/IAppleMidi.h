/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		RtpMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "IRtpMidi.h"

BEGIN_APPLEMIDI_NAMESPACE

class IAppleMidi : public IRtpMidi
{
public:
	virtual void invite(IPAddress ip, uint16_t port = CONTROL_PORT) = 0;

	virtual void OnInvitation(void* sender, AppleMIDI_Invitation&) = 0;
	virtual void OnEndSession(void* sender, AppleMIDI_EndSession&) = 0;
	virtual void OnReceiverFeedback(void* sender, AppleMIDI_ReceiverFeedback&) = 0;

	virtual void OnInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&) = 0;
	virtual void OnControlInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&) = 0;
	virtual void OnContentInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&) = 0;

	virtual void OnSyncronization(void* sender, AppleMIDI_Syncronization&) = 0;
	virtual void OnBitrateReceiveLimit(void* sender, AppleMIDI_BitrateReceiveLimit&) = 0;
	virtual void OnControlInvitation(void* sender, AppleMIDI_Invitation&) = 0;
	virtual void OnContentInvitation(void* sender, AppleMIDI_Invitation&) = 0;
};

END_APPLEMIDI_NAMESPACE
