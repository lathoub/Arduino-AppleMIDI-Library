#pragma once

#include "utility/Logging.h"

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

// this is an exported and stripped down version of the MIDI library by  47 blabla
// feat 4.4.0 summer 2019
#include "utility/midi_feat4_4_0/MIDI.h"
#include "utility/endian.h"

#include "IPAddress.h"

#include "AppleMIDI_Defs.h"
#include "AppleMIDI_Settings.h"

#include "rtp_Defs.h"
#include "rtpMIDI_Defs.h"
#include "rtpMIDI_Clock.h"

#include "AppleMIDI_Participant.h"

#include "AppleMIDI_Parser.h"
#include "rtpMIDI_Parser.h"

#include "AppleMIDI_Namespace.h"

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

BEGIN_APPLEMIDI_NAMESPACE

static unsigned long now;

template <class UdpClass, class _Settings = DefaultSettings>
class AppleMIDISession
{
	typedef _Settings Settings;

	// Allow these internal classes access to our private members
	// to avoid access by the .ino to internal messages
	friend class AppleMIDIParser<UdpClass, Settings>;
	friend class rtpMIDIParser<UdpClass, Settings>;
	friend class MIDI_NAMESPACE::MidiInterface<AppleMIDISession<UdpClass>>;

public:
	AppleMIDISession(const char *name, const uint16_t port = DEFAULT_CONTROL_PORT)
	{
        // Pseudo randomize
		randomSeed(analogRead(0));

		this->port = port;
        strncpy(this->localName, name, DefaultSettings::MaxSessionNameLen);
        
		_appleMIDIParser.session = this;
		_rtpMIDIParser.session   = this;
	};

	void setHandleConnected(void (*fptr)(ssrc_t, const char*)) { _connectedCallback = fptr; }
	void setHandleDisconnected(void (*fptr)(ssrc_t)) { _disconnectedCallback = fptr; }
    void setHandleError(void (*fptr)(ssrc_t, int32_t)) { _errorCallback = fptr; }
    void setHandleReceivedMidi(void (*fptr)(ssrc_t, byte)) { _receivedMidiByteCallback = fptr; }
    void setHandleReceivedRtp(void (*fptr)(ssrc_t, const Rtp_t&, const int32_t&)) { _receivedRtpCallback = fptr; }
    
    const char*    getName() { return this->localName; };
    const uint16_t getPort() { return this->port; };

#ifdef APPLEMIDI_INITIATOR
    bool sendInvite(IPAddress ip, uint16_t port = DEFAULT_CONTROL_PORT);
#endif
    void sendEndSession();

protected:
	void begin(MIDI_NAMESPACE::Channel inChannel = 1)
	{
		// Each stream is distinguished by a unique SSRC value and has a unique sequence
		// number and RTP timestamp space.
		// this is our SSRC
        //
        // NOTE: Arduino random only goes to INT32_MAX (not UINT32_MAX)
        
		this->ssrc = random(1, INT32_MAX) * 2;

		controlPort.begin(port);
		dataPort.begin(port + 1);

		rtpMidiClock.Init(rtpMidiClock.Now(), MIDI_SAMPLING_RATE_DEFAULT);
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
                writeRtpMidiToAllParticipants();
            else
                outMidiBuffer.push_back(0x00); // zero timestamp
        }
        
		// We can't start the writing process here, as we do not know the length
		// of what we are to send (The RtpMidi protocol start with writing the
		// length of the buffer). So we'll copy to a buffer in the 'write' method, 
		// and actually serialize for real in the endTransmission method
		return (dataPort.remoteIP() != 0 && participants.size() > 0);
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
                
                writeRtpMidiToAllParticipants();
				// and start again with a fresh continuation of
				// a next SysEx block.
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
        now = millis();
        
#ifdef APPLEMIDI_INITIATOR
        manageSessionInvites();
#endif

        // All MIDI commands queued up in the same cycle (during 1 loop execution)
        // are send in a single MIDI packet
        if (outMidiBuffer.size() > 0)
            writeRtpMidiToAllParticipants();
        // assert(outMidiBuffer.size() == 0); // must be empty
        
        if (inMidiBuffer.size() > 0)
            return true;
        
        // read packets from both UDP sockets and send the
        // bytes to the parsers. Valid MIDI data will be placed
        // in the inMidiBuffer buffer
        readDataPackets();
        readControlPackets();

