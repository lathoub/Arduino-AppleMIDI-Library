/*!
 *	@brief		RtpMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include "IMidiCallbacks.h"
#include "utility/AppleMidi_Defs.h"

BEGIN_APPLEMIDI_NAMESPACE

class IAppleMidiCallbacks : public IMidiCallbacks
{
public:
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
