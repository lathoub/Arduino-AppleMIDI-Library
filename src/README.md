# AppleMIDI (rtpMIDI) for Arduino
[![Build Status](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library.svg?branch=master)](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/) [![GitHub version](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library.svg)](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library)

Enables an Arduino with IP/UDP capabilities (Ethernet shield, ESP8266, ESP32, ...) to particpate in an AppleMIDI session.

## Features
* Tested with AppleMIDI on Mac OS (10.13 High Sierra & 10.14 Mojave) and using [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) from Tobias Erichsen on Windows 10
* Send and receive all MIDI messages
* Uses callbacks to receive MIDI commands (no need for polling)
* Automatic instantiation of AppleMIDI object (see at the end of 'AppleMidi.h')

## Installation
From the Arduino IDE Library Manager, search for AppleMIDI
<img src="https://user-images.githubusercontent.com/4082369/34467930-15f909ca-eefe-11e7-9bc0-614884b234f8.PNG">

## Basic Usage
```
#include "AppleMidi.h"

APPLEMIDI_CREATE_DEFAULT_INSTANCE(); 

void setup()
{
  // ...setup ethernet connection
  AppleMIDI.begin("test"); // 'test' will show up as the session name
}

void loop()
{
  AppleMIDI.run();
  // ...
  
  // Send MIDI note 40 on, velocity 55 on channel 1
  AppleMIDI.noteOn(40, 55, 1);
}
```
More usages in the `examples` folder

## Hardware
* Arduino/Genuino (Mega, Uno, Arduino Ethernet, ...)
* ESP8266 (Adafruit HUZZAH ESP8266, Sparkfun ESP8266 Thing Dev)
* ESP32 (Adafruit HUZZAH32 – ESP32 Feather Board)
* Teensy 3.2
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 
 
## Memory usage
The code has been pseudo optimized to minimize the memory footprint.
Internal buffers also use valuable memory space. The biggest buffer `PACKET_MAX_SIZE` is set to 350 by default in `AppleMidi_Settings.h`. Albeit this number is somewhat arbitratry (large enough to receive full SysEx messages), it can be reduced significantly if you do not have to receive large messages.

On an Arduino, 2 sessions can be active at once (W5100 can have max 4 sockets open at the same time). Setting MAX_SESSIONS to 1 saves 228 bytes (each session takes 228 bytes). 
 
## Network Shields
* Arduino Ethernet shield (arduino.cc, Wiznet W5100)
* Arduino Wifi R3 shield (arduino.cc)
* Teensy WIZ820io W5200
 
## Arduino IDE (arduino.cc)
* 1.8.7

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
