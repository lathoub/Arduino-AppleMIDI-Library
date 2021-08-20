#pragma once

#include "AppleMIDI_Settings.h"
#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#define APPLEMIDI_LIBRARY_VERSION 0x030000
#define APPLEMIDI_LIBRARY_VERSION_MAJOR 3
#define APPLEMIDI_LIBRARY_VERSION_MINOR 0
#define APPLEMIDI_LIBRARY_VERSION_PATCH 0

#define DEFAULT_CONTROL_PORT 5004

typedef uint32_t ssrc_t;
typedef uint32_t initiatorToken_t;
typedef uint64_t timestamp_t;

union conversionBuffer
{
    uint8_t value8;
    uint16_t value16;
    uint32_t value32;
    uint64_t value64;
    byte buffer[8];
};


enum parserReturn: uint8_t
{
    Processed,
    NotSureGiveMeMoreData,
    NotEnoughData,
	UnexpectedData,
	UnexpectedMidiData,
	UnexpectedJournalData,
	SessionNameVeryLong,
};

#if defined(__AVR__)
#define APPLEMIDI_PROGMEM PROGMEM
typedef const __FlashStringHelper* AppleMIDIConstStr;
#define GFP(x) (reinterpret_cast<AppleMIDIConstStr>(x))
#define GF(x) F(x)
#else
#define APPLEMIDI_PROGMEM
typedef const char* AppleMIDIConstStr;
#define GFP(x) x
#define GF(x) x
#endif

#define RtpBuffer_t Deque<byte, Settings::MaxBufferSize>
#define MidiBuffer_t Deque<byte, Settings::MaxBufferSize>

// #define USE_EXT_CALLBACKS
// #define ONE_PARTICIPANT // memory optimization
// #define USE_DIRECTORY

// By defining NO_SESSION_NAME in the sketch, you can save 100 bytes
#ifndef NO_SESSION_NAME
#define KEEP_SESSION_NAME
#endif

#define MIDI_SAMPLING_RATE_176K4HZ 176400
#define MIDI_SAMPLING_RATE_192KHZ 192000
#define MIDI_SAMPLING_RATE_DEFAULT 10000

struct Rtp;
typedef Rtp Rtp_t;

struct RtpMIDI;
typedef RtpMIDI RtpMIDI_t;

#ifdef USE_DIRECTORY
enum WhoCanConnectToMe : uint8_t
{
	None,
	OnlyComputersInMyDirectory,
	Anyone,
};
#endif

// from: https://en.wikipedia.org/wiki/RTP-MIDI
// Apple decided to create their own protocol, imposing all parameters related to
// synchronization like the sampling frequency. This session protocol is called "AppleMIDI"
// in Wireshark software. Session management with AppleMIDI protocol requires two UDP ports,
// the first one is called "Control Port", the second one is called "Data Port". When used
// within a multithread implementation, only the Data port requires a "real-time" thread,
// the other port can be controlled by a normal priority thread. These two ports must be
// located at two consecutive locations (n / n+1); the first one can be any of the 65536
// possible ports.
enum amPortType : uint8_t
{
	Control = 0,
	Data = 1,
};

// from: https://en.wikipedia.org/wiki/RTP-MIDI
// AppleMIDI implementation defines two kind of session controllers: session initiators
// and session listeners. Session initiators are in charge of inviting the session listeners,
// and are responsible of the clock synchronization sequence. Session initiators can generally
// be session listeners, but some devices, such as iOS devices, can be session listeners only.
enum ParticipantKind : uint8_t
{
	Listener,
	Initiator,
};

enum InviteStatus : uint8_t
{
	Initiating,
    AwaitingControlInvitationAccepted,
    ControlInvitationAccepted,
    AwaitingDataInvitationAccepted,
    DataInvitationAccepted,
    Connected
};

enum Exception : uint8_t
{
    BufferFullException,
    ParseException,
    UnexpectedParseException,
    TooManyParticipantsException,
    ComputerNotInDirectory,
    NotAcceptingAnyone,
    UnexpectedInviteException,
    ParticipantNotFoundException,
    ListenerTimeOutException,
    MaxAttemptsException,
    NoResponseFromConnectionRequestException,
    SendPacketsDropped,
    ReceivedPacketsDropped,
    UdpBeginPacketFailed,
};

using connectedCallback             = void (*)(const ssrc_t&, const char *);
using disconnectedCallback          = void (*)(const ssrc_t&);
#ifdef USE_EXT_CALLBACKS
using startReceivedMidiByteCallback = void (*)(const ssrc_t&);
using receivedMidiByteCallback      = void (*)(const ssrc_t&, byte);
using endReceivedMidiByteCallback   = void (*)(const ssrc_t&);
using receivedRtpCallback           = void (*)(const ssrc_t&, const Rtp_t&, const int32_t&);
using exceptionCallback             = void (*)(const ssrc_t&, const Exception&, const int32_t value);
using sentRtpCallback               = void (*)(const Rtp_t&);
using sentRtpMidiCallback           = void (*)(const RtpMIDI_t&);
#endif

/* Signature "Magic Value" for Apple network MIDI session establishment */
const byte amSignature[] = {0xff, 0xff};

/* 2 (stored in network byte order (big-endian)) */
const byte amProtocolVersion[] = {0x00, 0x00, 0x00, 0x02};

/* Apple network MIDI valid commands */
const byte amInvitation[]          = {'I', 'N'};
const byte amEndSession[]          = {'B', 'Y'};
const byte amSynchronization[]     = {'C', 'K'};
const byte amInvitationAccepted[]  = {'O', 'K'};
const byte amInvitationRejected[]  = {'N', 'O'};
const byte amReceiverFeedback[]    = {'R', 'S'};
const byte amBitrateReceiveLimit[] = {'R', 'L'};

const uint8_t SYNC_CK0 = 0;
const uint8_t SYNC_CK1 = 1;
const uint8_t SYNC_CK2 = 2;

typedef struct PACKED AppleMIDI_Invitation
{
	initiatorToken_t initiatorToken;
	ssrc_t ssrc;

#ifdef KEEP_SESSION_NAME
    char sessionName[DefaultSettings::MaxSessionNameLen + 1];
	const size_t getLength() const
	{
		return sizeof(AppleMIDI_Invitation) - (DefaultSettings::MaxSessionNameLen) + strlen(sessionName);
	}
#else
	const size_t getLength() const
	{
		return sizeof(AppleMIDI_Invitation);
	}
#endif
} AppleMIDI_Invitation_t, AppleMIDI_InvitationAccepted_t, AppleMIDI_InvitationRejected_t;

typedef struct PACKED AppleMIDI_BitrateReceiveLimit
{
	ssrc_t ssrc;
	uint32_t bitratelimit;
} AppleMIDI_BitrateReceiveLimit_t;

typedef struct PACKED AppleMIDI_Synchronization
{
	ssrc_t ssrc;
	uint8_t count;
	uint8_t padding[3] = {0,0,0};
	timestamp_t timestamps[3];
} AppleMIDI_Synchronization_t;

typedef struct PACKED AppleMIDI_ReceiverFeedback
{
	ssrc_t ssrc;
	uint16_t sequenceNr;
	uint16_t dummy;
} AppleMIDI_ReceiverFeedback_t;

typedef struct PACKED AppleMIDI_EndSession
{
	initiatorToken_t initiatorToken;
	ssrc_t ssrc;
} AppleMIDI_EndSession_t;

END_APPLEMIDI_NAMESPACE
