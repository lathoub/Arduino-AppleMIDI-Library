/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire, chris-zen
 */

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief Call the begin method in the setup() function of the Arduino.

All parameters are set to their default values:
- Input channel set to 1 if no value is specified
- Full thru mirroring
*/
template<class UdpClass, class Settings>
inline bool AppleMidiInterface<UdpClass, Settings>::begin(const char* sessionName, uint16_t port)
{
#if (APPLEMIDI_DEBUG)
	if (strlen(sessionName) > SESSION_NAME_MAX_LEN)
	{
		DEBUGSTREAM.print(F("SessionName exceeds "));
		DEBUGSTREAM.print(sessionName);
		DEBUGSTREAM.print(" exceeds ");
		DEBUGSTREAM.print(SESSION_NAME_MAX_LEN);
		DEBUGSTREAM.println(F(" chars. Name will be clipped."));
	}
#endif

	strncpy(_sessionName, sessionName, SESSION_NAME_MAX_LEN);

	Port = port;

	// open UDP socket for control messages
	_controlPort.begin(Port);
	// open UDP socket for rtp messages
	_dataPort.begin(Port + 1);

	_controlPortDissector.init(Port, this);
	_controlPortDissector.addPacketDissector(&PacketAppleMidi::dissect_apple_midi);	// Add parser

	_dataPortDissector.init(Port + 1, this);
	_dataPortDissector.addPacketDissector(&PacketRtpMidi::dissect_rtp_midi);		// Add parser
	_dataPortDissector.addPacketDissector(&PacketAppleMidi::dissect_apple_midi);	// Add parser

	_rtpMidi.ssrc = 0; // will be initialized when first used
	_rtpMidi.sequenceNr = 1;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("Starting Session "));
	DEBUGSTREAM.print(_sessionName);
	DEBUGSTREAM.print(F(" on port "));
	DEBUGSTREAM.println(Port);
#endif

	return true;
}

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::write(UdpClass& udp, AppleMIDI_InvitationRejected ir, IPAddress ip, uint16_t port)
{
	int success = udp.beginPacket(ip, port);
	Debug::Assert(success, "udp.beginPacket failed");

	// To appropriate endian conversion
	ir.version = AppleMIDI_Util::toEndian(ir.version);
	ir.initiatorToken = AppleMIDI_Util::toEndian(ir.initiatorToken);
	ir.ssrc = AppleMIDI_Util::toEndian(ir.ssrc);

	size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&ir), sizeof(ir));
	Debug::Assert(bytesWritten == sizeof(ir), "error writing ir");

	success = udp.endPacket();
	Debug::Assert(success, "udp.endPacket failed");
	udp.flush();
}

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::write(UdpClass& udp, AppleMIDI_InvitationAccepted ia, IPAddress ip, uint16_t port)
{
	int success = udp.beginPacket(ip, port);
	Debug::Assert(success, "udp.beginPacket failed");

	// Set the correct endian
	ia.version = AppleMIDI_Util::toEndian(ia.version);
	ia.initiatorToken = AppleMIDI_Util::toEndian(ia.initiatorToken);
	ia.ssrc = AppleMIDI_Util::toEndian(ia.ssrc);

	size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&ia), ia.getLength());
	Debug::Assert(bytesWritten == ia.getLength(), "error writing ia");

	success = udp.endPacket();
	Debug::Assert(success, "udp.endPacket failed");
	udp.flush();
}

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::write(UdpClass& udp, AppleMIDI_Syncronization sy, IPAddress ip, uint16_t port)
{
	int success = udp.beginPacket(ip, port);
	Debug::Assert(success, "udp.beginPacket failed");

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("Syncronization timestamps: 0x");
	DEBUGSTREAM.print((uint32_t)sy.timestamps[0], HEX);
	DEBUGSTREAM.print(", 0x");
	DEBUGSTREAM.print((uint32_t)sy.timestamps[1], HEX);
	DEBUGSTREAM.print(", 0x");
	DEBUGSTREAM.print((uint32_t)sy.timestamps[2], HEX);
	DEBUGSTREAM.println("");
#endif

	sy.ssrc = AppleMIDI_Util::toEndian(sy.ssrc);
	sy.count = AppleMIDI_Util::toEndian(sy.count);
	sy.timestamps[0] = AppleMIDI_Util::toEndian(sy.timestamps[0]);
	sy.timestamps[1] = AppleMIDI_Util::toEndian(sy.timestamps[1]);
	sy.timestamps[2] = AppleMIDI_Util::toEndian(sy.timestamps[2]);

	size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&sy), sizeof(sy));
	Debug::Assert(bytesWritten == sizeof(sy), "error writing sy");

	success = udp.endPacket();
	Debug::Assert(success, "udp.endPacket failed");
	udp.flush();
}

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::write(UdpClass& udp, AppleMIDI_Invitation in, IPAddress ip, uint16_t port)
{
	int success = udp.beginPacket(ip, port);
	Debug::Assert(success, "udp.beginPacket failed");

	// To appropriate endian conversion
	in.version = AppleMIDI_Util::toEndian(in.version);
	in.initiatorToken = AppleMIDI_Util::toEndian(in.initiatorToken);
	in.ssrc = AppleMIDI_Util::toEndian(in.ssrc);

	size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&in), in.getLength());
	Debug::Assert(bytesWritten == in.getLength(), "error writing in");

	success = udp.endPacket();
	Debug::Assert(success, "udp.endPacket failed");
	udp.flush();
}

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::write(UdpClass& udp, AppleMIDI_BitrateReceiveLimit in, IPAddress ip, uint16_t port)
{
	int success = udp.beginPacket(ip, port);
	Debug::Assert(success, "udp.beginPacket failed");

	// To appropriate endian conversion
	in.ssrc = AppleMIDI_Util::toEndian(in.ssrc);
	in.bitratelimit = AppleMIDI_Util::toEndian(in.bitratelimit);

	size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&in), sizeof(in));
	Debug::Assert(bytesWritten == sizeof(in), "error writing in");

	success = udp.endPacket();
	Debug::Assert(success, "udp.endPacket failed");
	udp.flush();
}

