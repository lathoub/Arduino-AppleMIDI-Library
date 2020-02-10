#pragma once
#include <stdio.h>

#include "Arduino.h"

class EthernetUDP
{
	Deque<byte, 4096> _buffer;
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
            
            // sysex (command length is 63 (or 0x3f) in 2 bytes - B-FLAG)
            byte longsysex[] = {  0x80, 0x61, 0x9A, 0xF, 0x0, 0x2A, 0x7D, 0x3D, 0x29, 0xDC, 0x48, 0x99,
                              0x80, 0x3f,
                              0xF0,
                                0x41,
                                    0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                                    0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
                                    0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
                                    0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
                                    0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
                                    0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
                              0xF7 };

            byte sysex[] = { 0x80, 0x61, 0x7c, 0xbc, 0x0a, 0xff, 0x56, 0xba, 0x0a, 0x1a, 0x2f, 0x43, 0x05,
                        0xf0,
                            0x41, 0x19, 0x20,
                        0xf7 };
            
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
            
            write(longsysex, sizeof(longsysex));
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
		return _buffer.size();
	};

	size_t read(byte* buffer, size_t size)
	{
		size = min(size, _buffer.size());
            
        for (size_t i = 0; i < size; i++)
            buffer[i] = _buffer.pop_front();

		return size;
	};

	void write(uint8_t buffer) 
	{
		_buffer.push_back(buffer);
	};

	void write(uint8_t* buffer, size_t size)
	{
		for (size_t i = 0; i < size; i++)
			_buffer.push_back(buffer[i]);
	};

	void endPacket() { };
	void flush() { };

	uint32_t remoteIP() { return 1; }
	uint16_t remotePort() { return _port; }
};
