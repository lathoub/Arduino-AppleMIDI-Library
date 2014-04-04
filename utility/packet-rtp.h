/*!
 *  @file		packet-rtp.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		GPL
 */

#pragma once

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



/* from https://github.com/sjaeckel/wireshark/blob/master/epan/dissectors/packet-rtp.h */
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

	static int dissect_rtp(Dissector* dissector, AppleMidi_Class* rtpMidi, unsigned char* packetBuffer, size_t packetSize)
	{
		size_t offset = 0;

		// We need a minimum of 12 bytes
		if (packetSize < 12)
			return 0;

		/* Can tap up to 4 RTP packets within same packet */
		static struct _rtp_info rtp_info_arr[4];
		static int rtp_info_current = 0;
		struct _rtp_info *rtp_info;

		rtp_info_current++;
		if (rtp_info_current==4) {
			rtp_info_current=0;
		}
		rtp_info = &rtp_info_arr[rtp_info_current];

		/* Get the fields in the first octet */
		uint8_t octet1 = packetBuffer[offset];
		unsigned int version = RTP_VERSION(octet1);

		if (version == 0) {
			 /* Unknown or unsupported version (let it fall through) */
		}

		/* fill in the rtp_info structure */
		rtp_info->info_version = version;
		if (version != 2) {
			/*
			* Unknown or unsupported version.
			*/
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
		int length = 0;//tvb_length_remaining(tvb, offset);
		int reported_length = 0;//tvb_reported_length_remaining(tvb, offset);
		if (reported_length >= 0 && length >= reported_length) {
			/*
			* Yes.
			*/
			rtp_info->info_all_data_present = true;
			rtp_info->info_data_len = reported_length;

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
		}

		/* if it is dynamic payload, let use the conv data to see if it is defined */
		if ( (payload_type > 95) && (payload_type < 128) ) {
			// only 97 is supported
			//rtp_info->info_payload_type_str = "something";
			//rtp_info->info_payload_rate = encoding_name_and_rate_pt->sample_rate;
		}

		if (true) {
			
			// header
			offset++;

			// Payload type
			offset++;

			/* Sequence number 16 bits (2 octets) */
			//proto_tree_add_uint( rtp_tree, hf_rtp_seq_nr, tvb, offset, 2, seq_num );
			//if(p_conv_data != NULL) {
			//	item = proto_tree_add_uint( rtp_tree, hf_rtp_ext_seq_nr, tvb, offset, 2, p_conv_data->extended_seqno );
			//	PROTO_ITEM_SET_GENERATED(item);
			//}
			offset += 2;

			/* Timestamp 32 bits (4 octets) */
			//proto_tree_add_uint( rtp_tree, hf_rtp_timestamp, tvb, offset, 4, timestamp );
			offset += 4;

			/* Synchronization source identifier 32 bits (4 octets) */
			//proto_tree_add_uint( rtp_tree, hf_rtp_ssrc, tvb, offset, 4, sync_src );
			offset += 4;
		} 
		else 
		{
			offset += 12;
		}

		/* CSRC list*/
		if ( csrc_count > 0 ) {

			Serial.print  ("csrc_count = ");
			Serial.println(csrc_count);

			// Not supported
			if (true) {
			}
			for (int i = 0; i < csrc_count; i++ ) {
				uint32_t csrc_item = AppleMIDI_Util::readUInt32(packetBuffer + offset);
				offset += 4;
			}
		}

		if ( extension_set ) {

			Serial.print  ("extension ");

			// not supported
			/* Defined by profile field is 16 bits (2 octets) */
			int hdr_extension_id = AppleMIDI_Util::readUInt32(packetBuffer + offset);
			offset += 2;

			Serial.print  ("hdr_extension_id = ");
			Serial.print  (hdr_extension_id);

			int hdr_extension = AppleMIDI_Util::readUInt32(packetBuffer + offset);
			offset += 2;

			Serial.print  ("hdr_extension = ");
			Serial.println(hdr_extension);

			if ( hdr_extension > 0 ) {
				/* pass interpretation of header extension to a registered subdissector */

				/* 0xBEDE is defined by RFC 5215 as a header
				* extension with a one byte header
				*/
				if (hdr_extension_id == 0xBEDE) {
					//dissect_rtp_hext_rfc5215_onebyte (newtvb, pinfo, rtp_hext_tree);
				}
				else if ((hdr_extension_id & 0xFFF0) == 0x1000) {
					//dissect_rtp_hext_rfc5215_twobytes(tvb, offset - 4, hdr_extension_id, newtvb, pinfo, rtp_hext_tree);
				}
				else {
					int hdrext_offset = offset;
					for (int i = 0; i < hdr_extension; i++ ) {
						hdrext_offset += 4;
					}
				}
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
			Serial.println("padding = ");
	//		Serial.print  (hdr_extension_id);
			
			// not supported

			int padding_count = 0;
			int data_len = 0;

			if (data_len > 0) {
				offset += data_len;
			} else if (data_len < 0) {
			}
			if (padding_count > 1) {
			}
		}
		else
		{
			/*
			* No padding.
			*/
//			int consumed = dissect_rtp_data(dissector, appleMidi, packetBuffer, offset, 0, packetSize, payload_type );

			rtp_info->info_payload_offset = offset;
//			rtp_info->info_payload_len = tvb_length_remaining(tvb, offset);

			return offset;
		}

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