/*! \brief Evaluates incoming Rtp messages.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::read()
{
	// Process one control packet, if available
	int packetSize = _controlPort.parsePacket();
	if (packetSize) {
		packetSize = _controlPort.read(_packetBuffer, sizeof(_packetBuffer));
		_controlPortDissector.addPacket(_packetBuffer, packetSize);
	}

	// Process one control packet, if available
	packetSize = _dataPort.parsePacket();
	if (packetSize) {
		packetSize = _dataPort.read(_packetBuffer, sizeof(_packetBuffer));
		_dataPortDissector.addPacket(_packetBuffer, packetSize);
	}

#ifdef APPLEMIDI_REMOTE_SESSIONS
	// resend invitations
	ManageInvites();

	// do syncronization here
	ManageTiming();
#endif
}

/*! \brief Get Synchronization Source, initiatize the SSRC on first time usage (lazy init).
*/
template<class UdpClass, class Settings>
inline uint32_t AppleMidiInterface<UdpClass, Settings>::getSynchronizationSource()
{
	if (0 == _ssrc) // _ssrc initialized to 0 in constructor
	{
		// A call randonSeed is mandatory, with millis as a seed.
		// The time between booting and needing the SSRC for the first time (first network traffic) is
		// a good enough random seed.
		long seed = (long)micros();
		randomSeed(seed);

		// not full range of UINT32_MAX (unsigned!), but (signed) long should suffice
		_ssrc = random(1, INT32_MAX);

#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.print(F("Lazy init of SSRC. Value is 0x"));
		DEBUGSTREAM.println(_ssrc, HEX);
#endif
	}
	return _ssrc;
}

/*! \brief The Arduino initiates the session.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::invite(IPAddress ip, uint16_t port)
{
	CreateRemoteSession(ip, port);

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.println(F("Queued invite"));
#endif
}

/*! \brief The Arduino is being invited to a session.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::OnInvitation(void* sender, AppleMIDI_Invitation& invitation)
{
	Dissector* dissector = (Dissector*) sender;

	if (dissector->_identifier == Port)
		OnControlInvitation(sender, invitation);
	if (dissector->_identifier == Port + 1)
		OnContentInvitation(sender, invitation);
}

/*! \brief The session has been ended by the remote source.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::OnEndSession(void* sender, AppleMIDI_EndSession& sessionEnd)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> End Session for ssrc 0x"));
	DEBUGSTREAM.print(sessionEnd.ssrc, HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	DEBUGSTREAM.print(F(", initiatorToken = 0x"));
	DEBUGSTREAM.print(sessionEnd.initiatorToken, HEX);
#endif
	DEBUGSTREAM.println();
#endif

	DeleteSession(sessionEnd.ssrc);

	if (mDisconnectedCallback != 0)
		mDisconnectedCallback(sessionEnd.ssrc);
}

/* \brief With the receiver feedback packet, the recipient can tell the sender up to what sequence
* number in the RTP-stream the packets have been received; this can be used to shorten the
* recovery-journal-section in the RTP-session */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::OnReceiverFeedback(void* sender, AppleMIDI_ReceiverFeedback& receiverFeedback)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Receiver Feedback: seq = "));
	DEBUGSTREAM.println(receiverFeedback.sequenceNr);
