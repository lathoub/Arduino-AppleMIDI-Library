/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire, chris-zen
 */

BEGIN_APPLEMIDI_NAMESPACE

/*! \brief Default constructor for MIDI_Class. */
template<class UdpClass, class Settings>
inline AppleMidiInterface<UdpClass, Settings>::AppleMidiInterface()
{
#if APPLEMIDI_USE_CALLBACKS
	// Initialise callbacks to NULL pointer
	mConnectedCallback				= NULL;
	mDisconnectedCallback			= NULL;

	mNoteOffCallback				= NULL;
	mNoteOnCallback					= NULL;
	mAfterTouchPolyCallback			= NULL;
	mControlChangeCallback			= NULL;
	mProgramChangeCallback			= NULL;
	mAfterTouchChannelCallback		= NULL;
	mPitchBendCallback				= NULL;
	mSysExCallback					= NULL;
	mTimeCodeQuarterFrameCallback	= NULL;
	mSongPositionCallback			= NULL;
	mSongSelectCallback				= NULL;
	mTuneRequestCallback			= NULL;
	mClockCallback					= NULL;
	mStartCallback					= NULL;
	mContinueCallback				= NULL;
	mStopCallback					= NULL;
	mActiveSensingCallback			= NULL;
	mResetCallback			        = NULL;
#endif

	// initiative to 0, the actual SSRC will be generated lazily
	_ssrc = 0;

	uint32_t initialTimestamp_ = 0;
	_rtpMidiClock.Init(initialTimestamp_, MIDI_SAMPLING_RATE_DEFAULT);
}

/*! \brief Default destructor for MIDI_Class.

 This is not really useful for the Arduino, as it is never called...
 */
template<class UdpClass, class Settings>
inline AppleMidiInterface<UdpClass, Settings>::~AppleMidiInterface()
{
}

// ------------------------------------------------------------------------------------
//
// ------------------------------------------------------------------------------------

#if APPLEMIDI_USE_CALLBACKS

template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnConnected(void(*fptr)(uint32_t, char*))    { mConnectedCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnDisconnected(void(*fptr)(uint32_t))      { mDisconnectedCallback = fptr; }

template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveNoteOff(void(*fptr)(byte channel, byte note, byte velocity))          { mNoteOffCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveNoteOn(void(*fptr)(byte channel, byte note, byte velocity))           { mNoteOnCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveAfterTouchPoly(void(*fptr)(byte channel, byte note, byte pressure))   { mAfterTouchPolyCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveControlChange(void(*fptr)(byte channel, byte number, byte value))     { mControlChangeCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveProgramChange(void(*fptr)(byte channel, byte number))                 { mProgramChangeCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveAfterTouchChannel(void(*fptr)(byte channel, byte pressure))           { mAfterTouchChannelCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceivePitchBend(void(*fptr)(byte channel, int bend))                        { mPitchBendCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveSysEx(void(*fptr)(const byte* array, uint16_t size))                  { mSysExCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveTimeCodeQuarterFrame(void(*fptr)(byte data))                          { mTimeCodeQuarterFrameCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveSongPosition(void(*fptr)(unsigned short beats))                       { mSongPositionCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveSongSelect(void(*fptr)(byte songnumber))                              { mSongSelectCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveTuneRequest(void(*fptr)(void))                                        { mTuneRequestCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveClock(void(*fptr)(void))                                              { mClockCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveStart(void(*fptr)(void))                                              { mStartCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveContinue(void(*fptr)(void))                                           { mContinueCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveStop(void(*fptr)(void))                                               { mStopCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveActiveSensing(void(*fptr)(void))                                      { mActiveSensingCallback = fptr; }
template<class UdpClass, class Settings> inline void AppleMidiInterface<UdpClass, Settings>::OnReceiveReset(void(*fptr)(void))                                              { mResetCallback = fptr; }

#endif

END_APPLEMIDI_NAMESPACE
