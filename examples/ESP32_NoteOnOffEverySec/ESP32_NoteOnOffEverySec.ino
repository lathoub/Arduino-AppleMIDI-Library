#include <WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>
#include <ESPmDNS.h>

#define SerialMon Serial
#include <AppleMIDI.h>

char ssid[] = "Het internet ga weer nie"; //  your network SSID (name)
char pass[] = "MEEN_con2door@nusm";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  AM_DBG_SETUP(115200);
  AM_DBG(F("Booting"));

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    AM_DBG(F("Establishing connection to WiFi.."));
  }
  AM_DBG(F("Connected to network"));

  AM_DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  AM_DBG(F("Add device named Arduino with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  AM_DBG(F("Select and then press the Connect button"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes"));
  AM_DBG(F("Listen to incoming MIDI commands"));

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

  // Set up mDNS responder:
  // - first argument is the domain name, in this example
  //   the fully-qualified domain name is "esp32.local"
  // - second argument is the IP address to advertise
  //   we send our IP address on the WiFi network
  if (!MDNS.begin("esp32")) {
    AM_DBG("Error setting up MDNS responder!");
    while (1) {
      delay(1000);
    }
  }
  AM_DBG("mDNS responder started");

  // Add service to MDNS-SD
  MDNS.addService("apple-midi", "udp", 5004);

  AM_DBG(F("Sending NoteOn/Off of note 45, every second"));
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
  if ((isConnected > 0) && (millis() - t0) > 1000)
  {
    t0 = millis();

    byte note = 45;
    byte velocity = 55;
    byte channel = 1;

    MIDI.sendNoteOn(note, velocity, channel);
    MIDI.sendNoteOff(note, velocity, channel);
  }
}
