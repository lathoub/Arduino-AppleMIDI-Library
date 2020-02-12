#pragma once

#include "AppleMidi_Debug.h"

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

// this is an exported and stripped down version of the MIDI library by  47 blabla
// feat 4.4.0 summer 2019
#include "midi_feat4_4_0/MIDI.h"

#include "AppleMidi_Defs.h"
#include "AppleMidi_Settings.h"

#include "rtp_Defs.h"
#include "rtpMidi_Defs.h"
#include "rtpMidi_Clock.h"

#include "AppleMidi_Participant.h"

#include "AppleMidi_Parser.h"
#include "rtpMidi_Parser.h"

#include "endian.h"

#include "AppleMidi_Namespace.h"

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

BEGIN_APPLEMIDI_NAMESPACE

template <class UdpClass, class _Settings = DefaultSettings>
class AppleMidiTransport
{
	typedef _Settings Settings;

	// Allow these internal classes access to our private members
	// to avoid access by the .ino to internal messages
	friend class AppleMIDIParser<UdpClass, Settings>;
	friend class rtpMIDIParser<UdpClass, Settings>;
	friend class MIDI_NAMESPACE::MidiInterface<AppleMidiTransport<UdpClass>>;

public:
	AppleMidiTransport(const char *name, const uint16_t port = CONTROL_PORT)
	{
		randomSeed(analogRead(0));

		this->port = port;
		strncpy(this->localName, name, APPLEMIDI_SESSION_NAME_MAX_LEN);
#ifdef OPTIONAL_MDNS
		strncpy(this->bonjourName, name, APPLEMIDI_SESSION_NAME_MAX_LEN);
#endif

		for (auto i = 0; i < Settings::MaxNumberOfParticipants; i++)
			participants[i].ssrc = APPLEMIDI_PARTICIPANT_SLOT_FREE;

		_appleMIDIParser.session = this;
		_rtpMIDIParser.session = this;
	};

	void setHandleConnected(void (*fptr)(ssrc_t, const char*)) { _connectedCallback = fptr; }
	void setHandleDisconnected(void (*fptr)(ssrc_t)) { _disconnectedCallback = fptr; }
    void setHandleError(void (*fptr)(ssrc_t, uint32_t)) { _errorCallback = fptr; }

protected:
	void begin(MIDI_NAMESPACE::Channel inChannel = 1)
	{
		// Each stream is distinguished by a unique SSRC value and has a unique sequence
		// number and RTP timestamp space.
		// this is our SSRC
        //
        // NOTE: random(1, UINT32_MAX) doesn't seem to work!
		this->ssrc = random(1, INT32_MAX) * 2;

		// In an RTP MIDI stream, the 16-bit sequence number field is
		// initialized to a randomly chosen value and is incremented by one
		// (modulo 2^16) for each packet sent in the stream.
		// http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
        //
        // NOTE: random(1, UINT32_MAX) doesn't seem to work!
		this->sequenceNr = random(1, INT16_MAX) * 2;

		controlPort.begin(port);
		dataPort.begin(port + 1);

		uint32_t initialTimestamp = rtpMidiClock.Now();
		rtpMidiClock.Init(initialTimestamp, MIDI_SAMPLING_RATE_DEFAULT);
	}

	bool beginTransmission()
	{
		// We can't start the writing process here, as we do not know the length
		// of what we are to send (The RtpMidi protocol start with writing the
		// length of the buffer). So we'll copy to a buffer in the 'write' method, 
		// and actually serialize for real in the endTransmission method
		return (dataPort.remoteIP() != 0);
	};

	void write(byte byte)
	{
		// do we still have place in the buffer for 1 more character?
		if ((outMidiBuffer.size()) + 2 > Settings::MaxBufferSize)
		{
			// buffer is almost full, only 1 more character
			if (MIDI_NAMESPACE::MidiType::SystemExclusive == outMidiBuffer.front())
			{
				// Add Sysex at the end of this partial SysEx (in the last availble slot) ...
				outMidiBuffer.push_back(MIDI_NAMESPACE::MidiType::SystemExclusive);
				writeRtpMidiBuffer(dataPort, outMidiBuffer, sequenceNr++, ssrc, rtpMidiClock.Now());
				// and start again with a fresh continuation of
				// a next SysEx block. (writeRtpMidiBuffer empties the buffer!)
				outMidiBuffer.push_back(MIDI_NAMESPACE::MidiType::SystemExclusiveEnd);
			}
			else
			{
                if (NULL != _errorCallback)
                    _errorCallback(ssrc, -1);
				F_DEBUG_PRINTLN("buffer to small in write, and it's not sysex!!!");
			}
		}

		// store in local buffer, as we do *not* know the length of the message prior to sending
		outMidiBuffer.push_back(byte);
	};

