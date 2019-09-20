#pragma once

#include <IPAddress.h>

#include "AppleMidi_Namespace.h"
#include "AppleMidi_Settings.h"

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

#define SESSION_NAME_MAX_LEN 24

#define SYNC_CK0 0
#define SYNC_CK1 1
#define SYNC_CK2 2

BEGIN_APPLEMIDI_NAMESPACE

// amount of socket / 2
#define MAX_SESSIONS (2)

/* Signature "Magic Value" for Apple network MIDI session establishment */
const byte amSignature[] = { 0xff, 0xff };

/* 2 (stored in network byte order (big-endian)) */
const uint32_t amProtocolVersion = 2;

/* Apple network MIDI valid commands */
const byte amInvitation[] = { 'I', 'N' };
const byte amInvitationAccepted[] = { 'O', 'K' };
const byte amInvitationRejected[] = { 'N', 'O' };
const byte amEndSession[] = { 'B', 'Y' };
const byte amSyncronization[] = { 'C', 'K' };
const byte amReceiverFeedback[] = { 'R', 'S' };
const byte amBitrateReceiveLimit[] = { 'R', 'L' };

typedef struct __attribute__((packed)) AppleMIDI_Invitation

{
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];
} AppleMIDI_Invitation_t;

typedef struct __attribute__((packed)) AppleMIDI_InvitationAccepted
{
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

	//inline AppleMIDI_InvitationAccepted(uint32_t ssrc, uint32_t initiatorToken, const char* sessionName)
	//{
	//	this->initiatorToken = initiatorToken;
	//	this->ssrc = ssrc;
	//	strncpy(this->sessionName, sessionName, SESSION_NAME_MAX_LEN);
	//}

} AppleMIDI_InvitationAccepted_t;

typedef struct __attribute__((packed)) AppleMIDI_InvitationRejected
{
	uint32_t	initiatorToken;
	uint32_t	ssrc;
	char		sessionName[SESSION_NAME_MAX_LEN + 1];

	//inline AppleMIDI_InvitationRejected(uint32_t ssrc, uint32_t initiatorToken, const char* sessionName)
	//{
	//	this->initiatorToken = initiatorToken;
	//	this->ssrc = ssrc;
	//	strncpy(this->sessionName, sessionName, SESSION_NAME_MAX_LEN);
	//}

} AppleMIDI_InvitationRejected_t;

typedef struct __attribute__((packed)) AppleMIDI_BitrateReceiveLimit
{
	uint32_t	ssrc;
	uint32_t	bitratelimit;

	//AppleMIDI_BitrateReceiveLimit()
	//{
	//	bitratelimit = BUFFER_MAX_SIZE;
	//}

} AppleMIDI_BitrateReceiveLimit_t;

enum amPortType : uint8_t
{
	Control,
	Data,
};

enum SessionInviteStatus : uint8_t
{
	None,
	SendControlInvite,
	ReceiveControlInvitation,
	WaitingForControlInvitationAccepted,
	SendContentInvite,
	WaitingForContentInvitationAccepted,
};

enum SessionInitiator : uint8_t
{
	Undefined,
	Remote,
	Local,
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
	uint16_t				seqNum;
	SessionInitiator		initiator;
	SessionSyncronization_t	syncronization;
	IPAddress				contentIP;
	uint16_t				contentPort;
	SessionInvite_t			invite;
} Session_t;

END_APPLEMIDI_NAMESPACE