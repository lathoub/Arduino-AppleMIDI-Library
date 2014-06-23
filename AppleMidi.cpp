/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.4
 *  @author		lathoub 
 *	@date		13/04/14
 *  License		GPL
 */

// http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec.htm

#include "AppleMidi.h"

#include "utility/packet-rtp-midi.h"
#include "utility/packet-apple-midi.h"

#if !(APPLEMIDI_BUILD_INPUT) && !(APPLEMIDI_BUILD_OUTPUT)
#   error To use AppleMIDI, you need to enable at least input or output.
#endif

#if APPLEMIDI_AUTO_INSTANCIATE && defined(ARDUINO)
    APPLEMIDI_CREATE_INSTANCE;
#endif

BEGIN_APPLEMIDI_NAMESPACE

Session_t	AppleMidi_Class::Sessions[MAX_SESSIONS];

/*! \brief Default constructor for MIDI_Class. */
AppleMidi_Class::AppleMidi_Class()
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
	mSystemExclusiveCallback		= NULL;
	mTimeCodeQuarterFrameCallback	= NULL;
	mSongPositionCallback			= NULL;
	mSongSelectCallback				= NULL;
	mTuneRequestCallback			= NULL;
	mClockCallback					= NULL;
	mStartCallback					= NULL;
	mContinueCallback				= NULL;
	mStopCallback					= NULL;
	mActiveSensingCallback			= NULL;
	mSystemResetCallback			= NULL;
#endif

#if APPLEMIDI_USE_EVENTS
	mNoteOnSendingEvent				= NULL;
	mNoteOnSendEvent				= NULL;
	mNoteOffSendingEvent			= NULL;
	mNoteOffSendEvent				= NULL;
#endif
}

/*! \brief Default destructor for MIDI_Class.
 
 This is not really useful for the Arduino, as it is never called...
 */
AppleMidi_Class::~AppleMidi_Class()
{
}

/*! \brief Call the begin method in the setup() function of the Arduino.
 
 All parameters are set to their default values:
 - Input channel set to 1 if no value is specified
 - Full thru mirroring
 */
void AppleMidi_Class::begin(const char* name)
{
	srand(analogRead(0));

	//
	strcpy(Name, name);

	_inputChannel = MIDI_CHANNEL_OMNI;

	// Generate Synchronization Source
	// Unique 32 bit number (see definition)
	byte buffer[4];
	for (int i = 0; i < 4; i++)
		buffer[i] = 17 + (rand() % 255);
	_ssrc = *((uint32_t*)&buffer[0]);

	for (int i = 0; i < MAX_SESSIONS; i++)
		Sessions[i].ssrc = 0;

	// open UDP socket for control messages
	_controlUDP.begin(Port);
	// open UDP socket for rtp messages
	_contentUDP.begin(Port + 1);

	_controlDissector.init(Port, this);
	_controlDissector.addPacketDissector(&PacketAppleMidi::dissect_apple_midi);	// Add parser

	_contentDissector.init(Port + 1, this);
	_contentDissector.addPacketDissector(&PacketRtpMidi::dissect_rtp_midi);		// Add parser
	_contentDissector.addPacketDissector(&PacketAppleMidi::dissect_apple_midi);	// Add parser

	_rtpMidi.ssrc = _ssrc;
	_rtpMidi.sequenceNr = 1;

	// Note: this class does not start a session
	// by sending an invite, it waits for
	// the invitation to come in.

#if (APPLEMIDI_DEBUG)
	Serial.print  ("Starting");
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  (" Verbose");
#endif
	Serial.println(" logging");
#endif
}

/*! \brief Evaluates incoming Rtp messages.
  */
void AppleMidi_Class::run()
{
	byte _packetBuffer[UDP_TX_PACKET_MAX_SIZE];

	// 
	int packetSize = _controlUDP.parsePacket();
	while (_controlUDP.available() > 0)
	{
		int bytesRead = _controlUDP.read(_packetBuffer, UDP_TX_PACKET_MAX_SIZE);

		_controlDissector.addPacket(_packetBuffer, bytesRead);
		_controlDissector.dissect();
	}

	// 
	packetSize = _contentUDP.parsePacket();
	while (_contentUDP.available() > 0)
	{
		int bytesRead = _contentUDP.read(_packetBuffer, UDP_TX_PACKET_MAX_SIZE);

		_contentDissector.addPacket(_packetBuffer, bytesRead);
		_contentDissector.dissect();
	}
}

