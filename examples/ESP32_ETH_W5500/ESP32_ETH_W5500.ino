#define SerialMon Serial
#include <AppleMIDI.h>

unsigned long t1 = millis();
int8_t isAppleMIDIConnected = 0;

#include "ETH_Helper.h"

APPLEMIDI_CREATE_INSTANCE(NetworkUDP, MIDI, "AppleMIDI-Arduino", DEFAULT_CONTROL_PORT);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup() {
  AM_DBG_SETUP(115200);
  AM_DBG(F("\nDas Booting"));

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, const char* name) {
    isAppleMIDIConnected++;
    AM_DBG(F("AppleMIDI Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
    isAppleMIDIConnected--;
    AM_DBG(F("AppleMIDI Disconnected"), ssrc);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    AM_DBG(F("MIDI NoteOn"), channel, note, velocity);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    AM_DBG(F("MIDI NoteOff"), channel, note, velocity);
  });

  ETH_startup([](bool connected) {
    if (connected) {
      AM_DBG(F("Start MIDI"));
      MIDI.begin(MIDI_CHANNEL_OMNI);
    } else {
      isAppleMIDIConnected = 0;
      //MIDI.???
    }
  });
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop() {
  // Listen to incoming notes
  if (isETHconnected)
    MIDI.read();

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  if ((isAppleMIDIConnected > 0) && (millis() - t1) > 100) {
    t1 = millis();

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    //   AM_DBG(F("\nsendNoteOn"), note, velocity, channel);
    MIDI.sendNoteOn(note, velocity, channel);
    //MIDI.sendNoteOff(note, velocity, channel);
  }
}