	void endTransmission()
	{
		writeRtpMidiBuffer(dataPort, outMidiBuffer, sequenceNr++, ssrc, rtpMidiClock.Now());
	};

	byte read()
	{
        auto i = inMidiBuffer.front();
        inMidiBuffer.pop_front();
		return i;
	};

	unsigned available()
	{
        if (inMidiBuffer.size() > 0)
            return true;
        
        // read packets from both UDP sockets and send the
        // bytes to the parsers. Valid MIDI data will be placed
        // in the inMidiBuffer buffer
        readDataPackets();
        readControlPackets();

        manageReceiverFeedback();

        return false;
	};

private:
	UdpClass controlPort;
	UdpClass dataPort;

	// reading from the network
	Deque<byte, Settings::MaxBufferSize> controlBuffer;
	Deque<byte, Settings::MaxBufferSize> dataBuffer;

	AppleMIDIParser<UdpClass, Settings> _appleMIDIParser;
	rtpMIDIParser<UdpClass, Settings> _rtpMIDIParser;

	void (*_connectedCallback)(ssrc_t, const char *) = NULL;
	void (*_disconnectedCallback)(ssrc_t) = NULL;
    void (*_errorCallback)(ssrc_t, uint32_t) = NULL;

	// buffer for incoming and outgoing midi messages
	Deque<byte, Settings::MaxBufferSize> inMidiBuffer;
	Deque<byte, Settings::MaxBufferSize> outMidiBuffer;

	rtpMidi_Clock rtpMidiClock;

	ssrc_t ssrc = 0;

	uint16_t sequenceNr = 0; // counter for outgoing messages

	char localName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
#ifdef OPTIONAL_MDNS
	char bonjourName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
#endif
    
	uint16_t port = 5004;

	Participant<Settings> participants[Settings::MaxNumberOfParticipants];

private:
	void readControlPackets();
	void readDataPackets();
    
	// AppleMIDI callbacks from parser
	void ReceivedInvitation(AppleMIDI_Invitation &, const amPortType &);
	void ReceivedControlInvitation(AppleMIDI_Invitation &);
	void ReceivedDataInvitation(AppleMIDI_Invitation &);
	void ReceivedSynchronization(AppleMIDI_Synchronization &);
	void ReceivedReceiverFeedback(AppleMIDI_ReceiverFeedback &);
	void ReceivedEndSession(AppleMIDI_EndSession &);

	// rtpMIDI callback from parser
    void ReceivedRtp(const Rtp_t&);
    void ReceivedMidi(byte data);

	// Helpers
    static void writeInvitation(UdpClass &, AppleMIDI_Invitation_t &, const byte *command, ssrc_t);
    static void writeReceiverFeedback(UdpClass &, AppleMIDI_ReceiverFeedback_t &);
	static void writeRtpMidiBuffer(UdpClass &, Deque<byte, Settings::MaxBufferSize> &, uint16_t, ssrc_t, uint32_t);

	void managePendingInvites();
	void manageTiming();
    void manageReceiverFeedback();

	Participant<Settings> *getParticipant(const ssrc_t ssrc);
};

#define APPLEMIDI_CREATE_INSTANCE(midiName, appleMidiName) \
	MIDI_NAMESPACE::MidiInterface<__amt> midiName((__amt &)appleMidiName);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE(Type, sessionName, port) \
	typedef APPLEMIDI_NAMESPACE::AppleMidiTransport<Type> __amt;   \
	__amt AppleMIDI(sessionName, port);                        \
	APPLEMIDI_CREATE_INSTANCE(MIDI, AppleMIDI);

#define APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE() \
	APPLEMIDI_CREATE_DEFAULT_INSTANCE(EthernetUDP, "Arduino", 5004);

END_APPLEMIDI_NAMESPACE

#include "AppleMidi.hpp"