        manageReceiverFeedback();
        manageSynchronization();

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
	RtpBuffer_t controlBuffer;
	RtpBuffer_t dataBuffer;

	AppleMIDIParser<UdpClass, Settings> _appleMIDIParser;
	rtpMIDIParser<UdpClass, Settings> _rtpMIDIParser;

    void (*_connectedCallback)(ssrc_t, const char *) = NULL;
    void (*_receivedMidiByteCallback)(ssrc_t, byte) = NULL;
    void (*_receivedRtpCallback)(ssrc_t, const Rtp_t&, const int32_t&) = NULL;
	void (*_disconnectedCallback)(ssrc_t) = NULL;
    void (*_errorCallback)(ssrc_t, int32_t) = NULL;

	// buffer for incoming and outgoing MIDI messages
	MidiBuffer_t inMidiBuffer;
	MidiBuffer_t outMidiBuffer;
    
	rtpMidi_Clock rtpMidiClock;
            
	ssrc_t ssrc = 0;
    char localName[DefaultSettings::MaxSessionNameLen + 1];
	uint16_t port = DEFAULT_CONTROL_PORT;
    Deque<Participant<Settings>, Settings::MaxNumberOfParticipants> participants;
    int32_t latencyAdjustment = 0;
            
private:
	void readControlPackets();
	void readDataPackets();
    
	void ReceivedInvitation               (AppleMIDI_Invitation_t &, const amPortType &);
	void ReceivedControlInvitation        (AppleMIDI_Invitation_t &);
	void ReceivedDataInvitation           (AppleMIDI_Invitation_t &);
	void ReceivedSynchronization          (AppleMIDI_Synchronization_t &);
	void ReceivedReceiverFeedback         (AppleMIDI_ReceiverFeedback_t &);
	void ReceivedEndSession               (AppleMIDI_EndSession_t &);
    void ReceivedBitrateReceiveLimit      (AppleMIDI_BitrateReceiveLimit &);
    
    void ReceivedInvitationAccepted       (AppleMIDI_InvitationAccepted_t &, const amPortType &);
    void ReceivedControlInvitationAccepted(AppleMIDI_InvitationAccepted_t &);
    void ReceivedDataInvitationAccepted   (AppleMIDI_InvitationAccepted_t &);
    void ReceivedInvitationRejected       (AppleMIDI_InvitationRejected_t &);
    
	// rtpMIDI callback from parser
    void ReceivedRtp(const Rtp_t &);
    void ReceivedMidi(byte data);

	// Helpers
    void writeInvitation      (UdpClass &, IPAddress, uint16_t, AppleMIDI_Invitation_t &, const byte *command);
    void writeReceiverFeedback(const IPAddress &, const uint16_t &, AppleMIDI_ReceiverFeedback_t &);
    void writeSynchronization (const IPAddress &, const uint16_t &, AppleMIDI_Synchronization_t &);
    void writeEndSession      (const IPAddress &, const uint16_t &, AppleMIDI_EndSession_t &);

    void sendEndSession(Participant<Settings>*);
    
    void writeRtpMidiToAllParticipants();
    void writeRtpMidiBuffer(Participant<Settings>*);

    void manageReceiverFeedback();
   
    void manageSessionInvites();
    void manageSynchronization();
    void manageSynchronizationListener(size_t);
    void manageSynchronizationInitiator();
    void manageSynchronizationInitiatorHeartBeat(size_t);
    void manageSynchronizationInitiatorInvites(size_t);
    
    void sendSynchronization(Participant<Settings>*);

    Participant<Settings>* getParticipantBySSRC(const ssrc_t ssrc);
    Participant<Settings>* getParticipantByInitiatorToken(const uint32_t initiatorToken);
};

#define APPLEMIDI_CREATE_INSTANCE(midiName, appleMidiName) \
	MIDI_NAMESPACE::MidiInterface<__amt> midiName((__amt &)appleMidiName);

#define APPLEMIDI_CREATE_DEFAULT_INSTANCE(Type, sessionName, port) \
	typedef APPLEMIDI_NAMESPACE::AppleMIDISession<Type> __amt;   \
	__amt AppleMIDI(sessionName, port);                        \
	APPLEMIDI_CREATE_INSTANCE(MIDI, AppleMIDI);

#define APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE() \
	APPLEMIDI_CREATE_DEFAULT_INSTANCE(EthernetUDP, "Arduino", DEFAULT_CONTROL_PORT);

END_APPLEMIDI_NAMESPACE

#include "AppleMIDI.hpp"

