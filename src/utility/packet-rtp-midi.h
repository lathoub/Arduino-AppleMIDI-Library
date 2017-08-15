/*!
 *  @file		packet-rtp-midi.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.0
 *  @author		lathoub, hackmancoltaire, rolimat
 *	@date		01/04/13
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "AppleMidi_Settings.h"

#include "packet-rtp.h"

BEGIN_APPLEMIDI_NAMESPACE

/* Definitions for protocol name during dissector-register */
#define RTP_MIDI_DISSECTOR_NAME "RFC 4695/6295 RTP-MIDI"
#define RTP_MIDI_DISSECTOR_SHORTNAME "RTP-MIDI"
#define RTP_MIDI_DISSECTOR_ABBREVIATION "rtpmidi"

#define RTP_MIDI_NO_RUNNING_STATUS 0xffff

/*
* MIDI Controller Numbers
*/
#define RTP_MIDI_CTRL_BANK_SELECT_MSB 0
#define RTP_MIDI_CTRL_MODULATION_WHEEL_OR_LEVER_MSB 1
#define RTP_MIDI_CTRL_BREATH_CONTROLLER_MSB 2
/* #define RTP_MIDI_CTRL 3 */
#define RTP_MIDI_CTRL_FOOT_CONTROLLER_MSB 4
#define RTP_MIDI_CTRL_PORTAMENTO_TIME_MSB 5
#define RTP_MIDI_CTRL_DATA_ENTRY_MSB 6
#define RTP_MIDI_CTRL_CHANNEL_VOLUME_MSB 7
#define RTP_MIDI_CTRL_BALANCE_MSB 8
/* #define RTP_MIDI_CTRL 9 */
#define RTP_MIDI_CTRL_PAN_MSB 10
#define RTP_MIDI_CTRL_EXPRESSION_CONTROLLER_MSB 11
#define RTP_MIDI_CTRL_EFFECT_CONTROL_1_MSB 12
#define RTP_MIDI_CTRL_EFFECT_CONTROL_2_MSB 13
/* #define RTP_MIDI_CTRL_ 14
#define RTP_MIDI_CTRL_ 15 */
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_1_MSB 16
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_2_MSB 17
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_3_MSB 18
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_4_MSB 19
/* #define RTP_MIDI_CTRL_ 20
#define RTP_MIDI_CTRL_ 21
#define RTP_MIDI_CTRL_ 22
#define RTP_MIDI_CTRL_ 23
#define RTP_MIDI_CTRL_ 24
#define RTP_MIDI_CTRL_ 25
#define RTP_MIDI_CTRL_ 26
#define RTP_MIDI_CTRL_ 27
#define RTP_MIDI_CTRL_ 28
#define RTP_MIDI_CTRL_ 29
#define RTP_MIDI_CTRL_ 30
#define RTP_MIDI_CTRL_ 31 */
#define RTP_MIDI_CTRL_BANK_SELECT_LSB 32
#define RTP_MIDI_CTRL_MODULATION_WHEEL_OR_LEVER_LSB 33
#define RTP_MIDI_CTRL_BREATH_CONTROLLER_LSB 34
/* #define RTP_MIDI_CTRL_ 35 */
#define RTP_MIDI_CTRL_FOOT_CONTROLLER_LSB 36
#define RTP_MIDI_CTRL_PORTAMENTO_TIME_LSB 37
#define RTP_MIDI_CTRL_DATA_ENTRY_LSB 38
#define RTP_MIDI_CTRL_CHANNEL_VOLUME_LSB 39
#define RTP_MIDI_CTRL_BALANCE_LSB 40
/* #define RTP_MIDI_CTRL_ 41 */
#define RTP_MIDI_CTRL_PAN_LSB 42
#define RTP_MIDI_CTRL_EXPRESSION_CONTROLLER_LSB 43
#define RTP_MIDI_CTRL_EFFECT_CONTROL_1_LSB 44
#define RTP_MIDI_CTRL_EFFECT_CONTROL_2_LSB 45
/* #define RTP_MIDI_CTRL_ 46
#define RTP_MIDI_CTRL_ 47 */
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_1_LSB 48
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_2_LSB 49
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_3_LSB 50
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_4_LSB 51
/* #define RTP_MIDI_CTRL_ 52
#define RTP_MIDI_CTRL_ 53
#define RTP_MIDI_CTRL_ 54
#define RTP_MIDI_CTRL_ 55
#define RTP_MIDI_CTRL_ 56
#define RTP_MIDI_CTRL_ 57
#define RTP_MIDI_CTRL_ 58
#define RTP_MIDI_CTRL_ 59
#define RTP_MIDI_CTRL_ 60
#define RTP_MIDI_CTRL_ 61
#define RTP_MIDI_CTRL_ 62
#define RTP_MIDI_CTRL_ 63 */
#define RTP_MIDI_CTRL_DAMPER_PEDAL 64
#define RTP_MIDI_CTRL_PORTAMENTO_ON_OFF 65
#define RTP_MIDI_CTRL_SUSTENUTO 66
#define RTP_MIDI_CTRL_SOFT_PEDAL 67
#define RTP_MIDI_CTRL_LEGATO_FOOTSWITCH 68
#define RTP_MIDI_CTRL_HOLD_2 69
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_1 70
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_2 71
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_3 72
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_4 73
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_5 74
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_6 75
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_7 76
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_8 77
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_9 78
#define RTP_MIDI_CTRL_SOUND_CONTROLLER_10 79
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_5 80
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_6 81
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_7 82
#define RTP_MIDI_CTRL_GENERAL_PURPOSE_CONTROLLER_8 83
#define RTP_MIDI_CTRL_PORTAMENTO_CONTROL 84
/* #define RTP_MIDI_CTRL_ 85
#define RTP_MIDI_CTRL_ 86
#define RTP_MIDI_CTRL_ 87
#define RTP_MIDI_CTRL_ 88
#define RTP_MIDI_CTRL_ 89
#define RTP_MIDI_CTRL_ 90 */
#define RTP_MIDI_CTRL_EFFECTS_1_DEPTH 91
#define RTP_MIDI_CTRL_EFFECTS_2_DEPTH 92
#define RTP_MIDI_CTRL_EFFECTS_3_DEPTH 93
#define RTP_MIDI_CTRL_EFFECTS_4_DEPTH 94
#define RTP_MIDI_CTRL_EFFECTS_5_DEPTH 95
#define RTP_MIDI_CTRL_DATA_INCREMENT 96
#define RTP_MIDI_CTRL_DATA_DECREMENT 97
#define RTP_MIDI_CTRL_NON_REGISTERED_PARAM_LSB 98
#define RTP_MIDI_CTRL_NON_REGISTERED_PARAM_MSB 99
#define RTP_MIDI_CTRL_REGISTERED_PARAM_LSB 100
#define RTP_MIDI_CTRL_REGISTERED_PARAM_MSB 101
/* #define RTP_MIDI_CTRL_ 102
#define RTP_MIDI_CTRL_ 103
#define RTP_MIDI_CTRL_ 104
#define RTP_MIDI_CTRL_ 105
#define RTP_MIDI_CTRL_ 106
#define RTP_MIDI_CTRL_ 107
#define RTP_MIDI_CTRL_ 108
#define RTP_MIDI_CTRL_ 109
#define RTP_MIDI_CTRL_ 110
#define RTP_MIDI_CTRL_ 111
#define RTP_MIDI_CTRL_ 112
#define RTP_MIDI_CTRL_ 113
#define RTP_MIDI_CTRL_ 114
#define RTP_MIDI_CTRL_ 115
#define RTP_MIDI_CTRL_ 116
#define RTP_MIDI_CTRL_ 117
#define RTP_MIDI_CTRL_ 118
#define RTP_MIDI_CTRL_ 119 */
/* MIDI Channel Mode Messages */
#define RTP_MIDI_CTRL_ALL_SOUND_OFF 120
#define RTP_MIDI_CTRL_RESET_ALL_CONTROLLERS 121
#define RTP_MIDI_CTRL_LOCAL_CONTROL_ON_OFF 122
#define RTP_MIDI_CTRL_ALL_NOTES_OFF 123
#define RTP_MIDI_CTRL_OMNI_MODE_OFF 124
#define RTP_MIDI_CTRL_OMNI_MODE_ON 125
#define RTP_MIDI_CTRL_MONO_MODE_ON 126
#define RTP_MIDI_CTRL_POLY_MODE_ON 127


