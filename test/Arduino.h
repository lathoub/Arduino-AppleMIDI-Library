#pragma once

#include <iostream>

#define HEX 1
#define DEC 1

class _serial
{
public:
    void printf(const char * format, ...)  __attribute__ ((format (printf, 2, 3)));
    void print(const char a[]) { std::cout << a; };
    void print(char a) { std::cout << a; };
    void print(unsigned char a, int = DEC) { std::cout << a; };
    void print(int a, int = DEC) { std::cout << a; };
    void print(unsigned int a, int = DEC) { std::cout << a; };
    void print(long a, int = DEC) { std::cout << a; };
    void print(unsigned long a, int = DEC) { std::cout << a; };
    void print(double a, int = 2) { std::cout << a; };
    void print(struct tm * timeinfo, const char * format = NULL) {};

    void println(const char a[]) { std::cout << a << "\n"; };
    void println(char a) { std::cout << a << "\n"; };
    void println(unsigned char a, int = DEC) { std::cout << a << "\n"; };
    void println(int a, int = DEC) { std::cout << a << "\n"; };
    void println(unsigned int a, int = DEC) { std::cout << a << "\n"; };
    void println(long a, int = DEC) { std::cout << a << "\n"; };
    void println(unsigned long a, int = DEC) { std::cout << a << "\n"; };
    void println(double a, int = 2) { std::cout << a << "\n"; };
    void println(struct tm * timeinfo, const char * format = NULL) {};
    void println(void) { std::cout << "\n"; };
};

_serial Serial;

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

#include "../src/utilities/RingBuffer.h"

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

#define F(x) x
