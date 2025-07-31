#include <WiFi.h>

#include "ETH_Helper.h"       

#define SerialMon Serial
#include "midiHelpers.h"


#define ethernet true;        // set to false to demonstrate WiFi usage

char ssid[] = "ssid"; //  your network SSID (name)
char pass[] = "pass";    // your network password (use for WPA, or use as key for WEP)

MidiClient* midiClient;     // generic class offered as an alternative of the MACRO

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t&, const APPLEMIDI_NAMESPACE::Exception&, const int32_t);   

void WIFI_startup(){
  WiFi.begin(ssid, pass);
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    AM_DBG(F("Establishing connection to WiFi.."));
  }
}

void setup(){

  AM_DBG_SETUP(115200);

  bool useEth = false;

  if (useEth){
    ETH_startup();
    midiClient = new AppleMidiWithInterfaceWrapper<EthernetUDP>("APPLE_MIDIETHCLIENT",DEFAULT_CONTROL_PORT);
  }else{
    WIFI_startup();
    midiClient = new AppleMidiWithInterfaceWrapper<WiFiUDP>("APPLE_MIDIWIFICLIENT",DEFAULT_CONTROL_PORT);             
  }

  midiClient->begin();

  midiClient->setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    AM_DBG(F("Connected to session"), ssrc, name);
  });
  midiClient->setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    AM_DBG(F("Disconnected"), ssrc);
  });

  AM_DBG(F("OK, now make sure you have an rtpMIDI session that is Enabled"));
  AM_DBG(F("Add device named Arduino with Host"), useEth?Ethernet.localIP():WiFi.localIP(), "Port", midiClient->getPort(), "(Name", midiClient->getName(), ")");
  AM_DBG(F("Select and then press the Connect button"));
  AM_DBG(F("Then open a MIDI listener and monitor incoming notes")); 
}

void loop(){
  midiClient->read();
}