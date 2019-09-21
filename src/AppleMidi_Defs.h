#pragma once

#include <IPAddress.h>

#include "AppleMidi_Namespace.h"

#include "Midi_Defs.h"

#define SLAVE
//#define MASTER

BEGIN_APPLEMIDI_NAMESPACE

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

#define MIDI_SAMPLING_RATE_176K4HZ 176400
#define MIDI_SAMPLING_RATE_192KHZ 192000
#define MIDI_SAMPLING_RATE_DEFAULT 10000

#define SYNC_CK0 0
#define SYNC_CK1 1
#define SYNC_CK2 2

#define SESSION_NAME_MAX_LEN 24

#define MAX_SESSIONS 2
#define MAX_PARTICIPANTS 5

// Max size of dissectable packet
#define BUFFER_MAX_SIZE 64

/* Signature "Magic Value" for Apple network MIDI session establishment */
const byte amSignature[] = { 0xff, 0xff };

/* 2 (stored in network byte order (big-endian)) */
const byte amProtocolVersion[] = { 0x00, 0x00, 0x00, 0x02 };

/* Apple network MIDI valid commands */
const byte amInvitation[] = { 'I', 'N' };
const byte amEndSession[] = { 'B', 'Y' };
const byte amSyncronization[] = { 'C', 'K' };
const byte amInvitationAccepted[] = { 'O', 'K' };
const byte amInvitationRejected[] = { 'N', 'O' };
const byte amReceiverFeedback[] = { 'R', 'S' };
const byte amBitrateReceiveLimit[] = { 'R', 'L' };

// Same struct for Invitation, InvitationAccepted and InvitationRejected
typedef struct __attribute__((packed)) AppleMIDI_Invitation
{
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

	inline const uint8_t getLength() const
	{
		return sizeof(AppleMIDI_Invitation) - (SESSION_NAME_MAX_LEN) + strlen(sessionName);
	}

} AppleMIDI_Invitation_t;

typedef struct __attribute__((packed)) AppleMIDI_BitrateReceiveLimit
{
	uint32_t	ssrc;
	uint32_t	bitratelimit;
} AppleMIDI_BitrateReceiveLimit_t;

typedef struct __attribute__((packed)) AppleMIDI_Syncronization
{
	uint32_t	ssrc;
	uint8_t		count;
	uint8_t		padding[3];
	uint64_t	timestamps[3];
} AppleMIDI_Syncronization_t;

typedef struct __attribute__((packed)) AppleMIDI_EndSession
{
	uint32_t	initiatorToken;
	uint32_t	ssrc;
} AppleMIDI_EndSession_t;

enum amPortType : uint8_t
{
	Control,
	Data,
};

enum ParticipantMode : uint8_t
{
	Undefined,
	Slave,
	Master,
};

#ifdef MASTER
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