// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ArduinoEmulator.h"

#include "..\src\AppleMidi.h"

int main()
{
	APPLEMIDI_CREATE_DEFAULT_INSTANCE();

	// Create a session and wait for a remote host to connect to us
	MIDI.begin(1);
	MIDI.read();
}
