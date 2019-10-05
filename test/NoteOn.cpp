// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ArduinoEmulator.h"

#include "..\src\AppleMidi.h"

APPLEMIDI_CREATE_DEFAULT_INSTANCE();

void begin()
{
	// Create a session and wait for a remote host to connect to us
	MIDI.begin(1);
}

void loop()
{
	MIDI.read();
}

int main()
{
	begin();

	while (true)
	{
		loop();
	}
}

