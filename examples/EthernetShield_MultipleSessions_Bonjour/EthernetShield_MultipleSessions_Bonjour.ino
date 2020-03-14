#include <Ethernet.h>
#include <EthernetBonjour.h>

#define DEBUG 4
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
bool isConnected = false;

typedef APPLEMIDI_NAMESPACE::AppleMIDISession<EthernetUDP> AppleMIDISession_t;
AppleMIDISession_t Session1("Session 1", 5004);
AppleMIDISession_t Session2("Session 2", 5006);
MIDI_NAMESPACE::MidiInterface<AppleMIDISession_t> MIDI1((AppleMIDISession_t &)Session1);
MIDI_NAMESPACE::MidiInterface<AppleMIDISession_t> MIDI2((AppleMIDISession_t &)Session2);

USING_NAMESPACE_APPLEMIDI

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  DEBUG_BEGIN(115200);

  N_DEBUG_PRINTLN(F("Getting IP address..."));

  if (Ethernet.begin(mac) == 0) {
    F_DEBUG_PRINTLN(F("Failed DHCP, check network cable & reboot"));
    for (;;);
  }

  N_DEBUG_PRINT(F("IP address is "));
  N_DEBUG_PRINTLN(Ethernet.localIP());

  V_DEBUG_PRINTLN(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  V_DEBUG_PRINT(F("Add device named Arduino with Host/Port "));
  V_DEBUG_PRINT(Ethernet.localIP());
  V_DEBUG_PRINTLN(F(":5004"));
  V_DEBUG_PRINTLN(F("Then press the Connect button"));
  V_DEBUG_PRINTLN(F("Then open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Listen for MIDI messages on channel 1
  MIDI1.begin(1);
  MIDI2.begin(2);

   // Initialize the Bonjour/MDNS library. You can now reach or ping this
  // Arduino via the host name "arduino.local", provided that your operating
  // system is Bonjour-enabled (such as MacOS X).
  // Always call this before any other method!
  EthernetBonjour.begin("arduino");

  EthernetBonjour.addServiceRecord("Arduino_Session1._apple-midi",
                                   Session1.getPort(),
                                   MDNSServiceUDP);
                                   
  EthernetBonjour.addServiceRecord("Arduino_Session2._apple-midi",
                                   Session2.getPort(),
                                   MDNSServiceUDP);

  // Stay informed on connection status
  Session1.setHandleConnected(OnAppleMidiConnected);
  Session1.setHandleDisconnected(OnAppleMidiDisconnected);
  Session1.setHandleError(OnAppleMidiError);
  Session2.setHandleConnected(OnAppleMidiConnected);
  Session2.setHandleDisconnected(OnAppleMidiDisconnected);
  Session2.setHandleError(OnAppleMidiError);

  // and let us know ehen notes come in
  MIDI1.setHandleNoteOn(OnMidiNoteOn);
  MIDI2.setHandleNoteOn(OnMidiNoteOn);

  N_DEBUG_PRINTLN(F("Every second send a random NoteOn/Off"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  EthernetBonjour.run();

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
void OnAppleMidiConnected(ssrc_t ssrc, const char* name) {
  isConnected = true;
  N_DEBUG_PRINT(F("Connected to session "));
  N_DEBUG_PRINT(name);
  N_DEBUG_PRINT(" ssrc 0x");
  N_DEBUG_PRINTLN(ssrc, HEX);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(ssrc_t ssrc) {
  isConnected = false;
  N_DEBUG_PRINT(F("Disconnected from ssrc 0x"));
  N_DEBUG_PRINTLN(ssrc, HEX);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Error occorded during processing
// -----------------------------------------------------------------------------
void OnAppleMidiError(ssrc_t ssrc, int32_t errorCode) {
  N_DEBUG_PRINTLN(F("ERROR"));
  exit(1);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  N_DEBUG_PRINT(F("Incoming NoteOn  from channel: "));
  N_DEBUG_PRINT(channel);
  N_DEBUG_PRINT(F(", note: "));
  N_DEBUG_PRINT(note);
  N_DEBUG_PRINT(F(", velocity: "));
  N_DEBUG_PRINTLN(velocity);
}