#endif
}

/*! \brief The invitation that we have sent, has been accepted.
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
{
	Dissector* dissector = (Dissector*) sender;

	if (dissector->_identifier == Port)
		OnControlInvitationAccepted(sender, invitationAccepted);
	if (dissector->_identifier == Port + 1)
		OnContentInvitationAccepted(sender, invitationAccepted);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnControlInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> (OnControlInvitationAccepted) Control InvitationAccepted: peer = \""));
	DEBUGSTREAM.print(invitationAccepted.sessionName);
	DEBUGSTREAM.print("\"");
	DEBUGSTREAM.print(F(" ,ssrc 0x"));
	DEBUGSTREAM.print(invitationAccepted.ssrc, HEX);
	DEBUGSTREAM.print(F(" ,initiatorToken = 0x"));
	DEBUGSTREAM.print(invitationAccepted.initiatorToken, HEX);
	DEBUGSTREAM.println();
#endif

	CompleteLocalSessionControl(invitationAccepted);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnContentInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Content InvitationAccepted: peer = \""));
	DEBUGSTREAM.print(invitationAccepted.sessionName);
	DEBUGSTREAM.print("\"");
	DEBUGSTREAM.print(F(" ,ssrc 0x"));
	DEBUGSTREAM.print(invitationAccepted.ssrc, HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	DEBUGSTREAM.print(F(" ,initiatorToken = 0x"));
	DEBUGSTREAM.print(invitationAccepted.initiatorToken, HEX);
#endif
	DEBUGSTREAM.println();
#endif

	CompleteLocalSessionContent(invitationAccepted);
}

/*! \brief Part 1 of being invited into a session, the Control Invitation.
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnControlInvitation(void* sender, AppleMIDI_Invitation& invitation)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> (OnControlInvitation) Control Invitation: peer = \""));
	DEBUGSTREAM.print(invitation.sessionName);
	DEBUGSTREAM.print("\"");
	DEBUGSTREAM.print(F(" ,ssrc 0x"));
	DEBUGSTREAM.print(invitation.ssrc, HEX);
	DEBUGSTREAM.print(F(" ,initiatorToken = 0x"));
	DEBUGSTREAM.print(invitation.initiatorToken, HEX);
	DEBUGSTREAM.println();
#endif

	// Do we know this ssrc already?
	// In case initiator reconnects (after a crash of some sort)
	int index = GetSessionSlotUsingSSrc(invitation.ssrc);
	if (index < 0)
	{
		#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.println(F("Invition received for a new session."));
		#endif

		// No, not existing; must be a new initiator
		// Find a free slot to remember this session in
		index = GetFreeSessionSlot();
		if (index < 0)
		{
			#if (APPLEMIDI_DEBUG)
			DEBUGSTREAM.println(F("Session invitation rejected."));
			#endif

			// no free slots, we cant accept invite
			AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
			write(_controlPort, invitationRejected, _controlPort.remoteIP(), _controlPort.remotePort());

			return;
		} else {
			// Initiate a session got this ssrc
			CreateLocalSession(index, invitation.ssrc);
		}
	} else {
		#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.println(F("Session exists"));
		#endif
	}

	// Send the invitation acceptance packet
	AppleMIDI_InvitationAccepted acceptInvitation(getSynchronizationSource(), invitation.initiatorToken, getSessionName());
	write(_controlPort, acceptInvitation, _controlPort.remoteIP(), _controlPort.remotePort());

	#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.print(F("< (OnControlInvitation) Control InvitationAccepted: peer = \""));
		DEBUGSTREAM.print(getSessionName());
		DEBUGSTREAM.print("\"");
		DEBUGSTREAM.print(" ,ssrc 0x");
		DEBUGSTREAM.print(getSynchronizationSource(), HEX);
		DEBUGSTREAM.print(" ,initiatorToken = 0x");
		DEBUGSTREAM.print(invitation.initiatorToken, HEX);
	#if (APPLEMIDI_DEBUG_VERBOSE)
		DEBUGSTREAM.print(" ,in slot = ");
		DEBUGSTREAM.print(index);
	#endif
		DEBUGSTREAM.println();
	#endif
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnContentInvitation(void* sender, AppleMIDI_Invitation& invitation)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Content Invitation: peer = \""));
	DEBUGSTREAM.print(invitation.sessionName);
	DEBUGSTREAM.print("\"");
	DEBUGSTREAM.print(" ,ssrc 0x");
	DEBUGSTREAM.print(invitation.ssrc, HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	DEBUGSTREAM.print(F(" ,initiatorToken = 0x"));
	DEBUGSTREAM.print(invitation.initiatorToken, HEX);
#endif
	DEBUGSTREAM.println();
#endif

	// Find the slot, it should be there because created by control session
	int i = GetSessionSlotUsingSSrc(invitation.ssrc);
	if (i < 0)
	{
#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.print(F("Error - control session does not exists for "));
		DEBUGSTREAM.print(invitation.ssrc, HEX);
		DEBUGSTREAM.print(F(". Rejecting invitation."));
#endif
		AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
		write(_dataPort, invitationRejected, _dataPort.remoteIP(), _dataPort.remotePort());

		return;
	}

	AppleMIDI_InvitationAccepted acceptInvitation(getSynchronizationSource(), invitation.initiatorToken, getSessionName());
	write(_dataPort, acceptInvitation, _dataPort.remoteIP(), _dataPort.remotePort());

	// Send bitrate limit
	AppleMIDI_BitrateReceiveLimit rateLimit;
	write(_controlPort, rateLimit, _controlPort.remoteIP(), _controlPort.remotePort());

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("< Content InvitationAccepted: peer = \""));
	DEBUGSTREAM.print(getSessionName());
	DEBUGSTREAM.print("\"");
	DEBUGSTREAM.print(" ,ssrc 0x");
	DEBUGSTREAM.print(getSynchronizationSource(), HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	DEBUGSTREAM.print(F(" ,initiatorToken = 0x"));
	DEBUGSTREAM.print(invitation.initiatorToken, HEX);
	DEBUGSTREAM.print(" ,in slot = ");
	DEBUGSTREAM.print(i);
#endif
	DEBUGSTREAM.println();
#endif

	Sessions[i].contentIP = _dataPort.remoteIP();
	Sessions[i].contentPort = _dataPort.remotePort();
	Sessions[i].invite.status = None;
	Sessions[i].syncronization.enabled = true; // synchronisation can start

	if (mConnectedCallback != 0)
		mConnectedCallback(invitation.ssrc, invitation.sessionName);
}

/*! \brief .

From: http://en.wikipedia.org/wiki/RTP_MIDI

The session initiator sends a first message (named CK0) to the remote partner, giving its local time on
64 bits (Note that this is not an absolute time, but a time related to a local reference, generally given
in microseconds since the startup of operating system kernel). This time is expressed on 10 kHz sampling
clock basis (100 microseconds per increment) The remote partner must answer to this message with a CK1 message,
containing its own local time on 64 bits. Both partners then know the difference between their respective clocks
and can determine the offset to apply to Timestamp and Deltatime fields in RTP-MIDI protocol. The session
initiator finishes this sequence by sending a last message called CK2, containing the local time when it
received the CK1 message. This technique allows to compute the average latency of the network, and also to
compensate a potential delay introduced by a slow starting thread (this situation can occur with non-realtime
operating systems like Linux, Windows or OS X)

Apple recommends to repeat this sequence a few times just after opening the session, in order to get better
synchronization accuracy (in case of one of the sequence has been delayed accidentally because of a temporary
network overload or a latency peak in a thread activation)

This sequence must repeat cyclically (between 2 and 6 times per minute typically), and always by the session
initiator, in order to maintain long term synchronization accuracy by compensation of local clock drift, and also
to detect a loss of communication partner. A partner not answering to multiple CK0 messages shall consider that
the remote partner is disconnected. In most cases, session initiators switch their state machine into "Invitation"
state in order to re-establish communication automatically as soon as the distant partner reconnects to the
network. Some implementations (especially on personal computers) display also an alert message and offer to the
user to choose between a new connection attempt or closing the session.
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSyncronization(void* sender, AppleMIDI_Syncronization& synchronization)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Syncronization for ssrc 0x"));
	DEBUGSTREAM.print(synchronization.ssrc, HEX);
	DEBUGSTREAM.print(", count = ");
	DEBUGSTREAM.print(synchronization.count);
#if (APPLEMIDI_DEBUG_VERBOSE)
	//DEBUGSTREAM.print  (" Timestamps = ");
	//DEBUGSTREAM.print  (synchronization.timestamps[0], HEX);
	//DEBUGSTREAM.print  (" ");
	//DEBUGSTREAM.print  (synchronization.timestamps[1], HEX);
	//DEBUGSTREAM.print  (" ");
	//DEBUGSTREAM.print  (synchronization.timestamps[2], HEX);
#endif
	DEBUGSTREAM.println("");
#endif

	// If we don't know this session, ignore it.

	int index = GetSessionSlotUsingSSrc(synchronization.ssrc);
	if (index < 0)
	{
#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.println(F("hmmm - Syncronization for a session that has never started."));
#endif
		return;
	}

	switch (synchronization.count) {
		case SYNC_CK0: /* From session initiator */
			synchronization.count = SYNC_CK1;
			synchronization.timestamps[synchronization.count] = _rtpMidiClock.Now();
			break;

		case SYNC_CK1: /* From session responder */
			/* compute media delay */
			//uint64_t diff = (now - synchronization.timestamps[0]) / 2;
			/* approximate time difference between peer and self */
			//diff = synchronization.timestamps[2] + diff - now;

			// Send CK2
			synchronization.count = SYNC_CK2;
			synchronization.timestamps[synchronization.count] = _rtpMidiClock.Now();

			/* getting this message means that the responder is still alive! */
			/* remember the time, if it takes to long to respond, we can assume the responder is dead */
			/* not implemented at this stage*/
			//Sessions[index].syncronization.lastTime = _rtpMidiClock.Now();
			//Sessions[index].syncronization.count++;
			break;

		case SYNC_CK2: /* From session initiator */
			/* compute media delay */
			//uint64_t diff = (synchronization.timestamps[2] - synchronization.timestamps[0]) / 2;
			/* approximate time difference between peer and self */
			//diff = synchronization.timestamps[2] + diff - now;

			synchronization.count = SYNC_CK0;
			synchronization.timestamps[synchronization.count] = _rtpMidiClock.Now();
			break;
	}

	AppleMIDI_Syncronization synchronizationResponse(getSynchronizationSource(), synchronization.count, synchronization.timestamps);
	write(_dataPort, synchronizationResponse, _dataPort.remoteIP(), _dataPort.remotePort());

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("< Syncronization for ssrc 0x"));
	DEBUGSTREAM.print(getSynchronizationSource(), HEX);
	DEBUGSTREAM.print(", count = ");
	DEBUGSTREAM.print(synchronizationResponse.count);
