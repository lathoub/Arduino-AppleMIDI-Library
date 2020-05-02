#include "ETH_Helper.h"

#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Booting");

  ETH_startup();

  MDNS.begin(AppleMIDI.getName());

  Serial.print("IP address is ");
  Serial.println(ETH.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(ETH.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  MIDI.begin(1); // listen on channel 1

  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI.setHandleError(OnAppleMidiError);

  MIDI.setHandleNoteOn(OnAppleMidiNoteOn);
  MIDI.setHandleNoteOff(OnAppleMidiNoteOff);

  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
  MDNS.addService("http", "tcp", 80);

  Serial.println(F("Every second send a random NoteOn/Off"));

  // Initiate the session
  IPAddress remote(192, 168, 1, 4);
  AppleMIDI.sendInvite(remote); // port is 5004 by default
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
    //   Serial.print(F(".");

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
void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
  isConnected = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
  isConnected = false;
  Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
  Serial.print  (F("Exception "));
  Serial.print  (err);
  Serial.print  (F(" from ssrc 0x"));
  Serial.println(ssrc, HEX);

  switch (err)
  {
    case Exception::NoResponseFromConnectionRequestException:
      Serial.println(F("xxx:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
      break;
  }
}

// -----------------------------------------------------------------------------
// 
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOn  from channel: "));
  Serial.print(channel);
  Serial.print(F(", note: "));
  Serial.print(note);
  Serial.print(F(", velocity: "));
  Serial.println(velocity);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOff from channel: "));
  Serial.print(channel);
  Serial.print(F(", note: "));
  Serial.print(note);
  Serial.print(F(", velocity: "));
  Serial.println(velocity);
}