/*! \brief .
*/
void AppleMidi_Class::OnInvitation(void* sender, Invitation_t& invitation)
{
	Dissector* dissector = (Dissector*)sender;

	if (dissector->_identifier == Port)
		OnControlInvitation(sender, invitation);
	if (dissector->_identifier == Port + 1)
		OnContentInvitation(sender, invitation);
}

/*! \brief .
*/
void AppleMidi_Class::OnControlInvitation(void* sender, Invitation_t& invitation)
{
	Dissector* dissector = (Dissector*)sender;

#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Invitation: peer = \"");
	Serial.print  (invitation.name);
	Serial.print  ("\"");
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  (" ,ssrc = 0x");
	Serial.print  (invitation.ssrc, HEX);
	Serial.print  (" ,initiatorToken = 0x");
	Serial.print  (invitation.initiatorToken, HEX);
#endif
	Serial.println();
#endif

	// If we know this session already, ignore it.
	int index;
	for (index = 0; index < MAX_SESSIONS; index++)
		if (Sessions[index].ssrc == invitation.ssrc)
			break;
	if (index < MAX_SESSIONS)
	{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("Session exists already (maybe a resend of the invite). Free existing slot, then accept again.");
	Sessions[index].ssrc = 0;
		// session exists already (maybe a resend of the invite, ignore)
#endif
//		return; 
	}

	// Find a free slot to remember this session in
	for (index = 0; index < MAX_SESSIONS; index++)
		if (0 == Sessions[index].ssrc)
			break;
	if (index >= MAX_SESSIONS)
	{
		// no free slots, we cant accept invite
		return;
	}

#if (APPLEMIDI_DEBUG) && (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  ("New session at slot: ");
	Serial.println(index);
		// session exists already (maybe a resend of the invite, ignore)
#endif

	// initiatorToken
	Sessions[index].initiatorToken = invitation.initiatorToken;
	Sessions[index].ssrc = invitation.ssrc;
	Sessions[index].seqNum = 1;

	AppleMIDI_AcceptInvitation acceptInvitation(this->_ssrc, invitation.initiatorToken, Name);
	acceptInvitation.write(&this->_controlUDP);
}

/*! \brief .
*/
void AppleMidi_Class::OnContentInvitation(void* sender, Invitation_t& invitation)
{
	Dissector* dissector = (Dissector*)sender;

#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Invitation: peer = \"");
	Serial.print  (invitation.name);
	Serial.print  ("\"");
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  (" ,ssrc = 0x");
	Serial.print  (invitation.ssrc, HEX);
	Serial.print  (" ,initiatorToken = 0x");
	Serial.print  (invitation.initiatorToken, HEX);
#endif
	Serial.println();
#endif

	int index;
	for (index = 0; index < MAX_SESSIONS; index++)
		if (Sessions[index].ssrc == invitation.ssrc)
			break;
	if (index >= MAX_SESSIONS)
	{
		Serial.print("hmm - control session does not exists for ");
		Serial.println(invitation.ssrc, HEX);
		return; 
	}

	AppleMIDI_AcceptInvitation acceptInvitation(this->_ssrc, invitation.initiatorToken, Name);
	acceptInvitation.write(&this->_contentUDP);

#if (APPLEMIDI_DEBUG) && (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  ("New Session created for 0x");
	Serial.print  (invitation.ssrc, HEX);
	Serial.print  (" in slot ");
	Serial.println(index);
#endif

	// Save current sessions to MemoryCard
	// open the file. note that only one file can be open at a time,
	// so you have to close this one before opening another.
//	File sessionsList = SD.open("sessions.txt", FILE_WRITE);

	if (this->mConnectedCallback != 0)
		this->mConnectedCallback(invitation.name);
}

