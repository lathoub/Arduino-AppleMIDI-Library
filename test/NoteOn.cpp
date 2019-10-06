#include "Ethernet.h"

#include "..\src\AppleMidi.h"

APPLEMIDI_CREATE_DEFAULT_INSTANCE(EthernetUDP, "Arduino");

void begin()
{
	MIDI.begin(1);
}

void loop()
{
	MIDI.read();
}
