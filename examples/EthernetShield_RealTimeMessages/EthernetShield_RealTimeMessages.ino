#include <Ethernet.h>

#include "AppleMidi.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_DEFAULT_INSTANCE(); // see definition in AppleMidi_Defs.h

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print(F("Getting IP address..."));

  if (Ethernet.begin(mac) == 0) {
    Serial.println();
    Serial.println(F("Failed DHCP, check network cable & reboot"));
    for (;;)
      ;
  }

  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(Ethernet.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(Ethernet.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

  AppleMIDI.OnReceiveClock(OnAppleMidiClock);
  AppleMIDI.OnReceiveStart(OnAppleMidiStart);
  AppleMIDI.OnReceiveStop(OnAppleMidiStop);
  AppleMIDI.OnReceiveContinue(OnAppleMidiContinue);
  AppleMIDI.OnReceiveActiveSensing(OnAppleMidiActiveSensing);
  AppleMIDI.OnReceiveReset(OnAppleMidiReset);

  AppleMIDI.OnReceiveSongPosition(OnAppleMidiSongPosition);

  Serial.println(F("Listening for Clock events"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.read();
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected = false;
  Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOn from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOff from channel:"));
  Serial.print(channel);
  Serial.print(F(" note:"));
  Serial.print(note);
  Serial.print(F(" velocity:"));
  Serial.print(velocity);
  Serial.println();
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiClock() {
  Serial.println(F("Clock"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiStart() {
  Serial.println(F("Start"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiStop() {
  Serial.println(F("Stop"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiContinue() {
  Serial.println(F("Continue"));
}

// -----------------------------------------------------------------------------
// (https://www.midi.org/specifications/item/table-1-summary-of-midi-message)
// Active Sensing. 
// This message is intended to be sent repeatedly to tell the receiver that a 
// connection is alive. Use of this message is optional. When initially received, 
// the receiver will expect to receive another Active Sensing message each 300ms (max), 
// and if it does not then it will assume that the connection has been terminated. 
// At termination, the receiver will turn off all voices and return to normal 
// (non- active sensing) operation. 
// -----------------------------------------------------------------------------
static void OnAppleMidiActiveSensing() {
  Serial.println(F("ActiveSensing"));
  AppleMIDI.sendActiveSensing();
}

// -----------------------------------------------------------------------------
// (https://www.midi.org/specifications/item/table-1-summary-of-midi-message)
// Reset. 
// Reset all receivers in the system to power-up status. This should be used 
// sparingly, preferably under manual control. In particular, it should not be 
// sent on power-up.
// -----------------------------------------------------------------------------
static void OnAppleMidiReset() {
  Serial.println(F("Reset"));
}

// -----------------------------------------------------------------------------
// (https://www.midi.org/specifications/item/table-1-summary-of-midi-message)
// Song Position Pointer. 
// This is an internal 14 bit register that holds the number of MIDI beats 
// (1 beat= six MIDI clocks) since the start of the song. l is the LSB, m the MSB.
// -----------------------------------------------------------------------------
static void OnAppleMidiSongPosition(unsigned short a) {
  Serial.print  (F("SongPosition: "));
  Serial.println(a);
}
