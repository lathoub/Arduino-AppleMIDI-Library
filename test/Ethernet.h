#pragma once

#include "Arduino.h"

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

		if (port == 5004 && true)
		{
			// AppleMIDI messages
		}

		if (port == 5005 && true)
		{
			// rtp-midi and AppleMIDI messages

			// 36 bytes
			byte noteOnOff[] = { 0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
								 0x46, // flag
								 0x80, 0x3f, 0x00, // note  
								 0x00, // delta time
								 0x3d, 0x00, // note
								 0x20, // journal flag
								 0x27, 0x34, // sequence nr 
								 0x00, 0x0e, 0x08, // channel 1 channel flag
								 0x02, 0x59, // note on off
								 0xbd, 0x40, 0xbf, 0x40, // Log list 
								 0x15, 0xad, 0x5a, 0xdf, 0xa8 }; // offbit octets
			for (size_t i = 0; i < sizeof(noteOnOff); i++)
				write(noteOnOff[i]);
		}

		if (port == 5005 && true)
		{
			// rtp-midi and AppleMIDI messages
		}

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
		size = min(size, _buffer.getLength());
		
		for (size_t i = 0; i < size; i++)
			buffer[i] = _buffer.read();

		return size;
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
