#pragma once

#include "RingBuffer.h"
#include "endian.h"

#include "rtpMidi_Defs.h"
#include "rtp_Defs.h"
#include "midi_feat4_4_0/midi_Defs.h"

#include "AppleMidi_Settings.h"

#include "AppleMidi_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#ifndef PARSER_NOT_ENOUGH_DATA
#define PARSER_NOT_ENOUGH_DATA 0
#endif

#ifndef PARSER_UNEXPECTED_DATA
#define PARSER_UNEXPECTED_DATA -1
#endif

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
	int parse(RingBuffer<byte, Settings::MaxBufferSize> &buffer)
	{
		static byte a[8];

		auto minimumLen = sizeof(Rtp_t);
		if (buffer.getLength() < minimumLen)
		{
			return PARSER_NOT_ENOUGH_DATA;
		}

		size_t i = 0; // todo: rename to consumed

		Rtp_t rtp;
		rtp.vpxcc = buffer.peek(i++);
		rtp.mpayload = buffer.peek(i++);
		a[0] = buffer.peek(i++);
		a[1] = buffer.peek(i++);
		rtp.sequenceNr = ntohs(a[0], a[1]);
		a[0] = buffer.peek(i++);
		a[1] = buffer.peek(i++);
		a[2] = buffer.peek(i++);
		a[3] = buffer.peek(i++);
		rtp.timestamp = ntohl(a[0], a[1], a[2], a[3]);
		a[0] = buffer.peek(i++);
		a[1] = buffer.peek(i++);
		a[2] = buffer.peek(i++);
		a[3] = buffer.peek(i++);
		rtp.ssrc = ntohl(a[0], a[1], a[2], a[3]);

		uint8_t version = RTP_VERSION(rtp.vpxcc);
		bool padding = RTP_PADDING(rtp.vpxcc);
		bool extension = RTP_EXTENSION(rtp.vpxcc);
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

		V_DEBUG_PRINTLN(F("RTP OK"));
		V_DEBUG_PRINT(F("version: "));
		V_DEBUG_PRINTLN(version);
		V_DEBUG_PRINT(F("padding: "));
		V_DEBUG_PRINTLN(padding);
		V_DEBUG_PRINT(F("extension: "));
		V_DEBUG_PRINTLN(extension);
		V_DEBUG_PRINT(F("csrc_count: "));
		V_DEBUG_PRINTLN(csrc_count);

		// Next byte is the flag
		minimumLen += 1;
		if (buffer.getLength() < minimumLen)
		{
			return PARSER_NOT_ENOUGH_DATA;
		}

		/* RTP-MIDI starts with 4 bits of flags... */
		uint8_t rtpMidiFlags = buffer.peek(i++);

		// ...followed by a length-field of at least 4 bits
		uint16_t commandLength = rtpMidiFlags & RTP_MIDI_CS_MASK_SHORTLEN;

		/* see if we have small or large len-field */
		if (rtpMidiFlags & RTP_MIDI_CS_FLAG_B)
		{
			minimumLen += 1;
			if (buffer.getLength() < minimumLen)
			{
				return PARSER_NOT_ENOUGH_DATA;
			}

			uint8_t octet = buffer.peek(i++);
			commandLength = (commandLength << 8) | octet;
		}

		V_DEBUG_PRINT(F("Command length: "));
		V_DEBUG_PRINTLN(commandLength);

		minimumLen += commandLength;
		if (buffer.getLength() < minimumLen)
		{
			return PARSER_NOT_ENOUGH_DATA;
		}

		auto midiPosition = i;

		i += commandLength;

		/* if we have a journal-section -> dissect it */
		if (rtpMidiFlags & RTP_MIDI_CS_FLAG_J)
		{
			minimumLen += 1;
			if (buffer.getLength() < minimumLen)
			{
				return PARSER_NOT_ENOUGH_DATA;
			}

			/* lets get the main flags from the recovery journal header */
			uint8_t flags = buffer.peek(i++);

			/* At the same place we find the total channels encoded in the channel journal */
			uint8_t totalChannels = (flags & RTP_MIDI_JS_MASK_TOTALCHANNELS) + 1;

			V_DEBUG_PRINT(F("totalChannels: "));
			V_DEBUG_PRINTLN(totalChannels);

			// sequenceNr
			minimumLen += 2;
			if (buffer.getLength() < minimumLen)
				return PARSER_NOT_ENOUGH_DATA;

			/* the checkpoint-sequence-number can be used to see if the recovery journal covers all lost events */
			a[0] = buffer.peek(i++);
			a[1] = buffer.peek(i++);
			uint16_t checkPoint = ntohs(a[0], a[1]);

			/* do we have system journal? */
			if (flags & RTP_MIDI_JS_FLAG_S)
			{
			}

			if (flags & RTP_MIDI_JS_FLAG_Y)
			{
			}

			/* do we have channel journal(s)? */
			if (flags & RTP_MIDI_JS_FLAG_A)
			{
				/* iterate through all the channels specified in header */

				minimumLen += 3;
				if (buffer.getLength() < minimumLen)
					return PARSER_NOT_ENOUGH_DATA;

				for (auto j = 0; j < totalChannels; j++)
				{
					a[0] = buffer.peek(i++);
					a[1] = buffer.peek(i++);
					a[2] = buffer.peek(i++);

					uint32_t chanflags = ntohl(0x00, a[0], a[1], a[2]);
					uint16_t chanjourlen = (chanflags & RTP_MIDI_CJ_MASK_LENGTH) >> 8;

					/* Do we have a program change chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_P)
					{
						minimumLen += 3;
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;
						i += 3;
					}

					/* Do we have a control chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_C)
					{
					}

					/* Do we have a parameter changes? */
					if (chanflags & RTP_MIDI_CJ_FLAG_M)
					{
					}

					/* Do we have a pitch-wheel chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_W)
					{
						minimumLen += 2;
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;
						i += 2;
					}

					/* Do we have a note on/off chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_N)
					{
						minimumLen += 2;
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;

						a[0] = buffer.peek(i++);
						a[1] = buffer.peek(i++);
						const uint16_t header = ntohs(a[0], a[1]);

						uint8_t logListCount = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_LENGTH) >> 8;
						const uint8_t low = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_LOW) >> 4;
						const uint8_t high = (header & RTP_MIDI_CJ_CHAPTER_N_MASK_HIGH);

						// how many offbits octets do we have?
						uint8_t offbitCount = 0;
						if (low <= high)
							offbitCount = high - low + 1;
						else if ((low == 15) && (high == 0))
							offbitCount = 0;
						else if ((low == 15) && (high == 1))
							offbitCount = 0;
						else
							return PARSER_UNEXPECTED_DATA; // (LOW > HIGH) value pairs MUST NOT appear in the header.

						// special case -> no offbit octets, but 128 note-logs
						if ((logListCount == 127) && (low == 15) && (high == 0))
						{
							logListCount = 128;
							// offbitCount should be 0 (empty)
						}

						minimumLen += ((logListCount * 2) + offbitCount);
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;

						i += ((logListCount * 2) + offbitCount);

						// // Log List
						// for (auto j = 0; j < logListCount; j++ ) {
						// 	buffer.peek(i++);
						// 	buffer.peek(i++);
						// }

						// // Offbit Octets
						// for (auto j = 0; j < offbitCount; j++ ) {
						// 	buffer.peek(i++);
						// }
					}

					/* Do we have a note command extras chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_E)
					{
						minimumLen += 1;
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;

						/* first we need to get the flags & length of this chapter */
						uint8_t header = buffer.peek(i++);
						uint8_t log_count = header & RTP_MIDI_CJ_CHAPTER_E_MASK_LENGTH;

						log_count++;

						minimumLen += (log_count * 2);
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;

						for (auto j = 0; j < log_count; j++ ) {
							uint8_t note = buffer.peek(i++) & 0x7f;
							uint8_t octet = buffer.peek(i++);
							uint8_t count_vel = octet & 0x7f;
						}				
					}

					/* Do we have channel aftertouch chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_T)
					{
						minimumLen += 1;
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;

						i += 1;
					}

					/* Do we have a poly aftertouch chapter? */
					if (chanflags & RTP_MIDI_CJ_FLAG_A)
					{
						minimumLen += 2;
						if (buffer.getLength() < minimumLen)
							return PARSER_NOT_ENOUGH_DATA;

						/* first we need to get the flags & length of this chapter */
						uint8_t flags = buffer.peek(i++);
						uint8_t log_count = flags & RTP_MIDI_CJ_CHAPTER_A_MASK_LENGTH;

						/* count is encoded n+1 */
						log_count++;	

						for (auto j = 0; j < log_count; j++ ) {
							uint8_t note = buffer.peek(i++);
							uint8_t pressure = buffer.peek(i++);
						}				
					}
				}
			}

			if (flags & RTP_MIDI_JS_FLAG_H)
			{
			}
		}

		// OK we have parsed all the data
		// and we know the final length of this message (in i)

		// Always a midi section
		if (commandLength > 0)
			decodeMidiSection(rtpMidiFlags, buffer, midiPosition);

		buffer.pop(i); // consume all the bytes used so far

		return i;
	}

	size_t decodeMidiSection(uint8_t rtpMidiFlags, RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t i)
	{
		// ...followed by a length-field of at least 4 bits
		size_t commandLength = rtpMidiFlags & RTP_MIDI_CS_MASK_SHORTLEN;

		/* see if we have small or large len-field */
		if (rtpMidiFlags & RTP_MIDI_CS_FLAG_B)
		{
			uint8_t octet = buffer.peek(i++);
			commandLength = (commandLength << 8) | octet;
		}

		/* if we have a command-section -> dissect it */
		if (commandLength > 0)
		{
			int cmdCount = 0;

			uint8_t runningstatus = 0;

			/* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
			while (commandLength)
			{

				/* for the first command we only have a delta-time if Z-Flag is set */
				if ((cmdCount) || (rtpMidiFlags & RTP_MIDI_CS_FLAG_Z))
				{
					auto consumed = decodeTime(buffer, i);
					commandLength -= consumed;
					i += consumed;
				}

				if (commandLength > 0)
				{
					/* Decode a MIDI-command - if 0 is returned something went wrong */
					size_t consumed = decodeMidi(buffer, i, runningstatus);

					if (consumed == 0)
					{
						E_DEBUG_PRINTLN(F("decodeMidi indicates it did not consumed bytes ??!?!"));
						E_DEBUG_PRINT(F("decodeMidi commandLength is "));
						E_DEBUG_PRINTLN(commandLength);
					}

#if DEBUG >= LOG_LEVEL_TRACE
					T_DEBUG_PRINT(F("MIDI: consumed: "));
					T_DEBUG_PRINT(consumed);
					T_DEBUG_PRINT(F(" 0x"));
					for (auto j = 0; j < consumed; j++)
					{
						T_DEBUG_PRINT(buffer.peek(i + j), HEX);
						T_DEBUG_PRINT(" ");
					}
					T_DEBUG_PRINTLN();
#endif

					for (size_t j = 0; j < consumed; j++)
						session->ReceivedMidi(buffer.peek(i + j));

					commandLength -= consumed;
					i += consumed;

					cmdCount++;
				}
			}
		}

		return i;
	}

	size_t decodeTime(RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t i)
	{
		uint8_t consumed = 0;
		uint32_t deltatime = 0;

		/* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
		for (uint8_t j = 0; j < 4; j++)
		{
			uint8_t octet = buffer.peek(i + consumed);
			deltatime = (deltatime << 7) | (octet & RTP_MIDI_DELTA_TIME_OCTET_MASK);
			consumed++;

			if ((octet & RTP_MIDI_DELTA_TIME_EXTENSION) == 0)
				break;
		}
		return consumed;
	}

	size_t decodeMidi(RingBuffer<byte, Settings::MaxBufferSize> &buffer, size_t i, uint8_t &runningstatus)
	{
		size_t consumed = 0;

		uint8_t octet = buffer.peek(i);
		bool using_rs;

		/* midi realtime-data -> one octet  -- unlike serial-wired MIDI realtime-commands in RTP-MIDI will
         * not be intermingled with other MIDI-commands, so we handle this case right here and return */
		if (octet >= 0xf8)
		{
			return 1;
		}

		/* see if this first octet is a status message */
		if ((octet & RTP_MIDI_COMMAND_STATUS_FLAG) == 0)
		{
			/* if we have no running status yet -> error */
			if (((runningstatus)&RTP_MIDI_COMMAND_STATUS_FLAG) == 0)
			{
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
			if (octet < 0xf0)
			{
				runningstatus = octet;
			}
			else
			{
				/* system-realtime-commands maintain the current running-status
                 * other system-commands clear the running-status, since we
                 * already handled realtime, we can reset it here */
				runningstatus = 0;
			}
			consumed++;
		}

		/* non-system MIDI-commands encode the command in the high nibble and the channel
         * in the low nibble - so we will take care of those cases next */
		if (octet < 0xf0)
		{
			uint8_t type = (octet & 0xf0);
			uint8_t channel = (octet & 0x0f) + 1;

			switch (type)
			{
			case MIDI_NAMESPACE::MidiType::NoteOff:
			{
				uint8_t note = buffer.peek(i + consumed);
				consumed++;
				uint8_t velocity = buffer.peek(i + consumed);
				consumed++;
				//ext_consumed = decode_note_off( tvb, pinfo, tree, cmd_count, offset,  cmd_len, octet, *rsoffset, using_rs );
			}
			break;
			case MIDI_NAMESPACE::MidiType::NoteOn:
			{
				uint8_t note = buffer.peek(i + consumed);
				consumed++;
				uint8_t velocity = buffer.peek(i + consumed);
				consumed++;
				//ext_consumed = decode_note_on( tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
			}
			break;
			case MIDI_NAMESPACE::MidiType::AfterTouchPoly:
			{
				consumed += 2;
				//ext_consumed = decode_poly_pressure(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
			}
			break;
			case MIDI_NAMESPACE::MidiType::ControlChange:
				consumed += 2;
				//ext_consumed = decode_control_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
				break;
			case MIDI_NAMESPACE::MidiType::ProgramChange:
				consumed += 1;
				//ext_consumed = decode_program_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
				break;
			case MIDI_NAMESPACE::MidiType::AfterTouchChannel:
				consumed += 1;
				//ext_consumed = decode_channel_pressure(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
				break;
			case MIDI_NAMESPACE::MidiType::PitchBend:
				consumed += 2;
				//ext_consumed = decode_pitch_bend_change(tvb, pinfo, tree, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs );
				break;
			}

			return consumed;
		}

		/* Here we catch the remaining system-common commands */
		switch (octet)
		{
		case MIDI_NAMESPACE::MidiType::SystemExclusiveStart:
			//ext_consumed =  decode_sysex_start( tvb, pinfo, tree, cmd_count, offset, cmd_len );
			break;
		case MIDI_NAMESPACE::MidiType::TimeCodeQuarterFrame:
			consumed += 1;
			//ext_consumed =  decode_mtc_quarter_frame( tvb, pinfo, tree, cmd_count, offset, cmd_len );
			break;
		case MIDI_NAMESPACE::MidiType::SongPosition:
			consumed += 2;
			//ext_consumed =  decode_song_position_pointer( tvb, pinfo, tree, cmd_count, offset, cmd_len );
			break;
		case MIDI_NAMESPACE::MidiType::SongSelect:
			consumed += 1;
			//ext_consumed =  decode_song_select( tvb, pinfo, tree, cmd_count, offset, cmd_len );
			break;
		case MIDI_NAMESPACE::MidiType::TuneRequest:
			//ext_consumed =  decode_tune_request( tvb, pinfo, tree, cmd_count, offset, cmd_len );
			break;
		case MIDI_NAMESPACE::MidiType::SystemExclusiveEnd:
			//ext_consumed =  decode_sysex_end( tvb, pinfo, tree, cmd_count, offset, cmd_len );
			break;
		}

		return consumed;
	}
};

END_APPLEMIDI_NAMESPACE