/*! \brief .
*/
void AppleMidi_Class::OnSyncronization(void* sender, Syncronization_t& synchronization)
{
	Dissector* dissector = (Dissector*)sender;

#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Syncronization: count = ");
	Serial.print  (synchronization.count);
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  (" Timestamps = ");
	Serial.print  (synchronization.timestamps[0].tv_sec, HEX);
	Serial.print  (" ");
	Serial.print  (synchronization.timestamps[0].tv_usec, HEX);
	Serial.print  (" ");
	Serial.print  (synchronization.timestamps[1].tv_sec, HEX);
	Serial.print  (" ");
	Serial.print  (synchronization.timestamps[1].tv_usec, HEX);
	Serial.print  (" ");
	Serial.print  (synchronization.timestamps[2].tv_sec, HEX);
	Serial.print  (" ");
	Serial.print  (synchronization.timestamps[2].tv_usec, HEX);
#endif
	Serial.println("");
#endif

	// If we know this session already, ignore it.
	int index;
	for (index = 0; index < MAX_SESSIONS; index++)
		if (Sessions[index].ssrc == synchronization.ssrc)
			break;
	if (index >= MAX_SESSIONS)
	{
#if (APPLEMIDI_DEBUG)
		Serial.println("hmmm - Syncronization for a session that has never started.");
#endif
		// TODO: send EndSession?
//		AppleMIDI_EndSession endSession(this->_ssrc);
//		endSession.write(&this->_controlUDP);
		return; 
	}

	synchronization.count++;
	if (synchronization.count > 2)
	{
		synchronization.timestamps[0].tv_sec = 0;
		synchronization.timestamps[0].tv_usec = 0;
		synchronization.timestamps[1].tv_sec = 0;
		synchronization.timestamps[1].tv_usec = 0;
		synchronization.timestamps[2].tv_sec = 0;
		synchronization.timestamps[2].tv_usec = 0;
		synchronization.count = 0;
	}

	Syncronization_t synchronizationResponse(this->_ssrc, synchronization.count, synchronization.timestamps);
	synchronizationResponse.write(&this->_contentUDP);
}

/* \brief With the receiver feedback packet, the recipient can tell the sender up to what sequence
* number in the RTP-stream the packets have been received; this can be used to shorten the
* recovery-journal-section in the RTP-session */
void AppleMidi_Class::OnReceiverFeedback(void* sender, ReceiverFeedback_t& receiverFeedback)
{
	Dissector* dissector = (Dissector*)sender;

#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Receiver Feedback: seq = ");
	Serial.println(receiverFeedback.sequenceNr);
#endif
}

/* With the bitrate receive limit packet, the recipient can tell the sender to limit
the transmission to a certain bitrate. This is important if the peer is a gateway
to a hardware-device that only supports a certain speed. Like the MIDI 1.0 DIN-cable
MIDI-implementation which is limited to 31250. */
void AppleMidi_Class::OnBitrateReceiveLimit(void* sender, BitrateReceiveLimit_t& bitrateReceiveLimit)
{
	Dissector* dissector = (Dissector*)sender;

#if (APPLEMIDI_DEBUG)
	Serial.print  ("> BitrateReceiveLimit: Limit = ");
	Serial.println(bitrateReceiveLimit.bitratelimit);
#endif

}

/*! \brief .
*/
void AppleMidi_Class::OnEndSession(void* sender, EndSession_t& sessionEnd)
{
	Dissector* dissector = (Dissector*)sender;

#if (APPLEMIDI_DEBUG)
	Serial.print  ("> End Session");
#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print  (": src = 0x");
	Serial.print  (sessionEnd.ssrc, HEX);
	Serial.print  (", initiatorToken = 0x");
	Serial.print  (sessionEnd.initiatorToken, HEX);
#endif
	Serial.println();
#endif

	// If we know this session already, ignore it.
	int index;
	for (index = 0; index < MAX_SESSIONS; index++)
		if (Sessions[index].ssrc == sessionEnd.ssrc)
			break;
	if (index >= MAX_SESSIONS)
	{
#if (APPLEMIDI_DEBUG)
		Serial.println("hmmm - ending session that has never started.");
#endif
		// TODO: shall we send an invite?
		return; 
	}

#if (APPLEMIDI_DEBUG_VERBOSE)
	Serial.print("Ending session. Freeing slot ");
	Serial.print(index);
	Serial.println(". Bye");
#endif

	// Free Session Slot
	Sessions[index].ssrc = 0;

	if (this->mDisconnectedCallback != 0)
		this->mDisconnectedCallback();
}

