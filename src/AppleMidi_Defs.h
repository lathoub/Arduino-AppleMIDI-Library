#pragma once

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#define APPLEMIDI_LIBRARY_VERSION 0x020000
#define APPLEMIDI_LIBRARY_VERSION_MAJOR 2
#define APPLEMIDI_LIBRARY_VERSION_MINOR 0
#define APPLEMIDI_LIBRARY_VERSION_PATCH 0

#define CONTROL_PORT 5004

#undef OPTIONAL_MDNS

typedef uint32_t ssrc_t;
typedef uint32_t initiatorToken_t;
typedef uint64_t timestamp_t;

#define APPLEMIDI_PARTICIPANT_SLOT_FREE 0

const static uint8_t APPLEMIDI_SESSION_NAME_MAX_LEN = 24; // TODO: change with APPLEMIDI_SESSION_NAME_MAX_LEN

#define APPLEMIDI_LISTENER
//#define APPLEMIDI_INITIATOR

#define MIDI_SAMPLING_RATE_176K4HZ 176400
#define MIDI_SAMPLING_RATE_192KHZ 192000
#define MIDI_SAMPLING_RATE_DEFAULT 10000

/* Signature "Magic Value" for Apple network MIDI session establishment */
const byte amSignature[] = {0xff, 0xff};

/* 2 (stored in network byte order (big-endian)) */
const byte amProtocolVersion[] = {0x00, 0x00, 0x00, 0x02};

/* Apple network MIDI valid commands */
const byte amInvitation[] = {'I', 'N'};
const byte amEndSession[] = {'B', 'Y'};
const byte amSynchronization[] = {'C', 'K'};
const byte amInvitationAccepted[] = {'O', 'K'};
const byte amInvitationRejected[] = {'N', 'O'};
const byte amReceiverFeedback[] = {'R', 'S'};
const byte amBitrateReceiveLimit[] = {'R', 'L'};

const uint8_t SYNC_CK0 = 0;
const uint8_t SYNC_CK1 = 1;
const uint8_t SYNC_CK2 = 2;

// Same struct for Invitation, InvitationAccepted and InvitationRejected
typedef struct __attribute__((packed)) AppleMIDI_Invitation
{
	initiatorToken_t initiatorToken;
	ssrc_t ssrc;
	char sessionName[APPLEMIDI_SESSION_NAME_MAX_LEN + 1];

	const size_t getLength() const
	{
		return sizeof(AppleMIDI_Invitation) - (APPLEMIDI_SESSION_NAME_MAX_LEN) + strlen(sessionName);
	}

} AppleMIDI_Invitation_t;

typedef struct __attribute__((packed)) AppleMIDI_BitrateReceiveLimit
{
	ssrc_t ssrc;
	uint32_t bitratelimit;
} AppleMIDI_BitrateReceiveLimit_t;

typedef struct __attribute__((packed)) AppleMIDI_Synchronization
{
	ssrc_t ssrc;
	uint8_t count;
	uint8_t padding[3];
	timestamp_t timestamps[3];
} AppleMIDI_Synchronization_t;

typedef struct __attribute__((packed)) AppleMIDI_ReceiverFeedback
{
	ssrc_t ssrc;
	uint16_t sequenceNr;
	uint16_t dummy;
} AppleMIDI_ReceiverFeedback_t;

typedef struct __attribute__((packed)) AppleMIDI_EndSession
{
	initiatorToken_t initiatorToken;
	ssrc_t ssrc;
} AppleMIDI_EndSession_t;

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
enum SessionController : uint8_t
{
	Listener,
	Initiator,
};

#ifdef APPLEMIDI_INITIATOR
enum SessionInviteStatus : uint8_t
{
	None,
	SendControlInvite,
	ReceiveControlInvitation,
	WaitingForControlInvitationAccepted,
	SendContentInvite,
	WaitingForContentInvitationAccepted,
};
#endif

END_APPLEMIDI_NAMESPACE
