#include <Ethernet.h>

//#define DISCARD_SESSION_NAME
//#define NO_LATENCY_CALCULATION
#define NO_EXT_CALLBACKS
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
bool isConnected = false;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, LOW);

  if (Ethernet.begin(mac) == 0)  for (;;);

  MIDI.begin();

  // Stay informed on connection status
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char*) {
    isConnected = true;
    digitalWrite(LED_BUILTIN, HIGH);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected = false;
    digitalWrite(LED_BUILTIN, LOW);
  });

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    digitalWrite(LED_BUILTIN, LOW);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    digitalWrite(LED_BUILTIN, HIGH);
  });
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
  if (isConnected && (millis() - t1) > 1000)
  {
    t1 = millis();

    MIDI.sendNoteOn(54, 100, 1);
  }
}
