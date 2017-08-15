/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		RtpMIDI Library for the Arduino
 *	Version		0.4
 *  @author		lathoub, hackmancoltaire
 *	@date		13/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "utility/AppleMidi_Settings.h"
#include "utility/AppleMidi_Defs.h"

#include "utility/RtpMidi.h"

#include "utility/AppleMidi_Invitation.h"
#include "utility/AppleMidi_InvitationAccepted.h"
#include "utility/AppleMidi_InvitationRejected.h"
#include "utility/AppleMidi_ReceiverFeedback.h"
#include "utility/AppleMidi_Syncronization.h"
#include "utility/AppleMidi_BitrateReceiveLimit.h"
#include "utility/AppleMidi_EndSession.h"

#include "utility/RtpMidi_Clock.h"

#include "utility/dissector.h"
#include "utility/PacketWriter.hpp"

#if defined(ARDUINO)
#if defined(ESP8266)
#define MAX_SESSIONS 4 // arbitrary number (tested up to 4 clients)
#else
#define MAX_SESSIONS 1 // should be 1. Response times drop significantly when more sessions are active
#endif
#else
#define MAX_SESSIONS 4 // arbitrary number
#endif


BEGIN_APPLEMIDI_NAMESPACE

class IRtpMidi
{
public:
	virtual bool PassesFilter(void* sender, DataByte, DataByte) = 0;

	virtual void OnNoteOn(void* sender, DataByte, DataByte, DataByte) = 0;
	virtual void OnNoteOff(void* sender, DataByte, DataByte, DataByte) = 0;
	virtual void OnPolyPressure(void* sender, DataByte, DataByte, DataByte) = 0;
	virtual void OnChannelPressure(void* sender, DataByte, DataByte) = 0;
	virtual void OnPitchBendChange(void* sender, DataByte, int) = 0;
	virtual void OnProgramChange(void* sender, DataByte, DataByte) = 0;
	virtual void OnControlChange(void* sender, DataByte, DataByte, DataByte) = 0;
	virtual void OnTimeCodeQuarterFrame(void* sender, DataByte) = 0;
	virtual void OnSongSelect(void* sender, DataByte) = 0;
	virtual void OnSongPosition(void* sender, unsigned short) = 0;
	virtual void OnTuneRequest(void* sender) = 0;
	virtual void OnClock(void* sender) = 0;
	virtual void OnStart(void* sender) = 0;
	virtual void OnContinue(void* sender) = 0;
	virtual void OnStop(void* sender) = 0;
	virtual void OnActiveSensing(void* sender) = 0;
	virtual void OnReset(void* sender) = 0;
	virtual void OnSysEx(void* sender, const byte* data, uint16_t size) = 0;
};

class IAppleMidi : public IRtpMidi
{
public:
	virtual void invite(IPAddress ip, uint16_t port = CONTROL_PORT) = 0;

	virtual void OnInvitation(void* sender, AppleMIDI_Invitation&) = 0;
	virtual void OnEndSession(void* sender, AppleMIDI_EndSession&) = 0;
	virtual void OnReceiverFeedback(void* sender, AppleMIDI_ReceiverFeedback&) = 0;

	virtual void OnInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&) = 0;
	virtual void OnControlInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&) = 0;
	virtual void OnContentInvitationAccepted(void* sender, AppleMIDI_InvitationAccepted&) = 0;

	virtual void OnSyncronization(void* sender, AppleMIDI_Syncronization&) = 0;
	virtual void OnBitrateReceiveLimit(void* sender, AppleMIDI_BitrateReceiveLimit&) = 0;
	virtual void OnControlInvitation(void* sender, AppleMIDI_Invitation&) = 0;
	virtual void OnContentInvitation(void* sender, AppleMIDI_Invitation&) = 0;
};

/*! \brief The main class for AppleMidi_Class handling.\n
	See member descriptions to know how to use it,
	or check out the examples supplied with the library.
 */
template<class UdpClass>
class AppleMidi_Class : public IAppleMidi
{
protected:
	//
	UdpClass _controlUDP;
	UdpClass _contentUDP;

	Dissector _controlDissector;
	Dissector _contentDissector;

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
	inline  AppleMidi_Class();
	inline ~AppleMidi_Class();

	int Port;

	inline bool begin(const char*, uint16_t port = CONTROL_PORT);

	inline uint32_t	getSynchronizationSource();
	inline char*	getSessionName() { return _sessionName; }

	inline void run();

	// IAppleMidi

	inline void invite(IPAddress ip, uint16_t port = CONTROL_PORT);

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

	// IRtpMidi
	inline bool PassesFilter (void* sender, DataByte, DataByte);

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

private:
	inline void write(UdpClass&, AppleMIDI_InvitationRejected, IPAddress ip, uint16_t port);
	inline void write(UdpClass&, AppleMIDI_InvitationAccepted, IPAddress ip, uint16_t port);
	inline void write(UdpClass&, AppleMIDI_Syncronization, IPAddress ip, uint16_t port);
	inline void write(UdpClass&, AppleMIDI_Invitation, IPAddress ip, uint16_t port);
	inline void write(UdpClass&, AppleMIDI_BitrateReceiveLimit, IPAddress ip, uint16_t port);

#if APPLEMIDI_BUILD_OUTPUT

public:
    inline void noteOn(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel);
    inline void noteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel);
    inline void programChange(DataByte inProgramNumber, Channel inChannel);
    inline void controlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel);
    inline void pitchBend(int inPitchValue,    Channel inChannel);
    inline void pitchBend(double inPitchValue, Channel inChannel);
    inline void polyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel);
    inline void afterTouch(DataByte inPressure, Channel inChannel);
    inline void sysEx(const byte*, uint16_t inLength);
    inline void timeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble);
    inline void timeCodeQuarterFrame(DataByte inData);
    inline void songPosition(unsigned short inBeats);
    inline void songSelect(DataByte inSongNumber);
    inline void tuneRequest();
    inline void activeSensing();
    inline void start();
    inline void _continue();
    inline void stop();
    inline void reset();
    inline void clock();
    inline void tick();

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

#if APPLEMIDI_USE_EVENTS
public:
 //   void xxx(void (*fptr)(byte channel, byte note, byte velocity));
private:
    void (*mNoteOnSendingEvent)(byte channel, byte note, byte velocity);
    void (*mNoteOnSendEvent)(byte channel, byte note, byte velocity);
    void (*mNoteOffSendingEvent)(byte channel, byte note, byte velocity);
    void (*mNoteOffSendEvent)(byte channel, byte note, byte velocity);
#endif

#endif // APPLEMIDI_BUILD_OUTPUT

public:
	inline int	GetFreeSessionSlot();
	inline int	GetSessionSlotUsingSSrc(const uint32_t ssrc);
	inline int	GetSessionSlotUsingInitiatorToken(const uint32_t initiatorToken);
	inline void	CreateLocalSession(const int slot, const uint32_t ssrc);
	inline void	CreateRemoteSession(IPAddress ip, uint16_t port);
	inline void	CompleteLocalSessionControl(AppleMIDI_InvitationAccepted& invitationAccepted);
	inline void	CompleteLocalSessionContent(AppleMIDI_InvitationAccepted& invitationAccepted);
	inline void	DeleteSession(const uint32_t ssrc);
	inline void	DeleteSession(int slot);
	inline void	DeleteSessions();

	inline void	DumpSession();

	inline void ManageInvites();
	inline void ManageTiming();

#if APPLEMIDI_BUILD_INPUT

private:
    StatusByte mRunningStatus_RX;
    Channel    _inputChannel;

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

#include "AppleMidi.hpp"
