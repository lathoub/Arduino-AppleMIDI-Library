/*!
 *  @file       AppleMIDI_Defs.h
 *  Project     Arduino AppleMIDI Library
 *  @brief      AppleMIDI Library for the Arduino - Definitions
 *	Version		0.3
 *  @author		lathoub, hackmancoltaire
 *	@date		04/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include "AppleMidi_Namespace.h"

#if ARDUINO
	#include <Arduino.h>
#else
	#include <inttypes.h>
	typedef uint8_t byte;
#endif

#include <IPAddress.h>

#ifndef UDP_TX_PACKET_MAX_SIZE
#define UDP_TX_PACKET_MAX_SIZE 24
#endif

#define SESSION_NAME_MAX_LEN 24

#define SYNC_CK0 0
#define SYNC_CK1 1
#define SYNC_CK2 2


BEGIN_APPLEMIDI_NAMESPACE

// -----------------------------------------------------------------------------

#define MIDI_CHANNEL_OMNI       0
#define MIDI_CHANNEL_OFF        17 // and over

#define MIDI_PITCHBEND_MIN      -8192
#define MIDI_PITCHBEND_MAX      8191

#define MIDI_SAMPLING_RATE_8KHZ			8000
#define MIDI_SAMPLING_RATE_11KHZ		11025
#define MIDI_SAMPLING_RATE_44K1HZ		44100
#define MIDI_SAMPLING_RATE_48KHZ		48000
#define MIDI_SAMPLING_RATE_88K2HZ		88200
#define MIDI_SAMPLING_RATE_96KHZ		96000
#define MIDI_SAMPLING_RATE_176K4HZ		176400
#define MIDI_SAMPLING_RATE_192KHZ		192000
#define MIDI_SAMPLING_RATE_DEFAULT		10000

// Channel Voice Messages
#define MIDI_STATUS_NOTE_OFF 0x80
#define MIDI_STATUS_NOTE_ON 0x90
#define MIDI_STATUS_POLYPHONIC_KEY_PRESSURE 0xA0
#define MIDI_STATUS_CONTROL_CHANGE 0xb0
#define MIDI_STATUS_PROGRAM_CHANGE 0xc0
#define MIDI_STATUS_CHANNEL_PRESSURE 0xd0
#define MIDI_STATUS_PITCH_WHEEL_CHANGE 0xe0

// MIDI Channel enumeration values
#define MIDI_CHANNEL_1 0x0
#define MIDI_CHANNEL_2 0x1
#define MIDI_CHANNEL_3 0x2
#define MIDI_CHANNEL_4 0x3
#define MIDI_CHANNEL_5 0x4
#define MIDI_CHANNEL_6 0x5
#define MIDI_CHANNEL_7 0x6
#define MIDI_CHANNEL_8 0x7
#define MIDI_CHANNEL_9 0x8
#define MIDI_CHANNEL_10 0x9
#define MIDI_CHANNEL_11 0xa
#define MIDI_CHANNEL_12 0xb
#define MIDI_CHANNEL_13 0xc
#define MIDI_CHANNEL_14 0xd
#define MIDI_CHANNEL_15 0xe
#define MIDI_CHANNEL_16 0xf
#define MIDI_CHANNEL_BASE 0x10
#define MIDI_CHANNEL_ALL 0x1f

#define MIDI_LSB( v ) (v) & 0x7F
#define MIDI_MSB( v ) ((v)>> 7)  & 0x7F

typedef byte MIDI_CHANNEL;
typedef byte MIDI_VELOCITY;
typedef byte MIDI_PRESSURE;

// -----------------------------------------------------------------------------
// Type definitions

typedef uint8_t byte;

typedef byte StatusByte;
typedef byte DataByte;
typedef byte Channel;
typedef byte FilterMode;

// -----------------------------------------------------------------------------

/*! Enumeration of AppleMIDI Command */
enum AppleMIDICommand {
	Idle,
	Invitation,
	InvitationRejected,
	InvitationAccepted,
	EndSession,
	Syncronization,
	ReceiverFeedback,
	Unknown,
};

const uint8_t amSignature [] = {0xff, 0xff};

