#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

#include "ETH_Helper.h"

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  DBG_SETUP(115200);
  DBG("Booting");

  ETH_startup();

  MDNS.begin(AppleMIDI.getName());

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(F("Then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));
  DBG(F("Listen to incoming MIDI commands"));

  MIDI.begin();

  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

  MIDI.setHandleNoteOn(OnMidiNoteOn);
  MIDI.setHandleNoteOff(OnMidiNoteOff);

  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
  MDNS.addService("http", "tcp", 80);

  DBG(F("Every second send a random NoteOn/Off"));
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
  if (isConnected && (millis() - t0) > 1000)
  {
    t0 = millis();

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    MIDI.sendNoteOn(note, velocity, channel);
    MIDI.sendNoteOff(note, velocity, channel);
  }
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
  isConnected = true;
  DBG(F("Connected to session"), name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  isConnected = false;
  DBG(F("Disconnected"));
}

// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  DBG(F("in\tNote on"), note, " Velocity", velocity, "\t", channel);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOff(byte channel, byte note, byte velocity) {
  DBG(F("in\tNote off"), note, " Velocity", velocity, "\t", channel);
}
