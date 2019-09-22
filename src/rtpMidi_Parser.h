#include <midi_RingBuffer.h>
using namespace MIDI_NAMESPACE;

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#include "rtpMidi_Defs.h"
#include "AppleMidi_Util.h"

template<class UdpClass>
class Session;

template<class UdpClass>
class rtpMIDIParser
{
public:
	static int Parser(midi::RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, Session<UdpClass>* session, const amPortType& portType)
	{
		uint16_t minimumLen = sizeof(Rtp);
		if (buffer.getLength() < minimumLen)
			return -1;

		uint16_t i = 0;

		Rtp rtp;
		rtp.vpxcc        = buffer.peek(i++);
		rtp.mpayload     = buffer.peek(i++);
		rtp.sequenceNr   = ntohs(buffer.peek(i++), buffer.peek(i++));
		rtp.timestamp    = ntohl(buffer.peek(i++), buffer.peek(i++), buffer.peek(i++), buffer.peek(i++));
		rtp.ssrc         = ntohl(buffer.peek(i++), buffer.peek(i++), buffer.peek(i++), buffer.peek(i++));

		uint8_t version    = RTP_VERSION(rtp.vpxcc);
		bool padding       = RTP_PADDING(rtp.vpxcc);
		bool extension     = RTP_EXTENSION(rtp.vpxcc);
		uint8_t csrc_count = RTP_CSRC_COUNT(rtp.vpxcc);
		if (2 != version)
		{
			return PARSER_UNEXPECTED_DATA;
		}

		bool marker = RTP_MARKER(rtp.mpayload);
		uint8_t payloadType = RTP_PAYLOAD_TYPE(rtp.mpayload);
		if (PAYLOADTYPE_RTPMIDI != payloadType)
		{
			return PARSER_UNEXPECTED_DATA;
		}

		minimumLen = sizeof(RtpMIDI);
		if (buffer.getLength() < minimumLen)
			return PARSER_NOT_ENOUGH_DATA;

		RtpMIDI rtpMidi;
		rtpMidi.flags = buffer.peek(i++);
		uint16_t cmdLen = rtpMidi.flags & RTP_MIDI_CS_MASK_SHORTLEN;

		/* see if we have small or large len-field */
		if (rtpMidi.flags & RTP_MIDI_CS_FLAG_B)
		{
			uint8_t	octet = buffer.peek(i++);
			cmdLen	= (cmdLen << 8) | octet;
		}

		if (buffer.getLength() < cmdLen)
			return PARSER_NOT_ENOUGH_DATA;

		Serial.print("rtp + rtpmidi bytes Consumed ");
		Serial.println(i);

		buffer.pop(i); // consume all the bytes used so far

		session->ReceivedMidi(rtp, rtpMidi, buffer, cmdLen);

		buffer.pop(cmdLen); // consume all the bytes used so far

		Serial.print("MIDI bytes Consumed: ");
		Serial.println(cmdLen);

		return i;
	}
};

END_APPLEMIDI_NAMESPACE