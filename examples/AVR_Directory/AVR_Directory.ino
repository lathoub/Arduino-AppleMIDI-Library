#include <Ethernet.h>

#define SerialMon Serial
#define USE_DIRECTORY
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

  AppleMIDI.directory.push_back(IPAddress(192, 168, 1, 63));
  AppleMIDI.directory.push_back(IPAddress(192, 168, 1, 66));
//  AppleMIDI.whoCanConnectToMe = APPLEMIDI_NAMESPACE::None;
  AppleMIDI.whoCanConnectToMe = APPLEMIDI_NAMESPACE::OnlyComputersInMyDirectory;
//  AppleMIDI.whoCanConnectToMe = APPLEMIDI_NAMESPACE::Anyone;

  AM_DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  AM_DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  AM_DBG(F("Select and then press the Connect button"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));

  MIDI.begin();

  // Stay informed on connection status
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

  AM_DBG(F("Sending MIDI messages every second"));
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
  if ((isConnected > 0) && (millis() - t1) > 1000)
  {
    t1 = millis();

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    MIDI.sendNoteOn(note, velocity, channel);
    //    MIDI.sendNoteOff(note, velocity, channel);
  }
}
