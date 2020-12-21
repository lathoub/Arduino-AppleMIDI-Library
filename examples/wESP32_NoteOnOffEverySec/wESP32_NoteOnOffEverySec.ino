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

  if (!MDNS.begin(AppleMIDI.getName()))
    DBG(F("Error setting up MDNS responder"));

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), ETH.localIP(), "Port", AppleMIDI.getPort());
  DBG(F("The device should also be visible in the directory as"), AppleMIDI.getName());
  DBG(F("Select and then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));

  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected = true;
    DBG(F("Connected to session"), name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected = false;
    DBG(F("Disconnected"));
  });
  AppleMIDI.setHandleException([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const APPLEMIDI_NAMESPACE::Exception & e, const int32_t value) {
    DBG(F("______________Exception"), e, value);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOff"), note);
  });

  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
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
    // MIDI.sendNoteOff(note, velocity, channel);
  }
}
