#include <Ethernet.h>

#define APPLEMIDI_INITIATOR
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
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Listen for MIDI messages on channel 1
  MIDI.begin(1);

  // Stay informed on connection status
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI.setHandleError(OnAppleMidiError);

  // and let us know ehen notes come in
  MIDI.setHandleNoteOn(OnMidiNoteOn);
  MIDI.setHandleNoteOff(OnMidiNoteOff);

  // Initiate the session
  IPAddress remote(192, 168, 1, 4);
  AppleMIDI.sendInvite(remote); // port is 5004 by default

  Serial.println(F("Every second send a random NoteOn/Off"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI.read();

  // send note on/off every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  if (isConnected && (millis() - t1) > 1000)
  {
    t1 = millis();
    //   Serial.print(F(".");

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

 //   MIDI.sendNoteOn(note, velocity, channel);
 //   MIDI.sendNoteOff(note, velocity, channel);
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
  Serial.print(F(" ssrc: 0x"));
  Serial.println(ssrc, HEX);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
  isConnected = false;
  Serial.print  (F("Disconnected from ssrc 0x"));
  Serial.println(ssrc, HEX);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
  Serial.print  (F("Exception "));
  Serial.println(err);
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
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
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
static void OnMidiNoteOff(byte channel, byte note, byte velocity) {
  Serial.print(F("Incoming NoteOff from channel: "));
  Serial.print(channel);
  Serial.print(F(", note: "));
  Serial.print(note);
  Serial.print(F(", velocity: "));
  Serial.println(velocity);
}