#include <Ethernet.h>

#include "AppleMidi.h"

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

bool isConnected = false;

APPLEMIDI_CREATE_DEFAULT_INSTANCE(); // see definition in AppleMidi_Defs.h

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

  if (Ethernet.begin(mac) == 0) {
    Serial.println();
    Serial.println(F("Failed DHCP, check network cable & reboot"));
    for (;;)
      ;
  }

  Serial.println();
  Serial.print(F("IP address is "));
  Serial.println(Ethernet.localIP());

  Serial.println(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  Serial.print(F("Add device named Arduino with Host/Port "));
  Serial.print(Ethernet.localIP());
  Serial.println(F(":5004"));
  Serial.println(F("Then press the Connect button"));
  Serial.println(F("Then open MIDI-OX, ,<View> <MTC Transport>. Then hit play button and see the MTC messages"));

  // Create a session and wait for a remote host to connect to us
  AppleMIDI.begin("test");

  AppleMIDI.OnConnected(OnAppleMidiConnected);
  AppleMIDI.OnDisconnected(OnAppleMidiDisconnected);

  AppleMIDI.OnReceiveTimeCodeQuarterFrame(OnAppleMidiTimeCodeQuarterFrame);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  AppleMIDI.read();
}

// ====================================================================================
// Event handlers for incoming MIDI messages
// ====================================================================================

// -----------------------------------------------------------------------------
// rtpMIDI session. Device connected
// -----------------------------------------------------------------------------
void OnAppleMidiConnected(uint32_t ssrc, char* name) {
  isConnected = true;
  Serial.print(F("Connected to session "));
  Serial.println(name);
}

// -----------------------------------------------------------------------------
// rtpMIDI session. Device disconnected
// -----------------------------------------------------------------------------
void OnAppleMidiDisconnected(uint32_t ssrc) {
  isConnected = false;
  Serial.println(F("Disconnected"));
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
static void OnAppleMidiTimeCodeQuarterFrame(byte data) {
  Serial.print("MTC: ");
  Serial.println(data, HEX);
}