#if (APPLEMIDI_DEBUG_VERBOSE)
	//DEBUGSTREAM.print  (" Timestamps = ");
	//DEBUGSTREAM.print  (synchronization.timestamps[0], HEX);
	//DEBUGSTREAM.print  (" ");
	//DEBUGSTREAM.print  (synchronization.timestamps[1], HEX);
	//DEBUGSTREAM.print  (" ");
	//DEBUGSTREAM.print  (synchronization.timestamps[2], HEX);
#endif
	DEBUGSTREAM.println("");
#endif

}

/* With the bitrate receive limit packet, the recipient can tell the sender to limit
the transmission to a certain bitrate. This is important if the peer is a gateway
to a hardware-device that only supports a certain speed. Like the MIDI 1.0 DIN-cable
MIDI-implementation which is limited to 31250. */
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnBitrateReceiveLimit(void* sender, AppleMIDI_BitrateReceiveLimit& bitrateReceiveLimit)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> BitrateReceiveLimit: Limit = "));
	DEBUGSTREAM.println(bitrateReceiveLimit.bitratelimit);
#endif

}

/*! \brief Find a free session slot.
 */
template<class UdpClass, class Settings>
int AppleMidiInterface<UdpClass, Settings>::GetFreeSessionSlot()
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		if (0 == Sessions[i].ssrc)
			return i;
	return -1;
}

