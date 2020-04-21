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

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

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

  // Listen for MIDI messages on channel 1
  MIDI.begin(1);

  // Stay informed on connection status
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

  Serial.println(F("Send MIDI messages to this session and see the latency on the Serial Monitor"));
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
  if (isConnected && (millis() - t1) > 1000)
  {
    t1 = millis();
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
