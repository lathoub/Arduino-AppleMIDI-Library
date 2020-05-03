# AppleMIDI (aka rtpMIDI) for Arduino
[![Build Status](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library.svg?branch=master)](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/) [![GitHub version](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library.svg)](https://badge.fury.io/gh/lathoub%2FArduino-AppleMidi-Library) [![Codacy Badge](https://api.codacy.com/project/badge/Grade/c8be2ccc3f104e0588572a39f8106070)](https://app.codacy.com/app/lathoub/Arduino-AppleMIDI-Library?utm_source=github.com&utm_medium=referral&utm_content=lathoub/Arduino-AppleMIDI-Library&utm_campaign=Badge_Grade_Dashboard)

Enables an Arduino with IP/UDP capabilities (Ethernet shield, ESP8266, ESP32, ...) to particpate in an AppleMIDI session.

## Features
* Build on top of the popular [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)
* Tested with AppleMIDI on Mac OS (Catalina) and using [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) from Tobias Erichsen on Windows 10
* Send and receive all MIDI messages
* Uses callbacks to receive MIDI commands (no need for polling)
* Automatic instantiation of AppleMIDI object (see at the end of 'AppleMidi.h')
* Compiles on Arduino, MacOS (XCode) and Windows (MSVS)

## Installation
From the Arduino IDE Library Manager, search for AppleMIDI

<img width="801" alt="Screenshot 2020-04-21 at 10 25 22 copy" src="https://user-images.githubusercontent.com/4082369/79904509-09b11000-8415-11ea-9b05-818373479625.png">

This will also install [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)

## Basic Usage
```
#include <Ethernet.h>
#include <AppleMIDI.h>

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE(); 

void setup()
{
  MIDI.begin(1);
  
  // Optional
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
}

void loop()
{
  // Listen to incoming notes
  MIDI.read();
  
  // Send MIDI note 40 on, velocity 55 on channel 1
  MIDI.sendNoteOn(40, 55, 1);
}

void OnAppleMidiConnected(uint32_t ssrc, const char* name) {
}
```

`APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();` creates 2 instance: `MIDI` and `AppleMIDI`. `MIDI` is the instance that manages all MIDI interaction, `AppleMIDI` is the instance this manages the rtp transport layer.


More usages in the [examples](https://github.com/lathoub/Arduino-AppleMIDI-Library/tree/master/examples) folder

## Hardware
* Arduino/Genuino (Mega, Uno, Arduino Ethernet, MKRZERO, ...)
* ESP8266 (Adafruit HUZZAH ESP8266, Sparkfun ESP8266 Thing Dev)
* ESP32 (Adafruit HUZZAH32 – ESP32 Feather Board)
* Teensy 3.2
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 
 
## Memory usage
This library is not using any dynamic memory allocation methods - all buffers have a fixed size, set in the `AppleMIDI_Settings.h` file, avoiding potential memory leaks and memory fragmentation.

The minimum buffer size (`MaxBufferSize`) should be set to 64 bytes (also the default). Setting it to a higher value will make sending larger SysEx messages more efficiant (large SysEx messages are chopped in pieces, the larger the buffer, the less pieces needed), at the price of a bigger memory footprint.

`MaxNumberOfParticipants` is another way to cut memory - each particpants uses approx 300 bytes. Default number of participants is 1 (using 2 sockets). 
Beware: the number of sockets on the Arduino is limited. The W5100 support 4, the W5200 and W5500 based IP chips can use 8 sockets. (Each participant uses 2 sockets: port 5004 and 5004+1). (Base port can be set in `APPLEMIDI_CREATE_DEFAULT_INSTANCE`)
 
## Network Shields
* Arduino Ethernet shield (Wiznet W5100 and W5500)
* Arduino Wifi R3 shield
* MKR ETH shield
* Teensy WIZ820io W5200
 
## Arduino IDE (arduino.cc)
* 1.8.10

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
