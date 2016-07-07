/*!
 *  @file		Dissector.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub, hackmancoltaire, bluebie
 *	@date		04/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "AppleMidi_Settings.h"

BEGIN_APPLEMIDI_NAMESPACE

class Dissector;
class IAppleMidi;

typedef int(*FPDISSECTOR_APPLEMIDI)(Dissector*, IAppleMidi*, unsigned char* packetBuffer, size_t packetSize);

class Dissector {
private:

	IAppleMidi* _appleMidi;

	FPDISSECTOR_APPLEMIDI _externalAppleMidiDissector[5];

public:
	int _identifier;

private:


public:

	Dissector()
	{
		int nelem = sizeof(_externalAppleMidiDissector) / sizeof(_externalAppleMidiDissector[0]);
		for (int i = 0; i < nelem; i++)
			_externalAppleMidiDissector[i] = NULL;
	}

	//
	void init(int identifier, IAppleMidi* appleMidi)
	{
		_identifier = identifier;
		_appleMidi = appleMidi;
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
		#ifdef APPLEMIDI_DEBUG_PARSING
		Serial.print("Incoming buffer of " + String(packetSize, DEC) + " bytes: ");
		int idx = 0;
		while (idx < packetSize) {
			Serial.print(String(packetBuffer[idx], HEX) + " ");
			idx++;
		}
		Serial.println();
		#endif

		int nelem = sizeof(_externalAppleMidiDissector) / sizeof(_externalAppleMidiDissector[0]);

		for (int i = 0; i < nelem; i++) {
			if (_externalAppleMidiDissector[i]) {
				int consumed = _externalAppleMidiDissector[i](this, _appleMidi, packetBuffer, packetSize);
				if (consumed) return;
			}
		}

		#ifdef APPLEMIDI_DEBUG_PARSING
		Serial.println("No dissectors handled last message");
		#endif
	}
};


END_APPLEMIDI_NAMESPACE