/*
* MIDI Status Bytes (Channel Voice Messages)
*/
#define RTP_MIDI_STATUS_CHANNEL_NOTE_OFF 0x08 /* 0x8n n->channel */
#define RTP_MIDI_STATUS_CHANNEL_NOTE_ON 0x09 /* 0x9n n->channel */
#define RTP_MIDI_STATUS_CHANNEL_POLYPHONIC_KEY_PRESSURE 0x0a /* 0xan n->channel */
#define RTP_MIDI_STATUS_CHANNEL_CONTROL_CHANGE 0x0b /* 0xbn n->channel */
#define RTP_MIDI_STATUS_CHANNEL_PROGRAM_CHANGE 0x0c /* 0xcn n->channel */
#define RTP_MIDI_STATUS_CHANNEL_CHANNEL_PRESSURE 0x0d /* 0xdn n->channel */
#define RTP_MIDI_STATUS_CHANNEL_PITCH_BEND_CHANGE 0x0e /* 0xen n->channel */

/*
* MIDI-Channels
*/
#define RTP_MIDI_CHANNEL_1 0x00
#define RTP_MIDI_CHANNEL_2 0x01
#define RTP_MIDI_CHANNEL_3 0x02
#define RTP_MIDI_CHANNEL_4 0x03
#define RTP_MIDI_CHANNEL_5 0x04
#define RTP_MIDI_CHANNEL_6 0x05
#define RTP_MIDI_CHANNEL_7 0x06
#define RTP_MIDI_CHANNEL_8 0x07
#define RTP_MIDI_CHANNEL_9 0x08
#define RTP_MIDI_CHANNEL_10 0x09
#define RTP_MIDI_CHANNEL_11 0x0a
#define RTP_MIDI_CHANNEL_12 0x0b
#define RTP_MIDI_CHANNEL_13 0x0c
#define RTP_MIDI_CHANNEL_14 0x0d
#define RTP_MIDI_CHANNEL_15 0x0e
#define RTP_MIDI_CHANNEL_16 0x0f
#define RTP_MIDI_CHANNEL_MASK 0x0f


/*
* MIDI Status Bytes (System Exclusive Messages, System Common Messages, System Realtime Messages )
*/
#define RTP_MIDI_STATUS_COMMON_SYSEX_START 0xf0
#define RTP_MIDI_STATUS_COMMON_MTC_QUARTER_FRAME 0xf1
#define RTP_MIDI_STATUS_COMMON_SONG_POSITION_POINTER 0xf2
#define RTP_MIDI_STATUS_COMMON_SONG_SELECT 0xf3
#define RTP_MIDI_STATUS_COMMON_UNDEFINED_F4 0xf4
#define RTP_MIDI_STATUS_COMMON_UNDEFINED_F5 0xf5
#define RTP_MIDI_STATUS_COMMON_TUNE_REQUEST 0xf6
#define RTP_MIDI_STATUS_COMMON_SYSEX_END 0xf7
#define RTP_MIDI_STATUS_COMMON_REALTIME_TIMING_CLOCK 0xf8
#define RTP_MIDI_STATUS_COMMON_REALTIME_MIDI_TICK 0xf9 /* Spec says undefined */
#define RTP_MIDI_STATUS_COMMON_REALTIME_START 0xfa
#define RTP_MIDI_STATUS_COMMON_REALTIME_CONTINUE 0xfb
#define RTP_MIDI_STATUS_COMMON_REALTIME_STOP 0xfc
#define RTP_MIDI_STATUS_COMMON_REALTIME_UNDEFINED_FD 0xfd
#define RTP_MIDI_STATUS_COMMON_REALTIME_ACTIVE_SENSING 0xfe
#define RTP_MIDI_STATUS_COMMON_REALTIME_SYSTEM_RESET 0xff

/* TODO: Add MMC Commands */

/* TODO: Add MMC Responses */

#define RTP_MIDI_COMMON_MTC_QF_FRAME_LS_NIBBLE 0x00
#define RTP_MIDI_COMMON_MTC_QF_FRAME_MS_NIBBLE 0x01
#define RTP_MIDI_COMMON_MTC_QF_SECONDS_LS_NIBBLE 0x02
#define RTP_MIDI_COMMON_MTC_QF_SECONDS_MS_NIBBLE 0x03
#define RTP_MIDI_COMMON_MTC_QF_MINUTES_LS_NIBBLE 0x04
#define RTP_MIDI_COMMON_MTC_QF_MINUTES_MS_NIBBLE 0x05
#define RTP_MIDI_COMMON_MTC_QF_HOURS_LS_NIBBLE 0x06
#define RTP_MIDI_COMMON_MTC_QF_HOURS_MS_NIBBLE 0x07

/* used to mask the most significant bit, which flags the start of a new midi-command! */
#define RTP_MIDI_COMMAND_STATUS_FLAG 0x80

/* used to mask the lower 7 bits of the single octets that make up the delta-time */
#define RTP_MIDI_DELTA_TIME_OCTET_MASK 0x7f
/* used to mask the most significant bit, which flags the extension of the delta-time */
#define RTP_MIDI_DELTA_TIME_EXTENSION 0x80

#define RTP_MIDI_CS_FLAG_B 0x80
#define RTP_MIDI_CS_FLAG_J 0x40
#define RTP_MIDI_CS_FLAG_Z 0x20
#define RTP_MIDI_CS_FLAG_P 0x10
#define RTP_MIDI_CS_MASK_SHORTLEN 0x0f
#define RTP_MIDI_CS_MASK_LONGLEN 0x0fff

#define RTP_MIDI_CJ_CHAPTER_M_FLAG_J 0x80
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_K 0x40
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_L 0x20
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_M 0x10
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_N 0x08
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_T 0x04
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_V 0x02
#define RTP_MIDI_CJ_CHAPTER_M_FLAG_R 0x01

#define RTP_MIDI_JS_FLAG_S 0x80
#define RTP_MIDI_JS_FLAG_Y 0x40
#define RTP_MIDI_JS_FLAG_A 0x20
#define RTP_MIDI_JS_FLAG_H 0x10
#define RTP_MIDI_JS_MASK_TOTALCHANNELS 0x0f

#define RTP_MIDI_SJ_FLAG_S 0x8000
#define RTP_MIDI_SJ_FLAG_D 0x4000
#define RTP_MIDI_SJ_FLAG_V 0x2000
#define RTP_MIDI_SJ_FLAG_Q 0x1000
#define RTP_MIDI_SJ_FLAG_F 0x0800
#define RTP_MIDI_SJ_FLAG_X 0x0400
#define RTP_MIDI_SJ_MASK_LENGTH 0x03ff

#define RTP_MIDI_SJ_CHAPTER_D_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_B 0x40
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_G 0x20
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_H 0x10
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_J 0x08
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_K 0x04
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_Y 0x02
#define RTP_MIDI_SJ_CHAPTER_D_FLAG_Z 0x01

#define RTP_MIDI_SJ_CHAPTER_D_RESET_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_D_RESET_COUNT 0x7f
#define RTP_MIDI_SJ_CHAPTER_D_TUNE_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_D_TUNE_COUNT 0x7f
#define RTP_MIDI_SJ_CHAPTER_D_SONG_SEL_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_D_SONG_SEL_VALUE 0x7f

#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_S 0x8000
#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_C 0x4000
#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_V 0x2000
#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_L 0x1000
#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_MASK_DSZ 0x0c00
#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_MASK_LENGTH 0x03ff
#define RTP_MIDI_SJ_CHAPTER_D_SYSCOM_MASK_COUNT 0xff

#define RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_C 0x40
#define RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_L 0x20
#define RTP_MIDI_SJ_CHAPTER_D_SYSREAL_MASK_LENGTH 0x1f
#define RTP_MIDI_SJ_CHAPTER_D_SYSREAL_MASK_COUNT 0xff