const uint8_t amInvitation          [] = {'I', 'N'};
const uint8_t amInvitationAccepted  [] = {'O', 'K'};
const uint8_t amInvitationRejected  [] = {'N', 'O'};
const uint8_t amEndSession          [] = {'B', 'Y'};
const uint8_t amSyncronization      [] = {'C', 'K'};
const uint8_t amReceiverFeedback    [] = {'R', 'S'};
const uint8_t amBitrateReceiveLimit [] = {'R', 'L'};

const unsigned char stringTerminator  [] = {0x00};


/*! Enumeration of MIDI types */
enum MidiType
{
    InvalidType           = 0x00,    ///< For notifying errors
    NoteOff               = 0x80,    ///< Note Off
    NoteOn                = 0x90,    ///< Note On
    AfterTouchPoly        = 0xA0,    ///< Polyphonic AfterTouch
    ControlChange         = 0xB0,    ///< Control Change / Channel Mode
    ProgramChange         = 0xC0,    ///< Program Change
    AfterTouchChannel     = 0xD0,    ///< Channel (monophonic) AfterTouch
    PitchBend             = 0xE0,    ///< Pitch Bend
	SysEx                 = 0xF0,    ///< System Exclusive
	SysExStart            = SysEx,   ///< System Exclusive Start
	SysExEnd              = 0xF7,    ///< System Exclusive End
	TimeCodeQuarterFrame  = 0xF1,    ///< System Common - MIDI Time Code Quarter Frame
    SongPosition          = 0xF2,    ///< System Common - Song Position Pointer
    SongSelect            = 0xF3,    ///< System Common - Song Select
    TuneRequest           = 0xF6,    ///< System Common - Tune Request
    Clock                 = 0xF8,    ///< System Real Time - Timing Clock
    Tick                  = 0xF9,    ///< System Real Time - Tick
    Start                 = 0xFA,    ///< System Real Time - Start
    Continue              = 0xFB,    ///< System Real Time - Continue
    Stop                  = 0xFC,    ///< System Real Time - Stop
    ActiveSensing         = 0xFE,    ///< System Real Time - Active Sensing
    Reset                 = 0xFF,    ///< System Real Time - System Reset
};

// -----------------------------------------------------------------------------

/*! Enumeration of Thru filter modes */
enum MidiFilterMode
{
    Off                   = 0,  ///< Thru disabled (nothing passes through).
    Full                  = 1,  ///< Fully enabled Thru (every incoming message is sent back).
    SameChannel           = 2,  ///< Only the messages on the Input Channel will be sent back.
    DifferentChannel      = 3,  ///< All the messages but the ones on the Input Channel will be sent back.
};

// -----------------------------------------------------------------------------

/*! \brief Enumeration of Control Change command numbers.
 See the detailed controllers numbers & description here:
 http://www.somascape.org/midi/tech/spec.html#ctrlnums
 */
enum MidiControlChangeNumber
{
    // High resolution Continuous Controllers MSB (+32 for LSB) ----------------
    BankSelect                  = 0,
    ModulationWheel             = 1,
    BreathController            = 2,
    // CC3 undefined
    FootController              = 4,
    PortamentoTime              = 5,
    DataEntry                   = 6,
    ChannelVolume               = 7,
    Balance                     = 8,
    // CC9 undefined
    Pan                         = 10,
    ExpressionController        = 11,
    EffectControl1              = 12,
    EffectControl2              = 13,
    // CC14 undefined
    // CC15 undefined
    GeneralPurposeController1   = 16,
    GeneralPurposeController2   = 17,
    GeneralPurposeController3   = 18,
    GeneralPurposeController4   = 19,

    // Switches ----------------------------------------------------------------
    Sustain                     = 64,
    Portamento                  = 65,
    Sostenuto                   = 66,
    SoftPedal                   = 67,
    Legato                      = 68,
    Hold                        = 69,

