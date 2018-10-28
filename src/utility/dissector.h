/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire, bluebie
 */

#pragma once

#include "AppleMidi_Settings.h"

BEGIN_APPLEMIDI_NAMESPACE

class Dissector;
class IAppleMidiCallbacks;

typedef int(*FPDISSECTOR_APPLEMIDI)(Dissector*, IAppleMidiCallbacks*, unsigned char* packetBuffer, size_t packetSize);

class Dissector {
private:

	IAppleMidiCallbacks* _appleMidi;

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
	void init(int identifier, IAppleMidiCallbacks* appleMidi)
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
		DEBUGSTREAM.print("Incoming buffer of " + String(packetSize, DEC) + " bytes: ");
		int idx = 0;
		while (idx < packetSize) {
			DEBUGSTREAM.print(String(packetBuffer[idx], HEX) + " ");
			idx++;
		}
		DEBUGSTREAM.println();
		#endif

		int nelem = sizeof(_externalAppleMidiDissector) / sizeof(_externalAppleMidiDissector[0]);

		for (int i = 0; i < nelem; i++) {
			if (_externalAppleMidiDissector[i]) {
				int consumed = _externalAppleMidiDissector[i](this, _appleMidi, packetBuffer, packetSize);
				if (consumed) return;
			}
		}

		#ifdef APPLEMIDI_DEBUG_PARSING
		DEBUGSTREAM.println("No dissectors handled last message");
		#endif
	}
};


END_APPLEMIDI_NAMESPACE
