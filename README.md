# AppleMIDI (aka rtpMIDI) for Arduino [![License: CC BY-SA 4.0](https://img.shields.io/badge/License-CC%20BY--SA%204.0-lightgrey.svg)](http://creativecommons.org/licenses/by-sa/4.0/)

Enables an Arduino with IP/UDP capabilities (Ethernet shield, ESP8266, ESP32, ...) to participate in an AppleMIDI session.

**Important:** Please read the [note below](https://github.com/lathoub/Arduino-AppleMIDI-Library#ethernet-buffer-size) on enlarging the standard Ethernet library buffersize to avoid dropping MIDI messages!

## Features
* Build on top of the popular [FortySevenEffects MIDI library](https://github.com/FortySevenEffects/arduino_midi_library)
* Tested with AppleMIDI on Mac OS (Big Sur) and using [rtpMIDI](https://www.tobias-erichsen.de/software/rtpmidi.html) from Tobias Erichsen on Windows 10
* Send and receive all MIDI messages
* Uses callbacks to receive MIDI commands (no need for polling)
* Automatic instantiation of AppleMIDI object (see at the end of 'AppleMIDI.h')
* Compiles on Arduino, MacOS (XCode) and Windows (MSVS)

### New in 3.5.0
* Safer session and RTP parsing (known-SSRC MIDI only, UDP datagram isolation, wrap-safe sequence numbers)
* RTP is sent only after the data-port handshake; invitation reject matches by initiator token
* Size gates: `APPLEMIDI_SMALL`, `ONE_PARTICIPANT`, `NO_SESSION_NAME` (see [Memory footprint](#memory-footprint))
* Large SysEx is split per RFC 6295 (`F0`/`F7`); see [SysEx and extra F0/F7 bytes](#sysex-and-extra-f0f7-bytes)

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
* ESP32 ETH W5500 (Native ESP32 W5500 support)
* Teensy 3.2 & 4.1
* Adafruit Feather M0 WiFi - ATSAMD21 + ATWINC1500 

## Network Shields
* Arduino Ethernet shield (Wiznet W5100 and W5500)
* Arduino Wifi R3 shield
* MKR ETH shield (W5500 and W6100 based)
* Teensy WIZ820io W5200
* Teensy 4.1 with [Ethernet Kit](https://www.pjrc.com/store/ethernet_kit.html)
* ESP32 with W5500 

## Notes

### SysEx and extra F0/F7 bytes

Outgoing SysEx that does not fit in [`MaxMidiOutSize`](https://github.com/lathoub/Arduino-AppleMIDI-Library/blob/v3.5.0/src/AppleMIDI_Settings.h) (default 64) is split across RTP packets per [RFC 6295](https://www.rfc-editor.org/rfc/rfc6295.html#section-3): the current packet ends with `F0` and the next starts with `F7`. USB, serial, and BLE do not add those markers, so a capture can look malformed ([#169](https://github.com/lathoub/Arduino-AppleMIDI-Library/issues/169)). AppleMIDI/rtpMIDI receivers reassemble them. Raise `MaxMidiOutSize` (and usually `MaxBufferSize`) in a custom Settings struct if you need a larger SysEx in one command section.

### Session names

Session names can get really long on Macs (eg 'Macbook Pro of Johann Gambolputty .. von Hautkopft of Ulm') and will be truncated to [`MaxSessionNameLen`](src/AppleMIDI_Settings.h).

### Memory footprint
Define these in the sketch **before** `#include <AppleMIDI.h>`:

* `ONE_PARTICIPANT` — one remote peer (saves a participant slot)
* `NO_SESSION_NAME` — do not store session names (~100 bytes)
* `APPLEMIDI_SMALL` — skip unused journal parsing and outbound RS  
  (`APPLEMIDI_PARSE_JOURNALS` / `APPLEMIDI_KEEP_RECEIVER_FEEDBACK` keep those while still using `APPLEMIDI_SMALL`)

`APPLEMIDI_SMALL` is used in `examples/AVR_MinMemUsage`. More on the [wiki](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Memory-footprint). 

### Ethernet buffer size
It's highly recommended to modify the [Ethernet library](https://github.com/arduino-libraries/Ethernet) or use the [Ethernet3 library](https://github.com/sstaub/Ethernet3) to avoid buffer overruns - [learn more](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Enlarge-Ethernet-buffer-size-to-avoid-dropping-UDP-packages)

### Latency
Use wired Ethernet to reduce latency, Wi-Fi increases latency and latency varies. More of the [wiki](https://github.com/lathoub/Arduino-AppleMIDI-Library/wiki/Keeping-Latency-under-control)  

## Arduino IDE (arduino.cc)
* 1.8.16
* 2.3.7

## Contributing
I would love to include your enhancements or bug fixes! In lieu of a formal styleguide, please take care to maintain the existing coding style. Please test your code before sending a pull request. It would be very helpful if you include a detailed explanation of your changes in the pull request.
