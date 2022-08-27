#pragma once

#include "AppleMIDI_Debug.h"

// https://developer.apple.com/library/archive/documentation/Audio/Conceptual/MIDINetworkDriverProtocol/MIDI/MIDI.html

#include <MIDI.h>
using namespace MIDI_NAMESPACE;

#include <IPAddress.h>

#include "AppleMIDI_PlatformBegin.h"
#include "AppleMIDI_Defs.h"
#include "AppleMIDI_Settings.h"

#include "rtp_Defs.h"
#include "rtpMIDI_Defs.h"
#include "rtpMIDI_Clock.h"

#include "AppleMIDI_Participant.h"

#include "AppleMIDI_Parser.h"
#include "rtpMIDI_Parser.h"

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

static unsigned long now;

struct AppleMIDISettings : public MIDI_NAMESPACE::DefaultSettings
{
    // Packet based protocols prefer the entire message to be parsed
    // as a whole.
    static const bool Use1ByteParsing = false;
};

template <class UdpClass, class _Settings = DefaultSettings, class _Platform = DefaultPlatform>
class AppleMIDISession
{
    typedef _Settings Settings;
    typedef _Platform Platform;

    // Allow these internal classes access to our private members
    // to avoid access by the .ino to internal messages
    friend class AppleMIDIParser<UdpClass, Settings, Platform>;
    friend class rtpMIDIParser<UdpClass, Settings, Platform>;

public:
    AppleMIDISession(const char *sessionName, const uint16_t port = DEFAULT_CONTROL_PORT)
    {
        this->port = port;
#ifdef KEEP_SESSION_NAME
        strncpy(this->localName, sessionName, Settings::MaxSessionNameLen);
#endif

#ifdef ONE_PARTICIPANT
        participant.ssrc = 0;
#endif
    };

    virtual ~AppleMIDISession(){};

