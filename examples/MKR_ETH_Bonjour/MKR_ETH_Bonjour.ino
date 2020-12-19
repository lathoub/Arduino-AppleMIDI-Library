#include <Ethernet.h>
#include <EthernetBonjour.h> // https://github.com/TrippyLighting/EthernetBonjour

#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  if (Ethernet.begin(mac) == 0)
    for (;;);

  MIDI.begin();

  // Initialize the Bonjour/MDNS library. You can now reach or ping this
  // Arduino via the host name "arduino.local", provided that your operating
  // system is Bonjour-enabled (such as MacOS X).
  // Always call this before any other method!
  EthernetBonjour.begin("arduino");

  EthernetBonjour.addServiceRecord("Arduino._apple-midi",
                                   AppleMIDI.getPort(),
                                   MDNSServiceUDP);
}

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void loop()
{
  // Listen to incoming notes
  MIDI.read();

  EthernetBonjour.run();
}
