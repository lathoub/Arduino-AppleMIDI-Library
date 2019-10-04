// ConsoleApplication1.cpp : This file contains the 'main' function. Program execution begins and ends there.
//

#include <iostream>
#include "ArduinoEmulator.h"

#include "..\src\AppleMidi.h"

int main()
{
	APPLEMIDI_CREATE_DEFAULT_INSTANCE();

	// 36 bytes
	byte uuu[] = { 0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
				   0x46, // flag
				   0x80, 0x3f, 0x00, // note  
				   0x00, // delta time
				   0x3d, 0x00, //note
				   0x20, // journal flag
				   0x27, 0x34, // sequence nr 
				   0x00, 0x0e, 0x08, // channel 1 channel flag
				   0x02, 0x59, // note on off
				   0xbd, 0x40, 0xbf, 0x40, // Log list 
				   0x15, 0xad, 0x5a, 0xdf, 0xa8 }; // offbit octets

	APPLEMIDI_NAMESPACE::RingBuffer<byte, 128> buffer;
	for (int i = 0; i < sizeof(uuu); i++)
		buffer.write(uuu[i]);
}
