#pragma once

#include "utility/Logging.h"

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

// this is an exported and stripped down version of the MIDI library by  47 blabla
// feat 4.4.0 summer 2019
#include "utility/midi_feat4_4_0/MIDI.h"
#include "utility/endian.h"

#include "IPAddress.h"

#include "AppleMidi_Defs.h"
#include "AppleMidi_Settings.h"

#include "rtp_Defs.h"
#include "rtpMidi_Defs.h"
#include "rtpMidi_Clock.h"

#include "AppleMidi_Participant.h"

#include "AppleMidi_Parser.h"
#include "rtpMidi_Parser.h"

#include "AppleMidi_Namespace.h"

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

BEGIN_APPLEMIDI_NAMESPACE

template <class UdpClass, class _Settings = DefaultSettings>
class AppleMidiSession
{
	typedef _Settings Settings;

	// Allow these internal classes access to our private members
	// to avoid access by the .ino to internal messages
	friend class AppleMIDIParser<UdpClass, Settings>;
	friend class rtpMIDIParser<UdpClass, Settings>;
	friend class MIDI_NAMESPACE::MidiInterface<AppleMidiSession<UdpClass>>;

public:
	AppleMidiSession(const char *name, const uint16_t port = CONTROL_PORT)
	{
        // Pseudo randomize
		randomSeed(analogRead(0));

		this->port = port;
		strncpy(this->localName, name, APPLEMIDI_SESSION_NAME_MAX_LEN);
        
        // initialise
		for (uint8_t i = 0; i < Settings::MaxNumberOfParticipants; i++)
			participants[i].ssrc = APPLEMIDI_PARTICIPANT_SLOT_FREE;

        _lastSyncExchangeTime = 0;
        _kind = Unknown;

		_appleMIDIParser.session = this;
		_rtpMIDIParser.session = this;
	};

	void setHandleConnected(void (*fptr)(ssrc_t, const char*)) { _connectedCallback = fptr; }
	void setHandleDisconnected(void (*fptr)(ssrc_t)) { _disconnectedCallback = fptr; }
    void setHandleError(void (*fptr)(ssrc_t, uint32_t)) { _errorCallback = fptr; }
    void setHandleReceivedMidi(void (*fptr)(ssrc_t, byte)) { _receivedMidiByteCallback = fptr; }

    const char*    getName() { return this->localName; };
    const uint16_t getPort() { return this->port; };

protected:
	void begin(MIDI_NAMESPACE::Channel inChannel = 1)
	{
		// Each stream is distinguished by a unique SSRC value and has a unique sequence
		// number and RTP timestamp space.
		// this is our SSRC
        //
        // NOTE: Arduino random only goes to INT32_MAX (not UINT32_MAX)
        
		this->ssrc = random(1, INT32_MAX) * 2;

		// In an RTP MIDI stream, the 16-bit sequence number field is
		// initialized to a randomly chosen value and is incremented by one
		// (modulo 2^16) for each packet sent in the stream.
		// http://www.rfc-editor.org/rfc/rfc6295.txt , 2.1.  RTP Header
        //
		this->sequenceNr = random(1, UINT16_MAX);

		controlPort.begin(port);
		dataPort.begin(port + 1);

		uint32_t initialTimestamp = rtpMidiClock.Now();
		rtpMidiClock.Init(initialTimestamp, MIDI_SAMPLING_RATE_DEFAULT);
    }

	bool beginTransmission()
	{
        // All MIDI commands queued up in the same cycle (during 1 loop execution)
        // are send in a single MIDI packet
        // (The actual sending happen in the available() method, called at the start of the
        // event loop() method.
        //
        // http://www.rfc-editor.org/rfc/rfc4696.txt
        //
        // 4.1.  Queuing and Coding Incoming MIDI Data
        // ...
        // More sophisticated sending algorithms
        // [GRAME] improve efficiency by coding small groups of commands into a
        // single packet, at the expense of increasing the sender queuing
        // latency.
        //
        if (!outMidiBuffer.empty())
        {
            // Check if there is still room for more - like for 3 bytes or so)
            if ((outMidiBuffer.size() + 1 + 3) > outMidiBuffer.max_size())
                writeRtpMidiBuffer(dataPort);
            else
                outMidiBuffer.push_back(0x00); // zero timestamp
        }
        
		// We can't start the writing process here, as we do not know the length
		// of what we are to send (The RtpMidi protocol start with writing the
		// length of the buffer). So we'll copy to a buffer in the 'write' method, 
		// and actually serialize for real in the endTransmission method
		return (dataPort.remoteIP() != 0);
	};

