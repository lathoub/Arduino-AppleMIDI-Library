#pragma once

#include "RingBuffer.h"
#include "midi_feat4_4_0/midi_Defs.h"

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
		decodeMidiSection(flags, buffer, session, i);

		/* if we have a journal-section -> dissect it */
		if (flags & RTP_MIDI_CS_FLAG_J) {
			decodeJournalSection(buffer, i);
		}

		buffer.pop(i); // consume all the bytes used so far

		return i;
	}

	static size_t decodeMidiSection(uint8_t flags, RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, AppleMidiTransport<UdpClass>* session, size_t& i)
	{
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

            uint8_t runningstatus = 0;
            
			/* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
			while (cmdLen) {
				/* for the first command we only have a delta-time if Z-Flag is set */
				if ( (cmdCount) || (flags & RTP_MIDI_CS_FLAG_Z) ) {
					size_t consumed = decodeTime(buffer, i);
					cmdLen -= consumed;
                    i += consumed;
				}
                
				if (cmdLen > 0) {
                    /* Decode a MIDI-command - if 0 is returned something went wrong */
					size_t consumed = decodeMidi(buffer, i, runningstatus);

					cmdLen -= consumed;
                    i += consumed;

					cmdCount++;
				}
			}
		}

		return i;
	}

	static size_t decodeTime(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t i)
	{
        uint8_t consumed = 0;
     	uint32_t deltatime = 0;

		/* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
		for (uint8_t j = 0; j < 4; j++) {
			uint8_t  octet = buffer.peek(i + consumed);
                        
			deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
            consumed++;
            
			if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
				break;
		}

        switch (consumed)
        {
            case 1:
                break;
            case 2:
                break;
            case 3:
                break;
            case 4:
                break;
        }
        
        
		return consumed;
	}

	static size_t decodeMidi(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t i, uint8_t& runningstatus)
	{
        int k = 0;
        
        uint8_t octet = buffer.peek(i + k);
        bool using_rs;
        
        /* midi realtime-data -> one octet  -- unlike serial-wired MIDI realtime-commands in RTP-MIDI will
         * not be intermingled with other MIDI-commands, so we handle this case right here and return */
        if ( octet >= 0xf8 ) {
            return k;
        }
        
        /* see if this first octet is a status message */
        if ((octet & RTP_MIDI_COMMAND_STATUS_FLAG) == 0) {
            /* if we have no running status yet -> error */
            if (((runningstatus) & RTP_MIDI_COMMAND_STATUS_FLAG) == 0 ) {
                return 0;
            }
            /* our first octet is "virtual" coming from a preceding MIDI-command,
             * so actually we have not really consumed anything yet */
            octet = runningstatus;
            using_rs = true;
        }
        else
        {
            /* We have a "real" status-byte */
            using_rs = false;
            /* Let's see how this octet influences our running-status */
            /* if we have a "normal" MIDI-command then the new status replaces the current running-status */
            if ( octet < 0xf0 ) {
                runningstatus = octet;
            } else {
                /* system-realtime-commands maintain the current running-status
                 * other system-commands clear the running-status, since we
                 * already handled realtime, we can reset it here */
                runningstatus = 0;
            }
            k++;
        }

        /* non-system MIDI-commands encode the command in the high nibble and the channel
         * in the low nibble - so we will take care of those cases next */
        if ( octet < 0xf0 ) {
            uint8_t type    = (octet & 0xf0);
            uint8_t channel = (octet & 0x0f) + 1;

            switch ( type ) {
                case MIDI_NAMESPACE::MidiType::NoteOff: {
                    uint8_t note     = buffer.peek(i + k); k++;
                    uint8_t velocity = buffer.peek(i + k); k++;
                    //ext_consumed = decode_note_off( tvb, pinfo, tree, cmd_count, offset,  cmd_len, octet, *rsoffset, using_rs );
                    } break;
                case MIDI_NAMESPACE::MidiType::NoteOn: {
                    uint8_t note     = buffer.peek(i + k); k++;
                    uint8_t velocity = buffer.peek(i + k); k++;
                    //ext_consumed = decode_note_on( tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
                    } break;
                case MIDI_NAMESPACE::MidiType::AfterTouchPoly: {
                    //ext_consumed = decode_poly_pressure(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
                    } break;
                case MIDI_NAMESPACE::MidiType::ControlChange:
                    //ext_consumed = decode_control_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
                    break;
                case MIDI_NAMESPACE::MidiType::ProgramChange:
                    //ext_consumed = decode_program_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
                    break;
                case MIDI_NAMESPACE::MidiType::AfterTouchChannel:
                    //ext_consumed = decode_channel_pressure(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
                    break;
                case MIDI_NAMESPACE::MidiType::PitchBend:
                    //ext_consumed = decode_pitch_bend_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
                    break;
            }
            
            return k;
        }

        /* Here we catch the remaining system-common commands */
        switch ( octet ) {
            case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
                //ext_consumed =  decode_sysex_start( tvb, pinfo, tree, cmd_count, offset, cmd_len );
                break;
            case MIDI_NAMESPACE::MidiType::TimeCodeQuarterFrame:
                //ext_consumed =  decode_mtc_quarter_frame( tvb, pinfo, tree, cmd_count, offset, cmd_len );
                break;
            case MIDI_NAMESPACE::MidiType::SongPosition:
                //ext_consumed =  decode_song_position_pointer( tvb, pinfo, tree, cmd_count, offset, cmd_len );
                break;
            case MIDI_NAMESPACE::MidiType::SongSelect:
                //ext_consumed =  decode_song_select( tvb, pinfo, tree, cmd_count, offset, cmd_len );
                break;
            case MIDI_NAMESPACE::MidiType::TuneRequest:
                //ext_consumed =  decode_tune_request( tvb, pinfo, tree, cmd_count, offset, cmd_len );
                break;
            case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
                //ext_consumed =  decode_sysex_end( tvb, pinfo, tree, cmd_count, offset, cmd_len );
                break;
        }
        
		return k;
	}

	static size_t decodeJournalSection(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
		/* lets get the main flags from the recovery journal header */
		uint8_t flags = buffer.peek(i++);

		/* At the same place we find the total channels encoded in the channel journal */
		uint8_t totalChannels = (flags & RTP_MIDI_JS_MASK_TOTALCHANNELS) + 1;

		/* the checkpoint-sequence-number can be used to see if the recovery journal covers all lost events */
		byte a = buffer.peek(i++);
		byte b = buffer.peek(i++);
        uint16_t checkPoint = ntohs(a,b);

		/* do we have system journal? */
        if ( flags & RTP_MIDI_JS_FLAG_S ) {
        }
        
		if (flags & RTP_MIDI_JS_FLAG_Y) {
			/* first we need to get the flags & length from the system-journal */
			decodeSystemJournal(buffer, i);
		}

		/* do we have channel journal(s)? */
		if (flags & RTP_MIDI_JS_FLAG_A) {
			/* iterate through all the channels specified in header */
			for (auto j = 0; j < totalChannels; j++ ) {
				decodeChannelJournal(buffer, i);
			}
		}	

        if (flags & RTP_MIDI_JS_FLAG_H) {
        }
        
		return i;
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

		return i;
	}

	static size_t decodeChannelJournal(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
	{
        byte a = buffer.peek(i++);
        byte b = buffer.peek(i++);
        byte c = buffer.peek(i++);
        uint32_t chanflags = ntohl(0x00, a,b,c);

        uint16_t chanjourlen = ( chanflags & RTP_MIDI_CJ_MASK_LENGTH ) >> 8;

        /* Do we have a program change chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_P ) {
        }
        
        /* Do we have a control chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_C ) {
        }
            
        /* Do we have a parameter changes? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_M ) {
        }
        
        /* Do we have a pitch-wheel chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_W ) {
        }
        
        /* Do we have a note on/off chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_N ) {
            decode_cj_chapter_n(buffer, i);
        }
        
        /* Do we have a note command extras chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_E ) {
        }
        
        /* Do we have channel aftertouch chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_T ) {
        }
        
        /* Do we have a poly aftertouch chapter? */
        if ( chanflags & RTP_MIDI_CJ_FLAG_A ) {
        }
        
        /* Make sanity check for consumed data vs. stated length of this channels journal */
        //if ( consumed != chanjourlen ) {
        //}
        
		return i;
	}
    
    static size_t decode_cj_chapter_n(RingBuffer<byte, BUFFER_MAX_SIZE>& buffer, size_t& i)
    {
        /* first we need to get the flags & length of this chapter */
        
        byte a = buffer.peek(i++);
        byte b = buffer.peek(i++);

        uint16_t header = ntohs(a,b);
        uint8_t logListCount = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_LENGTH) >> 8;
        uint8_t low  = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_LOW) >> 4;
        uint8_t high = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_HIGH);

        /* how many offbits octets do we have? */
        uint8_t offbitCount;
        if (low <= high) 
            offbitCount = high - low + 1;
        else if ((low == 15) && (high == 0)) 
            offbitCount = 0;
        else if ((low == 15) && (high == 1)) 
            offbitCount = 0;
        else
            return -1;

        /* special case -> no offbit octets, but 128 note-logs */
        if ((logListCount == 127 ) && (low == 15) && (high == 0)) 
            logListCount++;

        // Log List
        for (auto j = 0; j < logListCount; j++ ) {
            buffer.peek(i++);
            buffer.peek(i++);
        }

        // Offbit Octets
        for (auto j = 0; j < offbitCount; j++ ) {
            buffer.peek(i++);
        }
        
        return i;
    }

    
};

END_APPLEMIDI_NAMESPACE
