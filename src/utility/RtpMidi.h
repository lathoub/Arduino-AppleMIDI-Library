/*!
 *  @file		RtpMidi.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.0
 *  @author		lathoub
 *	@date		01/04/13
 *  License		GPL
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

#include "utility/AppleMidi_Util.h"

#define PAYLOADTYPE_RTPMIDI 97

BEGIN_APPLEMIDI_NAMESPACE

template<class UdpClass>
class RtpMidi {
public:
	uint8_t		ddddd;
	uint8_t		playLoadType;
	uint16_t	sequenceNr;
	uint32_t	timestamp;
	uint32_t	ssrc;

	RtpMidi()
	{
		ddddd = 0b10000000; // TODO

		// Payload types are 7-bit, so we add the Marker bit.
		// The Marker bit should be 0 only if the command length is 0. Otherwise 1.
		// Since all references to this have some length this is being hard coded.
		// NOTE: Although setting this would conform to the RFC, doing so seems to
		// cause an OSX receiver to ignore the commands.
		
		// playLoadType = PAYLOADTYPE_RTPMIDI | 128; // TODO

		playLoadType = PAYLOADTYPE_RTPMIDI;
	}

	void beginWrite(UdpClass& udp, IPAddress remoteIP, uint16_t remotePort)
	{
		udp.beginPacket(remoteIP, remotePort);

		_write(&udp);
	}

	void endWrite(UdpClass& udp)
	{
		udp.endPacket();
		udp.flush(); // Waits for the transmission of outgoing serial data to complete
	}

private:
	void _write(Stream* stream)
	{
		stream->write(&ddddd, sizeof(ddddd));
		stream->write(&playLoadType, sizeof(playLoadType));

		uint16_t _sequenceNr = AppleMIDI_Util::toEndian(sequenceNr);
		uint32_t _timestamp  = AppleMIDI_Util::toEndian(timestamp);
		uint32_t _ssrc       = AppleMIDI_Util::toEndian(ssrc);

		stream->write((uint8_t*) ((void*) (&_sequenceNr)), sizeof(_sequenceNr));
		stream->write((uint8_t*) ((void*) (&_timestamp)), sizeof(_timestamp));
		stream->write((uint8_t*) ((void*) (&_ssrc)), sizeof(_ssrc));
	}

};

END_APPLEMIDI_NAMESPACE
