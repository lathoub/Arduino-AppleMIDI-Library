/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		RtpMIDI Library for the Arduino
 *	Version		0.4
 *  @author		lathoub 
 *	@date		13/04/14
 *  License		GPL
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

#include <EthernetUdp.h>

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief The main class for AppleMidi_Class handling.\n
	See member descriptions to know how to use it,
	or check out the examples supplied with the library.
 */
class AppleMidi_Class //: public IAppleMidi, public IRtpMidi
{
protected:
	//
	EthernetUDP _controlUDP;
	EthernetUDP _contentUDP;

	Dissector _controlDissector;
	Dissector _contentDissector;

	RtpMidi		_rtpMidi;

	RtpMidi_Clock _rtpMidiClock;

	SessionInvite_t _sessionInvite;
	
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

	// Allow for multiple sessions????
	Session_t	Sessions[MAX_SESSIONS];

	char SessionName[50];

public:
	// Constructor and Destructor
	AppleMidi_Class();
	~AppleMidi_Class();

	int Port;

	void begin(const char*, uint16_t port = CONTROL_PORT);
	
	uint32_t	getSynchronizationSource() { return _ssrc; }

	void run();

	// IAppleMidi

	virtual void Invite(IPAddress ip, uint16_t port = CONTROL_PORT);

	virtual void OnInvitation(void* sender, Invitation_t&);
	virtual void OnEndSession(void* sender, EndSession_t&);
	virtual void OnReceiverFeedback(void* sender, ReceiverFeedback_t&);

	virtual void OnInvitationAccepted(void* sender, InvitationAccepted_t&);
	virtual void OnControlInvitationAccepted(void* sender, InvitationAccepted_t&);
	virtual void OnContentInvitationAccepted(void* sender, InvitationAccepted_t&);

	virtual void OnSyncronization(void* sender, Syncronization_t&);
	virtual void OnBitrateReceiveLimit(void* sender, BitrateReceiveLimit_t&);
	virtual void OnControlInvitation(void* sender, Invitation_t&);
	virtual void OnContentInvitation(void* sender, Invitation_t&);

	// IRtpMidi
	virtual bool PassesFilter (void* sender, DataByte, DataByte);

	virtual void OnNoteOn (void* sender, DataByte, DataByte, DataByte);
	virtual void OnNoteOff(void* sender, DataByte, DataByte, DataByte);
	virtual void OnPolyPressure(void* sender, DataByte, DataByte, DataByte);
	virtual void OnChannelPressure(void* sender, DataByte, DataByte);
	virtual void OnPitchBendChange(void* sender, DataByte, int);
	virtual void OnProgramChange(void* sender, DataByte, DataByte);
	virtual void OnControlChange(void* sender, DataByte, DataByte, DataByte);
	virtual void OnTimeCodeQuarterFrame(void* sender, DataByte);
	virtual void OnSongSelect(void* sender, DataByte);
	virtual void OnSongPosition(void* sender, int);
	virtual void OnTuneRequest(void* sender);

#if APPLEMIDI_BUILD_OUTPUT
    
public:
    void noteOn(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel);
    void noteOff(DataByte inNoteNumber, DataByte inVelocity, Channel inChannel);
    void programChange(DataByte inProgramNumber, Channel inChannel); 
    void controlChange(DataByte inControlNumber, DataByte inControlValue, Channel inChannel);
    void pitchBend(int inPitchValue,    Channel inChannel);
    void pitchBend(double inPitchValue, Channel inChannel);   
    void polyPressure(DataByte inNoteNumber, DataByte inPressure, Channel inChannel);
    void afterTouch(DataByte inPressure, Channel inChannel); 
    void sysEx(unsigned int inLength, const byte* inArray, bool inArrayContainsBoundaries = true);    
    void timeCodeQuarterFrame(DataByte inTypeNibble, DataByte inValuesNibble);
    void timeCodeQuarterFrame(DataByte inData);
    void songPosition(unsigned int inBeats);
    void songSelect(DataByte inSongNumber);
    void tuneRequest();
    void activeSensing();
    void start();
    void _continue();
    void stop();
    void systemReset();
    void clock();
    void tick();
    
public:
    void send(MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel);
    void send(MidiType inType, DataByte inData1, DataByte inData2);
    void send(MidiType inType, DataByte inData);
    void send(MidiType inType);
      
private:
    StatusByte getStatus(MidiType inType, Channel inChannel) const;
   
    void internalSend(Session_t*, MidiType inType, DataByte inData1, DataByte inData2, Channel inChannel);
    void internalSend(Session_t*, MidiType inType, DataByte inData1, DataByte inData2);
    void internalSend(Session_t*, MidiType inType, DataByte inData);
    void internalSend(Session_t*, MidiType inType);