#define RTP_MIDI_SJ_CHAPTER_Q_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_Q_FLAG_N 0x40
#define RTP_MIDI_SJ_CHAPTER_Q_FLAG_D 0x20
#define RTP_MIDI_SJ_CHAPTER_Q_FLAG_C 0x10
#define RTP_MIDI_SJ_CHAPTER_Q_FLAG_T 0x80
#define RTP_MIDI_SJ_CHAPTER_Q_MASK_TOP 0x07
#define RTP_MIDI_SJ_CHAPTER_Q_MASK_CLOCK 0x07ffff
#define RTP_MIDI_SJ_CHAPTER_Q_MASK_TIMETOOLS 0xffffff

#define RTP_MIDI_SJ_CHAPTER_F_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_F_FLAG_C 0x40
#define RTP_MIDI_SJ_CHAPTER_F_FLAG_P 0x20
#define RTP_MIDI_SJ_CHAPTER_F_FLAG_Q 0x10
#define RTP_MIDI_SJ_CHAPTER_F_FLAG_D 0x08
#define RTP_MIDI_SJ_CHAPTER_F_MASK_POINT 0x07
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT0 0xf0000000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT1 0x0f000000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT2 0x00f00000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT3 0x000f0000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT4 0x0000f000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT5 0x00000f00
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT6 0x000000f0
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MT7 0x0000000f
#define RTP_MIDI_SJ_CHAPTER_F_MASK_HR 0xff000000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_MN 0x00ff0000
#define RTP_MIDI_SJ_CHAPTER_F_MASK_SC 0x0000ff00
#define RTP_MIDI_SJ_CHAPTER_F_MASK_FR 0x000000ff

#define RTP_MIDI_SJ_CHAPTER_X_FLAG_S 0x80
#define RTP_MIDI_SJ_CHAPTER_X_FLAG_T 0x40
#define RTP_MIDI_SJ_CHAPTER_X_FLAG_C 0x20
#define RTP_MIDI_SJ_CHAPTER_X_FLAG_F 0x10
#define RTP_MIDI_SJ_CHAPTER_X_FLAG_D 0x08
#define RTP_MIDI_SJ_CHAPTER_X_FLAG_L 0x04
#define RTP_MIDI_SJ_CHAPTER_X_MASK_STA 0x03
#define RTP_MIDI_SJ_CHAPTER_X_MASK_TCOUNT 0xff
#define RTP_MIDI_SJ_CHAPTER_X_MASK_COUNT 0xff

#define RTP_MIDI_CJ_FLAG_S 0x800000
#define RTP_MIDI_CJ_FLAG_H 0x040000
#define RTP_MIDI_CJ_FLAG_P 0x000080
#define RTP_MIDI_CJ_FLAG_C 0x000040
#define RTP_MIDI_CJ_FLAG_M 0x000020
#define RTP_MIDI_CJ_FLAG_W 0x000010
#define RTP_MIDI_CJ_FLAG_N 0x000008
#define RTP_MIDI_CJ_FLAG_E 0x000004
#define RTP_MIDI_CJ_FLAG_T 0x000002
#define RTP_MIDI_CJ_FLAG_A 0x000001
#define RTP_MIDI_CJ_MASK_LENGTH 0x03ff00
#define RTP_MIDI_CJ_MASK_CHANNEL 0x780000
#define RTP_MIDI_CJ_CHANNEL_SHIFT 19

#define RTP_MIDI_CJ_CHAPTER_M_MASK_LENGTH 0x3f

#define RTP_MIDI_CJ_CHAPTER_N_MASK_LENGTH 0x7f00
#define RTP_MIDI_CJ_CHAPTER_N_MASK_LOW 0x00f0
#define RTP_MIDI_CJ_CHAPTER_N_MASK_HIGH 0x000f

#define RTP_MIDI_CJ_CHAPTER_E_MASK_LENGTH 0x7f
#define RTP_MIDI_CJ_CHAPTER_A_MASK_LENGTH 0x7f


class PacketRtpMidi {

public:
	PacketRtpMidi() {
#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("PacketRtpMidi verbose");
#endif
	}

	static int dissect_rtp_midi(Dissector* dissector, IAppleMidi* appleMidi, unsigned char* packetBuffer, size_t packetSize) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.print ("dissect_rtp_midi ");
		Serial.print (dissector->_identifier);
		Serial.print (", packetSize is ");
		Serial.println (packetSize);
#endif

		int consumed = PacketRtp::dissect_rtp(dissector, appleMidi, packetBuffer, packetSize);

		if (consumed <= 0) {
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			Serial.println("Unprocessed packet (No valid rtp midi content).");
			#endif
			return 0;
		}

		int offset = consumed;
		int totchan = 0;
		unsigned int	rsoffset = 0;

		/*
		* MIDI command section
		*/

		/* RTP-MIDI starts with 4 bits of flags... */
		uint8_t flags = packetBuffer[offset];

		/* ...followed by a length-field of at least 4 bits */
		unsigned int cmd_len = flags & RTP_MIDI_CS_MASK_SHORTLEN;

#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.print ("cmd_len is ");
		Serial.println (cmd_len);
#endif

		/* see if we have small or large len-field */
		if (flags & RTP_MIDI_CS_FLAG_B) {
			uint8_t	octet = packetBuffer[offset + 1];
			cmd_len	= ( cmd_len << 8 ) | octet;
			offset	+= 2;
		} else {
			offset++;
		}

		/* if we have a command-section -> dissect it */
		if (cmd_len) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
			Serial.print ("dissect command section with packet size ");
			Serial.println (packetSize);
#endif

			/* No commands decoded yet */
			int cmd_count = 0;

			/* RTP-MIDI-pdus always start with no running status */
			byte runningstatus = 0;

			/* Multiple MIDI-commands might follow - the exact number can only be discovered by really decoding the commands! */
			while (cmd_len) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
				Serial.print ("cmd count is ");
				Serial.println (cmd_count);
#endif

				/* for the first command we only have a delta-time if Z-Flag is set */
				if ( (cmd_count) || (flags & RTP_MIDI_CS_FLAG_Z) ) {
					/* Decode a delta-time - if 0 is returned something went wrong */
					int consumed = decodetime(appleMidi, packetBuffer, offset, cmd_len);
					if ( -1 == consumed ) {
#ifdef APPLEMIDI_DEBUG
						Serial.print ("ReportedBoundsError 1");
#endif

						return offset;
					}

					/* seek to next command and set remaining length */
					offset += consumed;
					cmd_len -= consumed;
				}

