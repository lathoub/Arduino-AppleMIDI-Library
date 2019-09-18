#pragma once

#pragma once

#include "appleMidi_Defs.h"

BEGIN_MIDI_NAMESPACE

// Max size of dissectable packet
#define PACKET_MAX_SIZE 350

// Rtp UDP ports
#define CONTROL_PORT 5004

#define MIDI_SYSEX_ARRAY_SIZE 255

#define MIDI_SYSEX_ARRAY_SIZE_CONTENT (MIDI_SYSEX_ARRAY_SIZE - 2)

END_MIDI_NAMESPACE