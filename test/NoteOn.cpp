#include "Ethernet.h"

#define DEBUG 7

#include "../src/AppleMidi.h"

unsigned long t0 = millis();
bool isConnected = false;

byte sysex14[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0xF7 };
byte sysex15[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0x4D, 0xF7 };
byte sysex16[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x32, 0x50, 0x4D, 0xF7 };

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, const char* name) {
  isConnected = true;
  N_DEBUG_PRINT(F("Connected to session "));
  N_DEBUG_PRINTLN(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected = false;
  N_DEBUG_PRINTLN(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  N_DEBUG_PRINT(F("Incoming NoteOn from channel: "));
  N_DEBUG_PRINT(channel);
  N_DEBUG_PRINT(F(", note: "));
  N_DEBUG_PRINT(note);
  N_DEBUG_PRINT(F(", velocity: "));
  N_DEBUG_PRINTLN(velocity);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOff(byte channel, byte note, byte velocity) {
  N_DEBUG_PRINT(F("Incoming NoteOff from channel: "));
  N_DEBUG_PRINT(channel);
  N_DEBUG_PRINT(F(", note: "));
  N_DEBUG_PRINT(note);
  N_DEBUG_PRINT(F(", velocity: "));
  N_DEBUG_PRINTLN(velocity);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
char getSysExStatus(const byte* data, uint16_t length)
{
    if (data[0] == 0xF0 && data[length - 1] == 0xF7)
        return 'F'; // Full SysEx Command
    else if (data[0] == 0xF0 && data[length - 1] != 0xF7)
        return 'S'; // Start of SysEx-Segment
    else if (data[0] != 0xF0 && data[length - 1] != 0xF7)
        return 'M'; // Middle of SysEx-Segment
    else
        return 'E'; // End of SysEx-Segment
}

static void OnMidiSystemExclusive(byte* array, unsigned size) {
    N_DEBUG_PRINT(F("Incoming SysEx: "));
    N_DEBUG_PRINT(getSysExStatus(array, size));
    unsigned i = 0;
    for (; i < size - 1; i++)
    {
        N_DEBUG_PRINT(F(" 0x"));
        N_DEBUG_PRINT(array[i], HEX);
    }
    N_DEBUG_PRINT(F(" 0x"));
    N_DEBUG_PRINT(array[i], HEX);
    N_DEBUG_PRINTLN();
}

void begin()
{
    V_DEBUG_PRINTLN(F("OK, now make sure you an rtpMIDI session that is Enabled"));
    V_DEBUG_PRINT(F("Add device named Arduino with Host/Port "));
  //  V_DEBUG_PRINT(Ethernet.localIP());
    V_DEBUG_PRINTLN(F(":5004"));
    V_DEBUG_PRINTLN(F("Then press the Connect button"));
    V_DEBUG_PRINTLN(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

	MIDI.begin(1);
    
    AppleMIDI.setHandleConnected(OnAppleMidiConnected);
    AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

    MIDI.setHandleNoteOn(OnMidiNoteOn);
    MIDI.setHandleNoteOff(OnMidiNoteOff);
    MIDI.setHandleSystemExclusive(OnMidiSystemExclusive);
}

void loop()
{
	MIDI.read();
    // send a note every second
     // (dont cáll delay(1000) as it will stall the pipeline)
     if (isConnected && (millis() - t0) > 1000)
     {
       //MIDI.sendSysEx(sizeof(sysex14), sysex14, true);
       //MIDI.sendSysEx(sizeof(sysex15), sysex15, true);
       //MIDI.sendSysEx(sizeof(sysex16), sysex16, true);

       t0 = millis();
       //   Serial.print(F(".");

       byte note = random(1, 127);
       byte velocity = 55;
       byte channel = 1;

       MIDI.sendNoteOn(note, velocity, channel);
       MIDI.sendNoteOff(note, velocity, channel);

       MIDI.sendSysEx(sizeof(sysex16), sysex16, true);
     }
}

