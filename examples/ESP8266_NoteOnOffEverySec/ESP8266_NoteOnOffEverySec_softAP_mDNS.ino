// Example to start ESP8266 in soft access point / hotspot mode
// and also enable mDNS response on local network. This allows
// client to discover the AppleMIDI service and connect to it
// without having to type the IP address and port
// Tested on iOS 9 (old iPad) and iOS 13 (iPhone 6)
// On Win10 (rtpMIDI), ESP8266 did not show in directory,
// but connects fine with default IP(192.168.4.1)/port(5004)

#include <ESP8266WiFi.h>
#include <ESP8266mDNS.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

char ssid[] = "ssid"; //  your network SSID (name)
char pass[] = "password";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  DBG_SETUP(115200);
  DBG("Booting");

  WiFi.softAP(ssid, pass);

  DBG(F("Started soft access point:"), WiFi.softAPIP(), "Port", AppleMIDI.getPort());
  DBG(F("AppleMIDI device name:"), AppleMIDI.getName());
  // Set up mDNS responder:
  if (!MDNS.begin(AppleMIDI.getName()))
    DBG(F("Error setting up MDNS responder!"));
  char str[128] = "";
  strcat(str, AppleMIDI.getName());
  strcat(str,".local");
  DBG(F("mDNS responder started at:"), str);
  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
  DBG(F("Open Wifi settings and connect to soft acess point using 'ssid'"));
  DBG(F("Start MIDI Network app on iPhone/iPad or rtpMIDI on Windows"));
  DBG(F("AppleMIDI-ESP8266 will show in the 'Directory' list (rtpMIDI) or"));
  DBG(F("under 'Found on the network' list (iOS). Select and click 'Connect'"));
  
  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });
  
  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOff"), note);
  });

  DBG(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{

  MDNS.update();
  
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