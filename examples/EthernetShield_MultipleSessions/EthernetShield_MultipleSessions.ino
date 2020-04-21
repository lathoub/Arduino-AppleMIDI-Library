#include <Ethernet.h>

#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI1, "Arduino1", DEFAULT_CONTROL_PORT);
APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI2, "Arduino2", DEFAULT_CONTROL_PORT + 2);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  Serial.begin(115200);
  while (!Serial);
  Serial.println("Booting");

  Serial.println(F("Getting IP address..."));

  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed DHCP, check network cable & reboot"));
    for (;;);
  }

  Serial.print(F("IP address is "));
  Serial.println(Ethernet.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(Ethernet.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Listen for MIDI messages on channel 1
  MIDI1.begin(1);
  MIDI2.begin(2);

  // Stay informed on connection status
  AppleMIDI1.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI1.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI2.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI2.setHandleDisconnected(OnAppleMidiDisconnected);

  // and let us know ehen notes come in
  MIDI1.setHandleNoteOn(OnMidiNoteOn);
  MIDI2.setHandleNoteOn(OnMidiNoteOn);

  Serial.println(F("Every second send a random NoteOn/Off"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI1.read();
  MIDI2.read();

  // send note on/off every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  if (isConnected && (millis() - t1) > 1000)
  {
    t1 = millis();
    //   Serial.print(F(".");

    byte note = random(1, 127);
    byte velocity = 55;

    MIDI1.sendNoteOn(note, velocity, 1);
    MIDI2.sendNoteOn(note, velocity, 2);
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
  Serial.print(name);
  Serial.print(" ssrc 0x");
  Serial.println(ssrc, HEX);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
  isConnected = false;
  Serial.print(F("Disconnected from ssrc 0x"));
  Serial.println(ssrc, HEX);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOn  from channel: "));
  Serial.print(channel);
  Serial.print(F(", note: "));
  Serial.print(note);
  Serial.print(F(", velocity: "));
  Serial.println(velocity);
}
