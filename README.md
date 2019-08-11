# AppleMIDI (rtpMIDI) for Arduino
[![Build Status](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library.svg?branch=master)](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/) [![GitHub version](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library.svg)](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/c8be2ccc3f104e0588572a39f8106070)](https://app.codacy.com/app/lathoub/Arduino-AppleMIDI-Library?utm_source=github.com&utm_medium=referral&utm_content=lathoub/Arduino-AppleMIDI-Library&utm_campaign=Badge_Grade_Dashboard)

Enables an Arduino with IP/UDP capabilities (Ethernet shield, ESP8266, ESP32, ...) to particpate in an AppleMIDI session.

## Features
* Tested with AppleMIDI on Mac OS (High Sierra) and using [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) from Tobias Erichsen on Windows 10
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
  AppleMIDI.sendNoteOn(40, 55, 1);
}
```
More usages in the `examples` folder

## Hardware
* Arduino/Genuino (Mega, Uno, Arduino Ethernet, MKRZERO, ...)
* ESP8266 (Adafruit HUZZAH ESP8266, Sparkfun ESP8266 Thing Dev)
* ESP32 (Adafruit HUZZAH32 – ESP32 Feather Board)
* Teensy 3.2
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 
 
## Memory usage
The code has been pseudo optimized to minimize the memory footprint.
Internal buffers also use valuable memory space. The biggest buffer `PACKET_MAX_SIZE` is set to 350 by default in `AppleMidi_Settings.h`. Albeit this number is somewhat arbitratry (large enough to receive full SysEx messages), it can be reduced significantly if you do not have to receive large messages.

On an Arduino, 2 sessions can be active at once (W5100 can have max 4 sockets open at the same time, each session needs 2 UDP sockets). Setting MAX_SESSIONS to 1 saves 228 bytes (each session takes 228 bytes).

Save memory (about 2000 bytes) when the device does not initiate sessions by `#undef APPLEMIDI_REMOTE_SESSIONS` in `AppleMidi_Settings.h`. See the `EthernetShield_NoteOnOffEverySec.ino` example
 
## Network Shields
* Arduino Ethernet shield (Wiznet W5100)
* Arduino Wifi R3 shield
* MKR ETH shield
* Teensy WIZ820io W5200
 
## Arduino IDE (arduino.cc)
* 1.8.9

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
