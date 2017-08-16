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
		Serial.print("SessionName exceeds ");
		Serial.print(sessionName);
		Serial.print(" exceeds ");
		Serial.print(SESSION_NAME_MAX_LEN);
		Serial.println(" chars. Name will be clipped.");
	}
#endif

	strncpy(_sessionName, sessionName, SESSION_NAME_MAX_LEN);

	Port = port;

	_inputChannel = MIDI_CHANNEL_OMNI;

	// Initialize Sessions
	DeleteSessions();

	// open UDP socket for control messages
	_controlUDP.begin(Port);
	// open UDP socket for rtp messages
	_contentUDP.begin(Port + 1);

	_controlDissector.init(Port, this);
	_controlDissector.addPacketDissector(&PacketAppleMidi::dissect_apple_midi);	// Add parser

	_contentDissector.init(Port + 1, this);
	_contentDissector.addPacketDissector(&PacketRtpMidi::dissect_rtp_midi);		// Add parser
	_contentDissector.addPacketDissector(&PacketAppleMidi::dissect_apple_midi);	// Add parser

	_rtpMidi.ssrc = 0; // will be initialized when first used
	_rtpMidi.sequenceNr = 1;

#if (APPLEMIDI_DEBUG)
	Serial.print("Starting Session ");
	Serial.print(_sessionName);
	Serial.print(" on port ");
	Serial.println(Port);
#endif

	return true;
}

