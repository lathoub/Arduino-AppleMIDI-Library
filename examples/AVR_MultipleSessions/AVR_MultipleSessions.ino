#include <Ethernet.h>

#define SerialMon Serial
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI1, "Arduino1", DEFAULT_CONTROL_PORT);
APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI2, "Arduino2", DEFAULT_CONTROL_PORT + 2);

void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t&, const char*);
void OnAppleMidiDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t &);
void OnMidiNoteOn(byte channel, byte note, byte velocity);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  AM_DBG_SETUP(115200);
  AM_DBG(F("Booting"));

  if (Ethernet.begin(mac) == 0) {
    AM_DBG(F("Failed DHCP, check network cable & reboot"));
    for (;;);
  }

  AM_DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  AM_DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI1.getPort(), "(Name", AppleMIDI1.getName(), ")");
  AM_DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI2.getPort(), "(Name", AppleMIDI2.getName(), ")");
  AM_DBG(F("Select and then press the Connect button"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));

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

  AM_DBG(F("Every second, send a random NoteOn/Off, from multiple sessions"));
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
  if ((isConnected > 0) && (millis() - t1) > 1000)
  {
    t1 = millis();

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
void OnAppleMidiConnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
  isConnected++;
  AM_DBG(F("Connected to session"), ssrc, name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  isConnected--;
  AM_DBG(F("Disconnected"), ssrc);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  AM_DBG(F("in\tNote on"), note, " Velocity", velocity, "\t", channel);
}
