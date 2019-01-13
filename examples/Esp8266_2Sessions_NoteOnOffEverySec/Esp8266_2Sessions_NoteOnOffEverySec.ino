// These need to be included when using standard Ethernet
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <WiFiUdp.h>

#include "AppleMidi.h"

char ssid[] = "xxx"; //  your network SSID (name)
char pass[] = "yyy";    // your network password (use for WPA, or use as key for WEP)

unsigned long t0 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_INSTANCE(WiFiUDP, appleMIDI1);
APPLEMIDI_CREATE_INSTANCE(WiFiUDP, appleMIDI2);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  // Serial communications and wait for port to open:
  Serial.begin(115200);
  while (!Serial) {
    ; // wait for serial port to connect. Needed for Leonardo only
  }

  Serial.print(F("Getting IP address..."));

  WiFi.begin(ssid, pass);

  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(F("."));
  }
  Serial.println(F(""));
  Serial.println(F("WiFi connected"));

  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(WiFi.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add peer named 'session1' with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.print(F("and add another peer named 'session2' with Host/Port "));
  Serial.print(WiFi.localIP());
  Serial.println(F(":5006"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Now open a MIDI listener (eg MIDI-OX) and monitor incoming notes"));

  // Create a session and wait for a remote host to connect to us
  appleMIDI1.begin("session1", 5004);
  appleMIDI2.begin("session2", 5006);

  Serial.println(F("Sending NoteOn/Off of note 45, every second"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  appleMIDI1.read();
  appleMIDI2.read();

  // send a note every second
  // (dont cáll delay(1000) as it will stall the pipeline)
  if (true && (millis() - t0) > 1000)
  {
    t0 = millis();
    //   Serial.print(".");

    byte note = 45;
    byte velocity = 55;
    byte channel = 1;

    appleMIDI1.sendNoteOn(note, velocity, channel);
    appleMIDI1.sendNoteOff(note, velocity, channel);

    appleMIDI2.sendNoteOn(note + 1, velocity, channel);
    appleMIDI2.sendNoteOff(note + 1, velocity, channel);
  }

}

