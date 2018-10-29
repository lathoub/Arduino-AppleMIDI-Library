/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire, chris-zen
 */

BEGIN_APPLEMIDI_NAMESPACE

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
inline void AppleMidiInterface<UdpClass, Settings>::sendNoteOn(DataByte inNoteNumber, DataByte inVelocity, Channel  inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendNoteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendProgramChange(DataByte inProgramNumber, Channel inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendControlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendPolyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendAfterTouch(DataByte inPressure, Channel inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendPitchBend(int inPitchValue, Channel inChannel)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendPitchBend(double inPitchValue, Channel inChannel)
{
	const int value = inPitchValue * MIDI_PITCHBEND_MAX;
	pitchBend(value, inChannel);
}

/*! \brief Generate and send a System Ex frame.
\param inLength  The size of the array to send
\param inArray   The byte array containing the data to send
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendSysEx(const byte* data, uint16_t length)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendTuneRequest()
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
inline void AppleMidiInterface<UdpClass, Settings>::sendActiveSensing()
{
	send(ActiveSensing);
}

/*! \brief
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendStart()
{
	send(Start);
}

/*! \brief
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendContinue()
{
	send(Continue);
}

/*! \brief
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendStop()
{
	send(Stop);
}

/*! \brief Send a MIDI Time Code Quarter Frame.

\param inTypeNibble      MTC type
\param inValuesNibble    MTC data
See MIDI Specification for more information.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendTimeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble)
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
inline void AppleMidiInterface<UdpClass, Settings>::sendTimeCodeQuarterFrame(DataByte inData)
{
	send(TimeCodeQuarterFrame, inData);
}

/*! \brief Send a Song Position Pointer message.
\param inBeats    The number of beats since the start of the song.
*/
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendSongPosition(unsigned short inBeats)
{
	byte octet1 = inBeats & 0x7F;
	byte octet2 = (inBeats >> 7) & 0x7F;

	send(SongPosition, octet1, octet2);
}

/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendSongSelect(DataByte inSongNumber)
{
	byte octet = inSongNumber & 0x7F;

	send(SongSelect, octet);
}


/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendReset()
{
	send(Reset);
}

/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendClock()
{
	send(Clock);
}

/*! \brief Send a Song Select message */
template<class UdpClass, class Settings>
inline void AppleMidiInterface<UdpClass, Settings>::sendTick()
{
	send(Tick);
}

#endif

END_APPLEMIDI_NAMESPACE
