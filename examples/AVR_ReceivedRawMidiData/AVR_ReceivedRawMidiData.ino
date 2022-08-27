#include <Ethernet.h>

#define SerialMon Serial
#define USE_EXT_CALLBACKS
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

void OnAppleMidiStartReceived(const APPLEMIDI_NAMESPACE::ssrc_t&);
void OnAppleMidiReceivedByte(const APPLEMIDI_NAMESPACE::ssrc_t&, byte);
void OnAppleMidiEndReceive(const APPLEMIDI_NAMESPACE::ssrc_t&);

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
  AM_DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  AM_DBG(F("Select and then press the Connect button"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));

  MIDI.begin();

  // check: zien we de connecttion binnenkomen?? Anders terug een ref van maken
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    AM_DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    AM_DBG(F("Disconnected"), ssrc);
  });
  AppleMIDI.setHandleStartReceivedMidi(OnAppleMidiStartReceived);
  AppleMIDI.setHandleReceivedMidi(OnAppleMidiReceivedByte);
  AppleMIDI.setHandleEndReceivedMidi(OnAppleMidiEndReceive);

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    AM_DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    AM_DBG(F("NoteOff"), note);
  });
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
//
// -----------------------------------------------------------------------------
void OnAppleMidiStartReceived(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  AM_DBG(F("Start receiving"), ssrc);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiReceivedByte(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, byte data) {
  SerialMon.println(data, HEX);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiEndReceive(const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
  AM_DBG(F("End receiving"), ssrc);
}
