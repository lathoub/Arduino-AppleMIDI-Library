#include "Ethernet.h"

#define DEBUG 7
#define APPLEMIDI_INITIATOR

#include "AppleMIDI.h"

unsigned long t0 = millis();
bool isConnected = false;

byte sysex14[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0xF7 };
byte sysex15[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0x4D, 0xF7 };
byte sysex16[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x32, 0x50, 0x4D, 0xF7 };
byte sysexBig[] = { 0xF0, 0x41,
                           0x20, 0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, 0x29,
                           0x30, 0x31, 0x32, 0x33, 0x34, 0x35, 0x36, 0x37, 0x38, 0x39,
                           0x40, 0x41, 0x42, 0x43, 0x44, 0x45, 0x46, 0x47, 0x48, 0x49,
                           0x50, 0x51, 0x52, 0x53, 0x54, 0x55, 0x56, 0x57, 0x58, 0x59,
                           0x60, 0x61, 0x62, 0x63, 0x64, 0x65, 0x66, 0x67, 0x68, 0x69,
                           0x70, 0x71, 0x72, 0x73, 0x74, 0x75, 0x76, 0x77, 0x78, 0x79,
                           0x80,
    
                           0x81, 0x82, 0x83, 0x84, 0x85, 0x86, 0x87, 0x88, 0x89,
                           0x90, 0x91, 0x92, 0x93, 0x94, 0x95, 0x96, 0x97, 0x98, 0x99,
                           0xa0, 0xa1, 0xa2, 0xa3, 0xa4, 0xa5, 0xa6, 0xa7, 0xa8, 0xa9,
                           0xb0, 0xb1, 0xb2, 0xb3, 0xb4, 0xb5, 0xb6, 0xb7, 0xb8, 0xb9,
                           0xc0, 0xc1, 0xc2, 0xc3, 0xc4, 0xc5, 0xc6, 0xc7, 0xc8,
                    0xF7 };

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
  isConnected = true;
  AM_DBG(F("Connected to session"), ssrc, name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  isConnected = false;
  AM_DBG(F("Disconnected"), ssrc);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiByte(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, byte data) {
  AM_DBG(F("MIDI: "));
  AM_DBG(data);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  AM_DBG(F("in\tNote on"), note, " Velocity", velocity, "\t", channel);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOff(byte channel, byte note, byte velocity) {
  AM_DBG(F("in\tNote off"), note, " Velocity", velocity, "\t", channel);
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
    AM_DBG(F("Incoming SysEx: "));
    AM_DBG(getSysExStatus(array, size));
    unsigned i = 0;
    for (; i < size - 1; i++)
    {
        AM_DBG(F(" 0x"));
        AM_DBG(array[i], HEX);
    }
    AM_DBG(F(" 0x"));
    AM_DBG(array[i], HEX);
    AM_DBG();
}

void begin()
{
  AM_DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  AM_DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  AM_DBG(F("Select and then press the Connect button"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));

	MIDI.begin(MIDI_CHANNEL_OMNI);
    
    AppleMIDI.setHandleConnected(OnAppleMidiConnected);
    AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
 //   AppleMIDI.setHandleReceivedMidi(OnAppleMidiByte);
    
    MIDI.setHandleNoteOn(OnMidiNoteOn);
    MIDI.setHandleNoteOff(OnMidiNoteOff);
    MIDI.setHandleSystemExclusive(OnMidiSystemExclusive);
    
    IPAddress remote(192, 168, 1, 156);
  //  AppleMIDI.sendInvite(remote);
}

void loop()
{
	MIDI.read();
    
    // send a note every second
     // (dont cáll delay(1000) as it will stall the pipeline)
     if ((isConnected) && (millis() - t0) > 10000)
     {
       t0 = millis();

       byte note = random(1, 127);
       byte velocity = 55;
       byte channel = 1;

       MIDI.sendNoteOn(note, velocity, channel);
       MIDI.sendNoteOff(note, velocity, channel);

     }
 //   MIDI.sendSysEx(sizeof(sysexBig), sysexBig, true);
}
