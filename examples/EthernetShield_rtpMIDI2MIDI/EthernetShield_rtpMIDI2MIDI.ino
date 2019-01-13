#include <Ethernet.h>

#include "MIDI.h" // dependency: https://github.com/FortySevenEffects/arduino_midi_library 
#include "AppleMidi.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t0 = millis();
bool isConnected = false;

// Created and binds the rtpMIDI interface to the default port and EthernetUdp implementation
APPLEMIDI_CREATE_DEFAULT_INSTANCE();
// Created and binds the MIDI interface to the default hardware Serial port
MIDI_CREATE_DEFAULT_INSTANCE();

// Code size:
//
// IDE 1.8.7
//
// Arduino Ethernet / Uno
// Sketch uses 26192 bytes (81%) of program storage space. Maximum is 32256 bytes.
// Global variables use 1425 bytes (69%) of dynamic memory, leaving 623 bytes for local variables. Maximum is 2048 bytes.

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
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX on Windows) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveNoteOn(OnAppleMidiNoteOn);
  AppleMIDI.OnReceiveNoteOff(OnAppleMidiNoteOff);

  MIDI.begin(MIDI_CHANNEL_OMNI);  // Listen to all incoming messages

  Serial.println(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.read();

  // Read incoming messages
  MIDI.read();
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
  Serial.println(F("received noteOn via rtp, sending via serial MIDI"));
  MIDI.sendNoteOn(note, velocity, channel);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.println(F("received noteOff via rtp, sending via serial MIDI"));
  MIDI.sendNoteOff(note, velocity, channel);
}
