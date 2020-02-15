#pragma once

#include "utilities/Deque.h"
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
private:
    bool _rtpHeadersComplete = false;
    bool _journalSectionComplete = false;
    uint16_t midiCommandLength;
    uint8_t _journalTotalChannels;
    uint8_t rtpMidi_Flags = 0;

public:
	AppleMidiTransport<UdpClass, Settings> * session;
    
	//  Parse the incoming string
	// return:
	// - return 0, when the parse does not have enough data
	// - return a negative number, when the parser encounters invalid or
	//      unexpected data. The negative number indicates the amount of bytes
	//      that were processed. They can be purged safely
	// - a positive number indicates the amount of valid bytes processed
	// 
	parserReturn parse(Deque<byte, Settings::MaxBufferSize> &buffer)
	{
		conversionBuffer cb;
        
        T_DEBUG_PRINT("RtpMIDI_Parser::Parser received ");
        T_DEBUG_PRINT(buffer.size());
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
            
        if (_rtpHeadersComplete == false)
        {
            auto minimumLen = sizeof(Rtp_t);
            if (buffer.size() < minimumLen)
                return parserReturn::NotSureGiveMeMoreData;

            size_t i = 0; // todo: rename to consumed

            Rtp_t rtp;
            rtp.vpxcc    = buffer[i++];
            rtp.mpayload = buffer[i++];
            
            cb.buffer[0] = buffer[i++];
            cb.buffer[1] = buffer[i++];
            rtp.sequenceNr = ntohs(cb.value16);
            cb.buffer[0] = buffer[i++];
            cb.buffer[1] = buffer[i++];
            cb.buffer[2] = buffer[i++];
            cb.buffer[3] = buffer[i++];
            rtp.timestamp = ntohl(cb.value32);
            cb.buffer[0] = buffer[i++];
            cb.buffer[1] = buffer[i++];
            cb.buffer[2] = buffer[i++];
            cb.buffer[3] = buffer[i++];
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
            if (buffer.size() < minimumLen)
                return parserReturn::NotSureGiveMeMoreData;

            // The payload MUST begin with the MIDI command section. The
            // MIDI command section codes a (possibly empty) list of timestamped
            // MIDI commands and provides the essential service of the payload
            // format.
            
            /* RTP-MIDI starts with 4 bits of flags... */
            rtpMidi_Flags = buffer[i++];

            V_DEBUG_PRINT(F("rtpMidi_Flags: 0x"));
            V_DEBUG_PRINTLN(rtpMidi_Flags, HEX);

            // ...followed by a length-field of at least 4 bits
            midiCommandLength = rtpMidi_Flags & RTP_MIDI_CS_MASK_SHORTLEN;

            /* see if we have small or large len-field */
            if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_B)
            {
                minimumLen += 1;
                if (buffer.size() < minimumLen)
                    return parserReturn::NotSureGiveMeMoreData;
                
                // long header
                uint8_t octet = buffer[i++];
                midiCommandLength = (midiCommandLength << 8) | octet;
            }

            while (i--)
                buffer.pop_front();
            
            _rtpHeadersComplete = true;
            
            V_DEBUG_PRINT(F("MIDI Command length: "));
            V_DEBUG_PRINTLN(midiCommandLength);
            
            // initialize the Journal Section
            _journalSectionComplete = false;
            _journalTotalChannels = 0;
        }
  
		// Always a midi section
		if (midiCommandLength > 0)
        {
			auto retVal = decodeMidiSection(buffer);
            if (retVal != parserReturn::Processed)
                return retVal;
        }
  
        // The payload MAY also contain a journal section. The journal section
        // provides resiliency by coding the recent history of the stream. A
        // flag in the MIDI command section codes the presence of a journal
        // section in the payload.

        if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_J)
        {
            auto retVal = decodeJournalSection(buffer);
            if (retVal != parserReturn::Processed)
                return retVal;
        }

        V_DEBUG_PRINT(F("Remaining control bytes "));
        V_DEBUG_PRINT(buffer.size());
        V_DEBUG_PRINTLN(F(" bytes"));

        _rtpHeadersComplete = false;
        
        return parserReturn::Processed;
	}

    #include "rtpMidi_Parser_JournalSection.hpp"
    
    #include "rtpMidi_Parser_MidiCommandSection.hpp"
};

END_APPLEMIDI_NAMESPACE
