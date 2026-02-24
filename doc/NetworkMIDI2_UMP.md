# Network MIDI 2.0 - UMP Base (Phase 1)

Phase 1 adds **Universal MIDI Packet (UMP)** support for Network MIDI 2.0. The implementation is **parallel** to AppleMIDI – no dependency on the RTP-MIDI code.

## Enabling

Uncomment in `src/NetworkMIDI2_Config.h`:
```c
#define NETWORK_MIDI_2_SUPPORT
```
Or add `-DNETWORK_MIDI_2_SUPPORT` to build flags.

## Files

| File | Purpose |
|------|---------|
| `NetworkMIDI2_Config.h` | Enable/disable switch |
| `ump_Defs.h` | UMP constants, Message Types, bit layouts |
| `ump_Convert.h` | Byte↔UMP conversion API |
| `ump_Convert.cpp` | Conversion implementation |

## Examples (standalone, no AppleMIDI)

- **UMP_Conversion** – Byte↔UMP conversion (Serial only)
- **ESP8266_NetworkMIDI2_mDNS** – mDNS `_midi2` + UMP (no transport yet)

## API

```c
// Byte stream -> UMP
ump_bytestream_parser_t parser;
ump_parser_init(&parser);
ump_byte_to_ump(&parser, byte, &ump, group);

// UMP -> bytes
ump_to_bytes(ump, bytes, maxlen);

// Direct build
ump_build_midi1_channel_voice(status, data1, data2, group);
```

## Supported

- **MT 2** (MIDI 1.0 Channel Voice): Note On/Off, CC, Program Change, etc.
- **MT 1** (System Real-Time): Timing Clock, Start, Stop, etc.

## References

- M2-104-UM: UMP and MIDI 2.0 Protocol
- M2-124-UM: Network MIDI 2.0 UDP Transport
