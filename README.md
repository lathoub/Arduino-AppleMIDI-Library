# AppleMIDI (aka rtpMIDI) for Arduino
[![arduino-library-badge](https://www.ardu-badge.com/badge/AppleMIDI.svg?)](https://www.ardu-badge.com/AppleMIDI) [![Build Status](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library.svg?branch=master)](https://travis-ci.org/lathoub/Arduino-AppleMIDI-Library) [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/)

Enables an Arduino with IP/UDP capabilities (Ethernet shield, ESP8266, ESP32, ...) to particpate in an AppleMIDI session.

**Important:** Please read the [note below](https://github.com/lathoub/Arduino-AppleMIDI-Library#ethernet-buffer-size) on enlarging the standard Ethernet library buffersize to avoid dropping MIDI messages!

## Features
* Build on top of the popular [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)
* Tested with AppleMIDI on Mac OS (Big Sur) and using [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) from Tobias Erichsen on Windows 10
* Send and receive all MIDI messages
* Uses callbacks to receive MIDI commands (no need for polling)
* Automatic instantiation of AppleMIDI object (see at the end of 'AppleMidi.h')
* Compiles on Arduino, MacOS (XCode) and Windows (MSVS)

## New in 3.0.0
* Bug Fixes (long session names get cropped)
* Reduced memory footprint (see AVR_MinMemUsage example and note below)
* Extended and revised callbacks to receive AppleMIDI protocol feedback (see AVR_Callbacks example)
* Who may connect to me (Directory) (see AVR_Directory example)

## Installation
From the Arduino IDE Library Manager, search for AppleMIDI

<img width="801" alt="Screenshot 2020-04-21 at 10 25 22 copy" src="https://user-images.githubusercontent.com/4082369/79904509-09b11000-8415-11ea-9b05-818373479625.png">

This will also install [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)

## Basic Usage
```cpp
#include <Ethernet.h>
#include <AppleMIDI.h>

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE(); 

void setup()
{
  MIDI.begin();
  
  // Optional
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
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

void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char*) {
}
```

`APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();` creates 2 instance: `MIDI` and `AppleMIDI`. `MIDI` is the instance that manages all MIDI interaction, `AppleMIDI` is the instance this manages the rtp transport layer.


More usages in the [examples](https://github.com/lathoub/Arduino-AppleMIDI-Library/tree/master/examples) folder

## Hardware
* Arduino/Genuino (Mega, Uno, Arduino Ethernet, MKRZERO, ...)
* ESP8266 (Adafruit HUZZAH ESP8266, Sparkfun ESP8266 Thing Dev)
* ESP32 (Adafruit HUZZAH32 – ESP32 Feather Board) Wi-Fi
* ESP32 with W5500
* Teensy 3.2 & 4.1
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 

## Network Shields
* Arduino Ethernet shield (Wiznet W5100 and W5500)
* Arduino Wifi R3 shield
* MKR ETH shield (W5500 and W6100 based)
* Teensy WIZ820io W5200
* Teensy 4.1 with [Ethernet Kit](https://www.pjrc.com/store/ethernet_kit.html)

## Arduino Memory usage
Out of the box, this library has been setup to use a 'medium' amount of memory (read below how to reduce the memory footprint). Extended callbacks are not enabled by default, and can be anabled by `#USE_EXT_CALLBACKS`. See the callback examples.

This library is **not using any dynamic memory** allocation methods - all buffers have a fixed size, set in the `AppleMIDI_Settings.h` file, avoiding potential memory leaks and memory fragmentation (also, no usage of the String() class).

The minimum buffer size (`MaxBufferSize`) should be set to 64 bytes (also the default). Setting it to a higher value will make sending larger SysEx messages more efficiant (large SysEx messages are chopped in pieces, the larger the buffer, the less pieces needed), at the price of a bigger memory footprint.

`MaxNumberOfParticipants` is another way to cut memory - each session particpant uses approx 300 bytes. Default number of participants is 2 (each using 2 sockets). 
Beware: the number of sockets on the Arduino is limited. The W5100 support 4 (hance default number of participants is 2), the W5200 and W5500 based IP chips can use 8 sockets. (Base port can be set in `APPLEMIDI_CREATE_DEFAULT_INSTANCE`)

Reduce the memory footprint by a further wopping 500 bytes by `#define NO_SESSION_NAME` before `#include <AppleMIDI.h>`. This will leave out all the code to manage the optional session name. The RtpMIDI service client will show the name as (manually) defined in the directory.

Even further reduce the memory footprint by `#define ONE_PARTICIPANT` limiting the number of particpants to just 1.
On an UNO the absolute minimum memory footprint is 21966 bytes (68%) and 945 global variables (46%). For a Leonardo that is 24906 bytes (86%) and 1111 bytes  (43%) of global variables.

## Notes

### Session names

Session names can get really long on Macs (eg 'Macbook Pro of Johann Gambolputty .. von Hautkopft of Ulm') and will be trunctated to the `MaxSessionNameLen` (as set in the settings file).

### Ethernet buffer size
It's highly recommended to modify the [Ethernet library](https://github.com/arduino-libraries/Ethernet) or use the [Ethernet3 library](https://github.com/sstaub/Ethernet3) to avoid buffer overruns - [learn more](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Enlarge-Ethernet-buffer-size-to-avoid-dropping-UDP-packages)

### Latency
Use wired Ethernet to reduce latency, Wi-Fi increases latency and latency varies. More of the [wiki](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Keeping-Latency-under-control)  

## Arduino IDE (arduino.cc)
* 1.8.13

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
