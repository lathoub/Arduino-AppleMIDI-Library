#include <Ethernet.h>

#define SerialMon Serial
#define APPLEMIDI_INITIATOR
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

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

  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    AM_DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    AM_DBG(F("Disconnected"), ssrc);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    AM_DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    AM_DBG(F("NoteOff"), note);
  });

  // Initiate the session
  IPAddress remote(192, 168, 1, 65);
  AppleMIDI.sendInvite(remote, DEFAULT_CONTROL_PORT); // port is 5004 by default

  AM_DBG(F("Connecting to "), remote, "Port", DEFAULT_CONTROL_PORT, "(Name", AppleMIDI.getName(), ")");
  AM_DBG(F("Watch as this session is added to the Participants list"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));

  AM_DBG(F("Sending a random NoteOn/Off every second"));
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
  if ((isConnected > 0) && (millis() - t1) > 1000)
  {
    t1 = millis();

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    MIDI.sendNoteOn(note, velocity, channel);
    MIDI.sendNoteOff(note, velocity, channel);
  }
}
