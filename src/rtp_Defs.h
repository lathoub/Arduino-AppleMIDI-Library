#pragma once

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

/* magic number */
#define PAYLOADTYPE_RTPMIDI 97

/* Version is the first 2 bits of the first octet*/
#define RTP_VERSION(octet) ((octet) >> 6)

/* Padding is the third bit; No need to shift, because true is any value
other than 0! */
#define RTP_PADDING(octet) ((octet)&0x20)

/* Extension bit is the fourth bit */
#define RTP_EXTENSION(octet) ((octet)&0x10)

/* CSRC count is the last four bits */
#define RTP_CSRC_COUNT(octet) ((octet)&0xF)

/* Marker is the first bit of the second octet */
#define RTP_MARKER(octet) ((octet)&0x80)

/* Payload type is the last 7 bits */
#define RTP_PAYLOAD_TYPE(octet) ((octet)&0x7F)

typedef struct PACKED Rtp
{
	uint8_t vpxcc;
	uint8_t mpayload;
	uint16_t sequenceNr;
	uint32_t timestamp;
	ssrc_t ssrc;
} Rtp_t;

END_APPLEMIDI_NAMESPACE
