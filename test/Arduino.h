#pragma once

#include <chrono>
#include <iostream>

#include "IPAddress.h"

#define HEX 0
#define DEC 1

class _serial
{
public:
    void print(const char a[]) { std::cout << a; };
    void print(char a) { std::cout << a; };
    void print(unsigned char a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << (int)a; };
    void print(int a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a; };
    void print(unsigned int a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a; };
    void print(long a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a; };
    void print(unsigned long a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a; };
    void print(double a, int = 2) { std::cout << a; };
    void print(struct tm * timeinfo, const char * format = nullptr) {};
    void print(IPAddress) {};

    void println(const char a[]) { std::cout << a << "\n"; };
    void println(char a) { std::cout << a << "\n"; };
    void println(unsigned char a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << (int)a << "\n"; };
    void println(int a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a << "\n"; };
    void println(unsigned int a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a << "\n"; };
    void println(long a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a << "\n"; };
    void println(unsigned long a, int format = DEC) { std::cout << (format == DEC ? std::dec : std::hex) << a << "\n"; };
    void println(double a, int format = 2) { std::cout << a << "\n"; };
    void println(struct tm * timeinfo, const char * format = nullptr) {};
    void println(IPAddress) {};
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

#include "../src/utility/Deque.h"

#include <midi_Defs.h>

float analogRead(int pin)
{
	return 0.0f;
}

void randomSeed(float)
{
    srand(static_cast<unsigned int>(time(0)));
}

unsigned long millis()
{
    auto now = std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count();
    return (unsigned long)now;
}

int random(int min, int max)
{
	return RAND_MAX % std::rand() % (max-min) + min;
}

template <class T> const T& min(const T& a, const T& b) {
    return !(b < a) ? a : b;     // or: return !comp(b,a)?a:b; for version (2)
}

#define F(x) x
