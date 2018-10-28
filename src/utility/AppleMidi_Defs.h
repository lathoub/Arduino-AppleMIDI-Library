/*!
 *  @brief      AppleMIDI Library for the Arduino - Definitions
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include "AppleMidi_Namespace.h"

#include "Midi_Defs.h"

#if ARDUINO
	#include <Arduino.h>
#else
	#include <inttypes.h>
	typedef uint8_t byte;
#endif

#include <IPAddress.h>

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

#define SESSION_NAME_MAX_LEN 24

#define SYNC_CK0 0
#define SYNC_CK1 1
#define SYNC_CK2 2


BEGIN_APPLEMIDI_NAMESPACE

// Signatures
const uint8_t amSignature[]           = { 0xff, 0xff };
const uint8_t amInvitation[]          = { 'I', 'N' };
const uint8_t amInvitationAccepted[]  = { 'O', 'K' };
const uint8_t amInvitationRejected[]  = { 'N', 'O' };
const uint8_t amEndSession[]          = { 'B', 'Y' };
const uint8_t amSyncronization[]      = { 'C', 'K' };
const uint8_t amReceiverFeedback[]    = { 'R', 'S' };
const uint8_t amBitrateReceiveLimit[] = { 'R', 'L' };

const unsigned char stringTerminator[] = { 0x00 };


typedef struct __attribute__((packed)) AppleMIDI_Invitation
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

	AppleMIDI_Invitation()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitation, sizeof(amInvitation));
		version = 2;
	}

	inline uint8_t getLength()
	{
		return sizeof(AppleMIDI_Invitation) - (SESSION_NAME_MAX_LEN)+strlen(sessionName);
	}

} AppleMIDI_Invitation_t;

typedef struct __attribute__((packed)) AppleMIDI_InvitationAccepted
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

	AppleMIDI_InvitationAccepted()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationAccepted, sizeof(amInvitationAccepted));
		version = 2;
	}

	inline AppleMIDI_InvitationAccepted(uint32_t ssrc, uint32_t initiatorToken, char* name) : AppleMIDI_InvitationAccepted()
	{
		this->initiatorToken = initiatorToken;
		this->ssrc = ssrc;
		strncpy(this->sessionName, static_cast<const char*>(name), SESSION_NAME_MAX_LEN);
	}

	inline uint8_t getLength()
	{
		return sizeof(AppleMIDI_InvitationAccepted) - (SESSION_NAME_MAX_LEN)+strlen(sessionName);
	}

} AppleMIDI_InvitationAccepted_t;

typedef struct __attribute__((packed)) AppleMIDI_InvitationRejected
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

	AppleMIDI_InvitationRejected()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amInvitationRejected, sizeof(amInvitationRejected));
		version = 2;
	}

	inline AppleMIDI_InvitationRejected(uint32_t ssrc, uint32_t initiatorToken, char* sessionName) : AppleMIDI_InvitationRejected()
	{
		this->initiatorToken = initiatorToken;
		this->ssrc = ssrc;
		strncpy(this->sessionName, static_cast<const char*>(sessionName), SESSION_NAME_MAX_LEN);
	}

} AppleMIDI_InvitationRejected_t;

typedef struct __attribute__((packed)) AppleMIDI_ReceiverFeedback
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint16_t	sequenceNr;
	uint16_t	dummy;

	AppleMIDI_ReceiverFeedback()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amReceiverFeedback, sizeof(amReceiverFeedback));
	}

} AppleMIDI_ReceiverFeedback_t;

typedef struct __attribute__((packed)) AppleMIDI_Syncronization
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint8_t		count;
	uint8_t		padding[3];
	uint64_t	timestamps[3];

	AppleMIDI_Syncronization()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amSyncronization, sizeof(amSyncronization));
		memset(padding, 0, sizeof(padding));
	}

	AppleMIDI_Syncronization(uint32_t ssrc, uint8_t count, uint64_t* ts) : AppleMIDI_Syncronization()
	{
		this->ssrc = ssrc;
		this->count = count;
		this->timestamps[0] = ts[0];
		this->timestamps[1] = ts[1];
		this->timestamps[2] = ts[2];
	}

} AppleMIDI_Syncronization_t;

typedef struct __attribute__((packed)) AppleMIDI_EndSession
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	version;
	uint32_t	initiatorToken;
	uint32_t	ssrc;


	inline AppleMIDI_EndSession()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amEndSession, sizeof(amEndSession));
	}
} AppleMIDI_EndSession_t;

typedef struct __attribute__((packed)) AppleMIDI_BitrateReceiveLimit
{
	uint8_t		signature[2];
	uint8_t		command[2];
	uint32_t	ssrc;
	uint32_t	bitratelimit;

	AppleMIDI_BitrateReceiveLimit()
	{
		memcpy(signature, amSignature, sizeof(amSignature));
		memcpy(command, amBitrateReceiveLimit, sizeof(amBitrateReceiveLimit));
		bitratelimit = PACKET_MAX_SIZE;
	}
} AppleMIDI_BitrateReceiveLimit_t;

// SSRC, Synchronization source.
// (RFC 1889) The source of a stream of RTP packets, identified by a 32-bit numeric SSRC identifier
// carried in the RTP header so as not to be dependent upon the network address. All packets from a
// synchronization source form part of the same timing and sequence number space, so a receiver groups
// packets by synchronization source for playback. Examples of synchronization sources include the
// sender of a stream of packets derived from a signal source such as a microphone or a camera, or an
// RTP mixer. A synchronization source may change its data format, e.g., audio encoding, over time.
// The SSRC identifier is a randomly chosen value meant to be globally unique within a particular RTP
// session. A participant need not use the same SSRC identifier for all the RTP sessions in a
// multimedia session; the binding of the SSRC identifiers is provided through RTCP. If a participant
// generates multiple streams in one RTP session, for example from separate video cameras, each must
// be identified as a different SSRC.

enum Accept
{
	NoOne = 0,
	Peer = 1,
	Anyone = 0xff,
};

enum SessionInitiator
{
	Undefined,
	Remote,
	Local,
};

enum SessionInviteStatus
{
	None,
	SendControlInvite,
	ReceiveControlInvitation,
	WaitingForControlInvitationAccepted,
	SendContentInvite,
	WaitingForContentInvitationAccepted,
};

typedef struct {
	SessionInviteStatus	status;
	unsigned long		lastSend;
    IPAddress			remoteHost;
	uint16_t			remotePort;
	int					attempts;
    uint32_t			ssrc;
	uint32_t			initiatorToken;
} SessionInvite_t;

typedef struct {
	bool				enabled;
	unsigned long		lastTime;
	uint32_t			count;
	bool				busy;
} SessionSyncronization_t;

typedef struct {
    uint32_t				ssrc; // the unique identifier
	unsigned short			seqNum;
	SessionInitiator		initiator;
	SessionSyncronization_t	syncronization;
	IPAddress				contentIP;
	uint16_t				contentPort;
	SessionInvite_t			invite;
} Session_t;

typedef uint32_t MIDISamplingRate;
typedef uint64_t MIDITimeStamp;

// -----------------------------------------------------------------------------

/*! \brief Create an instance of the library
 */
#define APPLEMIDI_CREATE_INSTANCE(Type, Name)                            \
    APPLEMIDI_NAMESPACE::AppleMidiInterface<Type> Name;


/*! \brief Create an instance of the library with EnternetUDP.
*/
#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()                                      \
    APPLEMIDI_CREATE_INSTANCE(EthernetUDP, AppleMIDI);

/*! \brief Create a custom instance
 */
#define APPLEMIDI_CREATE_CUSTOM_INSTANCE(Type, Name, Settings)           \
    APPLEMIDI_NAMESPACE::AppleMidiInterface<Type, Settings> Name;


END_APPLEMIDI_NAMESPACE
