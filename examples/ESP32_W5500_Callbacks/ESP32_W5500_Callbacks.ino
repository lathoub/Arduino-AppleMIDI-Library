#include "ETH_Helper.h"

#define ONE_PARTICIPANT
#define USE_EXT_CALLBACKS
#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

unsigned long t1 = millis();
int8_t isConnected = 0;

APPLEMIDI_CREATE_INSTANCE(EthernetUDP, MIDI, "AppleMIDI-Arduino", DEFAULT_CONTROL_PORT);

void OnAppleMidiException(const APPLEMIDI_NAMESPACE::ssrc_t&, const APPLEMIDI_NAMESPACE::Exception&, const int32_t);

// -----------------------------------------------------------------------------
//
// -----------------------------------------------------------------------------
void setup()
{
  DBG_SETUP(115200);
  DBG("Das Booting");

  ETH_startup();

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), Ethernet.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(F("Select and then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));

  MIDI.begin(MIDI_CHANNEL_OMNI);

  // Normal callbacks - always available
  // Stay informed on connection status
  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected++;
    DBG(F("Connected to session"), ssrc, name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected--;
    DBG(F("Disconnected"), ssrc);
  });

  // Extended callback, only available when defining USE_EXT_CALLBACKS
  AppleMIDI.setHandleSentRtp([](const APPLEMIDI_NAMESPACE::Rtp_t & rtp) {
    //  DBG(F("an rtpMessage has been sent with sequenceNr"), rtp.sequenceNr);
  });
  AppleMIDI.setHandleSentRtpMidi([](const APPLEMIDI_NAMESPACE::RtpMIDI_t& rtpMidi) {
    DBG(F("an rtpMidiMessage has been sent"), rtpMidi.flags);
  });
  AppleMIDI.setHandleReceivedRtp([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const APPLEMIDI_NAMESPACE::Rtp_t & rtp, const int32_t& latency) {
    //  DBG(F("setHandleReceivedRtp"), ssrc, rtp.sequenceNr , "with", latency, "ms latency");
  });
  AppleMIDI.setHandleStartReceivedMidi([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
    //  DBG(F("setHandleStartReceivedMidi from SSRC"), ssrc);
  });
  AppleMIDI.setHandleReceivedMidi([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc, byte value) {
    //    DBG(F("setHandleReceivedMidi from SSRC"), ssrc, ", value:", value);
  });
  AppleMIDI.setHandleEndReceivedMidi([](const APPLEMIDI_NAMESPACE::ssrc_t& ssrc) {
    //  DBG(F("setHandleEndReceivedMidi from SSRC"), ssrc);
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
  if ((isConnected > 0) && (millis() - t1) > 100)
  {
    t1 = millis();

    byte note = random(1, 127);
    byte velocity = 55;
    byte channel = 1;

    //   DBG(F("\nsendNoteOn"), note, velocity, channel);
    MIDI.sendNoteOn(note, velocity, channel);
    //MIDI.sendNoteOff(note, velocity, channel);
  }

#ifndef ETHERNET3
  EthernetBonjour.run();
#endif
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
      DBG(F("******************************************** ReceivedPacketsDropped"), value);
      break;
    case APPLEMIDI_NAMESPACE::Exception::UdpBeginPacketFailed:
      DBG(F("*** UdpBeginPacketFailed"), value);
      break;
  }
}