				/* Only decode MIDI-command if there is any data left - it is valid to only have delta-time! */
				if (cmd_len) {
					/* Decode a MIDI-command - if 0 is returned something went wrong */
					int consumed = decodemidi(appleMidi, packetBuffer, cmd_count, offset, cmd_len, &runningstatus, &rsoffset);
					if (-1 == consumed) {
#ifdef APPLEMIDI_DEBUG
						Serial.print ("ReportedBoundsError 2");
#endif

						return offset;
					}

					/* seek to next delta-time and set remaining length */
					offset += consumed;
					cmd_len -= consumed;

					/* as we have successfully decoded another command, increment count */
					cmd_count++;
				}
			}
		}

		/*
		* Journal section
		*/

		/* if we have a journal-section -> dissect it */
		if ( flags & RTP_MIDI_CS_FLAG_J ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
			Serial.println("journal section");
#endif

			/* lets get the main flags from the recovery journal header */
			flags = packetBuffer[offset];

			/* At the same place we find the total channels encoded in the channel journal */
			totchan = flags & RTP_MIDI_JS_MASK_TOTALCHANNELS;
			offset++;

			/* the checkpoint-sequence-number can be used to see if the recovery journal covers all lost events */
			offset += 2;

			/* do we have system journal? */
			if ( flags & RTP_MIDI_JS_FLAG_Y ) {
				/* first we need to get the flags & length from the system-journal */
				int consumed = decode_system_journal(appleMidi, packetBuffer, offset);

				if ( -1 == consumed ) {
#ifdef APPLEMIDI_DEBUG
					Serial.print ("ReportedBoundsError 3");
#endif

					return offset;
				}

				/* seek to optional channel-journals-section */
				offset += consumed;
			}

			/* do we have channel journal(s)? */
			if ( flags & RTP_MIDI_JS_FLAG_A	 ) {
				/* iterate through all the channels specified in header */
				for (int i = 0; i <= totchan; i++ ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
					Serial.print("Processing channel journal: ");
					Serial.println(i);
#endif

					int consumed = decode_channel_journal(appleMidi, packetBuffer, offset);

#ifdef APPLEMIDI_DEBUG_VERBOSE
					Serial.print("Consumed by channel journal (");
					Serial.print(i);
					Serial.print("): ");
					Serial.println(consumed);
#endif

					if ( -1 == consumed ) {
#ifdef APPLEMIDI_DEBUG
						Serial.println("ReportedBoundsError 4");
#endif

						return offset;
					}

					/* seek to next channel-journal */
					offset += consumed;
				}
			}
		}

		return offset;
	}

	/*
	* Here the system-journal is decoded.
	*/
	static int
	decode_system_journal(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset)
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("decode_system_journal");
#endif
		unsigned int start_offset = offset;
		int				consumed     = 0;
		int				ext_consumed = 0;

		uint16_t systemflags = packetBuffer[offset]; // 2 bytes!!! tvb_get_ntohs
		uint16_t sysjourlen  = systemflags & RTP_MIDI_SJ_MASK_LENGTH;

		offset	 += 2;
		consumed += 2;

		/* Do we have a simple system commands chapter? */
		if ( systemflags & RTP_MIDI_SJ_FLAG_D ) {
			offset += decode_sj_chapter_d(rtpMidi, packetBuffer, offset );
		}

		/* Do we have a active sensing chapter? */
		if ( systemflags & RTP_MIDI_SJ_FLAG_V ) {
			offset++;
		}

		/* Do we have a sequencer state commands chapter? */
		if ( systemflags & RTP_MIDI_SJ_FLAG_Q ) {
			offset += decode_sj_chapter_q( rtpMidi, packetBuffer, offset );
		}

		/* Do we have a MTC chapter? */
		if ( systemflags & RTP_MIDI_SJ_FLAG_F ) {
			offset += decode_sj_chapter_f( rtpMidi, packetBuffer, offset );
		}


		/* Do we have a Sysex chapter? */
		if ( systemflags & RTP_MIDI_SJ_FLAG_X ) {
			ext_consumed = decode_sj_chapter_x( rtpMidi, packetBuffer, offset, sysjourlen - consumed );
			if ( ext_consumed < 0 ) {
				return ext_consumed;
			}
			offset += ext_consumed;
		}

		/* Make sanity check for consumed data vs. stated length of system journal */
		if ( offset-start_offset != sysjourlen ) {
			return -1;
		}
		return offset-start_offset;
	}

	/*
	 * Here a channel-journal is decoded.
	 */
	static int
	decode_channel_journal(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
	   uint32_t				chanflags;
	   uint16_t				chanjourlen;
	   int				consumed = 0;
	   int				ext_consumed = 0;

	   /* first we need to get the flags & length of this channel-journal */
	   //memcpy(&chanflags, packetBuffer + offset, 3);

	   chanflags = ((uint32_t)*((const uint8_t *)packetBuffer + offset)<<16|  \
	  			   (uint32_t)*((const uint8_t *)packetBuffer + (offset+1))<<8|   \
	  			   (uint32_t)*((const uint8_t *)packetBuffer + (offset+2))<<0);

		chanjourlen = ( chanflags & RTP_MIDI_CJ_MASK_LENGTH ) >> 8;

	#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.print("Chan Flags: ");
		Serial.print(chanflags, HEX);

	   Serial.print("decode_channel_journal - Reported channel length: ");
	   Serial.print(chanjourlen);
	   Serial.print(", Beginning offset: ");
	   Serial.println(offset);
	#endif

			/* take care of length of header */
			offset	 += 3;
			consumed += 3;

			/* Do we have a program change chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_P ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
					Serial.println("cj_flag_p: 3");
#endif

				offset	 += 3;
				consumed += 3;
			}

			/* Do we have a control chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_C ) {
				ext_consumed = decode_cj_chapter_c(rtpMidi, packetBuffer, offset );
#ifdef APPLEMIDI_DEBUG_VERBOSE
				Serial.print("cj_chapter_c: ");
				Serial.println(ext_consumed);
#endif

				if ( ext_consumed < 0 ) {
					return ext_consumed;
				}
				consumed += ext_consumed;
				offset	 += ext_consumed;

			}

			/* Do we have a parameter changes? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_M ) {
				ext_consumed = decode_cj_chapter_m(rtpMidi, packetBuffer, offset );
#ifdef APPLEMIDI_DEBUG_VERBOSE
				Serial.print("cj_chapter_m: ");
				Serial.println(ext_consumed);
#endif

				if ( ext_consumed < 0 ) {
					//return ext_consumed;
				}
				consumed += ext_consumed;
				offset	 += ext_consumed;
			}

			/* Do we have a pitch-wheel chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_W ) {

				offset++;
				consumed++;

				offset++;
				consumed++;
			}

			/* Do we have a note on/off chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_N ) {
				ext_consumed = decode_cj_chapter_n(rtpMidi, packetBuffer, offset );
#ifdef APPLEMIDI_DEBUG_VERBOSE
				Serial.print("cj_chapter_n: ");
				Serial.println(ext_consumed);
#endif

				if ( ext_consumed < 0 ) {
					//return ext_consumed;
				}
				consumed += ext_consumed;
				offset	 += ext_consumed;
			}

			/* Do we have a note command extras chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_E ) {
				ext_consumed = decode_cj_chapter_e(rtpMidi, packetBuffer, offset );
#ifdef APPLEMIDI_DEBUG_VERBOSE
				Serial.print("cj_chapter_e: ");
				Serial.println(ext_consumed);
#endif

				if ( ext_consumed < 0 ) {
					//return ext_consumed;
				}
				consumed += ext_consumed;
				offset	 += ext_consumed;
			}

			/* Do we have channel aftertouch chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_T ) {

				offset++;
				consumed++;
			}

			/* Do we have a poly aftertouch chapter? */
			if ( chanflags & RTP_MIDI_CJ_FLAG_A ) {
				ext_consumed = decode_cj_chapter_a(rtpMidi, packetBuffer, offset );
#ifdef APPLEMIDI_DEBUG_VERBOSE
				Serial.print("cj_chapter_a: ");
				Serial.println(ext_consumed);
#endif

				if ( ext_consumed < 0 ) {
					//return ext_consumed;
				}
				consumed += ext_consumed;
			}

			/* Make sanity check for consumed data vs. stated length of this channels journal */
			if ( consumed != chanjourlen ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
			   Serial.print("Calculated consumption for channel journal: ");
			   Serial.print(consumed);
			 Serial.print(" != Channel Journal Length: ");
			   Serial.println(chanjourlen);
#endif
				return chanjourlen;
			}

			return consumed;
		}

	/*
	* Here each single MIDI-command is decoded.
	* The Status-octet is read and then the decoding
	* of the individual MIDI-commands is punted to
	* external decoders.
	*/
	static int
	decodemidi(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte *runningstatus, unsigned int *rsoffset )
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println(F("decodemidi"));
#endif

		int	consumed = 0;
		int	ext_consumed = 0;
		bool using_rs;

		/* extra sanity check */
		if ( !cmd_len ) {
#ifdef APPLEMIDI_DEBUG
Serial.print ("sanity check failed");
#endif
			return -1;
		}

		byte octet = packetBuffer[offset];

		/* midi realtime-data -> one octet -- unlike serial-wired MIDI realtime-commands in RTP-MIDI will
		* not be intermingled with other MIDI-commands, so we handle this case right here and return */
		if ( octet >= 0xf8 ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println(F("RealTime"));
#endif

			switch (octet)
			{
			case RTP_MIDI_STATUS_COMMON_REALTIME_TIMING_CLOCK:
				rtpMidi->OnClock(NULL);
				break;
			case RTP_MIDI_STATUS_COMMON_REALTIME_START:
				rtpMidi->OnStart(NULL);
				break;
			case RTP_MIDI_STATUS_COMMON_REALTIME_CONTINUE:
				rtpMidi->OnContinue(NULL);
				break;
			case RTP_MIDI_STATUS_COMMON_REALTIME_STOP:
				rtpMidi->OnStop(NULL);
				break;
			case RTP_MIDI_STATUS_COMMON_REALTIME_ACTIVE_SENSING:
				rtpMidi->OnActiveSensing(NULL);
				break;
			case RTP_MIDI_STATUS_COMMON_REALTIME_SYSTEM_RESET:
				rtpMidi->OnReset(NULL);
				break;
			}

			return 1;
		}

		/* see if this first octet is a status message */
		if ( ( octet & RTP_MIDI_COMMAND_STATUS_FLAG ) == 0 ) {
			/* if we have no running status yet -> error */
			if ( ( ( *runningstatus ) & RTP_MIDI_COMMAND_STATUS_FLAG ) == 0 ) {
				return -1;
			}
			/* our first octet is "virtual" coming from a preceding MIDI-command,
			* so actually we have not really consumed anything yet */
			octet = *runningstatus;
			using_rs = true;
		} else {

			/* We have a "real" status-byte */
			using_rs = false;

			/* Let's see how this octet influences our running-status */
			/* if we have a "normal" MIDI-command then the new status replaces the current running-status */
			if ( octet < 0xf0 ) {
				*rsoffset = offset ;
				*runningstatus = octet;
			}
			else {
				/* system-realtime-commands maintain the current running-status
				* other system-commands clear the running-status, since we
				* already handled realtime, we can reset it here */
				*runningstatus = 0;
			}

			/* lets update our pointers */
			consumed++;
			cmd_len--;
			offset++;
		}

		/* non-system MIDI-commands encode the command in the high nibble and the channel
		* in the low nibble - so we will take care of those cases next */

		if (octet < 0xf0) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
			Serial.print("MIDI command ");
			Serial.println(octet >> 4);
