#include <Ethernet.h>
#include <AppleMidi.h>

#include <ArduinoLog.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t0 = millis();
bool isConnected = false;

byte sysex14[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0xF7 };
byte sysex15[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x50, 0x4D, 0xF7 };
byte sysex16[] = { 0xF0, 0x43, 0x20, 0x7E, 0x4C, 0x4D, 0x20, 0x20, 0x38, 0x39, 0x37, 0x33, 0x32, 0x50, 0x4D, 0xF7 };

APPLEMIDI_CREATE_DEFAULT_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial);

  Log.begin(LOG_LEVEL_NOTICE, &Serial);

  Log.notice("Getting IP address..." CR);

  if (Ethernet.begin(mac) == 0) {
    Log.notice(F(CR "Failed DHCP, check network cable & reboot" CR));
    for (;;);
  }

  Log.notice("IP address is %s" CR, Ethernet.localIP());

  Log.notice("OK, now make sure you an rtpMIDI session that is Enabled" CR);
  Log.notice("Add device named Arduino with Host/Port %s:5004" CR, Ethernet.localIP());
  Log.notice("Then press the Connect button" CR);
  Log.notice("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes" CR );

  // Create a session and wait for a remote host to connect to us
  MIDI.begin(1);

  // check: zien we de connecttion binnenkomen?? Anders terug een ref van maken
  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);

  MIDI.setHandleNoteOn(OnAppleMidiNoteOn);
  MIDI.setHandleNoteOff(OnAppleMidiNoteOff);

  Log.notice(F("Sending NoteOn/Off of note 45, every second" CR));
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
    //MIDI.sendSysEx(sizeof(sysex14), sysex14, true);
    //MIDI.sendSysEx(sizeof(sysex15), sysex15, true);
    //MIDI.sendSysEx(sizeof(sysex16), sysex16, true);

    t0 = millis();
    //   Serial.print(F(".");

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    //   MIDI.sendNoteOn(note, velocity, channel);
    //   MIDI.sendNoteOff(note, velocity, channel);

    //MIDI.sendSysEx(sizeof(se), se, true);
  }
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, const char* name) {
  isConnected = true;
  Log.notice(F("Connected to session %s" CR), name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected = false;
  Log.notice(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOn(byte channel, byte note, byte velocity) {
  Log.notice(F("Incoming NoteOn from channel: %d note: %d velocity: %d" CR), channel, note, velocity);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiNoteOff(byte channel, byte note, byte velocity) {
  Log.notice(F("Incoming NoteOff from channel: %d note: %d velocity: %d" CR), channel, note, velocity);
}