    AppleMIDISession &setHandleConnected(void (*fptr)(const ssrc_t &, const char *))
    {
        _connectedCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleDisconnected(void (*fptr)(const ssrc_t &))
    {
        _disconnectedCallback = fptr;
        return *this;
    }
#ifdef USE_EXT_CALLBACKS
    AppleMIDISession &setHandleException(void (*fptr)(const ssrc_t &, const Exception &, const int32_t value))
    {
        _exceptionCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleReceivedRtp(void (*fptr)(const ssrc_t &, const Rtp_t &, const int32_t &))
    {
        _receivedRtpCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleStartReceivedMidi(void (*fptr)(const ssrc_t &))
    {
        _startReceivedMidiByteCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleReceivedMidi(void (*fptr)(const ssrc_t &, byte))
    {
        _receivedMidiByteCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleEndReceivedMidi(void (*fptr)(const ssrc_t &))
    {
        _endReceivedMidiByteCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleSentRtp(void (*fptr)(const Rtp_t &))
    {
        _sentRtpCallback = fptr;
        return *this;
    }
    AppleMIDISession &setHandleSentRtpMidi(void (*fptr)(const RtpMIDI_t &))
    {
        _sentRtpMidiCallback = fptr;
        return *this;
    }
#endif

#ifdef KEEP_SESSION_NAME
    const char *getName() const
    {
        return this->localName;
    };
    AppleMIDISession &setName(const char *sessionName)
    {
        strncpy(this->localName, sessionName, Settings::MaxSessionNameLen);
        return *this;
    };
#else
    const char *getName() const
    {
        return nullptr;
    };
    AppleMIDISession &setName(const char *sessionName) { return *this; };
#endif

    const uint16_t getPort() const
    {
        return this->port;
    };

    // call this method *before* calling begin()
    AppleMIDISession & setPort(const uint16_t port)
    {
        this->port = port;
        return *this;
    }

    const ssrc_t getSynchronizationSource() const { return this->ssrc; };

#ifdef APPLEMIDI_INITIATOR
    bool sendInvite(IPAddress ip, uint16_t port = DEFAULT_CONTROL_PORT);
#endif
    void sendEndSession();

public:
    // Override default thruActivated. Must be false for all packet based messages
    static const bool thruActivated = false;

#ifdef USE_DIRECTORY
    Deque<IPAddress, Settings::MaxNumberOfComputersInDirectory> directory;
    WhoCanConnectToMe whoCanConnectToMe = Anyone;
#endif

    void begin()
    {
        _appleMIDIParser.session = this;
        _rtpMIDIParser.session = this;

        // analogRead(0) is not available on all platforms. The use of millis()
        // as it preceded by network calls, so timing is variable and usable
        // for the random generator.
        randomSeed(millis());

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

    void end()
    {
#ifdef ONE_PARTICIPANT
        participant.ssrc = 0;
#endif
        controlPort.stop();
        dataPort.stop();
    }

    bool beginTransmission(MIDI_NAMESPACE::MidiType)
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
#ifndef ONE_PARTICIPANT
        return (dataPort.remoteIP() != (IPAddress)INADDR_NONE && participants.size() > 0);
#else
        return (dataPort.remoteIP() != (IPAddress)INADDR_NONE && participant.ssrc != 0);
#endif
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
#ifdef USE_EXT_CALLBACKS
                if (nullptr != _exceptionCallback)
                    _exceptionCallback(ssrc, BufferFullException, 0);
#endif
            }
        }

        // store in local buffer, as we do *not* know the length of the message prior to sending
        outMidiBuffer.push_back(byte);
    };

    void endTransmission(){};

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
            return inMidiBuffer.size();

        if (readDataPackets())  // from socket into dataBuffer
            parseDataPackets(); // from dataBuffer into inMidiBuffer

        if (readControlPackets())  // from socket into controlBuffer
            parseControlPackets(); // from controlBuffer to AppleMIDI

        manageReceiverFeedback();
        manageSynchronization();

        return inMidiBuffer.size();
    };

    byte read()
    {
        auto byte = inMidiBuffer.front();
        inMidiBuffer.pop_front();

        return byte;
    };

protected:
    UdpClass controlPort;
    UdpClass dataPort;

private:
    RtpBuffer_t controlBuffer;
    RtpBuffer_t dataBuffer;

    byte packetBuffer[Settings::UdpTxPacketMaxSize];

    AppleMIDIParser<UdpClass, Settings, Platform> _appleMIDIParser;
    rtpMIDIParser<UdpClass, Settings, Platform> _rtpMIDIParser;

    connectedCallback _connectedCallback = nullptr;
    disconnectedCallback _disconnectedCallback = nullptr;
#ifdef USE_EXT_CALLBACKS
    startReceivedMidiByteCallback _startReceivedMidiByteCallback = nullptr;
    receivedMidiByteCallback _receivedMidiByteCallback = nullptr;
    endReceivedMidiByteCallback _endReceivedMidiByteCallback = nullptr;
    receivedRtpCallback _receivedRtpCallback = nullptr;
    sentRtpCallback _sentRtpCallback = nullptr;
    sentRtpMidiCallback _sentRtpMidiCallback = nullptr;
    exceptionCallback _exceptionCallback = nullptr;
#endif
    // buffer for incoming and outgoing MIDI messages
    MidiBuffer_t inMidiBuffer;
    MidiBuffer_t outMidiBuffer;

    rtpMidi_Clock rtpMidiClock;

    ssrc_t ssrc = 0;
    uint16_t port = DEFAULT_CONTROL_PORT;
#ifdef ONE_PARTICIPANT
    Participant<Settings> participant;
#else
    Deque<Participant<Settings>, Settings::MaxNumberOfParticipants> participants;
#endif

#ifdef KEEP_SESSION_NAME
    char localName[Settings::MaxSessionNameLen + 1];
#endif

private:
    size_t readControlPackets();
    size_t readDataPackets();

    void parseControlPackets();
    void parseDataPackets();

    void ReceivedInvitation(AppleMIDI_Invitation_t &, const amPortType &);
    void ReceivedControlInvitation(AppleMIDI_Invitation_t &);
    void ReceivedDataInvitation(AppleMIDI_Invitation_t &);
    void ReceivedSynchronization(AppleMIDI_Synchronization_t &);
    void ReceivedReceiverFeedback(AppleMIDI_ReceiverFeedback_t &);
    void ReceivedEndSession(AppleMIDI_EndSession_t &);
    void ReceivedBitrateReceiveLimit(AppleMIDI_BitrateReceiveLimit &);

    void ReceivedInvitationAccepted(AppleMIDI_InvitationAccepted_t &, const amPortType &);
    void ReceivedControlInvitationAccepted(AppleMIDI_InvitationAccepted_t &);
    void ReceivedDataInvitationAccepted(AppleMIDI_InvitationAccepted_t &);
    void ReceivedInvitationRejected(AppleMIDI_InvitationRejected_t &);

    // rtpMIDI callback from parser
    void ReceivedRtp(const Rtp_t &);
    void StartReceivedMidi();
    void ReceivedMidi(byte data);
    void EndReceivedMidi();

    // Helpers
    void writeInvitation(UdpClass &, const IPAddress &, const uint16_t &, AppleMIDI_Invitation_t &, const byte *command);
    void writeReceiverFeedback(const IPAddress &, const uint16_t &, AppleMIDI_ReceiverFeedback_t &);
    void writeSynchronization(const IPAddress &, const uint16_t &, AppleMIDI_Synchronization_t &);
    void writeEndSession(const IPAddress &, const uint16_t &, AppleMIDI_EndSession_t &);

    void sendEndSession(Participant<Settings> *);

    void writeRtpMidiToAllParticipants();
    void writeRtpMidiBuffer(Participant<Settings> *);

    void manageReceiverFeedback();

    void manageSessionInvites();
    void manageSynchronization();
    void manageSynchronizationInitiator();
    void manageSynchronizationInitiatorHeartBeat(Participant<Settings> *);
    void manageSynchronizationInitiatorInvites(size_t);

    void sendSynchronization(Participant<Settings> *);

#ifndef ONE_PARTICIPANT
    Participant<Settings> *getParticipantBySSRC(const ssrc_t &);
    Participant<Settings> *getParticipantByInitiatorToken(const uint32_t &initiatorToken);
#endif
#ifdef USE_DIRECTORY
    bool IsComputerInDirectory(IPAddress) const;
#endif
};

END_APPLEMIDI_NAMESPACE

#include "AppleMIDI.hpp"

#define APPLEMIDI_CREATE_INSTANCE(Type, Name, SessionName, Port)                \
    APPLEMIDI_NAMESPACE::AppleMIDISession<Type> Apple##Name(SessionName, Port); \
    MIDI_NAMESPACE::MidiInterface<APPLEMIDI_NAMESPACE::AppleMIDISession<Type>, APPLEMIDI_NAMESPACE::AppleMIDISettings> Name((APPLEMIDI_NAMESPACE::AppleMIDISession<Type> &)Apple##Name);

#if defined(ARDUINO_ARCH_ESP32) || defined(ESP32)
#define APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE() \
    APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "AppleMIDI-ESP32", DEFAULT_CONTROL_PORT);
#elif defined(ESP8266)
#define APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE() \
    APPLEMIDI_CREATE_INSTANCE(WiFiUDP, MIDI, "AppleMIDI-ESP8266", DEFAULT_CONTROL_PORT);
#else
#define APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE() \
    APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, "AppleMIDI-Arduino", DEFAULT_CONTROL_PORT);
#endif

#include "AppleMIDI_PlatformEnd.h"
