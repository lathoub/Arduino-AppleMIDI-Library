# AppleMidi for Arduino
[![Build Status](https://travis-ci.org/lathoub/Arduino-AppleMidi-Library.svg?branch=master)](https://travis-ci.org/lathoub/Arduino-AppleMidi-Library) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/) [![GitHub version](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library.svg)](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library)

The goal of this library is to enable an Arduino (with an Ethernet shield) or ESP8266 to particpate in an AppleMIDI session.

## Features
* Tested with AppleMIDI on Mac OS and using rtpMIDI from Tobias Erichsen on Windows
* Send and receive MIDI commands
* Uses callbacks to receive MIDI commands (no need for polling)
* Automatic instantiation of AppleMIDI object (see at the end of 'AppleMidi.h')

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
* Arduino/Genuino (Mega, Uno and Arduino Ethernet)
* ESP8266 (Adafruit HUZZAH ESP8266, Sparkfun ESP8266 Thing Dev)
* Teensy 3.2
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 
 
## Memory usage
The code has been pseudo optimized to minimize the memory footprint.
Internal buffers also use valuable memory space. The biggest buffer `PACKET_MAX_SIZE` is set to 350 by default in `AppleMidi_Settings.h`. Albeit this number is somewhat arbitratry (large enough to receive full SysEx messages), it can be reduced significantly if you do not have to receive large messages.
 
## Network Shields
* Arduino Ethernet shield (arduino.cc, Wiznet W5100)
* Arduino Wifi R3 shield (arduino.cc)
* Teensy WIZ820io W5200
 
## Arduino IDE (arduino.cc)
* 1.8.3

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
