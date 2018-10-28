/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
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
	uint8_t		vpxcc;
	uint8_t		playLoadType;
	uint16_t	sequenceNr;
	uint32_t	timestamp;
	uint32_t	ssrc;

	RtpMidi()
	{
		vpxcc = 0b10000000; // TODO

		// Payload types are 7-bit, so we add the Marker bit.
		// The Marker bit should be 0 only if the command length is 0. Otherwise 1.
		// Since all references to this have some length this is being hard coded.
		// NOTE: Although setting this would conform to the RFC, doing so seems to
		// cause an OSX receiver to ignore the commands.

		playLoadType = PAYLOADTYPE_RTPMIDI;
	}

	void beginWrite(UdpClass& udp, IPAddress remoteIP, uint16_t remotePort)
	{
		int success = udp.beginPacket(remoteIP, remotePort);
		Debug::Assert(success, "udp.beginPacket failed");

		_write(&udp);
	}

	void endWrite(UdpClass& udp)
	{
		int success = udp.endPacket();
		Debug::Assert(success, "udp.endPacket failed");
		udp.flush(); // Waits for the transmission of outgoing serial data to complete
	}

private:
	void _write(Stream* stream)
	{
		size_t bytesWritten = 0;

		bytesWritten = stream->write(&vpxcc, sizeof(vpxcc));
		Debug::Assert(bytesWritten == sizeof(vpxcc), "error writing vpxcc");
		bytesWritten = stream->write(&playLoadType, sizeof(playLoadType));
		Debug::Assert(bytesWritten == sizeof(playLoadType), "error writing playLoadType");

		uint16_t _sequenceNr = AppleMIDI_Util::toEndian(sequenceNr);

		/*
		https://developer.apple.com/library/ios/documentation/CoreMidi/Reference/MIDIServices_Reference/#//apple_ref/doc/uid/TP40010316-CHMIDIServiceshFunctions-SW30
		The time at which the events occurred, if receiving MIDI, or, if sending MIDI,
		the time at which the events are to be played. Zero means "now." The time stamp
		applies to the first MIDI byte in the packet.
		*/
		uint32_t _timestamp  = AppleMIDI_Util::toEndian(timestamp);
		uint32_t _ssrc       = AppleMIDI_Util::toEndian(ssrc);

		bytesWritten = stream->write((uint8_t*) ((void*) (&_sequenceNr)), sizeof(_sequenceNr));
		Debug::Assert(bytesWritten == sizeof(_sequenceNr), "error writing _sequenceNr");
		bytesWritten = stream->write((uint8_t*) ((void*) (&_timestamp)), sizeof(_timestamp));
		Debug::Assert(bytesWritten == sizeof(_timestamp), "error writing _timestamp");
		bytesWritten = stream->write((uint8_t*) ((void*) (&_ssrc)), sizeof(_ssrc));
		Debug::Assert(bytesWritten == sizeof(_ssrc), "error writing _ssrc");
	}

};

END_APPLEMIDI_NAMESPACE
