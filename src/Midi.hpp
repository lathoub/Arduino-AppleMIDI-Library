/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire, chris-zen
 */

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnNoteOn(void* sender, DataByte channel, DataByte note, DataByte velocity)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnNoteOff(void* sender, DataByte channel, DataByte note, DataByte velocity)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnPolyPressure(void* sender, DataByte channel, DataByte note, DataByte pressure)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnChannelPressure(void* sender, DataByte channel, DataByte pressure)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnPitchBendChange(void* sender, DataByte channel, int pitch)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnProgramChange(void* sender, DataByte channel, DataByte program)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnControlChange(void* sender, DataByte channel, DataByte controller, DataByte value)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSongSelect(void* sender, DataByte songNr)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSongPosition(void* sender, unsigned short value)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnTimeCodeQuarterFrame(void* sender, DataByte value)
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
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnTuneRequest(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> TuneRequest ()");
#endif

	if (mTuneRequestCallback)
		mTuneRequestCallback();
}


/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnClock(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Clock ()");
#endif

	if (mClockCallback)
		mClockCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnStart(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Start ()");
#endif

	if (mStartCallback)
		mStartCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnContinue(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Continue ()");
#endif

	if (mContinueCallback)
		mContinueCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnStop(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Stop ()");
#endif

	if (mStopCallback)
		mStopCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnActiveSensing(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> ActiveSensing ()");
#endif

	if (mActiveSensingCallback)
		mActiveSensingCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnReset(void* sender)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> Reset ()");
#endif

	if (mResetCallback)
		mResetCallback();
}

/*! \brief .
*/
template<class UdpClass, class Settings>
void AppleMidiInterface<UdpClass, Settings>::OnSysEx(void* sender, const byte* data, uint16_t length)
{
#if (APPLEMIDI_DEBUG)
	DEBUGSTREAM.print("> SysEx ()");
#endif

	if (mSysExCallback)
		mSysExCallback(data, length);
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::send(MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::send(MidiType inType, DataByte inData1, DataByte inData2)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::send(MidiType inType, DataByte inData)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::send(MidiType inType)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendSysEx(byte s, const byte* data, byte e, uint16_t length)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::internalSend(Session_t& session, MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::internalSend(Session_t& session, MidiType inType)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::internalSend(Session_t& session, MidiType inType, DataByte inData)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::internalSend(Session_t& session, MidiType inType, DataByte inData1, DataByte inData2)
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

template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::internalSendSysEx(Session_t& session, byte s, const byte* data, byte e, uint16_t length)
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

template<class UdpClass, class Settings>
inline StatusByte AppleMidiInterface<UdpClass, Settings>::getStatus(MidiType inType, Channel inChannel) const
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::noteOn(DataByte inNoteNumber, DataByte inVelocity, Channel  inChannel)
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

	send(NoteOn, inNoteNumber, inVelocity, inChannel);
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::noteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel)
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

	send(NoteOff, inNoteNumber, inVelocity, inChannel);
}

/*! \brief Send a Program Change message
\param inProgramNumber The Program to select (0 to 127).
\param inChannel       The channel on which the message will be sent (1 to 16).
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::programChange(DataByte inProgramNumber, Channel inChannel)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::controlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::polyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::afterTouch(DataByte inPressure, Channel inChannel)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::pitchBend(int inPitchValue, Channel inChannel)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::pitchBend(double inPitchValue, Channel inChannel)
{
	const int value = inPitchValue * MIDI_PITCHBEND_MAX;
	pitchBend(value, inChannel);
}

/*! \brief Generate and send a System Ex frame.
\param inLength  The size of the array to send
\param inArray   The byte array containing the data to send
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sysEx(const byte* data, uint16_t length)
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::tuneRequest()
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
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::activeSensing()
{
	send(ActiveSensing);
}

/*! \brief
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::start()
{
	send(Start);
}

/*! \brief
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::_continue()
{
	send(Continue);
}

/*! \brief
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::stop()
{
	send(Stop);
}

/*! \brief Send a MIDI Time Code Quarter Frame.

\param inTypeNibble      MTC type
\param inValuesNibble    MTC data
See MIDI Specification for more information.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::timeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble)
{
	const byte data = (((inTypeNibble & 0x07) << 4) | (inValuesNibble & 0x0F));
	timeCodeQuarterFrame(data);
}

/*! \brief Send a MIDI Time Code Quarter Frame.

See MIDI Specification for more information.
\param inData  if you want to encode directly the nibbles in your program,
you can send the byte here.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::timeCodeQuarterFrame(DataByte inData)
{
	send(TimeCodeQuarterFrame, inData);
}

/*! \brief Send a Song Position Pointer message.
\param inBeats    The number of beats since the start of the song.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::songPosition(unsigned short inBeats)
{
	byte octet1 = inBeats & 0x7F;
	byte octet2 = (inBeats >> 7) & 0x7F;

	send(SongPosition, octet1, octet2);
}

/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::songSelect(DataByte inSongNumber)
{
	byte octet = inSongNumber & 0x7F;

	send(SongSelect, octet);
}


/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::reset()
{
	send(Reset);
}

/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::clock()
{
	send(Clock);
}

/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::tick()
{
	send(Tick);
}

#endif

END_APPLEMIDI_NAMESPACE
