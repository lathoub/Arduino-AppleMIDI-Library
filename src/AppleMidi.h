/*!
 *	@brief		RtpMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

#include "utility/RtpMidi.h"

#include "utility/RtpMidi_Clock.h"

#include "utility/dissector.h"

#if defined(ARDUINO)
#if defined(ESP8266)
#define MAX_SESSIONS 4 // arbitrary number (tested up to 4 clients)
#else
#define MAX_SESSIONS 2 // Arduino can open max 4 socket. Each session needs 2 UDP ports. (Each session takes 228 bytes)
#endif
#else
#define MAX_SESSIONS 4 // arbitrary number
#endif

#include "IAppleMidiCallbacks.h"

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief The main class for AppleMidiInterface handling.\n
	See member descriptions to know how to use it,
	or check out the examples supplied with the library.
 */
template<class UdpClass, class Settings = DefaultSettings>
class AppleMidiInterface : public IAppleMidiCallbacks
{
protected:
	//
	UdpClass _controlPort;
	UdpClass _dataPort;

	Dissector _controlPortDissector;
	Dissector _dataPortDissector;

	RtpMidi<UdpClass>	_rtpMidi;

	RtpMidi_Clock _rtpMidiClock;

	// SSRC, Synchronization source.
	// (RFC 1889) The source of a stream of RTP packets, identified by a 32-bit numeric SSRC identifier
	// carried in the RTP header so as not to be dependent upon the network address. All packets from a
	// synchronization source form part of the same timing and sequence number space, so a receiver groups
	// packets by synchronization source for playback. Examples of synchronization sources include the
	// sender of a stream of packets derived from a signal source such as a microphone or a camera, or an
	// RTP mixer. A synchronization source may change its data format, e.g., audio encoding, over time.
	// The SSRC identifier is a randomly chosen value meant to be globally unique within a particular RTP
	// session. A participant need not use the same SSRC identifier for all the RTP sessions in a
	// multimedia session; the binding of the SSRC identifiers is provided through RTCP. If a participant
	// generates multiple streams in one RTP session, for example from separate video cameras, each must
	// be identified as a different SSRC.
	uint32_t _ssrc;

	Session_t	Sessions[MAX_SESSIONS];

	char _sessionName[SESSION_NAME_MAX_LEN + 1];

	byte _packetBuffer[PACKET_MAX_SIZE];

	inline uint32_t	createInitiatorToken();

public:
	// Constructor and Destructor
	inline  AppleMidiInterface();
	inline ~AppleMidiInterface();

	int Port;

	inline bool begin(const char*, uint16_t port = CONTROL_PORT);

	inline uint32_t	getSynchronizationSource();
	inline char*	getSessionName() { return _sessionName; }

    inline void run() __attribute__ ((deprecated("use read()"))) { read(); };
    inline void read();

	// IAppleMidi

	inline void invite(IPAddress ip, uint16_t port = CONTROL_PORT);

    // Callbacks
	inline void OnInvitation(void* sender, AppleMIDI_Invitation&);
	inline void OnEndSession(void* sender, AppleMIDI_EndSession&);
	inline void OnReceiverFeedback(void* sender, AppleMIDI_ReceiverFeedback&);

	inline void OnInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&);
	inline void OnControlInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&);
	inline void OnContentInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&);

	inline void OnSyncronization(void* sender, AppleMIDI_Syncronization&);
	inline void OnBitrateReceiveLimit(void* sender, AppleMIDI_BitrateReceiveLimit&);
	inline void OnControlInvitation(void* sender, AppleMIDI_Invitation&);
	inline void OnContentInvitation(void* sender, AppleMIDI_Invitation&);
    
    // Session mamangement
    inline int  GetFreeSessionSlot();
    inline int  GetSessionSlotUsingSSrc(const uint32_t ssrc);
    inline int  GetSessionSlotUsingInitiatorToken(const uint32_t initiatorToken);
    inline void CreateLocalSession(const int slot, const uint32_t ssrc);
    inline void CreateRemoteSession(IPAddress ip, uint16_t port);
    inline void CompleteLocalSessionControl(AppleMIDI_InvitationAccepted& invitationAccepted);
    inline void CompleteLocalSessionContent(AppleMIDI_InvitationAccepted& invitationAccepted);
    inline void DeleteSession(const uint32_t ssrc);
    inline void DeleteSession(int slot);
    inline void DeleteSessions();
    
    inline void DumpSession();
    
    inline void ManageInvites();
    inline void ManageTiming();

private:
    inline void write(UdpClass&, AppleMIDI_InvitationRejected,  IPAddress ip, uint16_t port);
    inline void write(UdpClass&, AppleMIDI_InvitationAccepted,  IPAddress ip, uint16_t port);
    inline void write(UdpClass&, AppleMIDI_Syncronization,      IPAddress ip, uint16_t port);
    inline void write(UdpClass&, AppleMIDI_Invitation,          IPAddress ip, uint16_t port);
    inline void write(UdpClass&, AppleMIDI_BitrateReceiveLimit, IPAddress ip, uint16_t port);

