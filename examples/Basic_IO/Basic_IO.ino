#include <Ethernet.h>
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

// Simple tutorial on how to receive and send MIDI messages.
// Here, when receiving any message on channel 4, the Arduino
// will blink a led and play back a note for 1 second.

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

void setup()
{
  if (Ethernet.begin(mac) == 0) {
    Serial.println(F("Failed DHCP, check network cable & reboot"));
    for (;;);
  }

  pinMode(LED_BUILTIN, OUTPUT);
  MIDI.begin(4);                      // Launch MIDI and listen to channel 4
}

void loop()
{
  if (MIDI.read())                    // If we have received a message
  {
    digitalWrite(LED_BUILTIN, HIGH);
    MIDI.sendNoteOn(42, 127, 1);    // Send a Note (pitch 42, velo 127 on channel 1)
    delay(1000);                    // Wait for a second
    MIDI.sendNoteOff(42, 0, 1);     // Stop the note
    digitalWrite(LED_BUILTIN, LOW);
  }
}
