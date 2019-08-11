/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include "AppleMidi_Settings.h"
#include "dissector.h"

BEGIN_APPLEMIDI_NAMESPACE

/*
* Fields in the first octet of the RTP header.
*/

/* Version is the first 2 bits of the first octet*/
#define RTP_VERSION(octet) ((octet) >> 6)

/* Padding is the third bit; No need to shift, because true is any value
other than 0! */
#define RTP_PADDING(octet) ((octet) & 0x20)

/* Extension bit is the fourth bit */
#define RTP_EXTENSION(octet) ((octet) & 0x10)

/* CSRC count is the last four bits */
#define RTP_CSRC_COUNT(octet) ((octet) & 0xF)

/*
* Fields in the second octet of the RTP header.
*/

/* Marker is the first bit of the second octet */
#define RTP_MARKER(octet) ((octet) & 0x80)

/* Payload type is the last 7 bits */
#define RTP_PAYLOAD_TYPE(octet) ((octet) & 0x7F)
/* http://www.iana.org/assignments/rtp-parameters */

/* from https://code.wireshark.org/review/gitweb?p=wireshark.git;a=blob;f=epan/dissectors/packet-rtp.h */
struct _rtp_info {
	unsigned int	info_version;
	bool			info_padding_set;
	bool			info_marker_set;
	bool			info_is_video;
	unsigned int	info_payload_type;
	unsigned int	info_padding_count;
	uint16_t		info_seq_num;
	uint32_t		info_timestamp;
	uint32_t		info_sync_src;
	unsigned int	info_data_len; /* length of raw rtp data as reported */
	bool			info_all_data_present; /* FALSE if data is cut off */
	unsigned int	info_payload_offset; /* start of payload relative to info_data */
	unsigned int	info_payload_len; /* length of payload (incl padding) */
	bool			info_is_srtp;
	uint32_t		info_setup_frame_num; /* the frame num of the packet that set this RTP connection */
	const uint8_t*	info_data; /* pointer to raw rtp data */
	unsigned char*	info_payload_type_str;
	int				info_payload_rate;
	/*
	* info_data: pointer to raw rtp data = header + payload incl. padding.
	* That should be safe because the "epan_dissect_t" constructed for the packet
	* has not yet been freed when the taps are called.
	* (destroying the "epan_dissect_t" will end up freeing all the tvbuffs
	* and hence invalidating pointers to their data).
	* See "add_packet_to_packet_list()" for details.
	*/
};

class PacketRtp {
public:

	static int dissect_rtp(Dissector* dissector, IMidiCallbacks*, unsigned char* packetBuffer, size_t packetSize) {
		#ifdef APPLEMIDI_DEBUG_VERBOSE
		DEBUGSTREAM.println("start: dissect_rtp");
		#endif

		size_t offset = 0;

		// We need a minimum of 12 bytes
		if (packetSize < 12) {
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("not enough bytes. Need at least 12, got");
			DEBUGSTREAM.println(packetSize);
			#endif
			return 0;
		}

		/* Can tap up to 4 RTP packets within same packet */
		static struct _rtp_info rtp_info_arr[4];
		static int rtp_info_current = 0;
		struct _rtp_info *rtp_info;

		#ifdef APPLEMIDI_DEBUG_VERBOSE
		DEBUGSTREAM.print("current: ");
		DEBUGSTREAM.println(rtp_info_current);
		#endif

		rtp_info_current++;
		if (rtp_info_current == 4) {
			rtp_info_current=0;
		}
		rtp_info = &rtp_info_arr[rtp_info_current];

		/* Get the fields in the first octet */
		uint8_t octet1 = packetBuffer[offset];
		unsigned int version = RTP_VERSION(octet1);

		/* fill in the rtp_info structure */
		rtp_info->info_version = version;
		if (version != 2) {
			/*
			* Unknown or unsupported version.
			*/
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("unsupported version ");
			DEBUGSTREAM.println(version);
			#endif
            
			return 0;
		}

		bool padding_set = RTP_PADDING(octet1);
		bool extension_set = RTP_EXTENSION(octet1);
		unsigned int csrc_count = RTP_CSRC_COUNT(octet1);

		/* Get the fields in the second octet */
		byte octet2 = packetBuffer[offset + 1];
		bool marker_set = RTP_MARKER( octet2 );
		unsigned int payload_type = RTP_PAYLOAD_TYPE(octet2);

		/* Get the subsequent fields */
		uint16_t seq_num   = AppleMIDI_Util::readUInt16(packetBuffer + offset + 2);
		uint32_t timestamp = AppleMIDI_Util::readUInt32(packetBuffer + offset + 4);
		uint32_t sync_src  = AppleMIDI_Util::readUInt32(packetBuffer + offset + 8);

		#ifdef APPLEMIDI_DEBUG_VERBOSE
		DEBUGSTREAM.print  ("Seq Number: ");
		DEBUGSTREAM.println(seq_num, HEX);
		DEBUGSTREAM.print  ("Timestamp : ");
		DEBUGSTREAM.println(timestamp, HEX);
		DEBUGSTREAM.print("Sync Src  : ");
		DEBUGSTREAM.println(sync_src, HEX);
		#endif

		/* fill in the rtp_info structure */
		rtp_info->info_padding_set = padding_set;
		rtp_info->info_padding_count = 0;
		rtp_info->info_marker_set = marker_set;
		rtp_info->info_is_video = false;
		rtp_info->info_payload_type = payload_type;
		rtp_info->info_seq_num = seq_num;
		rtp_info->info_timestamp = timestamp;
		rtp_info->info_sync_src = sync_src;
		rtp_info->info_is_srtp = false;
		rtp_info->info_setup_frame_num = 0;
		rtp_info->info_payload_type_str = NULL;
		rtp_info->info_payload_rate = 0;

		/*
		* Do we have all the data?
		*/
		// TODO: Port length functionality from Wireshark
		int length = 0;//tvb_length_remaining(tvb, offset);
		int reported_length = 0;//tvb_reported_length_remaining(tvb, offset);
		if (reported_length >= 0 && length >= reported_length) {
			/*
			* Yes.
			*/
			rtp_info->info_all_data_present = true;
			rtp_info->info_data_len = reported_length;

			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.println("We have all the data");
			#endif

			/*
			* Save the pointer to raw rtp data (header + payload incl.
			* padding).
			* That should be safe because the "epan_dissect_t"
			* constructed for the packet has not yet been freed when
			* the taps are called.
			* (Destroying the "epan_dissect_t" will end up freeing
			* all the tvbuffs and hence invalidating pointers to
			* their data.)
			* See "add_packet_to_packet_list()" for details.
			*/
			//rtp_info->info_data = tvb_get_ptr(tvb, 0, -1);
		} else {
			/*
			* No - packet was cut short at capture time.
			*/
			rtp_info->info_all_data_present = false;
			rtp_info->info_data_len = 0;
			rtp_info->info_data = NULL;

			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.println("do not have all the data");
			#endif
		}

		/* if it is dynamic payload, let use the conv data to see if it is defined */
		if ( (payload_type > 95) && (payload_type < 128) ) {
			// only 97 is supported
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("payload_type is ");
			DEBUGSTREAM.println(payload_type);
			#endif
		}

		// Skip ahead
		offset += 12;

		/* CSRC list*/
		if ( csrc_count > 0 ) {
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("csrc_count = ");
			DEBUGSTREAM.println(csrc_count);
			#endif
			offset += csrc_count * 4;
		}

		if ( extension_set ) {
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("extension ");
			#endif
			// not supported
			/* Defined by profile field is 16 bits (2 octets) */
//			int hdr_extension_id = AppleMIDI_Util::readUInt32(packetBuffer + offset);
			offset += 2;

			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("hdr_extension_id = ");
			DEBUGSTREAM.println(hdr_extension_id);
			#endif

			int hdr_extension = AppleMIDI_Util::readUInt32(packetBuffer + offset);
			offset += 2;

			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print  ("hdr_extension = ");
			DEBUGSTREAM.println(hdr_extension);
			#endif

			if ( hdr_extension > 0 ) {
				offset += hdr_extension * 4;
			}
		}

		if ( padding_set ) {
			/*
			* This RTP frame has padding - find it.
			*
			* The padding count is found in the LAST octet of
			* the packet; it contains the number of octets
			* that can be ignored at the end of the packet.
			*/
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.println("padding set");
			#endif

			// not supported
//			int padding_count = 0;
			int data_len = 0;

			if (data_len > 0) {
				offset += data_len;
			}
		}
		else
		{
			/*
			* No padding.
			*/
			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.println("No padding set");
			#endif

			rtp_info->info_payload_offset = offset;

			#ifdef APPLEMIDI_DEBUG_VERBOSE
			DEBUGSTREAM.print("end: dissect_rtp, offset = ");
			DEBUGSTREAM.println(offset);
			#endif

			return offset;
		}

		#ifdef APPLEMIDI_DEBUG_VERBOSE
		DEBUGSTREAM.println("end: dissect_rtp (nothing processed)");
		#endif
		return 0;
	}

	/* calculate the extended sequence number - top 16 bits of the previous sequence number,
	* plus our own; then correct for wrapping */
	static uint32_t
	calculate_extended_seqno(uint32_t previous_seqno, uint16_t raw_seqno)
	{
		uint32_t seqno = (previous_seqno & 0xffff0000) | raw_seqno;
		if(seqno + 0x8000 < previous_seqno) {
			seqno += 0x10000;
		} else if(previous_seqno + 0x8000 < seqno) {
			/* we got an out-of-order packet which happened to go backwards over the
			* wrap boundary */
			seqno -= 0x10000;
		}
		return seqno;
	}
};

END_APPLEMIDI_NAMESPACE
