#include <WiFi.h>

#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>

#include "arduino_secrets.h" // contains SECRET_SSID and SECRET_PASS

bool isConnected = false;

APPLEMIDI_CREATE_DEFAULTSESSION_INSTANCE();

void setup() {
  DBG_SETUP(115200);
  DBG("Booting");

  WiFi.begin(SECRET_SSID, SECRET_PASS);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DBG("Establishing connection to WiFi..");
  }
  DBG("Connected to network");

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(F("Select and then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));

  AppleMIDI.setHandleConnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc, const char* name) {
    isConnected = true;
    DBG(F("Connected to session"), name);
  });
  AppleMIDI.setHandleDisconnected([](const APPLEMIDI_NAMESPACE::ssrc_t & ssrc) {
    isConnected = false;
    DBG(F("Disconnected"));
  });
  AppleMIDI.setHandleException(OnAppleMidiException);

  MIDI.begin();

  MIDI.setHandleNoteOff(OnMidiNoteOff);
  MIDI.setHandleNoteOn(OnMidiNoteOn);
  MIDI.setHandleAfterTouchPoly(OnAfterTouchPoly);
  MIDI.setHandleControlChange(OnControlChange);
  MIDI.setHandleProgramChange(OnProgramChange);
  MIDI.setHandleAfterTouchChannel(OnAfterTouchChannel);
  MIDI.setHandlePitchBend(OnPitchBend);
  MIDI.setHandleSystemExclusive(OnSystemExclusive);
  MIDI.setHandleTimeCodeQuarterFrame(OnTimeCodeQuarterFrame);
  MIDI.setHandleSongPosition(OnSongPosition);
  MIDI.setHandleSongSelect(OnSongSelect);
  MIDI.setHandleTuneRequest(OnTuneRequest);
  MIDI.setHandleClock(OnClock);
  MIDI.setHandleStart(OnStart);
  MIDI.setHandleContinue(OnContinue);
  MIDI.setHandleStop(OnStop);
  MIDI.setHandleActiveSensing(OnActiveSensing);
  MIDI.setHandleSystemReset(OnSystemReset);

  DBG(F("Ready"));
}

void loop() {
  MIDI.read();
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

//------

static void OnMidiNoteOff(byte channel, byte note, byte velocity) {
  DBG(F("in\tNote off"), note, " Velocity", velocity, "\t", channel);
}

static void OnMidiNoteOn(byte channel, byte note, byte velocity) {
  DBG(F("in\tNote on"), note, " Velocity", velocity, "\t", channel);
}

static void OnAfterTouchPoly(byte channel, byte note, byte velocity) {
  DBG(F("AfterTouchPoly. Channel:"), channel, " Note:", note, " Velocity:", velocity);
}

static void OnControlChange(byte channel, byte note, byte velocity) {
  DBG(F("ControlChange. Channel:"), channel, " Note:", note, " Velocity:", velocity);
}

static void OnProgramChange(byte channel, byte note) {
  DBG(F("ProgramChange. Channel:"), channel, " Note:", note);
}

static void OnAfterTouchChannel(byte channel, byte note) {
  DBG(F("AfterTouchChannel. Channel:"), channel, " Note:", note);
}

static void OnPitchBend(byte channel, int note) {
  DBG(F("PitchBend. Channel:"), channel, " Note:", note);
}

static void OnSystemExclusive(byte* data, unsigned size) {
  DBG(F("System exclusive"));
  for (int i = 0; i < size; i++) {
    SerialMon.print(F(" 0x"));
    SerialMon.print(data[i], HEX);
  }
  SerialMon.println();
}

static void OnTimeCodeQuarterFrame(byte data) {
  DBG(F("TimeCodeQuarterFrame"));
}

static void OnSongPosition(unsigned beats) {
  DBG(F("SongPosition:"), beats);
}

static void OnSongSelect(byte songNumber) {
  DBG(F("SongSelect"),songNumber);
}

static void OnTuneRequest() {
  DBG(F("Tune request"));
}

static void OnClock() {
  DBG(F("Clock"));
}

static void OnStart() {
  DBG(F("Start"));
}

static void OnContinue() {
  DBG(F("Continue"));
}

static void OnStop() {
  DBG(F("Stop"));
}

static void OnActiveSensing() {
  DBG(F("ActiveSensing"));
}

static void OnSystemReset() {
  DBG(F("SystemReset"));
}
