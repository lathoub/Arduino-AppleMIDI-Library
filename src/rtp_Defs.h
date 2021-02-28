#pragma once

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

//   0                   1                   2                   3
//   0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
//  | V |P|X|  CC   |M|     PT      |        Sequence number        |
//  +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+

#define RTP_VERSION_2 2

// first octet
#define RTP_P_FIELD 0x20
#define RTP_X_FIELD 0x10
#define RTP_CC_FIELD 0xF

// second octet
#define RTP_M_FIELD 0x80
#define RTP_PT_FIELD 0x7F

/* magic number */
#define PAYLOADTYPE_RTPMIDI 97

/* Version is the first 2 bits of the first octet*/
#define RTP_VERSION(octet) ((octet) >> 6)

/* Padding is the third bit; No need to shift, because true is any value
other than 0! */
#define RTP_PADDING(octet) ((octet)&RTP_P_FIELD)

/* Extension bit is the fourth bit */
#define RTP_EXTENSION(octet) ((octet)&RTP_X_FIELD)

/* CSRC count is the last four bits */
#define RTP_CSRC_COUNT(octet) ((octet)&RTP_CC_FIELD)

/* Marker is the first bit of the second octet */
#define RTP_MARKER(octet) ((octet)&RTP_M_FIELD)

/* Payload type is the last 7 bits */
#define RTP_PAYLOAD_TYPE(octet) ((octet)&RTP_PT_FIELD)

typedef struct PACKED Rtp
{
	uint8_t vpxcc;
	uint8_t mpayload;
	uint16_t sequenceNr;
	uint32_t timestamp;
	ssrc_t ssrc;
} Rtp_t;

END_APPLEMIDI_NAMESPACE
