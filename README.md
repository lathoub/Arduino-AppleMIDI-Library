# AppleMIDI (aka rtpMIDI) for Arduino
[![arduino-library-badge](https://www.ardu-badge.com/badge/AppleMIDI.svg?)](https://www.ardu-badge.com/AppleMIDI) [![Build Status](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library.svg?branch=master)](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/)

Enables an Arduino with IP/UDP capabilities (Ethernet shield, ESP8266, ESP32, ...) to participate in an AppleMIDI session.

**Important:** Please read the [note below](https://github.com/lathoub/Arduino-AppleMIDI-Library#ethernet-buffer-size) on enlarging the standard Ethernet library buffersize to avoid dropping MIDI messages!

## Features
* Build on top of the popular [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)
* Tested with AppleMIDI on Mac OS (Big Sur) and using [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) from Tobias Erichsen on Windows 10
* Send and receive all MIDI messages
* Uses callbacks to receive MIDI commands (no need for polling)
* Automatic instantiation of AppleMIDI object (see at the end of 'AppleMidi.h')
* Compiles on Arduino, MacOS (XCode) and Windows (MSVS)

### New in 3.2.0
* Event chaining

### New in 3.3.0
* Better parsing of large incoming MIDI messages with a small internal Arduino buffer

## Installation
From the Arduino IDE Library Manager, search for AppleMIDI

<img width="786" alt="Installation" src="https://github.com/lathoub/Arduino-AppleMIDI-Library/blob/master/res/Install3-1.PNG">

This will also install [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)

## Basic Usage
```cpp
#include <Ethernet.h>
#include <AppleMIDI.h>

byte mac[] = { 0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED };

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE(); 

void setup()
{
  Ethernet.begin(mac);

  MIDI.begin(); // listens on channel 1
}

void loop()
{
  // Listen to incoming notes
  MIDI.read();
  
  ....
  if (something) {
    // Send MIDI note 40 on, velocity 55 on channel 1
    MIDI.sendNoteOn(40, 55, 1);
  }
}
```

More usages in the [examples](https://github.com/lathoub/Arduino-AppleMIDI-Library/tree/master/examples) folder and in the [wiki](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki)

## Hardware
* Arduino/Genuino (Mega, Uno, Arduino Ethernet, MKRZERO, ...)
* ESP8266 (Adafruit HUZZAH ESP8266, Sparkfun ESP8266 Thing Dev)
* ESP32 (Adafruit HUZZAH32 – ESP32 Feather Board) Wi-Fi
* ESP32 with W5500 [Setup](https://github.com/lathoub/Arduino-AppleMIDI-Library/discussions/135)
* Teensy 3.2 & 4.1
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 

## Network Shields
* Arduino Ethernet shield (Wiznet W5100 and W5500)
* Arduino Wifi R3 shield
* MKR ETH shield (W5500 and W6100 based)
* Teensy WIZ820io W5200
* Teensy 4.1 with [Ethernet Kit](https://www.pjrc.com/store/ethernet_kit.html)

## Notes

### Session names

Session names can get really long on Macs (eg 'Macbook Pro of Johann Gambolputty .. von Hautkopft of Ulm') and will be truncated to the [`MaxSessionNameLen`](https://github.com/lathoub/Arduino-AppleMIDI-Library/blob/af4c7bd9a960a90e09e211f0ea00db2d9832d1f7/src/AppleMIDI_Settings.h#L13) 

### Memory footprint
The memory footprint of the library can be lowered significantly, read the [wiki](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Memory-footprint) 

### Ethernet buffer size
It's highly recommended to modify the [Ethernet library](https://github.com/arduino-libraries/Ethernet) or use the [Ethernet3 library](https://github.com/sstaub/Ethernet3) to avoid buffer overruns - [learn more](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Enlarge-Ethernet-buffer-size-to-avoid-dropping-UDP-packages)

### Latency
Use wired Ethernet to reduce latency, Wi-Fi increases latency and latency varies. More of the [wiki](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Keeping-Latency-under-control)  

## Arduino IDE (arduino.cc)
* 1.8.16

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