public:
	// IMidiCallbacks
	inline void OnNoteOn (void* sender, DataByte, DataByte, DataByte);
	inline void OnNoteOff(void* sender, DataByte, DataByte, DataByte);
	inline void OnPolyPressure(void* sender, DataByte, DataByte, DataByte);
	inline void OnChannelPressure(void* sender, DataByte, DataByte);
	inline void OnPitchBendChange(void* sender, DataByte, int);
	inline void OnProgramChange(void* sender, DataByte, DataByte);
	inline void OnControlChange(void* sender, DataByte, DataByte, DataByte);
	inline void OnTimeCodeQuarterFrame(void* sender, DataByte);
	inline void OnSongSelect(void* sender, DataByte);
	inline void OnSongPosition(void* sender, unsigned short);
	inline void OnTuneRequest(void* sender);
	inline void OnClock(void* sender);
	inline void OnStart(void* sender);
	inline void OnContinue(void* sender);
	inline void OnStop(void* sender);
	inline void OnActiveSensing(void* sender);
	inline void OnReset(void* sender);
	inline void OnSysEx(void* sender, const byte* data, uint16_t size);

#if APPLEMIDI_BUILD_OUTPUT

public:
    inline void sendNoteOn(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel);
    inline void sendNoteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel);
    inline void sendProgramChange(DataByte inProgramNumber, Channel inChannel);
    inline void sendControlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel);
    inline void sendPitchBend(int inPitchValue,    Channel inChannel);
    inline void sendPitchBend(double inPitchValue, Channel inChannel);
    inline void sendPolyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel);
    inline void sendAfterTouch(DataByte inPressure, Channel inChannel);
    inline void sendSysEx(const byte*, uint16_t inLength);
    inline void sendTimeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble);
    inline void sendTimeCodeQuarterFrame(DataByte inData);
    inline void sendSongPosition(unsigned short inBeats);
    inline void sendSongSelect(DataByte inSongNumber);
    inline void sendTuneRequest();
    inline void sendActiveSensing();
    inline void sendStart();
    inline void sendContinue();
    inline void sendStop();
    inline void sendReset();
    inline void sendClock();
    inline void sendTick();

    // begin deprecated - because we are aligning with the FortySevenEffects/arduino_midi_library
    inline void noteOn(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel) __attribute__ ((deprecated("use sendNoteOn"))) { sendNoteOn(inNoteNumber, inVelocity, inChannel); }
    inline void noteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel) __attribute__ ((deprecated("use sendNoteOff"))) { sendNoteOff(inNoteNumber, inVelocity, inChannel); }
    inline void programChange(DataByte inProgramNumber, Channel inChannel) __attribute__ ((deprecated("use sendProgramChange"))) { sendProgramChange(inProgramNumber, inChannel); }
    inline void controlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel) __attribute__ ((deprecated("use sendControlChange"))) { sendControlChange(inControlNumber, inControlValue, inChannel); }
    inline void pitchBend(int inPitchValue,    Channel inChannel) __attribute__ ((deprecated("use sendPitchBend"))) { sendPitchBend(inPitchValue, inChannel); }
    inline void pitchBend(double inPitchValue, Channel inChannel) __attribute__ ((deprecated("use sendPitchBend"))) { sendPitchBend(inPitchValue, inChannel); }
    inline void polyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel) __attribute__ ((deprecated("use sendPolyPressure"))) { sendPolyPressure(inNoteNumber, inPressure, inChannel); }
    inline void afterTouch(DataByte inPressure, Channel inChannel) __attribute__ ((deprecated("use sendAfterTouch"))) { sendAfterTouch(inPressure, inChannel); }
    inline void sysEx(const byte* data, uint16_t inLength) __attribute__ ((deprecated("use sendSysEx"))) { sendSysEx(data, inLength); }
    inline void timeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble) __attribute__ ((deprecated("use sendTimeCodeQuarterFrame"))) { sendTimeCodeQuarterFrame(inTypeNibble, inValuesNibble); }
    inline void timeCodeQuarterFrame(DataByte inData) __attribute__ ((deprecated("use sendTimeCodeQuarterFrame"))) { sendTimeCodeQuarterFrame(inData); }
    inline void songPosition(unsigned short inBeats) __attribute__ ((deprecated("use sendSongPosition"))) { sendSongPosition(inBeats); }
    inline void songSelect(DataByte inSongNumber) __attribute__ ((deprecated("use sendSongSelect"))) { sendSongSelect(inSongNumber); }
    inline void tuneRequest() __attribute__ ((deprecated("use sendTuneRequest"))) { sendTuneRequest(); }
    inline void activeSensing() __attribute__ ((deprecated("use sendActiveSensing"))) { sendActiveSensing(); }
    inline void start() __attribute__ ((deprecated("use sendStart"))) { sendStart(); }
    inline void _continue() __attribute__ ((deprecated("use sendContinue"))) { sendContinue(); }
    inline void stop() __attribute__ ((deprecated("use sendStop"))) { sendStop(); }
    inline void reset() __attribute__ ((deprecated("use sendReset"))) { sendReset(); }
    inline void clock() __attribute__ ((deprecated("use sendClock"))) { sendClock(); }
    inline void tick() __attribute__ ((deprecated("use sendTick"))) { sendTick(); }
    // end deprecated - because we are aligning with the FortySevenEffects/arduino_midi_library
    