	void ManageInvites();
	void ManageTiming();

	int GetFreeSessionSlot();
	int GetSessionSlot(const uint32_t ssrc);

#if APPLEMIDI_USE_CALLBACKS
public:
 //   void xxx(void (*fptr)(byte channel, byte note, byte velocity));
private:
    void (*mNoteOnSendingEvent)(byte channel, byte note, byte velocity);
    void (*mNoteOnSendEvent)(byte channel, byte note, byte velocity);
    void (*mNoteOffSendingEvent)(byte channel, byte note, byte velocity);
    void (*mNoteOffSendEvent)(byte channel, byte note, byte velocity);
#endif

#endif // APPLEMIDI_BUILD_OUTPUT

private:
	void sendInvite(IPAddress ipAddress, uint16_t port);

	void ControlInvitation();
	void ControlEndSession();
	void ControlReceiverFeedback();
	void RtpInvitation();
	void RtpSynchronization();

#if APPLEMIDI_BUILD_INPUT
    
public:
    bool read();
    bool read(Channel inChannel);
    
private:
    StatusByte mRunningStatus_RX;
    Channel    _inputChannel;
    
    // -------------------------------------------------------------------------
    // Input Callbacks
    
#if APPLEMIDI_USE_CALLBACKS
    
public:
    void OnConnected(void (*fptr)(char*));
    void OnDisconnected(void (*fptr)());

    void OnReceiveNoteOn(void (*fptr)(byte channel, byte note, byte velocity));
    void OnReceiveNoteOff(void (*fptr)(byte channel, byte note, byte velocity));
    void OnReceiveAfterTouchPoly(void (*fptr)(byte channel, byte note, byte pressure));
    void OnReceiveControlChange(void (*fptr)(byte channel, byte number, byte value));
    void OnReceiveProgramChange(void (*fptr)(byte channel, byte number));
    void OnReceiveAfterTouchChannel(void (*fptr)(byte channel, byte pressure));
    void OnReceivePitchBend(void (*fptr)(byte channel, int bend));
    void OnReceiveSystemExclusive(void (*fptr)(byte * array, byte size));
    void OnReceiveTimeCodeQuarterFrame(void (*fptr)(byte data));
    void OnReceiveSongPosition(void (*fptr)(unsigned int beats));
    void OnReceiveSongSelect(void (*fptr)(byte songnumber));
    void OnReceiveTuneRequest(void (*fptr)(void));
    void OnReceiveClock(void (*fptr)(void));
    void OnReceiveStart(void (*fptr)(void));
    void OnReceiveContinue(void (*fptr)(void));
    void OnReceiveStop(void (*fptr)(void));
    void OnReceiveActiveSensing(void (*fptr)(void));
    void OnReceiveSystemReset(void (*fptr)(void));
    
//    void disconnectCallbackFromType(MidiType inType);

private:
    
    inline void launchCallback();

    void (*mConnectedCallback)(char*);
    void (*mDisconnectedCallback)();

    void (*mNoteOffCallback)(byte channel, byte note, byte velocity);
    void (*mNoteOnCallback)(byte channel, byte note, byte velocity);
    void (*mAfterTouchPolyCallback)(byte channel, byte note, byte velocity);
    void (*mControlChangeCallback)(byte channel, byte, byte);
    void (*mProgramChangeCallback)(byte channel, byte);
    void (*mAfterTouchChannelCallback)(byte channel, byte);
    void (*mPitchBendCallback)(byte channel, int);
    void (*mSongPositionCallback)(unsigned int beats);
    void (*mSongSelectCallback)(byte songnumber);
    void (*mTuneRequestCallback)(void);
    void (*mTimeCodeQuarterFrameCallback)(byte data);

    void (*mSystemExclusiveCallback)(byte * array, byte size);
    void (*mClockCallback)(void);
    void (*mStartCallback)(void);
    void (*mContinueCallback)(void);
    void (*mStopCallback)(void);
    void (*mActiveSensingCallback)(void);
    void (*mSystemResetCallback)(void);
  
#endif // APPLEMIDI_USE_CALLBACKS    

#endif // APPLEMIDI_BUILD_INPUT

};

END_APPLEMIDI_NAMESPACE

// -----------------------------------------------------------------------------

#if APPLEMIDI_AUTO_INSTANCIATE && defined(ARDUINO)
    extern APPLEMIDI_NAMESPACE::AppleMidi_Class AppleMIDI;
#endif // APPLEMIDI_AUTO_INSTANCIATE

// -----------------------------------------------------------------------------

