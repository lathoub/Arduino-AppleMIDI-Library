#include <WiFi.h>

#define SerialMon Serial
#define APPLEMIDI_DEBUG SerialMon
#include <AppleMIDI.h>
USING_NAMESPACE_APPLEMIDI

bool isConnected = false;

APPLEMIDI_CREATE_DEFAULTSESSION_ESP32_INSTANCE();

void setup() {
  SerialMon.begin(115200);
  while (!SerialMon);

  DBG("Booting");

  WiFi.begin("xxxx", "uuuu");
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    DBG("Establishing connection to WiFi..");
  }
  DBG("Connected to network");

  DBG(F("OK, now make sure you an rtpMIDI session that is Enabled"));
  DBG(F("Add device named Arduino with Host"), WiFi.localIP(), "Port", AppleMIDI.getPort(), "(Name", AppleMIDI.getName(), ")");
  DBG(F("Then press the Connect button"));
  DBG(F("Then open a MIDI listener and monitor incoming notes"));
  DBG(F("Every second send a random NoteOn/Off"));

  AppleMIDI.setHandleConnected(OnAppleMidiConnected);
  AppleMIDI.setHandleDisconnected(OnAppleMidiDisconnected);
  AppleMIDI.setHandleError(OnAppleMidiError);

#if BYTE_ORDER == LITTLE_ENDIAN
  DBG("Little Endian");
#endif
#if BYTE_ORDER == BIG_ENDIAN
  DBG("Big Endian");
#endif

  MIDI.begin();

  MIDI.setHandleNoteOff(OnNoteOff);
  MIDI.setHandleNoteOn(OnNoteOn);
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

//------

void OnAppleMidiConnected(const ssrc_t & ssrc, const char* name) {
  isConnected = true;
  DBG(F("Connected to session"), name);
}

void OnAppleMidiDisconnected(const ssrc_t & ssrc) {
  isConnected = false;
  DBG(F("Disconnected"));
}

void OnAppleMidiError(const ssrc_t& ssrc, int32_t err) {
  switch (err)
  {
    case Exception::BufferFullException:
      DBG(F("*** BufferFullException"));
      break;
    case Exception::ParseException:
      DBG(F("*** ParseException"));
      break;
    case Exception::TooManyParticipantsException:
      DBG(F("*** TooManyParticipantsException"));
      break;
    case Exception::UnexpectedInviteException:
      DBG(F("*** UnexpectedInviteException"));
      break;
    case Exception::ParticipantNotFoundException:
      DBG(F("*** ParticipantNotFoundException"));
      break;
    case Exception::ListenerTimeOutException:
      DBG(F("*** ListenerTimeOutException"));
      break;
    case Exception::MaxAttemptsException:
      DBG(F("*** MaxAttemptsException"));
      break;
    case Exception::NoResponseFromConnectionRequestException:
      DBG(F("***:yyy did't respond to the connection request. Check the address and port, and any firewall or router settings. (time)"));
      break;
  }
}

//------

static void OnNoteOff(byte channel, byte note, byte velocity) {
  DBG(F("Note Off. Channel:"), channel, " Note:", note, " Velocity:", velocity);
}

static void OnNoteOn(byte channel, byte note, byte velocity) {
  DBG(F("Note On. Channel:"), channel, " Note:", note, " Velocity:", velocity);
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
