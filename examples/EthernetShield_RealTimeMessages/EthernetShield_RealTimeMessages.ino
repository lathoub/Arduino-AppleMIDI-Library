#include <Ethernet.h>

#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

bool isConnected;

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

  Serial.print("IP address is ");
  Serial.println(Ethernet.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(Ethernet.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  MIDI.begin(1);

  // check: zien we de connecttion binnenkomen?? Anders terug een ref van maken
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

  MIDI.setHandleClock(OnMidiClock);
  MIDI.setHandleStart(OnMidiStart);
  MIDI.setHandleStop(OnMidiStop);
  MIDI.setHandleContinue(OnMidiContinue);
  MIDI.setHandleActiveSensing(OnMidiActiveSensing);
  MIDI.setHandleSystemReset(OnMidiSystemReset);
  MIDI.setHandleSongPosition(OnMidiSongPosition);

  Serial.println(F("Every second send a random NoteOn/Off"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI.read();
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
// rtpMIDI session. Error occorded during processing
// -----------------------------------------------------------------------------
void OnAppleMidiError(const ssrc_t & ssrc, int32_t errorCode) {
  Serial.println(F("ERROR"));
  exit(1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiClock() {
  Serial.println(F("Clock"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiStart() {
  Serial.println(F("Start"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiStop() {
  Serial.println(F("Stop"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiContinue() {
  Serial.println(F("Continue"));
}

// -----------------------------------------------------------------------------
// (https://www.midi.org/specifications/item/table-1-summary-of-midi-message)
// Active Sensing.
// This message is intended to be sent repeatedly to tell the receiver that a
// connection is alive. Use of this message is optional. When initially received,
// the receiver will expect to receive another Active Sensing message each 300ms (max),
// and if it does not then it will assume that the connection has been terminated.
// At termination, the receiver will turn off all voices and return to normal
// (non- active sensing) operation.
// -----------------------------------------------------------------------------
static void OnMidiActiveSensing() {
  Serial.println(F("ActiveSensing"));
}

// -----------------------------------------------------------------------------
// (https://www.midi.org/specifications/item/table-1-summary-of-midi-message)
// Reset.
// Reset all receivers in the system to power-up status. This should be used
// sparingly, preferably under manual control. In particular, it should not be
// sent on power-up.
// -----------------------------------------------------------------------------
static void OnMidiSystemReset() {
  Serial.println(F("SystemReset"));
}

// -----------------------------------------------------------------------------
// (https://www.midi.org/specifications/item/table-1-summary-of-midi-message)
// Song Position Pointer.
// This is an internal 14 bit register that holds the number of MIDI beats
// (1 beat= six MIDI clocks) since the start of the song. l is the LSB, m the MSB.
// -----------------------------------------------------------------------------
static void OnMidiSongPosition(unsigned a) {
  Serial.print (F("SongPosition: "));
  Serial.println(a);
}
