#pragma once

#include "utilities/RingBuffer.h"
#include "utilities/Array.h"
#include "endian.h"

#include "rtpMidi_Defs.h"
#include "rtp_Defs.h"
#include "midi_feat4_4_0/midi_Defs.h"

#include "AppleMidi_Settings.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class UdpClass, class Settings>
class AppleMidiTransport;

template <class UdpClass, class Settings>
class rtpMIDIParser
{
public:
	AppleMidiTransport<UdpClass, Settings> *session;

	//  Parse the incoming string
	// return:
	// - return 0, when the parse does not have enough data
	// - return a negative number, when the parser encounters invalid or
	//      unexpected data. The negative number indicates the amount of bytes
	//      that were processed. They can be purged safely
	// - a positive number indicates the amount of valid bytes processed
	// 
	parserReturn parse(RingBuffer<byte, Settings::MaxBufferSize> &buffer)
	{
		conversionBuffer cb;
        
        T_DEBUG_PRINT("RtpMIDI_Parser::Parser received ");
        T_DEBUG_PRINT(buffer.getLength());
        T_DEBUG_PRINTLN(" bytes");

        // [RFC3550] provides a complete description of the RTP header fields.
        // In this section, we clarify the role of a few RTP header fields for
        // MIDI applications. All fields are coded in network byte order (big-
        // endian).
        
        //  0                   1                   2                   3
        //  0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1 2 3 4 5 6 7 8 9 0 1
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // | V |P|X|  CC   |M|     PT      |        Sequence number        |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                           Timestamp                           |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                             SSRC                              |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                     MIDI command section ...                  |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
        // |                       Journal section ...                     |
        // +-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+-+
            
		auto minimumLen = sizeof(Rtp_t);
		if (buffer.getLength() < minimumLen)
			return parserReturn::NotEnoughData;

		size_t i = 0; // todo: rename to consumed

		Rtp_t rtp;
		rtp.vpxcc = buffer.peek(i++);
		rtp.mpayload = buffer.peek(i++);
		
		cb.buffer[0] = buffer.peek(i++);
		cb.buffer[1] = buffer.peek(i++);
		rtp.sequenceNr = ntohs(cb.value16);
		cb.buffer[0] = buffer.peek(i++);
		cb.buffer[1] = buffer.peek(i++);
		cb.buffer[2] = buffer.peek(i++);
		cb.buffer[3] = buffer.peek(i++);
		rtp.timestamp = ntohl(cb.value32);
		cb.buffer[0] = buffer.peek(i++);
		cb.buffer[1] = buffer.peek(i++);
		cb.buffer[2] = buffer.peek(i++);
		cb.buffer[3] = buffer.peek(i++);
		rtp.ssrc = ntohl(cb.value32);

		uint8_t version = RTP_VERSION(rtp.vpxcc);
#ifdef DEBUG
		bool padding = RTP_PADDING(rtp.vpxcc);
		bool extension = RTP_EXTENSION(rtp.vpxcc);
		uint8_t csrc_count = RTP_CSRC_COUNT(rtp.vpxcc);
#endif
        
        V_DEBUG_PRINTLN(F("RTP"));
        V_DEBUG_PRINT(F("version: "));
        V_DEBUG_PRINTLN(version);
        V_DEBUG_PRINT(F("padding: "));
        V_DEBUG_PRINTLN(padding);
        V_DEBUG_PRINT(F("extension: "));
        V_DEBUG_PRINTLN(extension);
        V_DEBUG_PRINT(F("csrc_count: "));
        V_DEBUG_PRINTLN(csrc_count);

		if (2 != version)
		{
            return parserReturn::UnexpectedData;
		}

#ifdef DEBUG
		bool marker = RTP_MARKER(rtp.mpayload);
#endif
		uint8_t payloadType = RTP_PAYLOAD_TYPE(rtp.mpayload);
        
        V_DEBUG_PRINT(F("marker: "));
        V_DEBUG_PRINTLN(marker);
        V_DEBUG_PRINT(F("Payload type: "));
        V_DEBUG_PRINTLN(payloadType);
        
		if (PAYLOADTYPE_RTPMIDI != payloadType)
		{
            V_DEBUG_PRINT(F("Unexpected Payload: "));
            V_DEBUG_PRINTLN(payloadType);

            return parserReturn::UnexpectedData;
		}

        V_DEBUG_PRINT(F("Sequence Nr: "));
        V_DEBUG_PRINTLN(rtp.sequenceNr);
        V_DEBUG_PRINT(F("Timestamp: "));
        V_DEBUG_PRINTLN(rtp.timestamp);
        V_DEBUG_PRINT(F("SSRC: 0x"));
        V_DEBUG_PRINT(rtp.ssrc, HEX);
        V_DEBUG_PRINT(F(" ("));
        V_DEBUG_PRINT(rtp.ssrc);
        V_DEBUG_PRINTLN(F(")"));

        session->ReceivedRtp(rtp);

		// Next byte is the flag
		minimumLen += 1;
		if (buffer.getLength() < minimumLen)
            return parserReturn::NotEnoughData;

        // The payload MUST begin with the MIDI command section. The
        // MIDI command section codes a (possibly empty) list of timestamped
        // MIDI commands and provides the essential service of the payload
        // format.
        
		/* RTP-MIDI starts with 4 bits of flags... */
		uint8_t rtpMidi_Flags = buffer.peek(i++);

        V_DEBUG_PRINT(F("rtpMidi_Flags: 0x"));
        V_DEBUG_PRINTLN(rtpMidi_Flags, HEX);

		// ...followed by a length-field of at least 4 bits
		uint16_t commandLength = rtpMidi_Flags & RTP_MIDI_CS_MASK_SHORTLEN;

		/* see if we have small or large len-field */
		if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_B)
		{
			minimumLen += 1;
			if (buffer.getLength() < minimumLen)
                return parserReturn::NotEnoughData;

            // long header
			uint8_t octet = buffer.peek(i++);
			commandLength = (commandLength << 8) | octet;
		}

		V_DEBUG_PRINT(F("MIDI Command length: "));
		V_DEBUG_PRINTLN(commandLength);

		minimumLen += commandLength;
//		if (buffer.getLength() < minimumLen)
//        {
//            return parserReturn::NotEnoughData;
//        }
        
		auto midiPosition = i;

		i += commandLength;

		// The payload MAY also contain a journal section. The journal section
        // provides resiliency by coding the recent history of the stream. A
        // flag in the MIDI command section codes the presence of a journal
        // section in the payload.

		if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_J)
		{
            auto retVal = decodeJournalSection(buffer, i, minimumLen);
            if (retVal != parserReturn::Processed)
                return retVal;
		}

		// OK we have parsed all the data
		// and we know the final length of this message (in i)

		// Always a midi section
		if (commandLength > 0)
        {
			decodeMidiSection(rtpMidi_Flags, buffer, commandLength, midiPosition);
        }
        
        V_DEBUG_PRINT(F("rtpMidi consumed: "));
        V_DEBUG_PRINT(i);
        V_DEBUG_PRINTLN(F(" bytes"));

		buffer.pop(i); // consume all the bytes used so far

        V_DEBUG_PRINT(F("Remaining control bytes "));
        V_DEBUG_PRINT(buffer.getLength());
        V_DEBUG_PRINTLN(F(" bytes"));

        return parserReturn::Processed;
	}

    #include "rtpMidi_Parser_JournalSection.hpp"
    
    #include "rtpMidi_Parser_MidiCommandSection.hpp"
};

END_APPLEMIDI_NAMESPACE
