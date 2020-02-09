#include <Ethernet.h>

#define DEBUG 7
#include <AppleMidi.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
bool isConnected;

byte sysex14[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0xF7 };
byte sysex15[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0x4D, 0xF7 };
byte sysex16[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x32, 0x50, 0x4D, 0xF7 };

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  DEBUG_BEGIN(115200);

  N_DEBUG_PRINTLN(F("Getting IP address..."));

  if (Ethernet.begin(mac) == 0) {
    F_DEBUG_PRINTLN(F("Failed DHCP, check network cable & reboot"));
    for (;;);
  }

  N_DEBUG_PRINT("IP address is ");
  N_DEBUG_PRINTLN(Ethernet.localIP());

  V_DEBUG_PRINTLN(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  V_DEBUG_PRINT(F("Add device named Arduino with Host/Port "));
  V_DEBUG_PRINT(Ethernet.localIP());
  V_DEBUG_PRINTLN(F(":5004"));
  V_DEBUG_PRINTLN(F("Then press the Connect button"));
  V_DEBUG_PRINTLN(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  MIDI.begin(1);

  // check: zien we de connecttion binnenkomen?? Anders terug een ref van makenDw
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI.setHandleError(OnAppleMidiError);

  MIDI.setHandleSystemExclusive(OnMidiSysEx);

  N_DEBUG_PRINTLN(F("Every second send a random NoteOn/Off"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI.read();

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  if (isConnected && (millis() - t1) > 150)
  {
       MIDI.sendSysEx(sizeof(sysex16), sysex16, true);
       t1 = millis();
  }
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

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
// rtpMIDI session. Error occorded during processing
// -----------------------------------------------------------------------------
void OnAppleMidiError(uint32_t ssrc, uint32_t errorCode) {
  N_DEBUG_PRINTLN(F("ERROR"));
  exit(1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnMidiSysEx(byte* data, unsigned length) {
  N_DEBUG_PRINT(F("SYSEX: ("));
  N_DEBUG_PRINT(getSysExStatus(data, length));
  N_DEBUG_PRINT(F(", "));
  N_DEBUG_PRINT(length);
  N_DEBUG_PRINT(F(" bytes) "));
  for (uint16_t i = 0; i < length; i++)
  {
    N_DEBUG_PRINT(data[i], HEX);
    N_DEBUG_PRINT(" ");
  }
  N_DEBUG_PRINTLN();
}

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