/*! \brief Find the slot of a session, based on the ssrc.
*/
template<class UdpClass, class Settings>
int AppleMidiInterface<UdpClass, Settings>::GetSessionSlotUsingSSrc(const uint32_t ssrc)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		if (ssrc == Sessions[i].ssrc)
			return i;
	return -1;
}

/*! \brief Find the slot of a session, based on the ssrc.
*/
template<class UdpClass, class Settings>
int AppleMidiInterface<UdpClass, Settings>::GetSessionSlotUsingInitiatorToken(const uint32_t initiatorToken)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		if (initiatorToken == Sessions[i].invite.initiatorToken)
			return i;
	return -1;
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::CompleteLocalSessionControl(AppleMIDI_InvitationAccepted& invitationAccepted)
{
	// Find slot, based on initiator token
	int i = GetSessionSlotUsingInitiatorToken(invitationAccepted.initiatorToken);
	if (i < 0)
	{
#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.println("hmm, initiatorToken not found");
#endif
		return;
	}

	//
	if (Sessions[i].invite.status != WaitingForControlInvitationAccepted)
	{
#if (APPLEMIDI_DEBUG) // issue warning
		DEBUGSTREAM.println("status not what expected");
#endif
	}

	// Initiate next step in the invitation process
	Sessions[i].invite.lastSend = 0;
	Sessions[i].invite.attempts = 0;
	Sessions[i].invite.ssrc = invitationAccepted.ssrc;
	Sessions[i].invite.status = SendContentInvite;
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::CompleteLocalSessionContent(AppleMIDI_InvitationAccepted& invitationAccepted)
{
	// Find slot, based on initiator token
	int i = GetSessionSlotUsingInitiatorToken(invitationAccepted.initiatorToken);
	if (i < 0)
	{
#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.println("hmm, initiatorToken not found");
#endif
		return;
	}

	//
	if (Sessions[i].invite.status != WaitingForContentInvitationAccepted)
	{
#if (APPLEMIDI_DEBUG) // issue warning
		DEBUGSTREAM.println("status not what expected");
#endif
	}

	// Finalize invitation process
	Sessions[i].ssrc = invitationAccepted.ssrc;
	Sessions[i].invite.status = None;
	Sessions[i].syncronization.enabled = true; // synchronisation can start

	if (mConnectedCallback != 0)
		mConnectedCallback(Sessions[i].ssrc, invitationAccepted.sessionName);
}

/*! \brief Initialize session at slot 'index'.
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::CreateLocalSession(const int i, const uint32_t ssrc)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print  ("New Local Session in slot ");
	DEBUGSTREAM.print  (i);
	DEBUGSTREAM.print  (" with SSRC 0x");
	DEBUGSTREAM.println(ssrc, HEX);
#endif

	Sessions[i].ssrc = ssrc;
	Sessions[i].seqNum = 1;
	Sessions[i].initiator = Remote;
	Sessions[i].syncronization.lastTime = 0;
	Sessions[i].syncronization.count = 0;
	Sessions[i].syncronization.busy = false;
	Sessions[i].syncronization.enabled = false;
	Sessions[i].invite.status = ReceiveControlInvitation;
}

/*! \brief Initialize session at slot 'index'.
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::CreateRemoteSession(IPAddress ip, uint16_t port)
{
	int i = GetFreeSessionSlot();
	if (i < 0)
	{
#if (APPLEMIDI_DEBUG)
		DEBUGSTREAM.println("Invite: No free slot availble, invitation cancelled.");
#endif
		return;
	}

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("New Remote Session in slot ");
	DEBUGSTREAM.println(i);
#endif

	Sessions[i].ssrc = -1;
	Sessions[i].seqNum = 0;
	Sessions[i].initiator = Local;
	Sessions[i].contentIP = ip;
	Sessions[i].contentPort = port + 1;
	Sessions[i].syncronization.lastTime = 0;
	Sessions[i].syncronization.count = 0;
	Sessions[i].syncronization.busy = false;
	Sessions[i].syncronization.enabled = false;

	Sessions[i].invite.remoteHost = ip;
	Sessions[i].invite.remotePort = port;
	Sessions[i].invite.lastSend = 0;
	Sessions[i].invite.attempts = 0;
	Sessions[i].invite.status = SendControlInvite;
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::DeleteSession(const uint32_t ssrc)
{
	// Find the slot first
	int slot = GetSessionSlotUsingSSrc(ssrc);
	if (slot < 0)
		return;

	DeleteSession(slot);
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::DeleteSession(int slot)
{
	// Then zero-ize it
	Sessions[slot].ssrc = 0;
	Sessions[slot].seqNum = 0;
	Sessions[slot].initiator = Undefined;
	Sessions[slot].invite.status = None;
	Sessions[slot].syncronization.enabled = false;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("Freeing Session slot ");
	DEBUGSTREAM.println(slot);
#endif
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::DeleteSessions()
{
	for (int slot = 0; slot < MAX_SESSIONS; slot++)
	{
		Sessions[slot].ssrc = 0;
		Sessions[slot].initiator = Undefined;
		Sessions[slot].invite.status = None;
		Sessions[slot].invite.status = None;
		Sessions[slot].invite.attempts = 0;
		Sessions[slot].invite.lastSend = 0;
		Sessions[slot].syncronization.enabled = false;
	}
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::DumpSession()
{
#if (APPLEMIDI_DEBUG)
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		DEBUGSTREAM.print("Slot ");
		DEBUGSTREAM.print(i);
		DEBUGSTREAM.print(" ssrc = 0x");
		DEBUGSTREAM.println(Sessions[i].ssrc, HEX);
	}
#endif
}

/*! \brief .
*/
template<class UdpClass, class Settings>
inline uint32_t AppleMidiInterface<UdpClass, Settings>::createInitiatorToken()
{
	static int counter = 0;
	return 0x12345000 + ++counter;
}

/*! \brief .
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::ManageInvites()
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		Session_t* session = &Sessions[i];

		if (session->invite.status == None)
		{
			// No invitation pending
		}
		else if (session->invite.status == SendControlInvite)
		{
			// Send invitation
			AppleMIDI_Invitation invitation;
			invitation.initiatorToken = createInitiatorToken();
			invitation.ssrc = getSynchronizationSource();
			strncpy(invitation.sessionName, getSessionName(), SESSION_NAME_MAX_LEN);
			write(_controlPort, invitation, session->invite.remoteHost, session->invite.remotePort);

			session->invite.initiatorToken = invitation.initiatorToken;
			session->invite.lastSend = millis();
			session->invite.attempts = 1;
			session->invite.status = WaitingForControlInvitationAccepted;

#if (APPLEMIDI_DEBUG)
			DEBUGSTREAM.print("< (ManageInvites) Control Invitation: peer = \"");
			DEBUGSTREAM.print(invitation.sessionName);
			DEBUGSTREAM.print("\"");
			DEBUGSTREAM.print(" ,ssrc 0x");
			DEBUGSTREAM.print(invitation.ssrc, HEX);
			DEBUGSTREAM.print(" ,Attempt = ");
			DEBUGSTREAM.print(session->invite.attempts);
			DEBUGSTREAM.print(" ,initiatorToken = 0x");
			DEBUGSTREAM.print(invitation.initiatorToken, HEX);
			DEBUGSTREAM.println();
#endif
		}
		else if (session->invite.status == WaitingForControlInvitationAccepted)
		{
			if (session->invite.lastSend + 1000 < millis())
			{
				// If no response received after 1 second, send invitation again
				// with a maximum of 10 times.

				if (session->invite.attempts >= 10) // Max attempts
				{
					DeleteSession(i); // give up
					return;
				}

				// Send invitation
				AppleMIDI_Invitation invitation;
				invitation.initiatorToken = session->invite.initiatorToken;
				invitation.ssrc = getSynchronizationSource();
				strncpy(invitation.sessionName, getSessionName(), SESSION_NAME_MAX_LEN);
				write(_controlPort, invitation, session->invite.remoteHost, session->invite.remotePort);

				session->invite.lastSend = millis();
				session->invite.attempts++;

#if (APPLEMIDI_DEBUG)
				DEBUGSTREAM.print("< (ManageInvites2) Control Invitation: peer = \"");
				DEBUGSTREAM.print(invitation.sessionName);
				DEBUGSTREAM.print("\"");
				DEBUGSTREAM.print(" ,ssrc 0x");
				DEBUGSTREAM.print(invitation.ssrc, HEX);
				DEBUGSTREAM.print(" ,Attempt = ");
				DEBUGSTREAM.print(session->invite.attempts);
				DEBUGSTREAM.print(" ,initiatorToken = 0x");
				DEBUGSTREAM.print(invitation.initiatorToken, HEX);
				DEBUGSTREAM.println();
#endif
			}
		}
		else if (session->invite.status == SendContentInvite)
		{
			AppleMIDI_Invitation invitation;
			invitation.initiatorToken = session->invite.initiatorToken;
			invitation.ssrc = getSynchronizationSource();
			strncpy(invitation.sessionName, getSessionName(), SESSION_NAME_MAX_LEN);
			write(_dataPort, invitation, session->invite.remoteHost, session->invite.remotePort + 1);

			session->invite.lastSend = millis();
			session->invite.attempts = 1;
			session->invite.status = WaitingForContentInvitationAccepted;

#if (APPLEMIDI_DEBUG)
			DEBUGSTREAM.print("< Content Invitation: peer = \"");
			DEBUGSTREAM.print(invitation.sessionName);
			DEBUGSTREAM.print("\"");
			DEBUGSTREAM.print(" ,ssrc 0x");
			DEBUGSTREAM.print(invitation.ssrc, HEX);
			DEBUGSTREAM.print(" ,Attempt = ");
			DEBUGSTREAM.print(session->invite.attempts);
			DEBUGSTREAM.print(" ,initiatorToken = 0x");
			DEBUGSTREAM.print(invitation.initiatorToken, HEX);
			DEBUGSTREAM.println();
#endif
		}
		else if (session->invite.status == WaitingForContentInvitationAccepted)
		{
			if (session->invite.lastSend + 1000 < millis())
			{
				// If no response received after 1 second, send invitation again
				// with a maximum of 10 times.

				if (session->invite.attempts >= 10) // Max attempts
				{
					DeleteSession(session->invite.ssrc); // Give up
					return;
				}

				AppleMIDI_Invitation invitation;
				invitation.initiatorToken = session->invite.initiatorToken;
				invitation.ssrc = getSynchronizationSource();
				strncpy(invitation.sessionName, getSessionName(), SESSION_NAME_MAX_LEN);
				write(_dataPort, invitation, session->invite.remoteHost, session->invite.remotePort + 1);

				session->invite.lastSend = millis();
				session->invite.attempts++;

#if (APPLEMIDI_DEBUG)
				DEBUGSTREAM.print("< Content Invitation: peer = \"");
				DEBUGSTREAM.print(invitation.sessionName);
				DEBUGSTREAM.print("\"");
				DEBUGSTREAM.print(" ,ssrc 0x");
				DEBUGSTREAM.print(invitation.ssrc, HEX);
				DEBUGSTREAM.print(" ,Attempt = ");
				DEBUGSTREAM.print(session->invite.attempts);
				DEBUGSTREAM.print(" ,initiatorToken = 0x");
				DEBUGSTREAM.print(invitation.initiatorToken, HEX);
				DEBUGSTREAM.println();
#endif
			}
		}
	}
}

/*! \brief The initiator of the session polls if remote station is still alive.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::ManageTiming()
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].initiator == Local && Sessions[i].syncronization.enabled)
		{
			if (!Sessions[i].syncronization.busy)
			{
				bool doSyncronize = false;

				if (Sessions[i].syncronization.count < 2)
				{
					// immediately after last CK2
					Sessions[i].syncronization.count++;
					doSyncronize = true;
				}
				else if (Sessions[i].syncronization.count < 10)
				{
					// every second after last CK2
					if (Sessions[i].syncronization.lastTime + 1000 < millis())
					{
						Sessions[i].syncronization.count++;
						doSyncronize = true;
					}
				}
				else
				{
					// every 20 seconds after last CK2
					if (Sessions[i].syncronization.lastTime + 20000 < millis())
					{
						doSyncronize = true;
					}
				}

				if (doSyncronize)
				{
					Sessions[i].syncronization.lastTime = millis();

					AppleMIDI_Syncronization synchronization;
					synchronization.timestamps[0] = _rtpMidiClock.Now();
					synchronization.timestamps[1] = 0;
					synchronization.timestamps[2] = 0;
					synchronization.count = 0;

					AppleMIDI_Syncronization synchronizationResponse(getSynchronizationSource(), synchronization.count, synchronization.timestamps);
					write(_dataPort, synchronizationResponse, Sessions[i].contentIP, Sessions[i].contentPort);

#if (APPLEMIDI_DEBUG)
					DEBUGSTREAM.print("< Syncronization for ssrc 0x");
					DEBUGSTREAM.print(synchronizationResponse.ssrc, HEX);
					DEBUGSTREAM.print(", count = ");
					DEBUGSTREAM.print(synchronizationResponse.count);
					DEBUGSTREAM.print(", to = ");
					DEBUGSTREAM.print(Sessions[i].contentIP);
					DEBUGSTREAM.print(" ");
					DEBUGSTREAM.print(Sessions[i].contentPort);
#if (APPLEMIDI_DEBUG_VERBOSE)
					//DEBUGSTREAM.print  (" Timestamps = ");
					//DEBUGSTREAM.print  (synchronizationResponse.timestamps[0], HEX);
					//DEBUGSTREAM.print  (" ");
					//DEBUGSTREAM.print  (synchronizationResponse.timestamps[1], HEX);
					//DEBUGSTREAM.print  (" ");
					//DEBUGSTREAM.print  (synchronizationResponse.timestamps[2], HEX);
#endif
					DEBUGSTREAM.println("");
#endif
				}
			}
		}
	}

	//if (_lastTimeSessionSyncronized + 30000 < millis())
	//{
	//	// Send synchronization
	//	AppleMIDI_Syncronization synchronization;
	//	synchronization.timestamps[0] = _rtpMidiClock.Now();
	//	synchronization.timestamps[1] = 0;
	//	synchronization.timestamps[2] = 0;
	//	synchronization.count = 0;

	//	AppleMIDI_Syncronization synchronizationResponse(getSynchronizationSource(), synchronization.count, synchronization.timestamps);
	//	synchronizationResponse.write(&_dataPort);
	//}
}

END_APPLEMIDI_NAMESPACE
