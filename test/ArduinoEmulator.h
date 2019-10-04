#pragma once

#define _CRT_SECURE_NO_WARNINGS

#include <inttypes.h>
typedef uint8_t byte;

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

class EthernetUDP
{
public:
	//void begin() {};
	//bool beginTransmission() { return true; }
	//void write(byte) {}
	//void endTransmission() {}
	//byte read() { return ' ';  }
	//unsigned available() { return 0; }	
};