/*! \brief .
*/
bool AppleMidi_Class::PassesFilter(void* sender, DataByte type, DataByte channel)
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
		||  (_inputChannel == MIDI_CHANNEL_OMNI))
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
void AppleMidi_Class::OnNoteOn(void* sender, DataByte channel, DataByte note, DataByte velocity)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Note On (c=");
	Serial.print  (channel);
	Serial.print  (", n=");
	Serial.print  (note);
	Serial.print  (", v=");
	Serial.print  (velocity);
	Serial.println(")");
#endif

	if (mNoteOnCallback)
		mNoteOnCallback(channel, note, velocity);
}

/*! \brief .
*/
void AppleMidi_Class::OnNoteOff(void* sender, DataByte channel, DataByte note, DataByte velocity)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Note Off (c=");
	Serial.print  (channel);
	Serial.print  (", n=");
	Serial.print  (note);
	Serial.print  (", v=");
	Serial.print  (velocity);
	Serial.println(")");
#endif

	if (mNoteOffCallback)
		mNoteOffCallback(channel, note, velocity);
}

/*! \brief .
*/
void AppleMidi_Class::OnPolyPressure(void* sender, DataByte channel, DataByte note, DataByte pressure)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Poly Pressure (c=");
	Serial.print  (channel);
	Serial.print  (", n=");
	Serial.print  (note);
	Serial.print  (", p=");
	Serial.print  (pressure);
	Serial.println(")");
#endif

	if (mAfterTouchPolyCallback)
		mAfterTouchPolyCallback(channel, note, pressure);
}

/*! \brief .
*/
void AppleMidi_Class::OnChannelPressure(void* sender, DataByte channel, DataByte pressure)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Channel Pressure (c=");
	Serial.print  (channel);
	Serial.print  (", p=");
	Serial.print  (pressure);
	Serial.println(")");
#endif

	if (mAfterTouchChannelCallback)
		mAfterTouchChannelCallback(channel, pressure);
}

/*! \brief .
*/
void AppleMidi_Class::OnPitchBendChange(void* sender, DataByte channel, int pitch)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Pitch Bend (c=");
	Serial.print  (channel);
	Serial.print  (", p=");
	Serial.print  (pitch);
	Serial.println(")");
#endif

	if (mPitchBendCallback)
		mPitchBendCallback(channel, pitch);
}

/*! \brief .
*/
void AppleMidi_Class::OnProgramChange(void* sender, DataByte channel, DataByte program)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Program Change (c=");
	Serial.print  (channel);
	Serial.print  (", p=");
	Serial.print  (program);
	Serial.print  (") ");

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
void AppleMidi_Class::OnControlChange(void* sender, DataByte channel, DataByte controller, DataByte value)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> Control Change (c=");
	Serial.print  (channel);
	Serial.print  (", e=");
	Serial.print  (controller);
	Serial.print  (", v=");
	Serial.print  (value);
	Serial.print  (") ");

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
void AppleMidi_Class::OnSongSelect(void* sender, DataByte songNr)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> SongSelect (s=");
	Serial.print  (songNr);
	Serial.println(")");
#endif

	if (mSongSelectCallback)
		mSongSelectCallback(songNr);
}

/*! \brief .
*/
void AppleMidi_Class::OnSongPosition(void* sender, int value)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> SongPosition (c=");
	Serial.print  (value);
	Serial.println(")");
#endif

	if (mSongPositionCallback)
		mSongPositionCallback(value);
}

/*! \brief .
*/
void AppleMidi_Class::OnTimeCodeQuarterFrame(void* sender, DataByte value)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> TimeCodeQuarterFrame (c=");
	Serial.print  (value);
	Serial.println(")");
#endif

	if (mTimeCodeQuarterFrameCallback)
		mTimeCodeQuarterFrameCallback(value);
}

/*! \brief .
*/
void AppleMidi_Class::OnTuneRequest(void* sender)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("> TuneRequest ()");
#endif

	if (mTuneRequestCallback)
		mTuneRequestCallback();
}

// -----------------------------------------------------------------------------
//                                 Output
// -----------------------------------------------------------------------------

#if APPLEMIDI_BUILD_OUTPUT

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
void AppleMidi_Class::send(MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel)
{
	for (int i = 0 ; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(&Sessions[i], inType, inData1, inData2, inChannel);
		}
	}

	return;
}

