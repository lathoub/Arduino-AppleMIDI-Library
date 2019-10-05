#pragma once

#pragma warning(disable:4996)

#define __attribute__(A) /* do nothing */

#include <inttypes.h>
typedef uint8_t byte;

#include "../src/RingBuffer.h"

#include "..\src\midi_feat4_4_0/midi_Defs.h"

float analogRead(int pin)
{
	return 0.0f;
}

void randomSeed(float)
{
}

int millis()
{
	return 1000;
}

int random(int a, int)
{
	return a;
}

template <class T> const T & min(const T & a, const T & b) {
	return !(b < a) ? a : b;     // or: return !comp(b,a)?a:b; for version (2)
}

class EthernetUDP
{
	APPLEMIDI_NAMESPACE::RingBuffer<byte, 512> _buffer;
	uint16_t _port;

public:

	EthernetUDP()
	{
		_port = 0;
	}

	void begin(uint16_t port) 
	{
		_port = port;

		// 36 bytes
		byte buffer[] = { 0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
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

		for (size_t i = 0; i < sizeof(buffer); i++)
			write(buffer[i]);
	};
	bool beginPacket(uint32_t, uint16_t) 
	{ 
		return true;
	}
	size_t parsePacket() 
	{
		return _buffer.getLength(); 
	};
	size_t read(byte* buffer, size_t size)
	{
		return _buffer.read();
	};
	void write(uint8_t buffer) 
	{
		_buffer.write(buffer);
	};
	void write(uint8_t* buffer, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			_buffer.write(buffer[i]);
	};
	void endPacket() { };
	void flush() { };

	uint32_t remoteIP() { return 0; }
	uint16_t remotePort() { return _port; }
};

class dddd
{
public:
	void begin(MIDI_NAMESPACE::Channel inChannel = 1) {};
	bool beginTransmission() { return true; }
	void write(byte) {}
	void endTransmission() {}
	byte read() { return ' '; }
	unsigned available() { return 0; }
};