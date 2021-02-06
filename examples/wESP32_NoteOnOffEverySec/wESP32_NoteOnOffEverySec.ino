#define SerialMon Serial
#define USE_EXT_CALLBACKS
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

#include "./ETH_Helper.h"

unsigned long t0 = millis();
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

  ETH_startup();

  if (!MDNS.begin(AppleMIDI.getName()))
    DBG(F("Error setting up MDNS responder"));

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), ETH.localIP(), "Port", AppleMIDI.getPort());
  DBG(F("The device should also be visible in the directory as"), AppleMIDI.getName());
  DBG(F("Select and then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));

  MIDI.begin();

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });
  AppleMIDI.setHandleException(OnAppleMidiException);

  MIDI.setHandleNoteOn([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOn"), note);
  });
  MIDI.setHandleNoteOff([](byte channel, byte note, byte velocity) {
    DBG(F("NoteOff"), note);
  });

  MDNS.addService("apple-midi", "udp", AppleMIDI.getPort());
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
  if ((isConnected > 0) && (millis() - t0) > 100)
  {
    t0 = millis();

    byte note = random(15, 100);
    byte velocity = 55;
    byte channel = 1;

    MIDI.sendNoteOn(note, velocity, channel);
    // MIDI.sendNoteOff(note, velocity, channel);
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