	void write(byte byte)
	{
		// do we still have place in the buffer for 1 more character?
		if ((outMidiBuffer.size()) + 2 > outMidiBuffer.max_size())
		{
			// buffer is almost full, only 1 more character
			if (MIDI_NAMESPACE::MidiType::SystemExclusive == outMidiBuffer.front())
			{
				// Add Sysex at the end of this partial SysEx (in the last availble slot) ...
				outMidiBuffer.push_back(MIDI_NAMESPACE::MidiType::SystemExclusiveStart);
                
                writeRtpMidiBuffer(dataPort);
				// and start again with a fresh continuation of
				// a next SysEx block. (writeRtpMidiBuffer empties the buffer!)
                outMidiBuffer.clear();
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
	};

    // first things MIDI.read() calls in this method
    // MIDI-read() must be called at the start of loop()
	unsigned available()
	{
        // All MIDI commands queued up in the same cycle (during 1 loop execution)
        // are send in a single MIDI packet
        if (outMidiBuffer.size() > 0)
            writeRtpMidiBuffer(dataPort);
        // assert(outMidiBuffer.size() == 0); // must be empty
        
        if (inMidiBuffer.size() > 0)
            return true;
        
        // read packets from both UDP sockets and send the
        // bytes to the parsers. Valid MIDI data will be placed
        // in the inMidiBuffer buffer
        readDataPackets();
        readControlPackets();

        manageSyncExchange();
        manageReceiverFeedback();

        return false;
	};

    byte read()
    {
        return inMidiBuffer.pop_front();
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
    void (*_receivedMidiByteCallback)(ssrc_t, byte) = NULL;
	void (*_disconnectedCallback)(ssrc_t) = NULL;
    void (*_errorCallback)(ssrc_t, uint32_t) = NULL;

	// buffer for incoming and outgoing midi messages
	Deque<byte, Settings::MaxBufferSize> inMidiBuffer;
	Deque<byte, Settings::MaxBufferSize> outMidiBuffer;
    
	rtpMidi_Clock rtpMidiClock;
    
    // Session Information

    SessionKind _kind = Unknown;
    
    unsigned long _lastSyncExchangeTime = 0;
    
	ssrc_t ssrc = 0;

	uint16_t sequenceNr = 0; // counter for outgoing messages

	char localName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];
    
	uint16_t port = CONTROL_PORT;

	Participant<Settings> participants[Settings::MaxNumberOfParticipants];
            
public:
    bool sendInvite(IPAddress ip, uint16_t port = CONTROL_PORT);

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

    void SendInvitation(AppleMIDI_Invitation &, const amPortType &);
    
	// rtpMIDI callback from parser
    void ReceivedRtp(const Rtp_t&);
    void ReceivedMidi(byte data);

	// Helpers
    void writeInvitation(UdpClass &, AppleMIDI_Invitation_t &, const byte *command, ssrc_t);
    void writeReceiverFeedback(UdpClass &, AppleMIDI_ReceiverFeedback_t &);
    void writeRtpMidiBuffer(UdpClass &);

    void manageSyncExchange();
    void manageReceiverFeedback();
   
    void managePendingInvites();
    void manageTiming();
        
	Participant<Settings> *getParticipant(const ssrc_t ssrc);
};

#define APPLEMIDI_CREATE_INSTANCE(midiName, appleMidiName) \
	MIDI_NAMESPACE::MidiInterface<__amt> midiName((__amt &)appleMidiName);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE(Type, sessionName, port) \
	typedef APPLEMIDI_NAMESPACE::AppleMidiSession<Type> __amt;   \
	__amt AppleMIDI(sessionName, port);                        \
	APPLEMIDI_CREATE_INSTANCE(MIDI, AppleMIDI);

#define APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE() \
	APPLEMIDI_CREATE_DEFAULT_INSTANCE(EthernetUDP, "Arduino", CONTROL_PORT);

END_APPLEMIDI_NAMESPACE

#include "AppleMidi.hpp"

