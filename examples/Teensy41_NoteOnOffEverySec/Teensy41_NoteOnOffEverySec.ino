#include <NativeEthernet.h>

#define SerialMon Serial
#define USE_EXT_CALLBACKS
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

// Enter a MAC address for your controller below.
// Newer Ethernet shields have a MAC address printed on a sticker on the shield
byte mac[] = {
  0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED
};

unsigned long t1 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t&, const APPLEMIDI_NAMESPACE::Exception&, const int32_t);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  DBG_SETUP(115200);
  DBG("Booting");

  if (Ethernet.begin(mac) == 0) {
    DBG(F("Failed DHCP, check network cable & reboot"));
    for (;;);
  }

  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    DBG("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
    while (true) {
      delay(1); // do nothing, no point running without Ethernet hardware
    }
  }
  while (Ethernet.linkStatus() == LinkOFF) {
    DBG("Ethernet cable is not connected.");
    delay(500);
  }
  
  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(F("Select and then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));

  MIDI.begin(MIDI_CHANNEL_OMNI);

  // Stay informed on connection status
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });
  
  AppleMIDI.setHandleException(OnAppleMidiException);

  MIDI.setHandleControlChange([](Channel channel, byte v1, byte v2) {
    DBG(F("ControlChange"), channel, v1, v2);
  });
  MIDI.setHandleProgramChange([](Channel channel, byte v1) {
    DBG(F("ProgramChange"), channel, v1);
  });
  MIDI.setHandlePitchBend([](Channel channel, int v1) {
    DBG(F("PitchBend"), channel, v1);
  });
  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOn"), channel, note, velocity);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOff"), channel, note, velocity);
  });

  DBG(F("Sending MIDI messages every second"));
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

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, const APPLEMIDI_NAMESPACE::Exception& e, const int32_t value ) {
  switch (e)
  {
    case APPLEMIDI_NAMESPACE::Exception::BufferFullException:
      DBG(F("*** BufferFullException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParseException:
      DBG(F("*** ParseException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::TooManyParticipantsException:
      DBG(F("*** TooManyParticipantsException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::UnexpectedInviteException:
      DBG(F("*** UnexpectedInviteException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::ParticipantNotFoundException:
      DBG(F("*** ParticipantNotFoundException"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::ComputerNotInDirectory:
      DBG(F("*** ComputerNotInDirectory"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::NotAcceptingAnyone:
      DBG(F("*** NotAcceptingAnyone"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::ListenerTimeOutException:
      DBG(F("*** ListenerTimeOutException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::MaxAttemptsException:
      DBG(F("*** MaxAttemptsException"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::NoResponseFromConnectionRequestException:
      DBG(F("***:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
      break;
    case APPLEMIDI_NAMESPACE::Exception::SendPacketsDropped:
      DBG(F("*** SendPacketsDropped"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::ReceivedPacketsDropped:
      DBG(F("*** ReceivedPacketsDropped"), value);
      break;
  }
}
