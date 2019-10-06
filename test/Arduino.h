#pragma once

#include <iostream>

#include <inttypes.h>
typedef uint8_t byte;

void begin();
void loop();

int main()
{
	begin();

	while (true)
	{
		loop();
	}
}

// avoid strncpy security warning
#pragma warning(disable:4996)

#define __attribute__(A) /* do nothing */

#include "../src/RingBuffer.h"

#include "../src/midi_feat4_4_0/midi_Defs.h"

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

template <class T> const T& min(const T& a, const T& b) {
	return !(b < a) ? a : b;     // or: return !comp(b,a)?a:b; for version (2)
}