    // Low resolution continuous controllers -----------------------------------
    SoundController1            = 70,   ///< Synth: Sound Variation   FX: Exciter On/Off
    SoundController2            = 71,   ///< Synth: Harmonic Content  FX: Compressor On/Off
    SoundController3            = 72,   ///< Synth: Release Time      FX: Distortion On/Off
    SoundController4            = 73,   ///< Synth: Attack Time       FX: EQ On/Off
    SoundController5            = 74,   ///< Synth: Brightness        FX: Expander On/Off
    SoundController6            = 75,   ///< Synth: Decay Time        FX: Reverb On/Off
    SoundController7            = 76,   ///< Synth: Vibrato Rate      FX: Delay On/Off
    SoundController8            = 77,   ///< Synth: Vibrato Depth     FX: Pitch Transpose On/Off
    SoundController9            = 78,   ///< Synth: Vibrato Delay     FX: Flange/Chorus On/Off
    SoundController10           = 79,   ///< Synth: Undefined         FX: Special Effects On/Off
    GeneralPurposeController5   = 80,
    GeneralPurposeController6   = 81,
    GeneralPurposeController7   = 82,
    GeneralPurposeController8   = 83,
    PortamentoControl           = 84,
    // CC85 to CC90 undefined
    Effects1                    = 91,   ///< Reverb send level
    Effects2                    = 92,   ///< Tremolo depth
    Effects3                    = 93,   ///< Chorus send level
    Effects4                    = 94,   ///< Celeste depth
    Effects5                    = 95,   ///< Phaser depth

    // Channel Mode messages ---------------------------------------------------
    AllSoundOff                 = 120,
    ResetAllControllers         = 121,
    LocalControl                = 122,
    AllNotesOff                 = 123,
    OmniModeOff                 = 124,
    OmniModeOn                  = 125,
    MonoModeOn                  = 126,
    PolyModeOn                  = 127
};


// -----------------------------------------------------------------------------

/*! The midimsg structure contains decoded data of a MIDI message
    read from the serial port with read()
 */
struct Message
{
    /*! The MIDI channel on which the message was received.
     \n Value goes from 1 to 16.
     */
    Channel channel;

    /*! The type of the message
     (see the MidiType enum for types reference)
     */
    MidiType type;

    /*! The first data byte.
     \n Value goes from 0 to 127.
     */
    DataByte data1;

    /*! The second data byte.
     If the message is only 2 bytes long, this one is null.
     \n Value goes from 0 to 127.
     */
    DataByte data2;

    /*! System Ex dedicated byte array.
     \n Array length is stocked on 16 bits,
     in data1 (LSB) and data2 (MSB)
     */
    DataByte sysexArray[MIDI_SYSEX_ARRAY_SIZE];

    /*! This boolean indicates if the message is valid or not.
     There is no channel consideration here,
     validity means the message respects the MIDI norm.
     */
    bool valid;
};

// -----------------------------------------------------------------------------

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

enum Accept
{
	NoOne = 0,
	Peer = 1,
	Anyone = 0xff,
};

enum SessionInitiator
{
	Undefined,
	Remote,
	Local,
};

enum SessionInviteStatus
{
	None,
	SendControlInvite,
	ReceiveControlInvitation,
	WaitingForControlInvitationAccepted,
	SendContentInvite,
	WaitingForContentInvitationAccepted,
};






typedef struct {
	SessionInviteStatus	status;
	unsigned long		lastSend;
    IPAddress			remoteHost;
	uint16_t			remotePort;
	int					attempts;
    uint32_t			ssrc;
	uint32_t			initiatorToken;
} SessionInvite_t;

typedef struct {
	bool				enabled;
	unsigned long		lastTime;
	uint32_t			count;
	bool				busy;
} SessionSyncronization_t;

typedef struct {
    uint32_t				ssrc; // the unique identifier
	unsigned short			seqNum;
	SessionInitiator		initiator;
	SessionSyncronization_t	syncronization;
	IPAddress				contentIP;
	uint16_t				contentPort;
	SessionInvite_t			invite;
} Session_t;

typedef uint32_t MIDISamplingRate;
typedef uint64_t MIDITimeStamp;

// -----------------------------------------------------------------------------

/*! \brief Create an instance of the library
 */
#define APPLEMIDI_CREATE_INSTANCE(Type, Name)                            \
    APPLEMIDI_NAMESPACE::AppleMidi_Class<Type> Name;


/*! \brief Create an instance of the library with EnternetUDP.
*/
#define APPLEMIDI_CREATE_DEFAULT_INSTANCE()                                      \
    APPLEMIDI_CREATE_INSTANCE(EthernetUDP, AppleMIDI);

END_APPLEMIDI_NAMESPACE