#endif
			switch (octet >> 4) {
			case RTP_MIDI_STATUS_CHANNEL_NOTE_OFF:
				ext_consumed = decode_note_off(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			case RTP_MIDI_STATUS_CHANNEL_NOTE_ON:
				ext_consumed = decode_note_on(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			case RTP_MIDI_STATUS_CHANNEL_POLYPHONIC_KEY_PRESSURE:
				ext_consumed = decode_poly_pressure(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			case RTP_MIDI_STATUS_CHANNEL_CONTROL_CHANGE:
				ext_consumed = decode_control_change(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			case RTP_MIDI_STATUS_CHANNEL_PROGRAM_CHANGE:
				ext_consumed = decode_program_change(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			case RTP_MIDI_STATUS_CHANNEL_CHANNEL_PRESSURE:
				ext_consumed = decode_channel_pressure(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			case RTP_MIDI_STATUS_CHANNEL_PITCH_BEND_CHANGE:
				ext_consumed = decode_pitch_bend_change(rtpMidi, packetBuffer, cmd_count, offset, cmd_len, octet, *rsoffset, using_rs);
				break;
			default:
				ext_consumed = -1;
#ifdef APPLEMIDI_DEBUG
				Serial.println("Unknown midi command octet");
#endif
			}

			/* external decoder informed us of error -> pass this through */
			if (ext_consumed < 0) {
#ifdef APPLEMIDI_DEBUG
				Serial.print("Midi Consumed < 0. ");
				Serial.println(ext_consumed);
#endif
				return ext_consumed;
			}

			return consumed + ext_consumed;
		}

		/* Here we catch the remaining system-common commands */
		switch ( octet ) {
		case RTP_MIDI_STATUS_COMMON_SYSEX_START:
			ext_consumed = decode_sysex_start(rtpMidi, packetBuffer, cmd_count, offset, cmd_len );
			break;
		case RTP_MIDI_STATUS_COMMON_MTC_QUARTER_FRAME:
			ext_consumed = decode_mtc_quarter_frame(rtpMidi, packetBuffer, cmd_count, offset, cmd_len );
			break;
		case RTP_MIDI_STATUS_COMMON_SONG_POSITION_POINTER:
			ext_consumed = decode_song_position_pointer(rtpMidi, packetBuffer, cmd_count, offset, cmd_len );
			break;
		case RTP_MIDI_STATUS_COMMON_SONG_SELECT:
			ext_consumed = decode_song_select(rtpMidi, packetBuffer, cmd_count, offset, cmd_len );
			break;
		case RTP_MIDI_STATUS_COMMON_TUNE_REQUEST:
			ext_consumed = decode_tune_request(rtpMidi, packetBuffer, cmd_count, offset, cmd_len );
			break;
		case RTP_MIDI_STATUS_COMMON_SYSEX_END:
			ext_consumed = decode_sysex_end(rtpMidi, packetBuffer, cmd_count, offset, cmd_len);
			break;		
		default:
			ext_consumed = -1;
#ifdef APPLEMIDI_DEBUG
			Serial.println("Unknown system command octet");
#endif
			break;
		}

		/* external decoder informed us of error -> pass this through */
		if ( ext_consumed < 0 ) {
#ifdef APPLEMIDI_DEBUG
Serial.print("System Consumed < 0. ");
Serial.println(ext_consumed);
#endif

			return ext_consumed;
		}

		return consumed + ext_consumed;
	}

	/*
	* This decodes the delta-time before a MIDI-command
	*/
	static int
	decodetime(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset, unsigned int cmd_len)
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decodetime");
#endif

		unsigned int consumed = 0;

		/* RTP-MIDI deltatime is "compressed" using only the necessary amount of octets */
		for (int i = 0; i < 4; i++ ) {
			if (!cmd_len) {
				return -1;
			}

			uint8_t octet = packetBuffer[offset + consumed];
			unsigned long deltatime = ( deltatime << 7 ) | ( octet & RTP_MIDI_DELTA_TIME_OCTET_MASK );
			consumed++;

			if ( ( octet & RTP_MIDI_DELTA_TIME_EXTENSION ) == 0 ) {
				break;
			}
		}

		return consumed;
	}

	/*
	* Here a Note-Off command is decoded.
	*/
	static int
	decode_note_off(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte status, unsigned int rsoffset, bool using_rs ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_note_off");
#endif

		//status_str = val_to_str( status >> 4, rtp_midi_channel_status, rtp_midi_unknown_value_hex );

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t note = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( note & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command: note_off");
#endif
			return 1;
		}

		/* broken: we have only one further octet */
		if ( cmd_len < 2 ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("not enough q");
#endif
			return 1;
		}

		uint8_t velocity = packetBuffer[offset + 1];

		/* seems to be an aborted MIDI-command */
		if ( velocity & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command 2: note_off");
#endif

			// Aborted NoteOff packets might be trying to turn the note off.
			// We should do this manually just in case
			rtpMidi->OnNoteOff(NULL, channel, note, 0);

			return 1;
		}


		if ( using_rs ) {
		} else {
		}

		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnNoteOff(NULL, channel, note, velocity);

		return 2;
	}

	/*
	* Here a Note-On command is decoded.
	*/
	static int
	decode_note_on(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, uint8_t status, unsigned int rsoffset, bool using_rs )
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_note_on");
#endif

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this should never happen! */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("!cmd_len");
#endif
			return -1;
		}

		uint8_t note = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( note & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this should never happethis should never happen */
			if ( using_rs ) {
			} else {
			}
			return -1;
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command: note_on");
#endif
		}

		/* broken: we have only one further octet */
		if ( cmd_len < 2 ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("not enough w");
#endif
			return -1;
		}

		uint8_t velocity = packetBuffer[offset + 1];

		/* seems to be an aborted MIDI-command */
		if ( velocity & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command 2: note_on");
#endif
			// Aborted NoteOn packets might be trying to turn the note off.
			// We should do this manually just in case
			// rtpMidi->OnNoteOff(NULL, channel, note, 0);

			return 0;
		}

		if (rtpMidi->PassesFilter(NULL, type, channel)) {
			/* special case velocity=0 for Note-On means Note-Off (to preserve running-status!) */
			if (velocity == 0) {
				rtpMidi->OnNoteOff(NULL, channel, note, velocity);
			} else {
				rtpMidi->OnNoteOn(NULL, channel, note, velocity);
			}
		}

		return 2;
	}

	/*
	* Here polyphonic aftertouch is decoded.
	*/
	static int
	decode_poly_pressure(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte status, unsigned int rsoffset, bool using_rs ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_poly_pressure");
#endif

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t note = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( note & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command: poly_pressure");
#endif
			return 1;
		}

		/* broken: we have only one further octet */
		if ( cmd_len < 2 ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("not enough e");
#endif
			return cmd_len;
		}

		uint8_t pressure = packetBuffer[offset + 1];

		/* seems to be an aborted MIDI-command */
		if ( pressure  & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command: poly_pressure");
#endif
			return 1;
		}


		if ( using_rs ) {
		} else {
		}

		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnPolyPressure(NULL, channel, note, pressure);

		return 2;
	}

	/*
	* Here channel aftertouch is decoded.
	*/
	static int
	decode_channel_pressure(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte status, unsigned int rsoffset, bool using_rs ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_channel_pressure");
#endif

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t pressure = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( pressure & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("aborted MIDI-command: channel_pressure");
#endif
			return 1;
		}


		if ( using_rs ) {
		} else {
		}

		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnChannelPressure(NULL, channel, pressure);

		return 1;
	}

	/*
	* Here pitch-bend is decoded.
	*/
	static int
	decode_pitch_bend_change(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte status, unsigned int rsoffset, bool using_rs ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_pitch_bend_change");
#endif

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t octet1 = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( octet1 & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("aborted MIDI-command: pitch_bend_change");
#endif
			return cmd_len;
		}

		/* broken: we have only one further octet */
		if ( cmd_len < 2 ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("not enough r");
#endif
			return cmd_len;
		}

		uint8_t octet2 = packetBuffer[offset + 1];

		/* seems to be an aborted MIDI-command */
		if ( octet2 & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command 2: pitch_bend_change");
#endif
			return cmd_len;
		}

		int pitch = ( octet1 << 7 ) | octet2;

		if ( using_rs ) {
		} else {
		}

		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnPitchBendChange(NULL, channel, pitch);

		return 2;

	}

	/*
	* Here program_change is decoded.
	*/
	static int
	decode_program_change(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte status, unsigned int rsoffset, bool using_rs ) {

#ifdef APPLEMIDI_DEBUG
Serial.println("decode_program_change");
#endif

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t program = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( program & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command: program_change");
#endif
			return cmd_len;
		}


		if ( using_rs ) {
		} else {
		}

		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnProgramChange(NULL, channel, program);


		return 1;
	}

	/*
	* Here control change is decoded.
	*/
	static int
	decode_control_change(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len, byte status, unsigned int rsoffset, bool using_rs ) {

#ifdef APPLEMIDI_DEBUG
Serial.println("decode_control_change");
#endif

		uint8_t type    = (status >> 4);
		uint8_t channel = (status & RTP_MIDI_CHANNEL_MASK) + 1;

		/* broken: we have no further data */
		if ( !cmd_len ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t controller = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( controller & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command: control_change");
#endif
			return cmd_len;
		}

		/* broken: we have only one further octet */
		if ( cmd_len < 2 ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("not enough t");
#endif
			return cmd_len;
		}

		uint8_t value = packetBuffer[offset + 1];

		/* seems to be an aborted MIDI-command */
		if ( value  & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			if ( using_rs ) {
			} else {
			}
#ifdef APPLEMIDI_DEBUG
Serial.println("aborted MIDI-command 2: control_change");
#endif
			return cmd_len;
		}


		if ( using_rs ) {
		} else {
		}

		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnControlChange(NULL, channel, controller, value);

		return 2;

	}

	/*
	* Here a Sysex-Start command is decoded.
	*/
	static unsigned int
	decode_sysex_start(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("decode_sysex_start");
#endif

		uint16_t consumed = 0;

		/* we need to parse "away" data until the next command */
		while (cmd_len) {

			uint8_t octet = packetBuffer[offset + consumed];
			consumed++;

			/*
			 * lets check if we have a "normal" end -
			 * if so, the last status-byte is consumed
			 * as it belongs to the message
			 */
			if (octet == RTP_MIDI_STATUS_COMMON_SYSEX_END) {
				rtpMidi->OnSysEx(NULL, &packetBuffer[offset - 1], consumed + 1); // Complete message
				break;
			}
			else if (octet == RTP_MIDI_STATUS_COMMON_SYSEX_START) {
				rtpMidi->OnSysEx(NULL, &packetBuffer[offset - 1], consumed); // Start
				break;
			}
			else if (octet == RTP_MIDI_STATUS_COMMON_UNDEFINED_F4) {
				//	break;
			}

			/* Is this command through? */
			if ( octet & RTP_MIDI_COMMAND_STATUS_FLAG ) {
				break;
			}
		}

		return consumed;
	}

	/*
	* Here the MIDI-Time-Code (MTC) Quarter Frame command is decoded.
	*/
	static int
	decode_mtc_quarter_frame(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_mtc_quarter_frame");
#endif

		/* broken: we have no further data */
		if ( !cmd_len ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("no further data");
#endif
		return -1;
		}

		byte value = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( value & RTP_MIDI_COMMAND_STATUS_FLAG ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("aborted MIDI-command: mtc_quarter_frame");
#endif
			return -1;
		}

//		if (rtpMidi->PassesFilter(NULL, type, channel))
			rtpMidi->OnTimeCodeQuarterFrame(NULL, value);

		return 1;
	}

	/*
	* Here the Song Position Pointer command is decoded.
	*/
	static int
	decode_song_position_pointer(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_song_position_pointer");
#endif

		/* broken: we have no further data */
		if ( !cmd_len ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t octet1 = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( octet1 & RTP_MIDI_COMMAND_STATUS_FLAG ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("aborted MIDI-command: song_position_pointer");
#endif
			return -1;
		}

		/* broken: we have only one further octet */
		if ( cmd_len < 2 ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("not enough y");
#endif
			return -1;
		}

		uint8_t octet2 = packetBuffer[offset + 1];

		/* seems to be an aborted MIDI-command */
		if ( octet2 & RTP_MIDI_COMMAND_STATUS_FLAG ) {
			/* this case should never happen */
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("aborted MIDI-command 2: song_position_pointer");
#endif
			return -1;
		}

		unsigned short position = ( octet1 << 7 ) | octet2;

		rtpMidi->OnSongPosition(NULL, position);

		return 2;
	}

	/*
	* Here a Song-Select command is decoded.
	*/
	static int
	decode_song_select(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("decode_song_select");
#endif

		/* broken: we have no further data */
		if ( !cmd_len ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("no further data");
#endif
			return -1;
		}

		uint8_t song_nr = packetBuffer[offset];

		/* seems to be an aborted MIDI-command */
		if ( song_nr & RTP_MIDI_COMMAND_STATUS_FLAG ) {
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("aborted MIDI-command: decode_song_select");
#endif
			return -1;
		}

		rtpMidi->OnSongSelect(NULL, song_nr);

		return 1;
	}

	/*
	* Here a Tune-Request command is decoded.
	*/
	static int
	decode_tune_request(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("decode_tune_request");
#endif

		rtpMidi->OnTuneRequest(NULL);

		return 0;
	}

	/*
	* Here a Sysex-End command is decoded - in RTP-MIDI this has a special semantic, it either starts a segmented Sysex-frame or a Sysex-Cancel
	*/
	static int
	decode_sysex_end(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int cmd_count, unsigned int offset, unsigned int cmd_len ) {

#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.println("decode_sysex_end");
#endif

		int consumed = 0;

		/* we need to parse "away" data until the next command */
		while ( cmd_len ) {

			uint8_t octet = packetBuffer[offset + consumed];
			consumed++;

			/*
			 * lets check if we have a "normal" end -
			 * if so, the last status-byte is consumed
			 * as it belongs to the message
			 */
			if ( octet == RTP_MIDI_STATUS_COMMON_SYSEX_END ) {
				rtpMidi->OnSysEx(NULL, &packetBuffer[offset], consumed); // End
				break;
			} else if ( octet == RTP_MIDI_STATUS_COMMON_SYSEX_START ) {
				rtpMidi->OnSysEx(NULL, &packetBuffer[offset], consumed - 1); // Middle
				break;
			}

			/* Is this command through? */
			if ( octet & RTP_MIDI_COMMAND_STATUS_FLAG ) {
				break;
			}
		}

		return consumed;
	}





	/*
	 * Here the chapter Q of the channel-journal is decoded.
	 */
	static int
	decode_sj_chapter_q(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset ) {
		uint8_t				header;
		unsigned int start_offset = offset;
		int				len = 1;

		/* first we need to get the flags of this chapter */
		header = packetBuffer[offset];

		if ( header & RTP_MIDI_SJ_CHAPTER_Q_FLAG_C ) {
			len += 2;
		}
		if ( header & RTP_MIDI_SJ_CHAPTER_Q_FLAG_T ) {
			len += 3;
		}

		if ( header & RTP_MIDI_SJ_CHAPTER_Q_FLAG_C ) {
			offset	 += 3;
		} else {
			offset++;
		}

		if ( header & RTP_MIDI_SJ_CHAPTER_Q_FLAG_T ) {
			offset += 3;
		}

		return offset-start_offset;
	}

	/*
	 * Here the chapter F of the channel-journal is decoded.
	 */
	static int
	decode_sj_chapter_f(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		uint8_t				header;
		unsigned int start_offset = offset;
		int				len = 1;

		/* first we need to get the flags of this chapter */
		header = packetBuffer[offset];

		if ( header & RTP_MIDI_SJ_CHAPTER_F_FLAG_C ) {
			len += 4;
		}
		if ( header & RTP_MIDI_SJ_CHAPTER_F_FLAG_P ) {
			len += 4;
		}

		offset++;

		if ( header & RTP_MIDI_SJ_CHAPTER_F_FLAG_C ) {
			offset	 += 4;
		}

		if ( header & RTP_MIDI_SJ_CHAPTER_F_FLAG_P ) {
			offset += 4;
		}

		return offset-start_offset;
	}

	/*
	 * Here the chapter X of the channel-journal is decoded.
	 */
	static int
	decode_sj_chapter_x(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset, unsigned int max_length) {
		uint8_t				header;
		uint8_t				octet;
		unsigned int			consumed = 0;
		unsigned int			cmdlen   = 0;
		unsigned int			i;

		/* first we need to get the flags of this chapter */
		header = packetBuffer[offset];

		consumed++;
		offset++;

		if ( header & RTP_MIDI_SJ_CHAPTER_X_FLAG_T ) {
			consumed++;
			offset++;
		}

		if ( header & RTP_MIDI_SJ_CHAPTER_X_FLAG_C ) {
			consumed++;
			offset++;
		}

		if ( header & RTP_MIDI_SJ_CHAPTER_X_FLAG_F ) {
			unsigned int field    = 0;
			unsigned int fieldlen = 0;

			/* FIRST is "compressed" using only the necessary amount of octets, like delta-time */
			for ( i=0; i < 4; i++ ) {
				/* do we still fit in the dissected packet & in the length restriction of this chapter? */
				if ( ( !( consumed >= max_length ) ) || ( !packetBuffer[offset + fieldlen + 1] ) ) {
					return -1;
				}

				octet = packetBuffer[offset + fieldlen];
				field = ( field << 7 ) | ( octet & RTP_MIDI_DELTA_TIME_OCTET_MASK );
				fieldlen++;

				if ( ( octet & RTP_MIDI_DELTA_TIME_EXTENSION ) == 0 ) {
					break;
				}
			}

			consumed += fieldlen;
			offset	 += fieldlen;
		}

		/* XXX: 'cmdlen' in the following is always 0 (since initialized to 0 above) ??? */
		if ( header & RTP_MIDI_SJ_CHAPTER_X_FLAG_D ) {
			while ( consumed < max_length ) {
				octet = packetBuffer[offset + cmdlen];
				if ( octet & 0x80 ) {
					offset += cmdlen;
					cmdlen	= 0;
				}
				consumed += 1;
			}
			/* unfinished command still to put into tree */
			if ( cmdlen ) {
				offset += cmdlen;
			}
		}

		/* this should not ever enter - we still have data, but flag d was apparently not set...  */
		if ( consumed < max_length ) {
			consumed = max_length;
		}

		return consumed;
	}

   /*
    * Here the chapter D of the channel-journal is decoded.
    */
   static int
	decode_sj_chapter_d(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
      int				header;
      unsigned int start_offset = offset;
      int				ext_consumed;

      /* first we need to get the flags of this chapter */
      header = packetBuffer[offset];

      /* done with header */
      offset++;

      /* do we have Reset field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_B ) {
         offset++;
      }

      /* do we have Tune request field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_G ) {
         offset++;
      }

      /* do we have Song select field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_H ) {
         offset++;
      }

      /* do we have 0xF4 field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_J ) {
         ext_consumed = decode_sj_chapter_d_f4(rtpMidi, packetBuffer, offset);
         if ( ext_consumed < 0 ) {
            return ext_consumed;
         }
         offset	 += ext_consumed;
      }

      /* do we have 0xF5 field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_K ) {
         ext_consumed = decode_sj_chapter_d_f5(rtpMidi, packetBuffer, offset);
         if ( ext_consumed < 0 ) {
            return ext_consumed;
         }
         offset	 += ext_consumed;
      }

      /* do we have 0xF9 field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_Y ) {
         ext_consumed = decode_sj_chapter_d_f9(rtpMidi, packetBuffer, offset);
         if ( ext_consumed < 0 ) {
            return ext_consumed;
         }
         offset	 += ext_consumed;
      }

      /* do we have 0xFD field? */
      if ( header & RTP_MIDI_SJ_CHAPTER_D_FLAG_Z ) {
         ext_consumed = decode_sj_chapter_d_fd(rtpMidi, packetBuffer, offset);
         if ( ext_consumed < 0 ) {
            return ext_consumed;
         }
         offset += ext_consumed;
      }

      /* now we know the complete length and set it. */
      return offset-start_offset;
   }

	/*
	* Here the chapter D F4-field of the system-journal is decoded.
	*/
	static int
	decode_sj_chapter_d_f4(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
      int start_offset = offset;
      uint16_t		 f4flags;
      uint16_t		 f4length;

      /* Get flags & length */
      f4flags = packetBuffer[offset];
      f4length = f4flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_MASK_LENGTH;

      offset	 += 2;
      f4length -= 2;

      if ( f4flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_C ) {
         offset++;
         f4length--;
      }

      if ( f4flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_V ) {
         int valuelen = 0;
         uint8_t octet;

         /* variable length field - ends with an octet with MSB set */
         for (;;) {
            octet = packetBuffer[offset+valuelen];
            valuelen++;
            if ( octet & 0x80 ) {
               break;
            }
         }
         offset	 += valuelen;
         f4length -= valuelen;
      }

      if ( f4flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_L ) {
         offset	 += f4length;
      }

      /* if we still have data, the length-field was incorrect we dump the data here and abort! */
      if ( f4length > 0 ) {
         offset += f4length;
         /* must be a protocol error - since we have a length, we can recover...*/
      }

      return offset-start_offset;
   }

	static int
	decode_sj_chapter_d_f5(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
	unsigned int start_offset = offset;
	uint16_t		 f5flags;
	uint16_t		 f5length;

	/* Get flags & length */
	f5flags = packetBuffer[offset];
	f5length = f5flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_MASK_LENGTH;

	offset	 += 2;
	f5length -= 2;

	if ( f5flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_C ) {
		offset++;
		f5length--;
	}

	if ( f5flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_V ) {

		int valuelen = 0;
		uint8_t octet;

		/* variable length field - ends with an octet with MSB set */
		for (;;) {
			octet = packetBuffer[offset+valuelen];
			valuelen++;
			if ( octet & 0x80 ) {
				break;
			}
		}

		offset	 += valuelen;
		f5length -= valuelen;
	}

	if ( f5flags & RTP_MIDI_SJ_CHAPTER_D_SYSCOM_FLAG_L ) {
		offset	 += f5length;
		f5length = 0;
	}

	/* if we still have data, we dump it here - see above! */
	if ( f5length > 0 ) {
		offset += f5length;
		/* must be a protocol error - since we have a length, we can recover...*/
	}

	return offset-start_offset;
}

	/*
	 * Here the chapter D F9-field of the system-journal is decoded.
	 */
	static int
	decode_sj_chapter_d_f9(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		unsigned int start_offset = offset;
		uint8_t		 f9flags;
		uint8_t		 f9length;

		/* Get flags & length */
		f9flags = packetBuffer[offset];
		f9length = f9flags & RTP_MIDI_SJ_CHAPTER_D_SYSREAL_MASK_LENGTH;

		offset++;
		f9length--;

		if ( f9flags & RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_C ) {
			offset++;
			f9length--;
		}

		if ( f9flags & RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_L ) {
			offset	 += f9length;
			f9length = 0;
		}

		/* if we still have data, the length-field was incorrect we dump the data here and abort! */

		if ( f9length > 0 ) {
			offset += f9length;
			/* must be a protocol error - since we have a length, we can recover...*/
		}

		return offset-start_offset;
	}

	/*
	 * Here the chapter D FD-field of the system-journal is decoded.
	 */
	static int
	decode_sj_chapter_d_fd(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		unsigned int start_offset = offset;
		uint8_t		 fdflags;
		uint8_t		 fdlength;

		/* Get flags & length */
		fdflags  = packetBuffer[offset];
		fdlength = fdflags & RTP_MIDI_SJ_CHAPTER_D_SYSREAL_MASK_LENGTH;

		offset++;
		fdlength--;

		if ( fdflags & RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_C ) {
			offset++;
			fdlength--;
		}

		if ( fdflags & RTP_MIDI_SJ_CHAPTER_D_SYSREAL_FLAG_L ) {
			offset	 += fdlength;
			fdlength = 0;
		}

		/* if we still have data, the length-field was incorrect we dump the data here and abort! */

		if ( fdlength > 0 ) {
			offset += fdlength;
			/* must be a protocol error - since we have a length, we can recover...*/
		}

		return offset-start_offset;
	}

	/*
	 * Here the chapter c of the channel-journal is decoded.
	 */
	static int
	decode_cj_chapter_c(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		unsigned int start_offset = offset;
		uint8_t				 octet;
		int				 count;
		int				 i;

		octet = packetBuffer[offset];
		count = octet & 0x7f;

		/* count encoded is n+1 */
		count++;

		offset++;

		for ( i = 0; i < count; i++ ) {
			offset++;
			octet = packetBuffer[offset];
			offset++;
		}

		return offset-start_offset;
	}

	/*
	 * Here the chapter m of the channel-journal is decoded, possibly the most complex part of the RTP-MIDI stuff ;-)
	 */
	static int
	decode_cj_chapter_m(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		uint16_t				header;
		uint8_t				logitemheader = 0;
		int				length;
		int				logitemhdrlen;
		int				logitemlen;
		boolean			no_pnum_msb;
		unsigned int start_offset = offset;

		/* first we need to get the flags & length of this chapter */
		header = packetBuffer[offset];
		length = header & RTP_MIDI_CJ_CHAPTER_M_MASK_LENGTH;
		/* take of length of header */
		length -= 2;

		/* done with header */
		offset	 += 2;

		/* do we have the pending field? */
		if ( header & 0x4000 ) {
			offset++;
		}

		/*
		 * lets find out if we need to decode the pnum_msb:
		 * if Z = 1 and either U = 1 or W = 1 we don't
		 */
		no_pnum_msb = ( header & 0x0400 ) && ( ( header & 0x0800 ) || ( header & 0x1000 ) );
		logitemhdrlen = no_pnum_msb ? 2 : 3;

		/* lets step through the loglist */
		while ( length > 0 ) {
			if ( no_pnum_msb ) {
				logitemheader = packetBuffer[offset + 1];
			} else {
				logitemheader = packetBuffer[offset + 2];
			}

			logitemlen = logitemhdrlen;

			/* do we have a msb field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_J ) {
				logitemlen++;
			}

			/* do we have a lsb field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_K ) {
				logitemlen++;
			}

			/* do we have an a-button field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_L ) {
				logitemlen +=2;
			}

			/* do we have a c-button field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_M ) {
				logitemlen +=2;
			}

			/* do we have a count field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_N ) {
				logitemlen++;
			}

			offset++;
			length--;

			if ( !no_pnum_msb ) {
				offset++;
				length--;
			}

			offset++;
			length--;

			/* do we have a entry-msb field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_J ) {
				offset++;
				length--;
			}

			/* do we have a entry-lsb field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_K ) {
				offset++;
				length--;
			}

			/* do we have an a-button field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_L ) {
				offset	 += 2;
				length	 -= 2;
			}

			/* do we have a c-button field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_M ) {
				offset	 += 2;
				length	 -= 2;
			}

			/* do we have a count field? */
			if ( logitemheader & RTP_MIDI_CJ_CHAPTER_M_FLAG_N ) {
				offset++;
				length--;
			}
		}

		return offset-start_offset;
	}


	/*
	 * Here the chapter n of the channel-journal is decoded.
	 */
	static int
	decode_cj_chapter_n(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		unsigned int start_offset = offset;
		uint16_t				 header;
		//uint8_t				 note;
		//uint8_t				 velocity;
		int				 log_count;
		int				 octet_count;
		int				 low;
		int				 high;
		int				 i;

		/* first we need to get the flags & length of this chapter */
		header = packetBuffer[offset];
		log_count = ( header & RTP_MIDI_CJ_CHAPTER_N_MASK_LENGTH ) >> 8;
		low = ( header & RTP_MIDI_CJ_CHAPTER_N_MASK_LOW ) >> 4;
		high = header & RTP_MIDI_CJ_CHAPTER_N_MASK_HIGH;

		/* how many offbits octets do we have? */
		if ( low <= high ) {
			octet_count = high - low + 1;
		} else if ( ( low == 15 ) && ( high == 0 ) ) {
			octet_count = 0;
		} else if ( ( low == 15 ) && ( high == 1 ) ) {
			octet_count = 0;
		} else {
			return -1;
		}

		/* special case -> no offbit octets, but 128 note-logs */
		if ( ( log_count == 127 ) && ( low == 15) && ( high == 0 ) ) {
			log_count++;
		}

		offset	 += 2;

		if ( log_count > 0 ) {
			for ( i = 0; i < log_count; i++ ) {
				offset++;
				offset++;
			}
		}

		if ( octet_count > 0 ) {
			for ( i = 0; i < octet_count; i++ ) {
				offset++;
			}
		}

		return offset-start_offset;
	}


	/*
	 * Here the chapter e of the channel-journal is decoded.
	 */
	static int
	decode_cj_chapter_e(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		unsigned int start_offset = offset;
		uint8_t				 header;
		int				 log_count;
		int				 i;

		/* first we need to get the flags & length of this chapter */
		header = packetBuffer[offset];
		log_count = header & RTP_MIDI_CJ_CHAPTER_E_MASK_LENGTH;

		/* count is encoded n+1 */
		log_count++;
		offset++;

		for ( i = 0; i < log_count; i++ ) {
			offset++;
			offset++;
		}

		return offset-start_offset;
	}

	/*
	 * Here the chapter a of the channel-journal is decoded.
	 */
	static int
	decode_cj_chapter_a(IAppleMidi* rtpMidi, unsigned char* packetBuffer, unsigned int offset) {
		unsigned int start_offset = offset;
		uint8_t				 header;
		int				 log_count;
		int				 i;

		/* first we need to get the flags & length of this chapter */
		header = packetBuffer[offset];
		log_count = header & RTP_MIDI_CJ_CHAPTER_A_MASK_LENGTH;

		/* count is encoded n+1 */
		log_count++;

		offset++;

		for ( i = 0; i < log_count; i++ ) {
			offset++;
			offset++;
		}

		return offset-start_offset;
	}

};

END_APPLEMIDI_NAMESPACE