void AppleMidi_Class::send(MidiType inType, DataByte inData1, DataByte inData2)
{
	for (int i = 0 ; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(&Sessions[i], inType, inData1, inData2);
		}
	}

	return;
}

void AppleMidi_Class::send(MidiType inType, DataByte inData)
{
	for (int i = 0 ; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(&Sessions[i], inType, inData);
		}
	}

	return;
}

void AppleMidi_Class::send(MidiType inType)
{
	for (int i = 0 ; i < MAX_SESSIONS; i++)
	{
		if (Sessions[i].ssrc != 0)
		{
			internalSend(&Sessions[i], inType);
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
void AppleMidi_Class::internalSend(Session_t* session, MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel)
{
    // Then test if channel is valid
    if (inChannel >= MIDI_CHANNEL_OFF  || 
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

		_rtpMidi.sequenceNr++;
//		_rtpMidi.timestamp = 
		_rtpMidi.beginWrite(&_contentUDP);

		// Length
		uint8_t length = 3;
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

		_rtpMidi.endWrite(&_contentUDP);

        return;
    }
    else if (inType >= TuneRequest && inType <= SystemReset)
        internalSend(session, inType); // System Real-time and 1 byte.

}

void AppleMidi_Class::internalSend(Session_t*, MidiType inType)
{
	_rtpMidi.sequenceNr++;
//	_rtpMidi.timestamp = 
	_rtpMidi.beginWrite(&_contentUDP);

	uint8_t length = 1;
	_contentUDP.write(&length, 1);

	byte octet = (byte)inType;

    switch (inType) 
    {
        case TuneRequest: // Not really real-time, but one byte anyway.
        case Clock:
        case Start:
        case Stop:    
        case Continue:
        case ActiveSensing:
        case SystemReset:
			_contentUDP.write(&octet, 1);
            break;
        default:
            // Invalid Real Time marker
            break;
    }

 	_rtpMidi.endWrite(&_contentUDP);
   
    // Do not cancel Running Status for real-time messages as they can be 
    // interleaved within any message. Though, TuneRequest can be sent here, 
    // and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
    if (inType == TuneRequest) mRunningStatus_TX = InvalidType;
#endif
}

void AppleMidi_Class::internalSend(Session_t* session, MidiType inType, DataByte inData)
{
	_rtpMidi.sequenceNr++;
//	_rtpMidi.timestamp = 
	_rtpMidi.beginWrite(&_contentUDP);

	uint8_t length = 2;
	_contentUDP.write(&length, 1);

	DataByte octet = (DataByte)inType;

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

 	_rtpMidi.endWrite(&_contentUDP);
   
    // Do not cancel Running Status for real-time messages as they can be 
    // interleaved within any message. Though, TuneRequest can be sent here, 
    // and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
    if (inType == TuneRequest) mRunningStatus_TX = InvalidType;
#endif
}

void AppleMidi_Class::internalSend(Session_t* session, MidiType inType, DataByte inData1, DataByte inData2)
{
	_rtpMidi.sequenceNr++;
//	_rtpMidi.timestamp = 
	_rtpMidi.beginWrite(&_contentUDP);

	uint8_t length = 3;
	_contentUDP.write(&length, 1);

	DataByte octet = (DataByte)inType;

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

 	_rtpMidi.endWrite(&_contentUDP);
   
    // Do not cancel Running Status for real-time messages as they can be 
    // interleaved within any message. Though, TuneRequest can be sent here, 
    // and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
    if (inType == TuneRequest) mRunningStatus_TX = InvalidType;
#endif
}


StatusByte AppleMidi_Class::getStatus(MidiType inType,
                                      Channel inChannel) const
{
    return ((byte)inType | ((inChannel - 1) & 0x0F));
}






// -----------------------------------------------------------------------------

/*! \brief Send a Note On message 
 \param inNoteNumber  Pitch value in the MIDI format (0 to 127). 
 \param inVelocity    Note attack velocity (0 to 127). A NoteOn with 0 velocity 
 is considered as a NoteOff.
 \param inChannel     The channel on which the message will be sent (1 to 16). 
 
 Take a look at the values, names and frequencies of notes here: 
 http://www.phys.unsw.edu.au/jw/notes.html
 */
void AppleMidi_Class::noteOn(DataByte inNoteNumber,
                             DataByte inVelocity,
                             Channel  inChannel)
{ 
#if (APPLEMIDI_DEBUG)
	Serial.print  ("< Note On (c=");
	Serial.print  (inChannel);
	Serial.print  (", n=");
	Serial.print  (inNoteNumber);
	Serial.print  (", v=");
	Serial.print  (inVelocity);
	Serial.println(")");
#endif

	if (mNoteOnSendingEvent != 0)               
		mNoteOnSendingEvent(inChannel, inNoteNumber, inVelocity);

    send(NoteOn, inNoteNumber, inVelocity, inChannel);

	if (mNoteOnSendEvent != 0)               
		mNoteOnSendEvent(inChannel, inNoteNumber, inVelocity);
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
void AppleMidi_Class::noteOff(DataByte inNoteNumber,
                              DataByte inVelocity,
                              Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("< Note Off (c=");
	Serial.print  (inChannel);
	Serial.print  (", n=");
	Serial.print  (inNoteNumber);
	Serial.print  (", v=");
	Serial.print  (inVelocity);
	Serial.println(")");
#endif

	if (mNoteOffSendingEvent != 0)               
		mNoteOffSendingEvent(inChannel, inNoteNumber, inVelocity);

	send(NoteOff, inNoteNumber, inVelocity, inChannel);

	if (mNoteOffSendEvent != 0)               
		mNoteOffSendEvent(inChannel, inNoteNumber, inVelocity);
}

/*! \brief Send a Program Change message 
 \param inProgramNumber The Program to select (0 to 127).
 \param inChannel       The channel on which the message will be sent (1 to 16).
 */
void AppleMidi_Class::programChange(DataByte inProgramNumber,
                                        Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("sendProgramChange ProgramNumber:");
	Serial.print  (inProgramNumber);
	Serial.print  (" Channel:");
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
void AppleMidi_Class::controlChange(DataByte inControlNumber,
                                        DataByte inControlValue,
                                        Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("sendControlChange Number:");
	Serial.print  (inControlNumber);
	Serial.print  (" Value:");
	Serial.print  (inControlValue);
	Serial.print  (" Channel:");
	Serial.println(inChannel);
#endif

	send(ControlChange, inControlNumber, inControlValue, inChannel);
}

/*! \brief Send a Polyphonic AfterTouch message (applies to a specified note)
 \param inNoteNumber  The note to apply AfterTouch to (0 to 127).
 \param inPressure    The amount of AfterTouch to apply (0 to 127).
 \param inChannel     The channel on which the message will be sent (1 to 16). 
 */
void AppleMidi_Class::polyPressure(DataByte inNoteNumber,
                                       DataByte inPressure,
                                       Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("sendPolyPressure Note:");
	Serial.print  (inNoteNumber);
	Serial.print  (" Pressure:");
	Serial.print  (inPressure);
	Serial.print  (" Channel:");
	Serial.println(inChannel);
#endif

	send(AfterTouchPoly, inNoteNumber, inPressure, inChannel);
}

/*! \brief Send a MonoPhonic AfterTouch message (applies to all notes)
 \param inPressure    The amount of AfterTouch to apply to all notes.
 \param inChannel     The channel on which the message will be sent (1 to 16). 
 */
void AppleMidi_Class::afterTouch(DataByte inPressure,
                                     Channel inChannel)
{
#if (APPLEMIDI_DEBUG)
	Serial.print  ("sendafterTouch ");
	Serial.print  (" Pressure:");
	Serial.print  (inPressure);
	Serial.print  (" Channel:");
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
void AppleMidi_Class::pitchBend(int inPitchValue, Channel inChannel)
{
 #if (APPLEMIDI_DEBUG)
	Serial.print  ("pitchBend ");
	Serial.print  (" Pitch:");
	Serial.print  (inPitchValue);
	Serial.print  (" Channel:");
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
void AppleMidi_Class::pitchBend(double inPitchValue, Channel inChannel)
{
    const int value = inPitchValue * MIDI_PITCHBEND_MAX;
	pitchBend(value, inChannel);
}

/*! \brief Generate and send a System Exclusive frame.
 \param inLength  The size of the array to send
 \param inArray   The byte array containing the data to send
 \param inArrayContainsBoundaries When set to 'true', 0xF0 & 0xF7 bytes
 (start & stop SysEx) will NOT be sent
 (and therefore must be included in the array).
 default value for ArrayContainsBoundaries is set to 'false' for compatibility
 with previous versions of the library.
 */
void AppleMidi_Class::sysEx(unsigned int inLength,
                                const byte* inArray,
                                bool inArrayContainsBoundaries)
{
	_rtpMidi.sequenceNr++;
//	_rtpMidi.timestamp = 
	_rtpMidi.beginWrite(&_contentUDP);

	uint8_t length = inLength + 1 + ((inArrayContainsBoundaries) ? 0 : 2);
	_contentUDP.write(&length, 1);

	uint8_t type = SystemExclusive;
	_contentUDP.write(&type, 1);

	if (!inArrayContainsBoundaries)
	{
		uint8_t octet = 0xF0;
		_contentUDP.write(&octet, 1);
	}

	_contentUDP.write(inArray, inLength);

	if (!inArrayContainsBoundaries)
	{
		uint8_t octet = 0xF7;
		_contentUDP.write(&octet, 1);
	}

 	_rtpMidi.endWrite(&_contentUDP);
   
    // Do not cancel Running Status for real-time messages as they can be 
    // interleaved within any message. Though, TuneRequest can be sent here, 
    // and as it is a System Common message, it must reset Running Status.
#if APPLEMIDI_USE_RUNNING_STATUS
    if (inType == TuneRequest) mRunningStatus_TX = InvalidType;
#endif
}

/*! \brief Send a Tune Request message. 
 
 When a MIDI unit receives this message,
 it should tune its oscillators (if equipped with any).
 */
void AppleMidi_Class::tuneRequest()
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
void AppleMidi_Class::activeSensing()
{
	send(ActiveSensing);
}

/*! \brief 
 */
void AppleMidi_Class::start()
{
	send(Start);
}

/*! \brief 
 */
void AppleMidi_Class::_continue()
{
	send(Continue);
}

/*! \brief 
 */
void AppleMidi_Class::stop()
{
	send(Stop);
}

/*! \brief Send a MIDI Time Code Quarter Frame. 
 
 \param inTypeNibble      MTC type
 \param inValuesNibble    MTC data
 See MIDI Specification for more information.
 */
void AppleMidi_Class::timeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble)
{
    const byte data = ( ((inTypeNibble & 0x07) << 4) | (inValuesNibble & 0x0F) );
    timeCodeQuarterFrame(data);
}

/*! \brief Send a MIDI Time Code Quarter Frame. 
 
 See MIDI Specification for more information.
 \param inData  if you want to encode directly the nibbles in your program, 
                you can send the byte here.
 */
void AppleMidi_Class::timeCodeQuarterFrame(DataByte inData)
{
	send(TimeCodeQuarterFrame, inData);
}

/*! \brief Send a Song Position Pointer message.
 \param inBeats    The number of beats since the start of the song.
 */
void AppleMidi_Class::songPosition(unsigned int inBeats)
{
	byte octet1 = inBeats & 0x7F;
	byte octet2 = (inBeats >> 7) & 0x7F;
 
	send(SongPosition, octet1, octet2);
}

/*! \brief Send a Song Select message */
void AppleMidi_Class::songSelect(DataByte inSongNumber)
{    
	byte octet = inSongNumber & 0x7F;

	send(SongSelect, octet);
}


/*! \brief Send a Song Select message */
void AppleMidi_Class::systemReset()
{    
	send(SystemReset);
}

/*! \brief Send a Song Select message */
void AppleMidi_Class::clock()
{    
	send(Clock);
}

/*! \brief Send a Song Select message */
void AppleMidi_Class::tick()
{    
	send(Tick);
}

/*! @} */ // End of doc group MIDI Output

// -----------------------------------------------------------------------------

//StatusByte AppleMidi_Class::getStatus(MidiType inType,
//                                      Channel inChannel) const
//{
//    return ((byte)inType | ((inChannel - 1) & 0x0F));
//}
//


#endif // APPLEMIDI_BUILD_OUTPUT

// -----------------------------------------------------------------------------
//                                  Input
// -----------------------------------------------------------------------------

#if APPLEMIDI_BUILD_INPUT

#if APPLEMIDI_USE_EVENTS
//void AppleMidi_Class::xxx(void (*fptr)(byte channel, byte note, byte velocity))          { mNoteOffCallback = fptr; }
#endif

#if APPLEMIDI_USE_CALLBACKS

/*! \addtogroup callbacks
 @{
 */

void AppleMidi_Class::OnConnected(void (*fptr)(char*))    { mConnectedCallback    = fptr; }
void AppleMidi_Class::OnDisconnected(void (*fptr)())      { mDisconnectedCallback = fptr; }

void AppleMidi_Class::OnReceiveNoteOff(void (*fptr)(byte channel, byte note, byte velocity))          { mNoteOffCallback              = fptr; }
void AppleMidi_Class::OnReceiveNoteOn(void (*fptr)(byte channel, byte note, byte velocity))           { mNoteOnCallback               = fptr; }
void AppleMidi_Class::OnReceiveAfterTouchPoly(void (*fptr)(byte channel, byte note, byte pressure))   { mAfterTouchPolyCallback       = fptr; }
void AppleMidi_Class::OnReceiveControlChange(void (*fptr)(byte channel, byte number, byte value))     { mControlChangeCallback        = fptr; }
void AppleMidi_Class::OnReceiveProgramChange(void (*fptr)(byte channel, byte number))                 { mProgramChangeCallback        = fptr; }
void AppleMidi_Class::OnReceiveAfterTouchChannel(void (*fptr)(byte channel, byte pressure))           { mAfterTouchChannelCallback    = fptr; }
void AppleMidi_Class::OnReceivePitchBend(void (*fptr)(byte channel, int bend))                        { mPitchBendCallback            = fptr; }
void AppleMidi_Class::OnReceiveSystemExclusive(void (*fptr)(byte* array, byte size))                  { mSystemExclusiveCallback      = fptr; }
void AppleMidi_Class::OnReceiveTimeCodeQuarterFrame(void (*fptr)(byte data))                          { mTimeCodeQuarterFrameCallback = fptr; }
void AppleMidi_Class::OnReceiveSongPosition(void (*fptr)(unsigned int beats))                         { mSongPositionCallback         = fptr; }
void AppleMidi_Class::OnReceiveSongSelect(void (*fptr)(byte songnumber))                              { mSongSelectCallback           = fptr; }
void AppleMidi_Class::OnReceiveTuneRequest(void (*fptr)(void))                                        { mTuneRequestCallback          = fptr; }
void AppleMidi_Class::OnReceiveClock(void (*fptr)(void))                                              { mClockCallback                = fptr; }
void AppleMidi_Class::OnReceiveStart(void (*fptr)(void))                                              { mStartCallback                = fptr; }
void AppleMidi_Class::OnReceiveContinue(void (*fptr)(void))                                           { mContinueCallback             = fptr; }
void AppleMidi_Class::OnReceiveStop(void (*fptr)(void))                                               { mStopCallback                 = fptr; }
void AppleMidi_Class::OnReceiveActiveSensing(void (*fptr)(void))                                      { mActiveSensingCallback        = fptr; }
void AppleMidi_Class::OnReceiveSystemReset(void (*fptr)(void))                                        { mSystemResetCallback          = fptr; }

#endif // MIDI_USE_CALLBACKS

/*! @} */ // End of doc group MIDI Input

#endif // MIDI_BUILD_INPUT

END_APPLEMIDI_NAMESPACE