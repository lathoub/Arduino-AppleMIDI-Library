#pragma once

#include "RingBuffer.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#include "rtpMidi_Defs.h"
#include "endian.h"

template<class UdpClass>
class AppleMidiTransport;

template<class UdpClass>
class rtpMIDIParser
{
public:
	static size_t Parser(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, AppleMidiTransport<UdpClass>* session)
	{
		byte a[8]; 

		size_t minimumLen = sizeof(Rtp);
		if (buffer.getLength() < minimumLen)
			return -1;

		size_t i = 0;

		Rtp rtp;
		rtp.vpxcc        = buffer.peek(i++);
		rtp.mpayload     = buffer.peek(i++);
		a[0] = buffer.peek(i++); a[1] = buffer.peek(i++); 
		rtp.sequenceNr   = ntohs(a[0], a[1]);
		a[0] = buffer.peek(i++); a[1] = buffer.peek(i++); a[2] = buffer.peek(i++); a[3] = buffer.peek(i++); 
		rtp.timestamp    = ntohl(a[0], a[1], a[2], a[3]);
		a[0] = buffer.peek(i++); a[1] = buffer.peek(i++); a[2] = buffer.peek(i++); a[3] = buffer.peek(i++); 
		rtp.ssrc         = ntohl(a[0], a[1], a[2], a[3]);

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

		if (buffer.getLength() < 1)
			return PARSER_NOT_ENOUGH_DATA;

		/* RTP-MIDI starts with 4 bits of flags... */
		uint8_t flags = buffer.peek(i++);

		// Always a midi section
		i += decodeMidiSection(flags, buffer, session);

		/* if we have a journal-section -> dissect it */
		if (flags & RTP_MIDI_CS_FLAG_J) {
			decodeJournalSection(buffer, i);
		}

		buffer.pop(i); // consume all the bytes used so far

		Serial.print("rtpMIDI bytes Consumed: ");
		Serial.println(i);

		return i;
	}

	static size_t decodeMidiSection(uint8_t flags, RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, AppleMidiTransport<UdpClass>* session)
	{
		size_t i = 0;

		// ...followed by a length-field of at least 4 bits
		uint16_t cmdLen = flags & RTP_MIDI_CS_MASK_SHORTLEN;

		/* see if we have small or large len-field */
		if (flags & RTP_MIDI_CS_FLAG_B)
		{
			if (buffer.getLength() < 2)
				return PARSER_NOT_ENOUGH_DATA;

			uint8_t	octet = buffer.peek(i++);
			cmdLen	= (cmdLen << 8) | octet;
		}

		if (buffer.getLength() < 2 + cmdLen)
			return PARSER_NOT_ENOUGH_DATA;

		/* if we have a command-section -> dissect it */
		if (cmdLen > 0) {
			int cmdCount = 0;

			/* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
			while (cmdLen) {
				/* for the first command we only have a delta-time if Z-Flag is set */
				if ( (cmdCount) || (flags & RTP_MIDI_CS_FLAG_Z) ) {
					size_t consumed = decodeTime(buffer, i); // fixed amount of bytes??
					cmdLen -= consumed;
				}
				if (cmdLen > 0) {
					/* Decode a MIDI-command - if 0 is returned something went wrong */
					size_t consumed = decodeMidi(buffer, i);
					cmdLen -= consumed;

					cmdCount++;
				}
			}
		}

		return i;
	}

	static size_t decodeTime(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
		uint8_t consumed = 0;

		// TODO: always 4 bytes??

		/* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
		for (uint8_t j = 0; j < 4; j++) {
			uint8_t octet = buffer.peek(i++);
			unsigned long deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
			consumed++;

			if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
				break;
		}

		return consumed;
	}

	static size_t decodeMidi(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
		// parse, just to get the length

		// TODO: how to reuse the midi parser

	//	session->ReceivedMidi(rtp, rtpMidi, buffer, cmdLen);
		return 0;
	}

	static size_t decodeJournalSection(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
		/* lets get the main flags from the recovery journal header */
		uint8_t flags = buffer.peek(i++);

		/* At the same place we find the total channels encoded in the channel journal */
		uint8_t totalChannels = flags & RTP_MIDI_JS_MASK_TOTALCHANNELS;

		/* the checkpoint-sequence-number can be used to see if the recovery journal covers all lost events */
		buffer.peek(i++);
		buffer.peek(i++);

		/* do we have system journal? */
		if ( flags & RTP_MIDI_JS_FLAG_Y ) {
			/* first we need to get the flags & length from the system-journal */
			// int consumed = systemJournal(appleMidi, packetBuffer, offset);
		}

		/* do we have channel journal(s)? */
		if ( flags & RTP_MIDI_JS_FLAG_A	 ) {
			/* iterate through all the channels specified in header */
			for (auto i = 0; i <= totalChannels; i++ ) {
				//int consumed = channelJournal(appleMidi, packetBuffer, offset);
			}
		}	

		return 0;
	}

	static size_t decodeSystemJournal(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
		uint16_t systemflags = buffer.peek(i++); // 2 bytes!!!
		uint16_t sysjourlen  = systemflags & RTP_MIDI_SJ_MASK_LENGTH;

		/* Do we have a simple system commands chapter? */
		if (systemflags & RTP_MIDI_SJ_FLAG_D) {
			//offset += decode_sj_chapter_d(rtpMidi, packetBuffer, offset );
		}

		/* Do we have a active sensing chapter? */
		if (systemflags & RTP_MIDI_SJ_FLAG_V) {
			//offset++;
		}

		/* Do we have a sequencer state commands chapter? */
		if (systemflags & RTP_MIDI_SJ_FLAG_Q) {
			//offset += decode_sj_chapter_q( rtpMidi, packetBuffer, offset );
		}

		/* Do we have a MTC chapter? */
		if (systemflags & RTP_MIDI_SJ_FLAG_F) {
			//offset += decode_sj_chapter_f( rtpMidi, packetBuffer, offset );
		}

		/* Do we have a Sysex chapter? */
		if (systemflags & RTP_MIDI_SJ_FLAG_X) {
			// ext_consumed = decode_sj_chapter_x( rtpMidi, packetBuffer, offset, sysjourlen - consumed );
			// if ( ext_consumed < 0 ) {
			// 	return ext_consumed;
			// }
			// offset += ext_consumed;
		}

		return 0;
	}

	static size_t decodeChannelJournal(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
		return 0;
	}
};

END_APPLEMIDI_NAMESPACE