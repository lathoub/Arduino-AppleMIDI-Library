#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI

bool isConnected = false;

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

void TaskAppleMIDIcode(void* pvParameters)
{
  APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();

  Serial.print("AppleMIDI running on core ");
  Serial.println(xPortGetCoreID());

  Serial.println(F("Every second send a random NoteOn/Off"));

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(ETH.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

  MIDI.setHandleNoteOn(OnAppleMidiNoteOn);
  MIDI.setHandleNoteOff(OnAppleMidiNoteOff);

  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());

  MIDI.begin(1); // listen on channel 1

  auto t0 = millis();
  while (true)
  {
    // Listen to incoming notes
    MIDI.read();

    // send a note every second
    // (dont cáll delay(1000) as it will stall the pipeline)
    if ((millis() - t0) > 1000)
    {
      t0 = millis();

      if (isConnected)
      {
        byte note = random(1, 127);
        byte velocity = 55;
        byte channel = 1;

        Serial.println("Sending notes");

        MIDI.sendNoteOn(note, velocity, channel);
        MIDI.sendNoteOff(note, velocity, channel);
      }
    }
  }
}
