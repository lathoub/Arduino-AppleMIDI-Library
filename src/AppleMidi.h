#pragma once

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

// this is an exported and stripped down version of the MIDI library by  47 blabla
// feat 4.4.0 summer 2019
#include "midi_feat4_4_0/MIDI.h"

#include "AppleMidi_Defs.h"

#include "rtpMidi_Defs.h"
#include "rtpMidi_Clock.h"

#include "AppleMidi_Parser.h"
#include "rtpMidi_Parser.h"

#include "endian.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
class AppleMidiTransport 
{
public:
	AppleMidiTransport(const char* name, const uint16_t port = CONTROL_PORT)
	{
		// Each stream is distinguished by a unique SSRC value and has a unique sequence
   		// number and RTP timestamp space.
		// this is our SSRC
		this->ssrc = random(1, UINT32_MAX);

		// In an RTP MIDI stream, the 16-bit sequence number field is
   		// initialized to a randomly chosen value and is incremented by one
   		// (modulo 2^16) for each packet sent in the stream.
		// http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
		this->sequenceNr = random(1, UINT16_MAX);

		this->port = port;
		strncpy(this->localName, name, APPLEMIDI_SESSION_NAME_MAX_LEN);
#ifdef OPTIONAL_MDNS
		strncpy(this->bonjourName, name, APPLEMIDI_SESSION_NAME_MAX_LEN);
#endif		

		for (int i = 0; i < APPLEMIDI_MAX_PARTICIPANTS; i++)
			participants[i] = APPLEMIDI_PARTICIPANT_SLOT_FREE;

		// attach the parsers
		controlParsers[0] = &AppleMIDIParser<UdpClass>::Parser;

		dataParsers[0] = &rtpMIDIParser<UdpClass>::Parser;
		dataParsers[1] = &AppleMIDIParser<UdpClass>::Parser;

		enabled = true;
	};

protected:
	void begin(MIDI_NAMESPACE::Channel inChannel = 1)
	{
		controlPort.begin(port);
		dataPort.begin(port + 1);

		uint32_t initialTimestamp = 0; // TODO: is this supposed to be zero?
		rtpMidiClock.Init(initialTimestamp, MIDI_SAMPLING_RATE_DEFAULT);
	}

	bool beginTransmission()
	{
		// We can't start the writing process here, as we do not know the length
		// of what we are to send. The RtpMidi protocol start with writing the 
		// length of the buffer. So we'll copy to a buffer in write, and write
		// everything in endTransmission
		return true;
	};

	void write(byte byte)
	{
		if (outMidiBuffer.getLength() > 3)
		{
			Serial.println("een bericht > 3 bytes, wel wel wel");
		}

		// do we still have place in the buffer for 1 more character?
		if ((outMidiBuffer.getLength()) + 1 > BUFFER_MAX_SIZE) {
			// buffer is almost full, only 1 more character
			if (MIDI_NAMESPACE::MidiType::SystemExclusive == outMidiBuffer.peek(0)) {
				// Add Sysex at the end of this partial SysEx (in the last availble slot) ...
				outMidiBuffer.write(MIDI_NAMESPACE::MidiType::SystemExclusive);
				writeRtpMidiBuffer(dataPort, outMidiBuffer, sequenceNr++, ssrc);
				// and start again with a fresh continuation of
				// a next SysEx block. (writeRtpMidiBuffer empties the buffer!)
				outMidiBuffer.write(MIDI_NAMESPACE::MidiType::SystemExclusive);
			}
			else {
				// TODO: What is this very large message ???
				Serial.println("buffer to small in write, and not it's not sysex!!!");
				// TODO: outMidiBuffer.dump();
			}
		}

		// store in local buffer, as we do *not* know the length of the message prior to sending
		outMidiBuffer.write(byte);
	};

	void endTransmission()
	{
		writeRtpMidiBuffer(dataPort, outMidiBuffer, sequenceNr++, ssrc);
		// writeRtpMidiBuffer is clean now (writeRtpMidiBuffer empties the buffer!)
	};

