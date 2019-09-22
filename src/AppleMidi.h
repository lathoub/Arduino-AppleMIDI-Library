#pragma once

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

// this is an exported and stripped down version of the MIDI library by  47 blabla
// feat 4.4.0 summer 2019
#include "midi_feat4_4_0/MIDI.h"

#include "appleMidi_Defs.h"

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
		// this is our SSRC
		this->ssrc = random(1, INT32_MAX);

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
		if (!dataPort.beginPacket(dataPort.remoteIP(), dataPort.remotePort()))
			return false;

		Rtp rtp;
		rtp.vpxcc      = 0b10000000; // TODO: fun with flags
		rtp.mpayload   = PAYLOADTYPE_RTPMIDI; // TODO: set or unset marker
		rtp.ssrc       = htonl(ssrc);
		// https://developer.apple.com/library/ios/documentation/CoreMidi/Reference/MIDIServices_Reference/#//apple_ref/doc/uid/TP40010316-CHMIDIServiceshFunctions-SW30
		// The time at which the events occurred, if receiving MIDI, or, if sending MIDI,
		// the time at which the events are to be played. Zero means "now." The time stamp
		// applies to the first MIDI byte in the packet.
		rtp.timestamp  = htonl(0UL);
		rtp.sequenceNr = htonl(sequenceNr++);

		dataPort.write((uint8_t*)&rtp, sizeof(rtp));
		
		// TODO: what with larger SysEx messages???

		return true;
	};

	void write(byte byte)
	{
		// store in local buffer, as we do *not* know the length of the message prior to sending
		outMidiBuffer.write(byte);
	};

	void endTransmission()
	{
		// only now the length is known
		uint16_t bufferLen = outMidiBuffer.getLength();

		RtpMIDI rtpMidi;

		if (bufferLen <= 0x0F) // can we fit it in 4 bits?
		{	// fits in 4 bits, so small len
			rtpMidi.flags = (uint8_t)bufferLen;
			rtpMidi.flags &= RTP_MIDI_CS_FLAG_B; // TODO clear the RTP_MIDI_CS_FLAG_B
			// rtpMidi.flags |= RTP_MIDI_CS_FLAG_J; // TODO no journaling
			// rtpMidi.flags |= RTP_MIDI_CS_FLAG_Z; // TODO no Z
			// rtpMidi.flags |= RTP_MIDI_CS_FLAG_P; // TODO no P
			dataPort.write(rtpMidi.flags);
		}
		else
		{	// no, larger than 4 bits, so large len	
			rtpMidi.flags = (uint8_t)(bufferLen >> 8); // TODO shift something
			rtpMidi.flags |= RTP_MIDI_CS_FLAG_B; // TODO set the RTP_MIDI_CS_FLAG_B
			// rtpMidi.flags |= RTP_MIDI_CS_FLAG_J; // TODO no journaling
			// rtpMidi.flags |= RTP_MIDI_CS_FLAG_Z; // TODO no Z
			// rtpMidi.flags |= RTP_MIDI_CS_FLAG_P; // TODO no P
			dataPort.write(rtpMidi.flags);
			dataPort.write((uint8_t)(bufferLen)); // TODO shift??
		}

		// from local buffer onto the network
		while (!outMidiBuffer.isEmpty())
			dataPort.write(outMidiBuffer.read());

		dataPort.endPacket();
		dataPort.flush();
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

	uint32_t sequenceNr = 0; // counter for outgoing messages

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
	void writeInvitation(UdpClass& port, AppleMIDI_Invitation& invitation, uint8_t* command, ssrc_t ssrc);

#ifdef APPLEMIDI_INITIATOR
	void ManagePendingInvites();
	void ManageTiming();
#endif

	int8_t getParticipant(const ssrc_t ssrc) const;
};

#define APPLEMIDI_CREATE_INSTANCE(Type, midiName, appleMidiName, SessionName) \
	typedef APPLEMIDI_NAMESPACE::AppleMidiTransport<Type> __amt;   \
	__amt appleMidiName(SessionName);                                           \
	MIDI_NAMESPACE::MidiInterface<__amt> midiName((__amt&)appleMidiName);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()               \
	APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, AppleMIDI, "Arduino");

#include "AppleMidi.hpp"

END_APPLEMIDI_NAMESPACE
