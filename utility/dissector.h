/*!
 *  @file		Dissector.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  License		GPL
 */

#pragma once

#define PACKET_MAX_SIZE 96

BEGIN_APPLEMIDI_NAMESPACE

class Dissector;
class AppleMidi_Class;

typedef int (*FPDISSECTOR)(Dissector*, AppleMidi_Class*, unsigned char* packetBuffer, size_t packetSize);

class Dissector {
private:
	unsigned char _protocolBuffer[PACKET_MAX_SIZE];
	size_t  _protocolBufferIndex;

	AppleMidi_Class* _appleMidi;

	FPDISSECTOR _externalDissector[5];

public:
	int _identifier;


private:
	void resetBuffer(size_t index)
	{
		memcpy(_protocolBuffer, _protocolBuffer + index, PACKET_MAX_SIZE - index);
		_protocolBufferIndex -= index;
	}

	void reset()
	{
		_protocolBufferIndex = 0;
	}

	void call_dissector()
	{
		int nelem = sizeof(_externalDissector) / sizeof(_externalDissector[0]);

		for (int i = 0; i < nelem; i++) {
			if (_externalDissector[i]) {

				int consumed = _externalDissector[i](this, _appleMidi, _protocolBuffer, _protocolBufferIndex);
				if (consumed > 0) {
					resetBuffer(consumed);
					return;
				}
			}
		}
	}

public:

	Dissector()
	{
		int nelem = sizeof(_externalDissector) / sizeof(_externalDissector[0]);
		for (int i = 0; i < nelem; i++)
			_externalDissector[i] = NULL;

		_protocolBufferIndex = 0;
	}

	//
	void init(int identifier, AppleMidi_Class* appleMidi)
	{
		_identifier = identifier;
		_appleMidi = appleMidi;

		reset();
	}

	void addPacketDissector(FPDISSECTOR externalDissector)
	{
		int nelem = sizeof(_externalDissector) / sizeof(_externalDissector[0]);
		for (int i = 0; i < nelem; i++) {
			if (!_externalDissector[i]) {
				_externalDissector[i] = externalDissector;
				return;
			}
		}
	}

	void addPacket(unsigned char* packetBuffer, size_t packetSize)
	{
		// enough room in buffer? If so, reset protocolBuffer back to zero
		if (_protocolBufferIndex + packetSize > PACKET_MAX_SIZE)
			resetBuffer(0);

		// Add to the end of the protocolBuffer
		memcpy(_protocolBuffer + _protocolBufferIndex, packetBuffer, packetSize);
		_protocolBufferIndex += packetSize;
	}

	void dissect()
	{
		call_dissector();
	}

};


END_APPLEMIDI_NAMESPACE
