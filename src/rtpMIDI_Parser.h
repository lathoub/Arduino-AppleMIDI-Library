#pragma once

#include "utility/Deque.h"

#include <midi_Defs.h>

#include "rtpMIDI_Defs.h"
#include "rtp_Defs.h"

#include "AppleMIDI_Settings.h"
#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

template <class UdpClass, class Settings, class Platform>
class AppleMIDISession;

template <class UdpClass, class Settings, class Platform>
class rtpMIDIParser
{
private:
    bool _rtpHeadersComplete = false;
    bool _journalSectionComplete = false;
    bool _channelJournalSectionComplete = false;
    uint16_t midiCommandLength;
    uint8_t _journalTotalChannels;
    uint8_t rtpMidi_Flags = 0;
    int cmdCount = 0;
    uint8_t runningstatus = 0;
    size_t _bytesToFlush = 0;

protected:
    void debugPrintBuffer(RtpBuffer_t &buffer)
    {
#ifdef DEBUG
        for (int i = 0; i < buffer.size(); i++) 
        {
            SerialMon.print("  ");
            SerialMon.print(i);
            SerialMon.print(i < 10 ? "  " : " ");
        }
        for (int i = 0; i < buffer.size(); i++) 
        {
            SerialMon.print("0x");
            SerialMon.print(buffer[i] < 16 ? "0" : "");
            SerialMon.print(buffer[i], HEX);
            SerialMon.print(" ");
        }
#endif
    }

public:
	AppleMIDISession<UdpClass, Settings, Platform> * session;
    
	//  Parse the incoming string
	// return:
	// - return 0, when the parse does not have enough data
	// - return a negative number, when the parser encounters invalid or
	//      unexpected data. The negative number indicates the amount of bytes
	//      that were processed. They can be purged safely
	// - a positive number indicates the amount of valid bytes processed
	// 
	parserReturn parse(RtpBuffer_t &buffer)
	{
        debugPrintBuffer(buffer);

		conversionBuffer cb;
        
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
            rtp.sequenceNr = __ntohs(cb.value16);
            cb.buffer[0] = buffer[i++];
            cb.buffer[1] = buffer[i++];
            cb.buffer[2] = buffer[i++];
            cb.buffer[3] = buffer[i++];
            rtp.timestamp = __ntohl(cb.value32);
            cb.buffer[0] = buffer[i++];
            cb.buffer[1] = buffer[i++];
            cb.buffer[2] = buffer[i++];
            cb.buffer[3] = buffer[i++];
            rtp.ssrc = __ntohl(cb.value32);

            uint8_t version = RTP_VERSION(rtp.vpxcc);
    #ifdef DEBUG
            bool padding = RTP_PADDING(rtp.vpxcc);
            bool extension = RTP_EXTENSION(rtp.vpxcc);
            uint8_t csrc_count = RTP_CSRC_COUNT(rtp.vpxcc);
    #endif
            
            if (RTP_VERSION_2 != version)
            {
                return parserReturn::UnexpectedData;
            }

    #ifdef DEBUG
            bool marker = RTP_MARKER(rtp.mpayload);
    #endif
            uint8_t payloadType = RTP_PAYLOAD_TYPE(rtp.mpayload);
            
            if (PAYLOADTYPE_RTPMIDI != payloadType)
            {
                return parserReturn::UnexpectedData;
            }

            session->ReceivedRtp(rtp);

            // Next byte is the flag
            minimumLen += 1;
            if (buffer.size() < minimumLen)
                return parserReturn::NotSureGiveMeMoreData;

            // 2.2. MIDI Payload (https://www.ietf.org/rfc/rfc4695.html#section-2.2)
            // The payload MUST begin with the MIDI command section. The
            // MIDI command section codes a (possibly empty) list of timestamped
            // MIDI commands and provides the essential service of the payload
            // format.
            
            /* RTP-MIDI starts with 4 bits of flags... */
            rtpMidi_Flags = buffer[i++];

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

            cmdCount = 0;
            runningstatus = 0;

            while (i--)
                buffer.pop_front();
            
            _rtpHeadersComplete = true;
                        
            // initialize the Journal Section
            _journalSectionComplete = false;
            _channelJournalSectionComplete = false;
            _journalTotalChannels = 0;
        }
  
		// Always a MIDI section
		if (midiCommandLength > 0)
        {
			auto retVal = decodeMIDICommandSection(buffer);
            if (retVal != parserReturn::Processed) return retVal;
        }
  
        // The payload MAY also contain a journal section. The journal section
        // provides resiliency by coding the recent history of the stream. A
        // flag in the MIDI command section codes the presence of a journal
        // section in the payload.

        if (rtpMidi_Flags & RTP_MIDI_CS_FLAG_J)
        {
            auto retVal = decodeJournalSection(buffer);
            switch (retVal) {
            case parserReturn::Processed:
                break;
            case parserReturn::NotEnoughData:
                return parserReturn::NotEnoughData;
            case parserReturn::UnexpectedJournalData:
                _rtpHeadersComplete = false;
            default:
                return retVal;
            }
        }

        _rtpHeadersComplete = false;
        
        return parserReturn::Processed;
	}

    #include "rtpMIDI_Parser_JournalSection.hpp"
    
    #include "rtpMIDI_Parser_CommandSection.hpp"
};

END_APPLEMIDI_NAMESPACE
