/*!
 *  @brief      AppleMIDI Library for the Arduino - Definitions
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include "AppleMidi_Namespace.h"

#if ARDUINO
	#include <Arduino.h>
#else
	#include <inttypes.h>
	typedef uint8_t byte;
#endif

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

// MIDI Channel enumeration values
#define MIDI_CHANNEL_1 0x00
#define MIDI_CHANNEL_2 0x01
#define MIDI_CHANNEL_3 0x02
#define MIDI_CHANNEL_4 0x03
#define MIDI_CHANNEL_5 0x04
#define MIDI_CHANNEL_6 0x05
#define MIDI_CHANNEL_7 0x06
#define MIDI_CHANNEL_8 0x07
#define MIDI_CHANNEL_9 0x08
#define MIDI_CHANNEL_10 0x09
#define MIDI_CHANNEL_11 0x0a
#define MIDI_CHANNEL_12 0x0b
#define MIDI_CHANNEL_13 0x0c
#define MIDI_CHANNEL_14 0x0d
#define MIDI_CHANNEL_15 0x0e
#define MIDI_CHANNEL_16 0x0f
#define MIDI_CHANNEL_BASE 0x10
#define MIDI_CHANNEL_ALL 0x1f
#define MIDI_CHANNEL_MASK 0x0f

#define MIDI_LSB( v ) (v) & 0x7F
#define MIDI_MSB( v ) ((v)>> 7)  & 0x7F

typedef byte MIDI_CHANNEL;
typedef byte MIDI_VELOCITY;
typedef byte MIDI_PRESSURE;

// -----------------------------------------------------------------------------
// Type definitions

typedef byte StatusByte;
typedef byte DataByte;
typedef byte Channel;
typedef byte FilterMode;

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

END_APPLEMIDI_NAMESPACE
