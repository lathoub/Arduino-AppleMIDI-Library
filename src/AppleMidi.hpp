/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.4
 *  @author		lathoub, hackmancoltaire, chris-zen
 *	@date		13/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief Default constructor for MIDI_Class. */
template<class UdpClass>
inline AppleMidi_Class<UdpClass>::AppleMidi_Class()
{
#if APPLEMIDI_USE_CALLBACKS
	// Initialise callbacks to NULL pointer
	mConnectedCallback				= NULL;
	mDisconnectedCallback			= NULL;

	mNoteOffCallback				= NULL;
	mNoteOnCallback					= NULL;
	mAfterTouchPolyCallback			= NULL;
	mControlChangeCallback			= NULL;
	mProgramChangeCallback			= NULL;
	mAfterTouchChannelCallback		= NULL;
	mPitchBendCallback				= NULL;
	mSysExCallback					= NULL;
	mTimeCodeQuarterFrameCallback	= NULL;
	mSongPositionCallback			= NULL;
	mSongSelectCallback				= NULL;
	mTuneRequestCallback			= NULL;
	mClockCallback					= NULL;
	mStartCallback					= NULL;
	mContinueCallback				= NULL;
	mStopCallback					= NULL;
	mActiveSensingCallback			= NULL;
	mResetCallback			        = NULL;
#endif

#if APPLEMIDI_USE_EVENTS
	mNoteOnSendingEvent				= NULL;
	mNoteOnSendEvent				= NULL;
	mNoteOffSendingEvent			= NULL;
	mNoteOffSendEvent				= NULL;
#endif

	// initiative to 0, the actual SSRC will be generated lazily
	_ssrc = 0;

	uint32_t initialTimestamp_ = 0;
	_rtpMidiClock.Init(initialTimestamp_, MIDI_SAMPLING_RATE_DEFAULT);
}

/*! \brief Default destructor for MIDI_Class.

 This is not really useful for the Arduino, as it is never called...
 */
template<class UdpClass>
inline AppleMidi_Class<UdpClass>::~AppleMidi_Class()
{
}

//---------------------------------------------------------------------------------------

/*! \brief Call the begin method in the setup() function of the Arduino.

All parameters are set to their default values:
- Input channel set to 1 if no value is specified
- Full thru mirroring
*/
template<class UdpClass>
inline bool AppleMidi_Class<UdpClass>::begin(const char* sessionName, uint16_t port)
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

	_inputChannel = MIDI_CHANNEL_OMNI;

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

/*! \brief Evaluates incoming Rtp messages.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::run()
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

	// resend invitations
	ManageInvites();

	// do syncronization here
	ManageTiming();
}

/*! \brief Get Synchronization Source, initiatize the SSRC on first time usage (lazy init).
*/
template<class UdpClass>
inline uint32_t AppleMidi_Class<UdpClass>::getSynchronizationSource()
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
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::invite(IPAddress ip, uint16_t port)
{
	CreateRemoteSession(ip, port);

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.println(F("Queued invite"));
#endif
}