	byte read()
	{
		return inMidiBuffer.read();
	};

	unsigned available()
	{
		run();

		// if any MIDI came in, make it available for the read command
		return inMidiBuffer.getLength();
	};

	// avoids the above functions to become availble to the .ino programmer,
	// the above functions should only be availble to MidiInterface
	friend class MIDI_NAMESPACE::MidiInterface<AppleMidiTransport<UdpClass>>;

private:
	UdpClass		controlPort;
	UdpClass		dataPort;

	rtpMidi_Clock 	rtpMidiClock;

	// reading from the network
	RingBuffer<byte, BUFFER_MAX_SIZE> controlBuffer;
	RingBuffer<byte, BUFFER_MAX_SIZE> dataBuffer;

	typedef int(*FPPARSER)(RingBuffer<byte, BUFFER_MAX_SIZE>&, AppleMidiTransport<UdpClass>*, const amPortType&);

	FPPARSER controlParsers[1]; // TODO: these are static functions, can they be made static across ? (less memory usage when declaring multiple sessions)
	FPPARSER dataParsers[2];

	// Allow the parser access to protected messages, to prevent
	// outside world from calling public parser call back messages
	friend class AppleMIDIParser<UdpClass>;
	friend class rtpMIDIParser<UdpClass>;

	void(*_connectedCallback)(uint32_t, const char*);
	void(*_disconnectedCallback)(uint32_t);

	// buffer for incoming and outgoing midi messages
	RingBuffer<byte, BUFFER_MAX_SIZE> inMidiBuffer;
	RingBuffer<byte, BUFFER_MAX_SIZE> outMidiBuffer;

	ssrc_t	ssrc = 0;

	uint32_t sequenceNr = 0; // counter for outgoing messages // TODO: start with randam number??

	char localName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
#ifdef OPTIONAL_MDNS
	char bonjourName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
#endif
	uint16_t port; // controlPort, and dataPort = controlPort + 1

public:
	uint32_t participants[APPLEMIDI_MAX_PARTICIPANTS];

	bool enabled = true;

	void setHandleConnected(void(*fptr)(uint32_t, const char*)) { _connectedCallback = fptr; }
	void setHandleDisconnected(void(*fptr)(uint32_t)) { _disconnectedCallback = fptr; }

protected:

	void run();

	// AppleMIDI callbacks from parser
	void ReceivedInvitation(AppleMIDI_Invitation& invitation, const amPortType& portType);
	void ReceivedControlInvitation(AppleMIDI_Invitation& invitation);
	void ReceivedDataInvitation(AppleMIDI_Invitation& invitation);
	void ReceivedSyncronization(AppleMIDI_Syncronization& syncronization);
	void ReceivedEndSession(AppleMIDI_EndSession& endSession);
	
	// rtpMIDI callback from parser
	void ReceivedMidi(Rtp& rtp, RtpMIDI& rtpMidi, RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t cmdLen);

	// Helpers
	static void writeInvitation(UdpClass&, AppleMIDI_Invitation&, uint8_t* command, ssrc_t);
	static void writeRtpMidiBuffer(UdpClass&ort, RingBuffer<byte, BUFFER_MAX_SIZE>&, uint32_t, ssrc_t);

#ifdef APPLEMIDI_INITIATOR
	void ManagePendingInvites();
	void ManageTiming();
#endif

	static int8_t getParticipant(const uint32_t participants[], const ssrc_t ssrc);
};

#define APPLEMIDI_CREATE_INSTANCE(Type, midiName, appleMidiName, SessionName) \
	typedef APPLEMIDI_NAMESPACE::AppleMidiTransport<Type> __amt;   \
	__amt appleMidiName(SessionName);                                           \
	MIDI_NAMESPACE::MidiInterface<__amt> midiName((__amt&)appleMidiName);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()               \
	APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, AppleMIDI, "Arduino");

#include "AppleMidi.hpp"

END_APPLEMIDI_NAMESPACE
