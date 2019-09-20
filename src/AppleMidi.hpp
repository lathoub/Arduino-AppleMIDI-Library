BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ManageInvites()
{
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ManageTiming()
{
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedInvitation(const AppleMIDI_Invitation_t& invitation, const amPortType& portType)
{
	Serial.println(invitation.initiatorToken, HEX);
	Serial.println(invitation.ssrc, HEX);
	Serial.println(invitation.sessionName);

	if (portType == amPortType::Control)
		ReceivedControlInvitation(invitation);
	else
		ReceivedDataInvitation(invitation);
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedControlInvitation(const AppleMIDI_Invitation_t& invitation)
{
	Serial.println("ReceivedControlInvitation");

	// Do we know this ssrc already?
	// In case initiator reconnects (after a crash of some sort)
	int index = _sessionManager.GetSessionSlotUsingSSrc(invitation.ssrc);
	if (index < 0)
	{
		Serial.println("Session does not exist yet");

		// No, not existing; must be a new initiator
		// Find a free slot to remember this session in
		index = _sessionManager.GetFreeSessionSlot();
		if (index < 0)
		{
			Serial.println("No free slots");
			// no free slots, we cant accept invite
			//AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
			//write(_controlPort, invitationRejected, _controlPort.remoteIP(), _controlPort.remotePort());
			return;
		}

		// Initiate a session got this ssrc
		_sessionManager.CreateLocalSession(index, invitation.ssrc);
	}
	else
	{
		// session exists
	}

	auto success = _controlPort.beginPacket(_controlPort.remoteIP(), _controlPort.remotePort());

	uint32_t temp = toEndian(amProtocolVersion);
	_controlPort.write((uint8_t*)amSignature, sizeof(amSignature));
	_controlPort.write((uint8_t*)amInvitationAccepted, sizeof(amInvitationAccepted));
	_controlPort.write(reinterpret_cast<uint8_t*>(&temp), sizeof(amProtocolVersion));
	//_controlPort.write(reinterpret_cast<uint8_t*>(toEndian(_sessionManager.getSynchronizationSource())), sizeof(uint32_t));
	//_controlPort.write(reinterpret_cast<uint8_t*>(toEndian(invitation.initiatorToken)), sizeof(uint32_t));
	//_controlPort.write((uint8_t*)_sessionManager.getSessionName(), strlen(_sessionManager.getSessionName()));

	success = _controlPort.endPacket();
	_controlPort.flush();

	// Send the invitation acceptance packet
	//AppleMIDI_InvitationAccepted acceptInvitation(_sessionManager.getSynchronizationSource(), invitation.initiatorToken, _sessionManager.getSessionName());
	//write(_controlPort, acceptInvitation, _controlPort.remoteIP(), _controlPort.remotePort());
}

template<class UdpClass>
void AppleMidiTransport<UdpClass>::ReceivedDataInvitation(const AppleMIDI_Invitation_t& invitation)
{
	Serial.println("ReceivedDataInvitation");

	// Find the slot, it should be there because created by control session
	int i = _sessionManager.GetSessionSlotUsingSSrc(invitation.ssrc);
	if (i < 0)
	{
		//AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
		//write(_controlPort, invitationRejected, _controlPort.remoteIP(), _controlPort.remotePort());
		return;
	}

	//AppleMIDI_InvitationAccepted acceptInvitation(_sessionManager.getSynchronizationSource(), invitation.initiatorToken, _sessionManager.getSessionName());
	//write(_controlPort, acceptInvitation, _controlPort.remoteIP(), _controlPort.remotePort());

	// Send bitrate limit
	//AppleMIDI_BitrateReceiveLimit rateLimit;
	// write(_controlPort, rateLimit, _controlPort.remoteIP(), _controlPort.remotePort());

	//Sessions[i].contentIP = _dataPort.remoteIP();
	//Sessions[i].contentPort = _dataPort.remotePort();
	//Sessions[i].invite.status = None;
	//Sessions[i].syncronization.enabled = true; // synchronisation can start

	if (_connectedCallback != 0)
		_connectedCallback(invitation.ssrc, invitation.sessionName);
}

END_APPLEMIDI_NAMESPACE