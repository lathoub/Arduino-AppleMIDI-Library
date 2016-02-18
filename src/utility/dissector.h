/*!
 *  @file		Dissector.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "AppleMidi_Settings.h"

#define PACKET_MAX_SIZE 96

BEGIN_APPLEMIDI_NAMESPACE

class Dissector;
class IAppleMidi;

typedef int(*FPDISSECTOR_APPLEMIDI)(Dissector*, IAppleMidi*, unsigned char* packetBuffer, size_t packetSize);

class Dissector {
private:
	unsigned char _protocolBuffer[PACKET_MAX_SIZE];
	size_t  _protocolBufferIndex;

	IAppleMidi* _appleMidi;

	FPDISSECTOR_APPLEMIDI _externalAppleMidiDissector[5];

public:
	int _identifier;

private:
	void purgeBuffer(size_t index)
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.print  ("Purging left ");
Serial.print  (index);
Serial.println(" bytes ");
#endif

		memcpy(_protocolBuffer, _protocolBuffer + index, PACKET_MAX_SIZE - index);
		_protocolBufferIndex -= index;

		if (_protocolBufferIndex < 0)
		{
#ifdef APPLEMIDI_DEBUG
Serial.print  ("ProtocolBuffer Underrun. _protocolBufferIndex");
Serial.print  (_protocolBufferIndex);
Serial.print  (" index ");
Serial.print  (index);
Serial.println(" _protocolBufferIndex set to 0");
#endif

			_protocolBufferIndex = 0;
		}
	}

	void reset()
	{
		_protocolBufferIndex = 0;

#ifdef APPLEMIDI_DEBUG_VERBOSE
Serial.println("reset");
#endif
	}

	void call_dissector()
	{
		int nelem = sizeof(_externalAppleMidiDissector) / sizeof(_externalAppleMidiDissector[0]);

		for (int i = 0; i < nelem; i++) {
			if (_externalAppleMidiDissector[i]) {

				int consumed = _externalAppleMidiDissector[i](this, _appleMidi, _protocolBuffer, _protocolBufferIndex);
				if (consumed > 0) {
					purgeBuffer(consumed);
					return;
				}
			}
		}
	}

public:

	Dissector()
	{
		int nelem = sizeof(_externalAppleMidiDissector) / sizeof(_externalAppleMidiDissector[0]);
		for (int i = 0; i < nelem; i++)
			_externalAppleMidiDissector[i] = NULL;

		_protocolBufferIndex = 0;
	}

	//
	void init(int identifier, IAppleMidi* appleMidi)
	{
		_identifier = identifier;
		_appleMidi = appleMidi;

		_protocolBufferIndex = 0;
	}

	void addPacketDissector(FPDISSECTOR_APPLEMIDI externalDissector)
	{
		int nelem = sizeof(_externalAppleMidiDissector) / sizeof(_externalAppleMidiDissector[0]);
		for (int i = 0; i < nelem; i++) {
			if (!_externalAppleMidiDissector[i]) {
				_externalAppleMidiDissector[i] = externalDissector;
				return;
			}
		}
	}

	void addPacket(unsigned char* packetBuffer, size_t packetSize)
	{
#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.print  ("Incoming buffer of ");
		Serial.print  (packetSize);
		Serial.println(" bytes. These will be appended to the protocolBuffer");
		for (int i = 0; i < packetSize; i++)
		{
			Serial.print  (packetBuffer[i], HEX);
			Serial.print  (" ");
		}
		Serial.println();
#endif

		// enough room in buffer? If so, reset protocolBuffer back to zero
		if (_protocolBufferIndex + packetSize > PACKET_MAX_SIZE)
		{
#ifdef APPLEMIDI_DEBUG
Serial.println("Not enough memory in protocolBuffer, clearing existing parser buffer.");
#endif
			reset();
		}

		// Add to the end of the protocolBuffer
		memcpy(_protocolBuffer + _protocolBufferIndex, packetBuffer, packetSize);
		_protocolBufferIndex += packetSize;

#ifdef APPLEMIDI_DEBUG_VERBOSE
		Serial.print  ("Protocol buffer contains ");
		Serial.print  (_protocolBufferIndex);
		Serial.println(" bytes. Content:");
		for (int i = 0; i < _protocolBufferIndex; i++)
		{
			Serial.print  (_protocolBuffer[i], HEX);
			Serial.print  (" ");
		}
		Serial.println();
#endif	
	}

	void dissect()
	{
		call_dissector();
	}

};


END_APPLEMIDI_NAMESPACE
