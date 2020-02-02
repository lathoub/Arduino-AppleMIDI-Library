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

byte cc[] = { 0x80, 0x61, 0xF, 0xC2, 0x2, 0x0, 0xCA, 0x51, 0x6C, 0x75, 0xA8, 0x72, 0x6, 0xF0, 0x7E, 0x7F, 0x9, 0x1, 0xF7 };
           
byte aa[] = {  0x80, 0x61, 0x7B, 0xAF, 0x16, 0x7C, 0xF1, 0x2, 0x56, 0xF9, 0x59, 0xAF, 0x41, 0xFA, 0x40, 0x7B, 0xAE, 0x10, 0x5, 0x50, 0x0, 0x0 };
byte bb[] = {  0x61, 0x7B, 0xB0, 0x16, 0x7D, 0x41, 0xC3, 0x56, 0xF9, 0x59, 0xAF, 0x1, 0xFA };

			// 36 bytes
			byte noteOnOff[] = {
                                0x80, 0x61, 0x27, 0x9e, 0x00, 0x1d, 0xb5, 0x36, 0x36, 0x09, 0x2f, 0x2a, // rtp
                                0x46, // flag
                                0x80, 0x3f, 0x00, // note off 63 on channel 1,
                                0x00, // delta time
                                0x3d, 0x00, // note 61
                                0x20, // journal flag
                                0x27, 0x34, // sequence nr
                                0x00, 0x0e, 0x08, // channel 1 channel flag
                                0x02, 0x59, // note on off
                                0xbd, 0x40, 0xbf, 0x40, // Log list
                                0x15, 0xad, 0x5a, 0xdf, 0xa8 // offbit octets
                            };
            
            for (size_t i = 0; i < sizeof(cc); i++)
                write(cc[i]);
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