/*! \brief The Arduino is being invited to a session.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::OnInvitation(void* sender, AppleMIDI_Invitation& invitation)
{
	Dissector* dissector = (Dissector*) sender;

	if (dissector->_identifier == Port)
		OnControlInvitation(sender, invitation);
	if (dissector->_identifier == Port + 1)
		OnContentInvitation(sender, invitation);
}

/*! \brief The session has been ended by the remote source.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::OnEndSession(void* sender, AppleMIDI_EndSession& sessionEnd)
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
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::OnReceiverFeedback(void* sender, AppleMIDI_ReceiverFeedback& receiverFeedback)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Receiver Feedback: seq = "));
	DEBUGSTREAM.println(receiverFeedback.sequenceNr);
#endif
}

/*! \brief The invitation that we have sent, has been accepted.
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
{
	Dissector* dissector = (Dissector*) sender;

	if (dissector->_identifier == Port)
		OnControlInvitationAccepted(sender, invitationAccepted);
	if (dissector->_identifier == Port + 1)
		OnContentInvitationAccepted(sender, invitationAccepted);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnControlInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnContentInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnControlInvitation(void* sender, AppleMIDI_Invitation& invitation)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnContentInvitation(void* sender, AppleMIDI_Invitation& invitation)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnSyncronization(void* sender, AppleMIDI_Syncronization& synchronization)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnBitrateReceiveLimit(void* sender, AppleMIDI_BitrateReceiveLimit& bitrateReceiveLimit)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> BitrateReceiveLimit: Limit = "));
	DEBUGSTREAM.println(bitrateReceiveLimit.bitratelimit);
#endif

}

//------------------------------------------------------------------------------------

/*! \brief .
*/
template<class UdpClass>
bool AppleMidi_Class<UdpClass>::PassesFilter(void* sender, DataByte type, DataByte channel)
{
	// This method handles recognition of channel
	// (to know if the message is destinated to the Arduino)

	//if (mMessage.type == InvalidType)
	//    return false;

	// First, check if the received message is Channel
	if (type >= NoteOff && type <= PitchBend)
	{
		// Then we need to know if we listen to it
		if ((channel == _inputChannel)
			|| (_inputChannel == MIDI_CHANNEL_OMNI))
		{
			return true;
		}
		else
		{
			// We don't listen to this channel
			return false;
		}
	}
	else
	{
		// System messages are always received
		return true;
	}
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnNoteOn(void* sender, DataByte channel, DataByte note, DataByte velocity)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print(F("> Note On (c="));
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(note);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(velocity);
	DEBUGSTREAM.println(")");
#endif

	if (mNoteOnCallback)
		mNoteOnCallback(channel, note, velocity);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnNoteOff(void* sender, DataByte channel, DataByte note, DataByte velocity)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Note Off (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(note);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(velocity);
	DEBUGSTREAM.println(")");
#endif

	if (mNoteOffCallback)
		mNoteOffCallback(channel, note, velocity);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnPolyPressure(void* sender, DataByte channel, DataByte note, DataByte pressure)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Poly Pressure (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(note);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(pressure);
	DEBUGSTREAM.println(")");
#endif

	if (mAfterTouchPolyCallback)
		mAfterTouchPolyCallback(channel, note, pressure);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnChannelPressure(void* sender, DataByte channel, DataByte pressure)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Channel Pressure (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(pressure);
	DEBUGSTREAM.println(")");
#endif

	if (mAfterTouchChannelCallback)
		mAfterTouchChannelCallback(channel, pressure);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnPitchBendChange(void* sender, DataByte channel, int pitch)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Pitch Bend (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(pitch);
	DEBUGSTREAM.println(")");
#endif

	if (mPitchBendCallback)
		mPitchBendCallback(channel, pitch);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnProgramChange(void* sender, DataByte channel, DataByte program)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Program Change (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", p=");
	DEBUGSTREAM.print(program);
	DEBUGSTREAM.print(") ");

	switch (program)
	{
	default:
		DEBUGSTREAM.println("Other");
		break;
	}

#endif

	if (mProgramChangeCallback)
		mProgramChangeCallback(channel, program);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnControlChange(void* sender, DataByte channel, DataByte controller, DataByte value)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Control Change (c=");
	DEBUGSTREAM.print(channel);
	DEBUGSTREAM.print(", e=");
	DEBUGSTREAM.print(controller);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(value);
	DEBUGSTREAM.print(") ");

	switch (controller)
	{
	case BankSelect:
		DEBUGSTREAM.println("BankSelect");
		break;
	case ModulationWheel:
		DEBUGSTREAM.println("ModulationWheel");
		break;
	case BreathController:
		DEBUGSTREAM.println("BreathController");
		break;
	case FootController:
		DEBUGSTREAM.println("FootController");
		break;
	case PortamentoTime:
		DEBUGSTREAM.println("PortamentoTime");
		break;
	case DataEntry:
		DEBUGSTREAM.println("DataEntry");
		break;
	case ChannelVolume:
		DEBUGSTREAM.println("ChannelVolume");
		break;
	case Balance:
		DEBUGSTREAM.println("Balance");
		break;
	case Pan:
		DEBUGSTREAM.println("Pan");
		break;
	case ExpressionController:
		DEBUGSTREAM.println("ExpressionController");
		break;
	case EffectControl1:
		DEBUGSTREAM.println("EffectControl1");
		break;
	case EffectControl2:
		DEBUGSTREAM.println("EffectControl2");
		break;
	case GeneralPurposeController1:
		DEBUGSTREAM.println("GeneralPurposeController1");
		break;
	case GeneralPurposeController2:
		DEBUGSTREAM.println("GeneralPurposeController2");
		break;
	case GeneralPurposeController3:
		DEBUGSTREAM.println("GeneralPurposeController3");
		break;
	case GeneralPurposeController4:
		DEBUGSTREAM.println("GeneralPurposeController4");
		break;
	case Sustain:
		DEBUGSTREAM.println("Sustain");
		break;
	case Portamento:
		DEBUGSTREAM.println("Portamento");
		break;
	case Sostenuto:
		DEBUGSTREAM.println("Sostenuto");
		break;
	case SoftPedal:
		DEBUGSTREAM.println("SoftPedal");
		break;
	case Legato:
		DEBUGSTREAM.println("Legato");
		break;
	case Hold:
		DEBUGSTREAM.println("Hold");
		break;
	case SoundController1:
		DEBUGSTREAM.println("SoundController1");
		break;
	case SoundController2:
		DEBUGSTREAM.println("SoundController2");
		break;
	case SoundController3:
		DEBUGSTREAM.println("SoundController3");
		break;
	case SoundController4:
		DEBUGSTREAM.println("SoundController4");
		break;
	case SoundController5:
		DEBUGSTREAM.println("SoundController5");
		break;
	case SoundController6:
		DEBUGSTREAM.println("SoundController6");
		break;
	case SoundController7:
		DEBUGSTREAM.println("SoundController7");
		break;
	case SoundController8:
		DEBUGSTREAM.println("SoundController8");
		break;
	case SoundController9:
		DEBUGSTREAM.println("SoundController9");
		break;
	case SoundController10:
		DEBUGSTREAM.println("SoundController10");
		break;
	case GeneralPurposeController5:
		DEBUGSTREAM.println("GeneralPurposeController5");
		break;
	case GeneralPurposeController6:
		DEBUGSTREAM.println("GeneralPurposeController6");
		break;
	case GeneralPurposeController7:
		DEBUGSTREAM.println("GeneralPurposeController7");
		break;
	case GeneralPurposeController8:
		DEBUGSTREAM.println("GeneralPurposeController8");
		break;
	case PortamentoControl:
		DEBUGSTREAM.println("PortamentoControl");
		break;
	case Effects1:
		DEBUGSTREAM.println("Effects1");
		break;
	case Effects2:
		DEBUGSTREAM.println("Effects2");
		break;
	case Effects3:
		DEBUGSTREAM.println("Effects3");
		break;
	case Effects4:
		DEBUGSTREAM.println("Effects4");
		break;
	case Effects5:
		DEBUGSTREAM.println("Effects5");
		break;
	case AllSoundOff:
		DEBUGSTREAM.println("AllSoundOff");
		break;
	case ResetAllControllers:
		DEBUGSTREAM.println("ResetAllControllers");
		break;
	case LocalControl:
		DEBUGSTREAM.println("LocalControl");
		break;
	case AllNotesOff:
		DEBUGSTREAM.println("AllNotesOff");
		break;
	case OmniModeOff:
		DEBUGSTREAM.println("OmniModeOff");
		break;
	case OmniModeOn:
		DEBUGSTREAM.println("OmniModeOn");
		break;
	case MonoModeOn:
		DEBUGSTREAM.println("MonoModeOn");
		break;
	case PolyModeOn:
		DEBUGSTREAM.println("PolyModeOn");
		break;
	default:
		DEBUGSTREAM.println("Other");
		break;
	}
#endif

	if (mControlChangeCallback)
		mControlChangeCallback(channel, controller, value);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnSongSelect(void* sender, DataByte songNr)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SongSelect (s=");
	DEBUGSTREAM.print(songNr);
	DEBUGSTREAM.println(")");
#endif

	if (mSongSelectCallback)
		mSongSelectCallback(songNr);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnSongPosition(void* sender, unsigned short value)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SongPosition (c=");
	DEBUGSTREAM.print(value);
	DEBUGSTREAM.println(")");
#endif

	if (mSongPositionCallback)
		mSongPositionCallback(value);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnTimeCodeQuarterFrame(void* sender, DataByte value)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> TimeCodeQuarterFrame (c=");
	DEBUGSTREAM.print(value);
	DEBUGSTREAM.println(")");
#endif

	if (mTimeCodeQuarterFrameCallback)
		mTimeCodeQuarterFrameCallback(value);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnTuneRequest(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> TuneRequest ()");
#endif

	if (mTuneRequestCallback)
		mTuneRequestCallback();
}


/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnClock(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Clock ()");
#endif

	if (mClockCallback)
		mClockCallback();
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnStart(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Start ()");
#endif

	if (mStartCallback)
		mStartCallback();
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnContinue(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Continue ()");
#endif

	if (mContinueCallback)
		mContinueCallback();
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnStop(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Stop ()");
#endif

	if (mStopCallback)
		mStopCallback();
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnActiveSensing(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> ActiveSensing ()");
#endif

	if (mActiveSensingCallback)
		mActiveSensingCallback();
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnReset(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Reset ()");
#endif

	if (mResetCallback)
		mResetCallback();
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnSysEx(void* sender, const byte* data, uint16_t length)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SysEx ()");
#endif

	if (mSysExCallback)
		mSysExCallback(data, length);
}

//------------------------------------------------------------------------------

/*! \brief Find a free session slot.
 */
template<class UdpClass>
int AppleMidi_Class<UdpClass>::GetFreeSessionSlot()
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		if (0 == Sessions[i].ssrc)
			return i;
	return -1;
}

/*! \brief Find the slot of a session, based on the ssrc.
*/
template<class UdpClass>
int AppleMidi_Class<UdpClass>::GetSessionSlotUsingSSrc(const uint32_t ssrc)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		if (ssrc == Sessions[i].ssrc)
			return i;
	return -1;
}

/*! \brief Find the slot of a session, based on the ssrc.
*/
template<class UdpClass>
int AppleMidi_Class<UdpClass>::GetSessionSlotUsingInitiatorToken(const uint32_t initiatorToken)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
		if (initiatorToken == Sessions[i].invite.initiatorToken)
			return i;
	return -1;
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::CompleteLocalSessionControl(AppleMIDI_InvitationAccepted& invitationAccepted)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::CompleteLocalSessionContent(AppleMIDI_InvitationAccepted& invitationAccepted)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::CreateLocalSession(const int i, const uint32_t ssrc)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::CreateRemoteSession(IPAddress ip, uint16_t port)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::DeleteSession(const uint32_t ssrc)
{
	// Find the slot first
	int slot = GetSessionSlotUsingSSrc(ssrc);
	if (slot < 0)
		return;

	DeleteSession(slot);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::DeleteSession(int slot)
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::DeleteSessions()
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
template<class UdpClass>
void AppleMidi_Class<UdpClass>::DumpSession()
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
template<class UdpClass>
inline uint32_t AppleMidi_Class<UdpClass>::createInitiatorToken()
{
	static int counter = 0;
	return 0x12345000 + ++counter;
}

/*! \brief .
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::ManageInvites()
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
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::ManageTiming()
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::write(UdpClass& udp, AppleMIDI_InvitationRejected ir, IPAddress ip, uint16_t port)
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

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::write(UdpClass& udp, AppleMIDI_InvitationAccepted ia, IPAddress ip, uint16_t port)
{
	int success = udp.beginPacket(ip, port);
	Debug::Assert(success, "udp.beginPacket failed");

		// Set the correct endian
		ia.version        = AppleMIDI_Util::toEndian(ia.version);
		ia.initiatorToken = AppleMIDI_Util::toEndian(ia.initiatorToken);
		ia.ssrc           = AppleMIDI_Util::toEndian(ia.ssrc);

		size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&ia), ia.getLength());
		Debug::Assert(bytesWritten == ia.getLength(), "error writing ia");

	success = udp.endPacket();
	Debug::Assert(success, "udp.endPacket failed");
	udp.flush();
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::write(UdpClass& udp, AppleMIDI_Syncronization sy, IPAddress ip, uint16_t port)
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

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::write(UdpClass& udp, AppleMIDI_Invitation in, IPAddress ip, uint16_t port)
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

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::write(UdpClass& udp, AppleMIDI_BitrateReceiveLimit in, IPAddress ip, uint16_t port)
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

#if APPLEMIDI_BUILD_OUTPUT

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

/*! \brief Generate and send a MIDI message from the values given.
\param inType    The message type (see type defines for reference)
\param inData1   The first data byte.
\param inData2   The second data byte (if the message contains only 1 data byte,
set this one to 0).
\param inChannel The output channel on which the message will be sent
(values from 1 to 16). Note: you cannot send to OMNI.

This is an internal method, use it only if you need to send raw data
from your code, at your own risks.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::send(MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(Sessions[i], inType, inData1, inData2, inChannel);
		}
	}

	return;
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::send(MidiType inType, DataByte inData1, DataByte inData2)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(Sessions[i], inType, inData1, inData2);
		}
	}

	return;
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::send(MidiType inType, DataByte inData)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(Sessions[i], inType, inData);
		}
	}

	return;
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::send(MidiType inType)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(Sessions[i], inType);
		}
	}

	return;
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::sendSysEx(byte s, const byte* data, byte e, uint16_t length)
{
	for (int i = 0; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSendSysEx(Sessions[i], s, data, e, length);
		}
	}

	return;
}

/*! \brief Generate and send a MIDI message from the values given.
\param inType    The message type (see type defines for reference)
\param inData1   The first data byte.
\param inData2   The second data byte (if the message contains only 1 data byte,
set this one to 0).
\param inChannel The output channel on which the message will be sent
(values from 1 to 16). Note: you cannot send to OMNI.

This is an internal method, use it only if you need to send raw data
from your code, at your own risks.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::internalSend(Session_t& session, MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel)
{
	// Then test if channel is valid
	if (inChannel >= MIDI_CHANNEL_OFF ||
		inChannel == MIDI_CHANNEL_OMNI ||
		inType < NoteOff)
	{

#if APPLEMIDI_USE_RUNNING_STATUS
		mRunningStatus_TX = InvalidType;
#endif
		return; // Don't send anything
	}

	if (inType <= PitchBend)  // Channel messages
	{
		// Protection: remove MSBs on data
		inData1 &= 0x7F;
		inData2 &= 0x7F;

		_rtpMidi.ssrc = getSynchronizationSource();
		_rtpMidi.sequenceNr++;
		_rtpMidi.timestamp = _rtpMidiClock.Now();
		_rtpMidi.beginWrite(_dataPort, session.contentIP, session.contentPort);

		// Length
		uint8_t length = 16; // Add the P-Flag and then length octet
		if (inType == ProgramChange || inType == AfterTouchChannel) {
			length |= 2;
		}
		else {
			length |= 3;
		}
		_dataPort.write(&length, 1);

		// Command Section

		const StatusByte status = getStatus(inType, inChannel);

#if APPLEMIDI_USE_RUNNING_STATUS
		// Check Running Status
		if (mRunningStatus_TX != status)
		{
			// New message, memorise and send header
			mRunningStatus_TX = status;
			_dataPort.write(&mRunningStatus_TX, 1);
		}
#else
		// Don't care about running status, send the status byte.
		_dataPort.write(&status, sizeof(status));
#endif

		// Then send data
		_dataPort.write(&inData1, sizeof(inData1));
		if (inType != ProgramChange && inType != AfterTouchChannel)
			_dataPort.write(&inData2, sizeof(inData2));

		_rtpMidi.endWrite(_dataPort);

		return;
	}
	else if (inType >= TuneRequest && inType <= Reset)
		internalSend(session, inType); // System Real-time and 1 byte.
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::internalSend(Session_t& session, MidiType inType)
{
	_rtpMidi.ssrc = getSynchronizationSource();
	_rtpMidi.sequenceNr++;
	_rtpMidi.timestamp = _rtpMidiClock.Now();
	_rtpMidi.beginWrite(_dataPort, session.contentIP, session.contentPort);

	uint8_t length = 1;
	_dataPort.write(&length, 1);

	byte octet = (byte) inType;

	switch (inType)
	{
	case TuneRequest: // Not really real-time, but one byte anyway.
	case Clock:
	case Start:
	case Stop:
	case Continue:
	case ActiveSensing:
	case Reset:
		_dataPort.write(&octet, 1);
		break;
	default:
		// Invalid Real Time marker
		break;
	}

	_rtpMidi.endWrite(_dataPort);

	// Do not cancel Running Status for real-time messages as they can be
	// interleaved within any message. Though, TuneRequest can be sent here,
	// and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::internalSend(Session_t& session, MidiType inType, DataByte inData)
{
	_rtpMidi.ssrc = getSynchronizationSource();
	_rtpMidi.sequenceNr++;
	_rtpMidi.timestamp = _rtpMidiClock.Now();
	_rtpMidi.beginWrite(_dataPort, session.contentIP, session.contentPort);

	uint8_t length = 2;
	_dataPort.write(&length, 1);

	DataByte octet = (DataByte) inType;

	switch (inType)
	{
	case TimeCodeQuarterFrame: // Not really real-time, but one byte anyway.
		_dataPort.write(&octet, 1);
		_dataPort.write(&inData, 1);
		break;
	default:
		// Invalid Real Time marker
		break;
	}

	_rtpMidi.endWrite(_dataPort);

	// Do not cancel Running Status for real-time messages as they can be
	// interleaved within any message. Though, TuneRequest can be sent here,
	// and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::internalSend(Session_t& session, MidiType inType, DataByte inData1, DataByte inData2)
{
	_rtpMidi.ssrc = getSynchronizationSource();
	_rtpMidi.sequenceNr++;
	_rtpMidi.timestamp = _rtpMidiClock.Now();
	_rtpMidi.beginWrite(_dataPort, session.contentIP, session.contentPort);

	uint8_t length = 3;
	_dataPort.write(&length, 1);

	DataByte octet = (DataByte) inType;

	switch (inType)
	{
	case SongPosition: // Not really real-time, but one byte anyway.
		_dataPort.write(&octet, 1);
		_dataPort.write(&inData1, 1);
		_dataPort.write(&inData2, 1);
		break;
	default:
		// Invalid Real Time marker
		break;
	}

	_rtpMidi.endWrite(_dataPort);

	// Do not cancel Running Status for real-time messages as they can be
	// interleaved within any message. Though, TuneRequest can be sent here,
	// and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
}

template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::internalSendSysEx(Session_t& session, byte s, const byte* data, byte e, uint16_t length)
{
	_rtpMidi.ssrc = getSynchronizationSource();
	_rtpMidi.sequenceNr++;
	_rtpMidi.timestamp = _rtpMidiClock.Now();
	_rtpMidi.beginWrite(_dataPort, session.contentIP, session.contentPort);

	// always use long header
	uint16_t ss = length + 2;
	ss |= 1 << 15; // set B-Flag, long header!

	ss = AppleMIDI_Util::toEndian(ss); // use correct endian

	_dataPort.write(reinterpret_cast<uint8_t*>(&ss), sizeof(ss)); // write the 2 bytes
		
	_dataPort.write(&s, 1);
	_dataPort.write(data, length); 
	_dataPort.write(&e, 1);

	_rtpMidi.endWrite(_dataPort);

	// Do not cancel Running Status for real-time messages as they can be
	// interleaved within any message. Though, TuneRequest can be sent here,
	// and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
	mRunningStatus_TX = InvalidType;
#endif
}

template<class UdpClass>
inline StatusByte AppleMidi_Class<UdpClass>::getStatus(MidiType inType, Channel inChannel) const
{
	return ((byte) inType | ((inChannel - 1) & 0x0F));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------

/*! \brief Send a Note On message
\param inNoteNumber  Pitch value in the MIDI format (0 to 127).
\param inVelocity    Note attack velocity (0 to 127). A NoteOn with 0 velocity
is considered as a NoteOff.
\param inChannel     The channel on which the message will be sent (1 to 16).

Take a look at the values, names and frequencies of notes here:
http://www.phys.unsw.edu.au/jw/notes.html
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::noteOn(DataByte inNoteNumber, DataByte inVelocity, Channel  inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("< Note On (c=");
	DEBUGSTREAM.print(inChannel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(inNoteNumber);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(inVelocity);
	DEBUGSTREAM.println(")");
#endif

#if APPLEMIDI_USE_EVENTS
	if (mNoteOnSendingEvent != 0)
		mNoteOnSendingEvent(inChannel, inNoteNumber, inVelocity);
#endif

	send(NoteOn, inNoteNumber, inVelocity, inChannel);

#if APPLEMIDI_USE_EVENTS
	if (mNoteOnSendEvent != 0)
		mNoteOnSendEvent(inChannel, inNoteNumber, inVelocity);
#endif
}

/*! \brief Send a Note Off message
\param inNoteNumber  Pitch value in the MIDI format (0 to 127).
\param inVelocity    Release velocity (0 to 127).
\param inChannel     The channel on which the message will be sent (1 to 16).

Note: you can send NoteOn with zero velocity to make a NoteOff, this is based
on the Running Status principle, to avoid sending status messages and thus
sending only NoteOn data. This method will always send a real NoteOff message.
Take a look at the values, names and frequencies of notes here:
http://www.phys.unsw.edu.au/jw/notes.html
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::noteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("< Note Off (c=");
	DEBUGSTREAM.print(inChannel);
	DEBUGSTREAM.print(", n=");
	DEBUGSTREAM.print(inNoteNumber);
	DEBUGSTREAM.print(", v=");
	DEBUGSTREAM.print(inVelocity);
	DEBUGSTREAM.println(")");
#endif

#if APPLEMIDI_USE_EVENTS
	if (mNoteOffSendingEvent != 0)
		mNoteOffSendingEvent(inChannel, inNoteNumber, inVelocity);
#endif

	send(NoteOff, inNoteNumber, inVelocity, inChannel);

#if APPLEMIDI_USE_EVENTS
	if (mNoteOffSendEvent != 0)
		mNoteOffSendEvent(inChannel, inNoteNumber, inVelocity);
#endif
}

/*! \brief Send a Program Change message
\param inProgramNumber The Program to select (0 to 127).
\param inChannel       The channel on which the message will be sent (1 to 16).
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::programChange(DataByte inProgramNumber, Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("sendProgramChange ProgramNumber:");
	DEBUGSTREAM.print(inProgramNumber);
	DEBUGSTREAM.print(" Channel:");
	DEBUGSTREAM.println(inChannel);
#endif

	send(ProgramChange, inProgramNumber, 0, inChannel);
}

/*! \brief Send a Control Change message
\param inControlNumber The controller number (0 to 127).
\param inControlValue  The value for the specified controller (0 to 127).
\param inChannel       The channel on which the message will be sent (1 to 16).
@see MidiControlChangeNumber
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::controlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("sendControlChange Number:");
	DEBUGSTREAM.print(inControlNumber);
	DEBUGSTREAM.print(" Value:");
	DEBUGSTREAM.print(inControlValue);
	DEBUGSTREAM.print(" Channel:");
	DEBUGSTREAM.println(inChannel);
#endif

	send(ControlChange, inControlNumber, inControlValue, inChannel);
}

/*! \brief Send a Polyphonic AfterTouch message (applies to a specified note)
\param inNoteNumber  The note to apply AfterTouch to (0 to 127).
\param inPressure    The amount of AfterTouch to apply (0 to 127).
\param inChannel     The channel on which the message will be sent (1 to 16).
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::polyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("sendPolyPressure Note:");
	DEBUGSTREAM.print(inNoteNumber);
	DEBUGSTREAM.print(" Pressure:");
	DEBUGSTREAM.print(inPressure);
	DEBUGSTREAM.print(" Channel:");
	DEBUGSTREAM.println(inChannel);
#endif

	send(AfterTouchPoly, inNoteNumber, inPressure, inChannel);
}

/*! \brief Send a MonoPhonic AfterTouch message (applies to all notes)
\param inPressure    The amount of AfterTouch to apply to all notes.
\param inChannel     The channel on which the message will be sent (1 to 16).
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::afterTouch(DataByte inPressure, Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("sendafterTouch ");
	DEBUGSTREAM.print(" Pressure:");
	DEBUGSTREAM.print(inPressure);
	DEBUGSTREAM.print(" Channel:");
	DEBUGSTREAM.println(inChannel);
#endif

	send(AfterTouchChannel, inPressure, 0, inChannel);
}

/*! \brief Send a Pitch Bend message using a signed integer value.
\param inPitchValue  The amount of bend to send (in a signed integer format),
between MIDI_PITCHBEND_MIN and MIDI_PITCHBEND_MAX,
center value is 0.
\param inChannel     The channel on which the message will be sent (1 to 16).
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::pitchBend(int inPitchValue, Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("pitchBend ");
	DEBUGSTREAM.print(" Pitch:");
	DEBUGSTREAM.print(inPitchValue);
	DEBUGSTREAM.print(" Channel:");
	DEBUGSTREAM.println(inChannel);
#endif

	const unsigned int bend = inPitchValue - MIDI_PITCHBEND_MIN;
	send(PitchBend, (bend & 0x7F), (bend >> 7) & 0x7F, inChannel);
}


/*! \brief Send a Pitch Bend message using a floating point value.
\param inPitchValue  The amount of bend to send (in a floating point format),
between -1.0f (maximum downwards bend)
and +1.0f (max upwards bend), center value is 0.0f.
\param inChannel     The channel on which the message will be sent (1 to 16).
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::pitchBend(double inPitchValue, Channel inChannel)
{
	const int value = inPitchValue * MIDI_PITCHBEND_MAX;
	pitchBend(value, inChannel);
}

/*! \brief Generate and send a System Ex frame.
\param inLength  The size of the array to send
\param inArray   The byte array containing the data to send
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::sysEx(const byte* data, uint16_t length)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("sysEx ");
#endif

	const uint16_t contentLength = length - 2; //  remove start and end byte (SysExStart and SysExEnd)

	const int nrOfSegments = (contentLength % MIDI_SYSEX_ARRAY_SIZE_CONTENT == 0) ? contentLength / MIDI_SYSEX_ARRAY_SIZE_CONTENT : ((contentLength / MIDI_SYSEX_ARRAY_SIZE_CONTENT) + 1);
	const int lastSegment  = nrOfSegments - 1;

	byte s     = MidiType::SysExStart;
	byte e     = MidiType::SysExStart;
	uint16_t l = MIDI_SYSEX_ARRAY_SIZE_CONTENT;

	for (int i = 0; i < nrOfSegments; i++)
	{
		if (i == lastSegment)
		{
			e = MidiType::SysExEnd;
			l = contentLength - (lastSegment * MIDI_SYSEX_ARRAY_SIZE_CONTENT);
		}

		sendSysEx(s, data + 1 + (i * MIDI_SYSEX_ARRAY_SIZE_CONTENT), e, l); // Full SysEx Command

		s = MidiType::SysExEnd;
	}
}

/*! \brief Send a Tune Request message.

When a MIDI unit receives this message,
it should tune its oscillators (if equipped with any).
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::tuneRequest()
{
	send(TuneRequest);
}

/*! \brief .
A device sends out an Active Sense message (at least once) every 300 milliseconds
if there has been no other activity on the MIDI buss, to let other devices know
that there is still a good MIDI connection between the devices.

When a device receives an Active Sense message (from some other device), it should
expect to receive additional Active Sense messages at a rate of one approximately
every 300 milliseconds, whenever there is no activity on the MIDI buss during that
time. (Of course, if there are other MIDI messages happening at least once every 300
mSec, then Active Sense won't ever be sent. An Active Sense only gets sent if there
is a 300 mSec "moment of silence" on the MIDI buss. You could say that a device that
sends out Active Sense "gets nervous" if it has nothing to do for over 300 mSec, and
so sends an Active Sense just for the sake of reassuring other devices that this device
still exists). If a message is missed (ie, 0xFE nor any other MIDI message is received
for over 300 mSec), then a device assumes that the MIDI connection is broken, and
turns off all of its playing notes (which were turned on by incoming Note On messages,
versus ones played on the local keyboard by a musician). Of course, if a device never
receives an Active Sense message to begin with, it should not expect them at all. So,
it takes one "nervous" device to start the process by initially sending out an Active
Sense message to the other connected devices during a 300 mSec moment of silence
on the MIDI bus.

(http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec/sense.htm)
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::activeSensing()
{
	send(ActiveSensing);
}

/*! \brief
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::start()
{
	send(Start);
}

/*! \brief
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::_continue()
{
	send(Continue);
}

/*! \brief
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::stop()
{
	send(Stop);
}

/*! \brief Send a MIDI Time Code Quarter Frame.

\param inTypeNibble      MTC type
\param inValuesNibble    MTC data
See MIDI Specification for more information.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::timeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble)
{
	const byte data = (((inTypeNibble & 0x07) << 4) | (inValuesNibble & 0x0F));
	timeCodeQuarterFrame(data);
}

/*! \brief Send a MIDI Time Code Quarter Frame.

See MIDI Specification for more information.
\param inData  if you want to encode directly the nibbles in your program,
you can send the byte here.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::timeCodeQuarterFrame(DataByte inData)
{
	send(TimeCodeQuarterFrame, inData);
}

/*! \brief Send a Song Position Pointer message.
\param inBeats    The number of beats since the start of the song.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::songPosition(unsigned short inBeats)
{
	byte octet1 = inBeats & 0x7F;
	byte octet2 = (inBeats >> 7) & 0x7F;

	send(SongPosition, octet1, octet2);
}

/*! \brief Send a Song Select message */
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::songSelect(DataByte inSongNumber)
{
	byte octet = inSongNumber & 0x7F;

	send(SongSelect, octet);
}


/*! \brief Send a Song Select message */
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::reset()
{
	send(Reset);
}

/*! \brief Send a Song Select message */
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::clock()
{
	send(Clock);
}

/*! \brief Send a Song Select message */
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::tick()
{
	send(Tick);
}

#endif

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------

#if APPLEMIDI_USE_CALLBACKS

template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnConnected(void(*fptr)(uint32_t, char*))    { mConnectedCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnDisconnected(void(*fptr)(uint32_t))      { mDisconnectedCallback = fptr; }

template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveNoteOff(void(*fptr)(byte channel, byte note, byte velocity))          { mNoteOffCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveNoteOn(void(*fptr)(byte channel, byte note, byte velocity))           { mNoteOnCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveAfterTouchPoly(void(*fptr)(byte channel, byte note, byte pressure))   { mAfterTouchPolyCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveControlChange(void(*fptr)(byte channel, byte number, byte value))     { mControlChangeCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveProgramChange(void(*fptr)(byte channel, byte number))                 { mProgramChangeCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveAfterTouchChannel(void(*fptr)(byte channel, byte pressure))           { mAfterTouchChannelCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceivePitchBend(void(*fptr)(byte channel, int bend))                        { mPitchBendCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveSysEx(void(*fptr)(const byte* array, uint16_t size))                  { mSysExCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveTimeCodeQuarterFrame(void(*fptr)(byte data))                          { mTimeCodeQuarterFrameCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveSongPosition(void(*fptr)(unsigned short beats))                       { mSongPositionCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveSongSelect(void(*fptr)(byte songnumber))                              { mSongSelectCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveTuneRequest(void(*fptr)(void))                                        { mTuneRequestCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveClock(void(*fptr)(void))                                              { mClockCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveStart(void(*fptr)(void))                                              { mStartCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveContinue(void(*fptr)(void))                                           { mContinueCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveStop(void(*fptr)(void))                                               { mStopCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveActiveSensing(void(*fptr)(void))                                      { mActiveSensingCallback = fptr; }
template<class UdpClass> inline void AppleMidi_Class<UdpClass>::OnReceiveReset(void(*fptr)(void))                                              { mResetCallback = fptr; }

#endif

END_APPLEMIDI_NAMESPACE