/*! \brief Evaluates incoming Rtp messages.
*/
template<class UdpClass>
inline void AppleMidi_Class<UdpClass>::run()
{
	// Process one control packet, if available
	int packetSize = _controlUDP.parsePacket();
	if (packetSize) {
		packetSize = _controlUDP.read(_packetBuffer, sizeof(_packetBuffer));
		_controlDissector.addPacket(_packetBuffer, packetSize);
	}

	// Process one control packet, if available
	packetSize = _contentUDP.parsePacket();
	if (packetSize) {
		packetSize = _contentUDP.read(_packetBuffer, sizeof(_packetBuffer));
		_contentDissector.addPacket(_packetBuffer, packetSize);
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
		// The time between booting and needing the SSRC for the first time (first  network traffic) is
		// a good enough random seed.
		long seed = (long)micros();
		randomSeed(seed);

		// not full range of UINT32_MAX (unsigned!), but (signed) long should suffice
		_ssrc = random(1, LONG_MAX);

#if (APPLEMIDI_DEBUG)
		Serial.print("Lazy init of SSRC. Value is 0x");
		Serial.println(_ssrc, HEX);
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
	Serial.println("Queued invite");
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
	Serial.print("> End Session for ssrc 0x");
	Serial.print(sessionEnd.ssrc, HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print(", initiatorToken = 0x");
	Serial.print(sessionEnd.initiatorToken, HEX);
#endif
	Serial.println();
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
	Serial.print("> Receiver Feedback: seq = ");
	Serial.println(receiverFeedback.sequenceNr);
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
	Serial.print("> (OnControlInvitationAccepted) Control InvitationAccepted: peer = \"");
	Serial.print(invitationAccepted.sessionName);
	Serial.print("\"");
	Serial.print(" ,ssrc 0x");
	Serial.print(invitationAccepted.ssrc, HEX);
	Serial.print(" ,initiatorToken = 0x");
	Serial.print(invitationAccepted.initiatorToken, HEX);
	Serial.println();
#endif

	CompleteLocalSessionControl(invitationAccepted);
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnContentInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted& invitationAccepted)
{
#if (APPLEMIDI_DEBUG)
	Serial.print("> Content InvitationAccepted: peer = \"");
	Serial.print(invitationAccepted.sessionName);
	Serial.print("\"");
	Serial.print(" ,ssrc 0x");
	Serial.print(invitationAccepted.ssrc, HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print(" ,initiatorToken = 0x");
	Serial.print(invitationAccepted.initiatorToken, HEX);
#endif
	Serial.println();
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
	Serial.print("> (OnControlInvitation) Control Invitation: peer = \"");
	Serial.print(invitation.sessionName);
	Serial.print("\"");
	Serial.print(" ,ssrc 0x");
	Serial.print(invitation.ssrc, HEX);
	Serial.print(" ,initiatorToken = 0x");
	Serial.print(invitation.initiatorToken, HEX);
	Serial.println();
#endif

	// Do we know this ssrc already?
	// In case initiator reconnects (after a crash of some sort)
	int index = GetSessionSlotUsingSSrc(invitation.ssrc);
	if (index < 0)
	{
		#if (APPLEMIDI_DEBUG)
		Serial.println("Invition received for a new session.");
		#endif

		// No, not existing; must be a new initiator
		// Find a free slot to remember this session in
		index = GetFreeSessionSlot();
		if (index < 0)
		{
			#if (APPLEMIDI_DEBUG)
			Serial.println("Session invitation rejected.");
			#endif

			// no free slots, we cant accept invite
			AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
			write(_controlUDP, invitationRejected, _controlUDP.remoteIP(), _controlUDP.remotePort());

			return;
		} else {
			// Initiate a session got this ssrc
			CreateLocalSession(index, invitation.ssrc);
		}
	} else {
		#if (APPLEMIDI_DEBUG)
		Serial.println("Session exists");
		#endif
	}

	// Send the invitation acceptance packet
	AppleMIDI_InvitationAccepted acceptInvitation(getSynchronizationSource(), invitation.initiatorToken, getSessionName());
	write(_controlUDP, acceptInvitation, _controlUDP.remoteIP(), _controlUDP.remotePort());

	#if (APPLEMIDI_DEBUG)
		Serial.print("< (OnControlInvitation) Control InvitationAccepted: peer = \"");
		Serial.print(getSessionName());
		Serial.print("\"");
		Serial.print(" ,ssrc 0x");
		Serial.print(getSynchronizationSource(), HEX);
		Serial.print(" ,initiatorToken = 0x");
		Serial.print(invitation.initiatorToken, HEX);
	#if (APPLEMIDI_DEBUG_VERBOSE)
		Serial.print(" ,in slot = ");
		Serial.print(index);
	#endif
		Serial.println();
	#endif
}

/*! \brief .
*/
template<class UdpClass>
void AppleMidi_Class<UdpClass>::OnContentInvitation(void* sender, AppleMIDI_Invitation& invitation)
{
	//Dissector* dissector = (Dissector*) sender;

#if (APPLEMIDI_DEBUG)
	Serial.print("> Content Invitation: peer = \"");
	Serial.print(invitation.sessionName);
	Serial.print("\"");
	Serial.print(" ,ssrc 0x");
	Serial.print(invitation.ssrc, HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print(" ,initiatorToken = 0x");
	Serial.print(invitation.initiatorToken, HEX);
#endif
	Serial.println();
#endif

	// Find the slot, it should be there because created by control session
	int i = GetSessionSlotUsingSSrc(invitation.ssrc);
	if (i < 0)
	{
#if (APPLEMIDI_DEBUG)
		Serial.print("Error - control session does not exists for ");
		Serial.print(invitation.ssrc, HEX);
		Serial.print(". Rejecting invitation.");
#endif
		AppleMIDI_InvitationRejected invitationRejected(invitation.ssrc, invitation.initiatorToken, invitation.sessionName);
		write(_contentUDP, invitationRejected, _contentUDP.remoteIP(), _contentUDP.remotePort());

		return;
	}

	AppleMIDI_InvitationAccepted acceptInvitation(getSynchronizationSource(), invitation.initiatorToken, getSessionName());
	write(_contentUDP, acceptInvitation, _contentUDP.remoteIP(), _contentUDP.remotePort());

	// Send bitrate limit
	AppleMIDI_BitrateReceiveLimit rateLimit;
	write(_controlUDP, rateLimit, _controlUDP.remoteIP(), _controlUDP.remotePort());

#if (APPLEMIDI_DEBUG)
	Serial.print("< Content InvitationAccepted: peer = \"");
	Serial.print(getSessionName());
	Serial.print("\"");
	Serial.print(" ,ssrc 0x");
	Serial.print(getSynchronizationSource(), HEX);
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print(" ,initiatorToken = 0x");
	Serial.print(invitation.initiatorToken, HEX);
	Serial.print(" ,in slot = ");
	Serial.print(i);
#endif
	Serial.println();
#endif

	Sessions[i].contentIP = _contentUDP.remoteIP();
	Sessions[i].contentPort = _contentUDP.remotePort();
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
	Serial.print("> Syncronization for ssrc 0x");
	Serial.print(synchronization.ssrc, HEX);
	Serial.print(", count = ");
	Serial.print(synchronization.count);
#if (APPLEMIDI_DEBUG_VERBOSE)
	//Serial.print  (" Timestamps = ");
	//Serial.print  (synchronization.timestamps[0], HEX);
	//Serial.print  (" ");
	//Serial.print  (synchronization.timestamps[1], HEX);
	//Serial.print  (" ");
	//Serial.print  (synchronization.timestamps[2], HEX);
#endif
	Serial.println("");
#endif

	// If we don't know this session, ignore it.

	int index = GetSessionSlotUsingSSrc(synchronization.ssrc);
	if (index < 0)
	{
#if (APPLEMIDI_DEBUG)
		Serial.println("hmmm - Syncronization for a session that has never started.");
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
	write(_contentUDP, synchronizationResponse, _contentUDP.remoteIP(), _contentUDP.remotePort());

#if (APPLEMIDI_DEBUG)
	Serial.print("< Syncronization for ssrc 0x");
	Serial.print(getSynchronizationSource(), HEX);
	Serial.print(", count = ");
	Serial.print(synchronizationResponse.count);
#if (APPLEMIDI_DEBUG_VERBOSE)
	//Serial.print  (" Timestamps = ");
	//Serial.print  (synchronization.timestamps[0], HEX);
	//Serial.print  (" ");
	//Serial.print  (synchronization.timestamps[1], HEX);
	//Serial.print  (" ");
	//Serial.print  (synchronization.timestamps[2], HEX);
#endif
	Serial.println("");
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
	Serial.print("> BitrateReceiveLimit: Limit = ");
	Serial.println(bitrateReceiveLimit.bitratelimit);
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
	Serial.print("> Note On (c=");
	Serial.print(channel);
	Serial.print(", n=");
	Serial.print(note);
	Serial.print(", v=");
	Serial.print(velocity);
	Serial.println(")");
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
	Serial.print("> Note Off (c=");
	Serial.print(channel);
	Serial.print(", n=");
	Serial.print(note);
	Serial.print(", v=");
	Serial.print(velocity);
	Serial.println(")");
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
	Serial.print("> Poly Pressure (c=");
	Serial.print(channel);
	Serial.print(", n=");
	Serial.print(note);
	Serial.print(", p=");
	Serial.print(pressure);
	Serial.println(")");
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
	Serial.print("> Channel Pressure (c=");
	Serial.print(channel);
	Serial.print(", p=");
	Serial.print(pressure);
	Serial.println(")");
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
	Serial.print("> Pitch Bend (c=");
	Serial.print(channel);
	Serial.print(", p=");
	Serial.print(pitch);
	Serial.println(")");
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
	Serial.print("> Program Change (c=");
	Serial.print(channel);
	Serial.print(", p=");
	Serial.print(program);
	Serial.print(") ");

	switch (program)
	{
	default:
		Serial.println("Other");
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
	Serial.print("> Control Change (c=");
	Serial.print(channel);
	Serial.print(", e=");
	Serial.print(controller);
	Serial.print(", v=");
	Serial.print(value);
	Serial.print(") ");

	switch (controller)
	{
	case BankSelect:
		Serial.println("BankSelect");
		break;
	case ModulationWheel:
		Serial.println("ModulationWheel");
		break;
	case BreathController:
		Serial.println("BreathController");
		break;
	case FootController:
		Serial.println("FootController");
		break;
	case PortamentoTime:
		Serial.println("PortamentoTime");
		break;
	case DataEntry:
		Serial.println("DataEntry");
		break;
	case ChannelVolume:
		Serial.println("ChannelVolume");
		break;
	case Balance:
		Serial.println("Balance");
		break;
	case Pan:
		Serial.println("Pan");
		break;
	case ExpressionController:
		Serial.println("ExpressionController");
		break;
	case EffectControl1:
		Serial.println("EffectControl1");
		break;
	case EffectControl2:
		Serial.println("EffectControl2");
		break;
	case GeneralPurposeController1:
		Serial.println("GeneralPurposeController1");
		break;
	case GeneralPurposeController2:
		Serial.println("GeneralPurposeController2");
		break;
	case GeneralPurposeController3:
		Serial.println("GeneralPurposeController3");
		break;
	case GeneralPurposeController4:
		Serial.println("GeneralPurposeController4");
		break;
	case Sustain:
		Serial.println("Sustain");
		break;
	case Portamento:
		Serial.println("Portamento");
		break;
	case Sostenuto:
		Serial.println("Sostenuto");
		break;
	case SoftPedal:
		Serial.println("SoftPedal");
		break;
	case Legato:
		Serial.println("Legato");
		break;
	case Hold:
		Serial.println("Hold");
		break;
	case SoundController1:
		Serial.println("SoundController1");
		break;
	case SoundController2:
		Serial.println("SoundController2");
		break;
	case SoundController3:
		Serial.println("SoundController3");
		break;
	case SoundController4:
		Serial.println("SoundController4");
		break;
	case SoundController5:
		Serial.println("SoundController5");
		break;
	case SoundController6:
		Serial.println("SoundController6");
		break;
	case SoundController7:
		Serial.println("SoundController7");
		break;
	case SoundController8:
		Serial.println("SoundController8");
		break;
	case SoundController9:
		Serial.println("SoundController9");
		break;
	case SoundController10:
		Serial.println("SoundController10");
		break;
	case GeneralPurposeController5:
		Serial.println("GeneralPurposeController5");
		break;
	case GeneralPurposeController6:
		Serial.println("GeneralPurposeController6");
		break;
	case GeneralPurposeController7:
		Serial.println("GeneralPurposeController7");
		break;
	case GeneralPurposeController8:
		Serial.println("GeneralPurposeController8");
		break;
	case PortamentoControl:
		Serial.println("PortamentoControl");
		break;
	case Effects1:
		Serial.println("Effects1");
		break;
	case Effects2:
		Serial.println("Effects2");
		break;
	case Effects3:
		Serial.println("Effects3");
		break;
	case Effects4:
		Serial.println("Effects4");
		break;
	case Effects5:
		Serial.println("Effects5");
		break;
	case AllSoundOff:
		Serial.println("AllSoundOff");
		break;
	case ResetAllControllers:
		Serial.println("ResetAllControllers");
		break;
	case LocalControl:
		Serial.println("LocalControl");
		break;
	case AllNotesOff:
		Serial.println("AllNotesOff");
		break;
	case OmniModeOff:
		Serial.println("OmniModeOff");
		break;
	case OmniModeOn:
		Serial.println("OmniModeOn");
		break;
	case MonoModeOn:
		Serial.println("MonoModeOn");
		break;
	case PolyModeOn:
		Serial.println("PolyModeOn");
		break;
	default:
		Serial.println("Other");
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
	Serial.print("> SongSelect (s=");
	Serial.print(songNr);
	Serial.println(")");
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
	Serial.print("> SongPosition (c=");
	Serial.print(value);
	Serial.println(")");
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
	Serial.print("> TimeCodeQuarterFrame (c=");
	Serial.print(value);
	Serial.println(")");
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
	Serial.print("> TuneRequest ()");
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
	Serial.print("> Clock ()");
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
	Serial.print("> Start ()");
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
	Serial.print("> Continue ()");
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
	Serial.print("> Stop ()");
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
	Serial.print("> ActiveSensing ()");
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
	Serial.print("> Reset ()");
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
	Serial.print("> SysEx ()");
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
		Serial.println("hmm, initiatorToken not found");
#endif
		return;
	}

	//
	if (Sessions[i].invite.status != WaitingForControlInvitationAccepted)
	{
#if (APPLEMIDI_DEBUG) // issue warning
		Serial.println("status not what expected");
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
		Serial.println("hmm, initiatorToken not found");
#endif
		return;
	}

	//
	if (Sessions[i].invite.status != WaitingForContentInvitationAccepted)
	{
#if (APPLEMIDI_DEBUG) // issue warning
		Serial.println("status not what expected");
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
	Serial.print  ("New Local Session in slot ");
	Serial.print  (i);
	Serial.print  (" with SSRC 0x");
	Serial.println(ssrc, HEX);
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
		Serial.println("Invite: No free slot availble, invitation cancelled.");
#endif
		return;
	}

#if (APPLEMIDI_DEBUG)
	Serial.print("New Remote Session in slot ");
	Serial.println(i);
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
	Serial.print("Freeing Session slot ");
	Serial.println(slot);
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
		Serial.print("Slot ");
		Serial.print(i);
		Serial.print(" ssrc = 0x");
		Serial.println(Sessions[i].ssrc, HEX);
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
			write(_controlUDP, invitation, session->invite.remoteHost, session->invite.remotePort);

			session->invite.initiatorToken = invitation.initiatorToken;
			session->invite.lastSend = millis();
			session->invite.attempts = 1;
			session->invite.status = WaitingForControlInvitationAccepted;

#if (APPLEMIDI_DEBUG)
			Serial.print("< (ManageInvites) Control Invitation: peer = \"");
			Serial.print(invitation.sessionName);
			Serial.print("\"");
			Serial.print(" ,ssrc 0x");
			Serial.print(invitation.ssrc, HEX);
			Serial.print(" ,Attempt = ");
			Serial.print(session->invite.attempts);
			Serial.print(" ,initiatorToken = 0x");
			Serial.print(invitation.initiatorToken, HEX);
			Serial.println();
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
				write(_controlUDP, invitation, session->invite.remoteHost, session->invite.remotePort);

				session->invite.lastSend = millis();
				session->invite.attempts++;

#if (APPLEMIDI_DEBUG)
				Serial.print("< (ManageInvites2) Control Invitation: peer = \"");
				Serial.print(invitation.sessionName);
				Serial.print("\"");
				Serial.print(" ,ssrc 0x");
				Serial.print(invitation.ssrc, HEX);
				Serial.print(" ,Attempt = ");
				Serial.print(session->invite.attempts);
				Serial.print(" ,initiatorToken = 0x");
				Serial.print(invitation.initiatorToken, HEX);
				Serial.println();
#endif
			}
		}
		else if (session->invite.status == SendContentInvite)
		{
			AppleMIDI_Invitation invitation;
			invitation.initiatorToken = session->invite.initiatorToken;
			invitation.ssrc = getSynchronizationSource();
			strncpy(invitation.sessionName, getSessionName(), SESSION_NAME_MAX_LEN);
			write(_contentUDP, invitation, session->invite.remoteHost, session->invite.remotePort + 1);

			session->invite.lastSend = millis();
			session->invite.attempts = 1;
			session->invite.status = WaitingForContentInvitationAccepted;

#if (APPLEMIDI_DEBUG)
			Serial.print("< Content Invitation: peer = \"");
			Serial.print(invitation.sessionName);
			Serial.print("\"");
			Serial.print(" ,ssrc 0x");
			Serial.print(invitation.ssrc, HEX);
			Serial.print(" ,Attempt = ");
			Serial.print(session->invite.attempts);
			Serial.print(" ,initiatorToken = 0x");
			Serial.print(invitation.initiatorToken, HEX);
			Serial.println();
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
				write(_contentUDP, invitation, session->invite.remoteHost, session->invite.remotePort + 1);

				session->invite.lastSend = millis();
				session->invite.attempts++;

#if (APPLEMIDI_DEBUG)
				Serial.print("< Content Invitation: peer = \"");
				Serial.print(invitation.sessionName);
				Serial.print("\"");
				Serial.print(" ,ssrc 0x");
				Serial.print(invitation.ssrc, HEX);
				Serial.print(" ,Attempt = ");
				Serial.print(session->invite.attempts);
				Serial.print(" ,initiatorToken = 0x");
				Serial.print(invitation.initiatorToken, HEX);
				Serial.println();
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
					write(_contentUDP, synchronizationResponse, Sessions[i].contentIP, Sessions[i].contentPort);

#if (APPLEMIDI_DEBUG)
					Serial.print("< Syncronization for ssrc 0x");
					Serial.print(synchronizationResponse.ssrc, HEX);
					Serial.print(", count = ");
					Serial.print(synchronizationResponse.count);
					Serial.print(", to = ");
					Serial.print(Sessions[i].contentIP);
					Serial.print(" ");
					Serial.print(Sessions[i].contentPort);
#if (APPLEMIDI_DEBUG_VERBOSE)
					//Serial.print  (" Timestamps = ");
					//Serial.print  (synchronizationResponse.timestamps[0], HEX);
					//Serial.print  (" ");
					//Serial.print  (synchronizationResponse.timestamps[1], HEX);
					//Serial.print  (" ");
					//Serial.print  (synchronizationResponse.timestamps[2], HEX);
#endif
					Serial.println("");
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
	//	synchronizationResponse.write(&_contentUDP);
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
	Serial.print("Syncronization timestamps: 0x");
	Serial.print((uint32_t)sy.timestamps[0], HEX);
	Serial.print(", 0x");
	Serial.print((uint32_t)sy.timestamps[1], HEX);
	Serial.print(", 0x");
	Serial.print((uint32_t)sy.timestamps[2], HEX);
	Serial.println("");
#endif

	sy.ssrc = AppleMIDI_Util::toEndian(sy.ssrc);
	sy.count = AppleMIDI_Util::toEndian(sy.count);
	sy.timestamps[0] = AppleMIDI_Util::toEndian(sy.timestamps[0]);
	sy.timestamps[1] = AppleMIDI_Util::toEndian(sy.timestamps[1]);
	sy.timestamps[2] = AppleMIDI_Util::toEndian(sy.timestamps[2]);
	size_t bytesWritten = udp.write(reinterpret_cast<uint8_t*>(&sy), sizeof(sy));
	Debug::Assert(bytesWritten == sizeof(sy), "error writing sy");

/*	
	PacketWriter<UdpClass> writer(udp);
	writer.write(sy.signature[0]);
	writer.write(sy.signature[1]);
	writer.write(sy.command[0]);
	writer.write(sy.command[1]);
	writer.write(sy.ssrc);
	writer.write(sy.count);
	writer.writePadding(3);
	writer.write(sy.timestamps[0]);
	writer.write(sy.timestamps[1]);
	writer.write(sy.timestamps[2]);
	Debug::Assert(writer.allWritten(), "error writing sy");
*/
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
		_rtpMidi.beginWrite(_contentUDP, session.contentIP, session.contentPort);

		// Length
		uint8_t length = 16; // Add the P-Flag and then length octet
		if (inType == ProgramChange || inType == AfterTouchChannel) {
			length |= 2;
		}
		else {
			length |= 3;
		}
		_contentUDP.write(&length, 1);

		// Command Section

		const StatusByte status = getStatus(inType, inChannel);

#if APPLEMIDI_USE_RUNNING_STATUS
		// Check Running Status
		if (mRunningStatus_TX != status)
		{
			// New message, memorise and send header
			mRunningStatus_TX = status;
			_contentUDP.write(&mRunningStatus_TX, 1);
		}
#else
		// Don't care about running status, send the status byte.
		_contentUDP.write(&status, sizeof(status));
#endif

		// Then send data
		_contentUDP.write(&inData1, sizeof(inData1));
		if (inType != ProgramChange && inType != AfterTouchChannel)
			_contentUDP.write(&inData2, sizeof(inData2));

		_rtpMidi.endWrite(_contentUDP);

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
	_rtpMidi.beginWrite(_contentUDP, session.contentIP, session.contentPort);

	uint8_t length = 1;
	_contentUDP.write(&length, 1);

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
		_contentUDP.write(&octet, 1);
		break;
	default:
		// Invalid Real Time marker
		break;
	}

	_rtpMidi.endWrite(_contentUDP);

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
	_rtpMidi.beginWrite(_contentUDP, session.contentIP, session.contentPort);

	uint8_t length = 2;
	_contentUDP.write(&length, 1);

	DataByte octet = (DataByte) inType;

	switch (inType)
	{
	case TimeCodeQuarterFrame: // Not really real-time, but one byte anyway.
		_contentUDP.write(&octet, 1);
		_contentUDP.write(&inData, 1);
		break;
	default:
		// Invalid Real Time marker
		break;
	}

	_rtpMidi.endWrite(_contentUDP);

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
	_rtpMidi.beginWrite(_contentUDP, session.contentIP, session.contentPort);

	uint8_t length = 3;
	_contentUDP.write(&length, 1);

	DataByte octet = (DataByte) inType;

	switch (inType)
	{
	case SongPosition: // Not really real-time, but one byte anyway.
		_contentUDP.write(&octet, 1);
		_contentUDP.write(&inData1, 1);
		_contentUDP.write(&inData2, 1);
		break;
	default:
		// Invalid Real Time marker
		break;
	}

	_rtpMidi.endWrite(_contentUDP);

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
	_rtpMidi.beginWrite(_contentUDP, session.contentIP, session.contentPort);

	// always use long header
	uint16_t ss = length + 2;
	ss |= 1 << 15; // set B-Flag, long header!

	ss = AppleMIDI_Util::toEndian(ss); // use correct endian

	_contentUDP.write(reinterpret_cast<uint8_t*>(&ss), sizeof(ss)); // write the 2 bytes
		
	_contentUDP.write(&s, 1);
	_contentUDP.write(data, length); 
	_contentUDP.write(&e, 1);

	_rtpMidi.endWrite(_contentUDP);

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
	Serial.print("< Note On (c=");
	Serial.print(inChannel);
	Serial.print(", n=");
	Serial.print(inNoteNumber);
	Serial.print(", v=");
	Serial.print(inVelocity);
	Serial.println(")");
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
	Serial.print("< Note Off (c=");
	Serial.print(inChannel);
	Serial.print(", n=");
	Serial.print(inNoteNumber);
	Serial.print(", v=");
	Serial.print(inVelocity);
	Serial.println(")");
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
	Serial.print("sendProgramChange ProgramNumber:");
	Serial.print(inProgramNumber);
	Serial.print(" Channel:");
	Serial.println(inChannel);
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
	Serial.print("sendControlChange Number:");
	Serial.print(inControlNumber);
	Serial.print(" Value:");
	Serial.print(inControlValue);
	Serial.print(" Channel:");
	Serial.println(inChannel);
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
	Serial.print("sendPolyPressure Note:");
	Serial.print(inNoteNumber);
	Serial.print(" Pressure:");
	Serial.print(inPressure);
	Serial.print(" Channel:");
	Serial.println(inChannel);
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
	Serial.print("sendafterTouch ");
	Serial.print(" Pressure:");
	Serial.print(inPressure);
	Serial.print(" Channel:");
	Serial.println(inChannel);
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
	Serial.print("pitchBend ");
	Serial.print(" Pitch:");
	Serial.print(inPitchValue);
	Serial.print(" Channel:");
	Serial.println(inChannel);
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
	Serial.print("sysEx ");
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