protected:
	inline void send(MidiType inType, DataByte inData1, DataByte inData2, Channel);
	inline void send(MidiType inType, DataByte inData1, DataByte inData2);
    inline void send(MidiType inType, DataByte inData);
    inline void send(MidiType inType);
	inline void sendSysEx(byte, const byte* inData, byte, uint16_t);

private:
    inline void internalSend(Session_t&, MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel);
    inline void internalSend(Session_t&, MidiType inType, DataByte inData1, DataByte inData2);
    inline void internalSend(Session_t&, MidiType inType, DataByte inData);
	inline void internalSend(Session_t&, MidiType inType);
	inline void internalSendSysEx(Session_t&, byte, const byte*, byte, uint16_t);

	StatusByte getStatus(MidiType inType, Channel inChannel) const;

#endif // APPLEMIDI_BUILD_OUTPUT

#if APPLEMIDI_BUILD_INPUT

private:
    StatusByte mRunningStatus_RX;

    // -------------------------------------------------------------------------
    // Input Callbacks

#if APPLEMIDI_USE_CALLBACKS

public:
	inline void OnConnected(void(*fptr)(uint32_t, char*));
	inline void OnDisconnected(void(*fptr)(uint32_t));

    inline void OnReceiveNoteOn(void (*fptr)(byte channel, byte note, byte velocity));
    inline void OnReceiveNoteOff(void (*fptr)(byte channel, byte note, byte velocity));
    inline void OnReceiveAfterTouchPoly(void (*fptr)(byte channel, byte note, byte pressure));
    inline void OnReceiveControlChange(void (*fptr)(byte channel, byte number, byte value));
    inline void OnReceiveProgramChange(void (*fptr)(byte channel, byte number));
    inline void OnReceiveAfterTouchChannel(void (*fptr)(byte channel, byte pressure));
    inline void OnReceivePitchBend(void (*fptr)(byte channel, int bend));
    inline void OnReceiveSysEx(void (*fptr)(const byte * data, uint16_t size));
    inline void OnReceiveTimeCodeQuarterFrame(void (*fptr)(byte data));
    inline void OnReceiveSongPosition(void (*fptr)(unsigned short beats));
    inline void OnReceiveSongSelect(void (*fptr)(byte songnumber));
    inline void OnReceiveTuneRequest(void (*fptr)(void));
    inline void OnReceiveClock(void (*fptr)(void));
    inline void OnReceiveStart(void (*fptr)(void));
    inline void OnReceiveContinue(void (*fptr)(void));
    inline void OnReceiveStop(void (*fptr)(void));
    inline void OnReceiveActiveSensing(void (*fptr)(void));
    inline void OnReceiveReset(void (*fptr)(void));

private:

    inline void launchCallback();

	void(*mConnectedCallback)(uint32_t, char*);
	void(*mDisconnectedCallback)(uint32_t);

    void (*mNoteOffCallback)(byte channel, byte note, byte velocity);
    void (*mNoteOnCallback)(byte channel, byte note, byte velocity);
    void (*mAfterTouchPolyCallback)(byte channel, byte note, byte velocity);
    void (*mControlChangeCallback)(byte channel, byte, byte);
    void (*mProgramChangeCallback)(byte channel, byte);
    void (*mAfterTouchChannelCallback)(byte channel, byte);
    void (*mPitchBendCallback)(byte channel, int);
    void (*mSongPositionCallback)(unsigned short beats);
    void (*mSongSelectCallback)(byte songnumber);
    void (*mTuneRequestCallback)(void);
    void (*mTimeCodeQuarterFrameCallback)(byte data);

    void (*mSysExCallback)(const byte* array, uint16_t size);
    void (*mClockCallback)(void);
    void (*mStartCallback)(void);
    void (*mContinueCallback)(void);
    void (*mStopCallback)(void);
    void (*mActiveSensingCallback)(void);
    void (*mResetCallback)(void);

#endif // APPLEMIDI_USE_CALLBACKS

#endif // APPLEMIDI_BUILD_INPUT

};

END_APPLEMIDI_NAMESPACE

// -----------------------------------------------------------------------------

#include "utility/packet-rtp-midi.h"
#include "utility/packet-apple-midi.h"

#include "AppleMidiInterface.hpp"

#include "AppleMidi.hpp"
#if APPLEMIDI_BUILD_INPUT
#include "MidiInput.hpp"
#endif

#if APPLEMIDI_BUILD_OUTPUT
#include "MidiOutput.hpp"
#endif
