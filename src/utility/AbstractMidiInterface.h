/*!
 *  @file        AbstractMidiInterface.h
 */

#pragma once

namespace Midi {
    
#define MIDI_PITCHBEND_MIN      -8192
#define MIDI_PITCHBEND_MAX      8191
    
#define MIDI_SAMPLING_RATE_8KHZ            8000
#define MIDI_SAMPLING_RATE_11KHZ        11025
#define MIDI_SAMPLING_RATE_44K1HZ        44100
#define MIDI_SAMPLING_RATE_48KHZ        48000
#define MIDI_SAMPLING_RATE_88K2HZ        88200
#define MIDI_SAMPLING_RATE_96KHZ        96000
#define MIDI_SAMPLING_RATE_176K4HZ        176400
#define MIDI_SAMPLING_RATE_192KHZ        192000
#define MIDI_SAMPLING_RATE_DEFAULT        10000
    
    // Channel Voice Messages
#define MIDI_STATUS_NOTE_OFF 0x80
#define MIDI_STATUS_NOTE_ON 0x90
#define MIDI_STATUS_POLYPHONIC_KEY_PRESSURE 0xA0
#define MIDI_STATUS_CONTROL_CHANGE 0xb0
#define MIDI_STATUS_PROGRAM_CHANGE 0xc0
#define MIDI_STATUS_CHANNEL_PRESSURE 0xd0
#define MIDI_STATUS_PITCH_WHEEL_CHANGE 0xe0
    
    // MIDI Channel enumeration values
#define MIDI_CHANNEL_OMNI 0x0
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
#define MIDI_CHANNEL_OFF 0x1f
    
#define MIDI_LSB( v ) (v) & 0x7F
#define MIDI_MSB( v ) ((v)>> 7)  & 0x7F
    
    // -----------------------------------------------------------------------------
    // Type definitions
    
    typedef uint8_t byte;
    
    typedef byte StatusByte;
    typedef byte DataByte;
    typedef byte Channel;
    typedef byte FilterMode;
    
    typedef byte MIDI_CHANNEL;
    typedef byte MIDI_VELOCITY;
    typedef byte MIDI_PRESSURE;
    
    /*! Enumeration of MIDI types */
    enum MidiType : uint8_t
    {
        InvalidType           = 0x00,    ///< For notifying errors
        
        NoteOff               = 0x80,    ///< Note Off
        NoteOn                = 0x90,    ///< Note On
        AfterTouchPoly        = 0xA0,    ///< Polyphonic AfterTouch
        ControlChange         = 0xB0,    ///< Control Change / Channel Mode
        ProgramChange         = 0xC0,    ///< Program Change
        AfterTouchChannel     = 0xD0,    ///< Channel (monophonic) AfterTouch
        PitchBend             = 0xE0,    ///< Pitch Bend
        
        SystemExclusive       = 0xF0,    ///< System Exclusive
        SystemExclusiveStart  = SystemExclusive,
        SystemExclusiveEnd    = 0xF7,    ///< System Exclusive End
        
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
        SystemReset           = 0xFF,    ///< System Real Time - System Reset
    };
    
    /*! Enumeration of Thru filter modes */
    struct Thru
    {
        enum Mode
        {
            Off                   = 0,  ///< Thru disabled (nothing passes through).
            Full                  = 1,  ///< Fully enabled Thru (every incoming message is sent back).
            SameChannel           = 2,  ///< Only the messages on the Input Channel will be sent back.
            DifferentChannel      = 3,  ///< All the messages but the ones on the Input Channel will be sent back.
        };
    };
    
    /*! \brief Enumeration of Control Change command numbers.
     See the detailed controllers numbers & description here:
     http://www.somascape.org/midi/tech/spec.html#ctrlnums
     */
    enum MidiControlChangeNumber : uint8_t
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
    
    struct RPN
    {
        enum RegisteredParameterNumbers
        {
            PitchBendSensitivity    = 0x0000,
            ChannelFineTuning       = 0x0001,
            ChannelCoarseTuning     = 0x0002,
            SelectTuningProgram     = 0x0003,
            SelectTuningBank        = 0x0004,
            ModulationDepthRange    = 0x0005,
            NullFunction            = (0x7f << 7) + 0x7f,
        };
    };
    
    /*! \brief Extract an enumerated MIDI type from a status byte
     */
    static MidiType getTypeFromStatusByte(byte status)
    {
        if ((status  < 0x80) ||
            (status == 0xf4) ||
            (status == 0xf5) ||
            (status == 0xf9) ||
            (status == 0xfD))
        {
            // Data bytes and undefined.
            return MidiType::InvalidType;
        }
        if (status < 0xf0)
        {
            // Channel message, remove channel nibble.
            return MidiType(status & 0xf0);
        }
        
        return MidiType(status);
    }
    
    /*! \brief Returns type + channel
     */
    static StatusByte getStatus(MidiType type, Channel channel)
    {
        return ( type & 0xf0) | ((channel - 1) & 0x0f);
    }
    
    /*! \brief Returns channel in the range 1-16
     */
    static Channel getChannelFromStatusByte(byte status)
    {
        return Channel((status & 0x0f) + 1);
    }
    
    /*! \brief check if channel is in the range 1-16
     */
    static bool isChannelMessage(MidiType type)
    {
        return (type == MidiType::NoteOff           ||
                type == MidiType::NoteOn            ||
                type == MidiType::ControlChange     ||
                type == MidiType::AfterTouchPoly    ||
                type == MidiType::AfterTouchChannel ||
                type == MidiType::PitchBend         ||
                type == MidiType::ProgramChange);
    }
    
    class AbstractMidiInterface
    {
    protected:
        int _runningStatus;
        bool _thruActivated;
        
    public:
        AbstractMidiInterface()
        {
        }
        
    protected:
        void (*_noteOnCallback)(byte channel, byte note, byte velocity) = NULL;
        void (*_noteOffCallback)(byte channel, byte note, byte velocity) = NULL;
        void (*_afterTouchPolyCallback)(byte channel, byte note, byte velocity) = NULL;
        void (*_controlChangeCallback)(byte channel, byte, byte) = NULL;
        void (*_programChangeCallback)(byte channel, byte) = NULL;
        void (*_afterTouchChannelCallback)(byte channel, byte) = NULL;
        void (*_pitchBendCallback)(byte channel, int) = NULL;
        void (*_songPositionCallback)(unsigned short beats) = NULL;
        void (*_songSelectCallback)(byte songnumber) = NULL;
        void (*_tuneRequestCallback)(void) = NULL;
        void (*_timeCodeQuarterFrameCallback)(byte data) = NULL;
        void (*_sysExCallback)(const byte* array, uint16_t size) = NULL;
        void (*_clockCallback)(void) = NULL;
        void (*_startCallback)(void) = NULL;
        void (*_continueCallback)(void) = NULL;
        void (*_stopCallback)(void) = NULL;
        void (*_activeSensingCallback)(void) = NULL;
        void (*_resetCallback)(void) = NULL;
        
    public:
        // sending
        void sendNoteOn(DataByte note, DataByte velocity, Channel channel) {
            sendChannelMessage(MidiType::NoteOn, channel, note, velocity);
        }
        
        void sendNoteOff(DataByte note, DataByte velocity, Channel channel) {
            sendChannelMessage(MidiType::NoteOff, channel, note, velocity);
        }
        
        void sendProgramChange(DataByte number, Channel channel) {
            sendChannelMessage(MidiType::ProgramChange, number, 0, channel);
        }
        
        void sendControlChange(DataByte number, DataByte value, Channel channel) {
            sendChannelMessage(MidiType::ControlChange, number, value, channel);
        }
        
        void sendPitchBend(int value, Channel channel) {
            const unsigned bend = unsigned(value - int(MIDI_PITCHBEND_MIN));
            sendChannelMessage(MidiType::PitchBend, (bend & 0x7f), (bend >> 7) & 0x7f, channel);
        }
        
        void sendPitchBend(double pitchValue, Channel channel) {
            const int scale = pitchValue > 0.0 ? MIDI_PITCHBEND_MAX : MIDI_PITCHBEND_MIN;
            const int value = int(pitchValue * double(scale));
            sendPitchBend(value, channel);
        }
        
        void sendPolyPressure(DataByte note, DataByte pressure, Channel channel) {
            sendChannelMessage(MidiType::AfterTouchPoly, note, pressure, channel);
        }
        
        void sendAfterTouch(DataByte pressure, Channel channel) {
            sendChannelMessage(MidiType::AfterTouchChannel, pressure, 0, channel);
        }
        
        void sendAfterTouch(DataByte note, DataByte pressure, Channel channel) {
            sendChannelMessage(MidiType::AfterTouchChannel, note, pressure, channel);
        }
        
        
        void sendSysEx(const byte*, uint16_t inLength) {
            // TODO
        }
        
        
        void sendTimeCodeQuarterFrame(DataByte typeNibble, DataByte valuesNibble) {
            const byte data = byte((((typeNibble & 0x07) << 4) | (valuesNibble & 0x0f)));
            sendTimeCodeQuarterFrame(data);
        }
        
        void sendTimeCodeQuarterFrame(DataByte data) {
            sendSystemCommonMessage(MidiType::TimeCodeQuarterFrame, data);
        }
        
        void sendSongPosition(unsigned short beats) {
            byte data1 = beats & 0x7f;
            byte data2 = (beats >> 7) & 0x7f;
            
            sendSystemCommonMessage(MidiType::SongPosition, data1, data2);
        }
        
        void sendSongSelect(DataByte number) {
            sendSystemCommonMessage(MidiType::SongSelect, number & 0x7f);
        }
        
        void sendTuneRequest() {
            sendSystemCommonMessage(MidiType::TuneRequest);
        }
        
        void sendActiveSensing() {
            sendSystemCommonMessage(MidiType::ActiveSensing);
        }
        
        
        void sendStart() {
            sendRealTimeMessage(MidiType::Start);
        }
        
        void sendContinue() {
            sendRealTimeMessage(MidiType::Continue);
        }
        
        void sendStop() {
            sendRealTimeMessage(MidiType::Stop);
        }
        
        void sendClock() {
            sendRealTimeMessage(MidiType::Clock);
        }
        
        void sendTick() {
            sendRealTimeMessage(MidiType::Tick);
        }
        
        void sendReset() {
            sendRealTimeMessage(MidiType::SystemReset);
        }
        
        
        //receiving
        void setHandleNoteOff(void (*fptr)(byte channel, byte note, byte velocity)) {
            _noteOffCallback = fptr;
        }
        void setHandleNoteOn(void (*fptr)(byte channel, byte note, byte velocity)) {
            _noteOnCallback = fptr;
        }
        void setHandleAfterTouchPoly(void (*fptr)(byte channel, byte note, byte pressure)) {
            _afterTouchPolyCallback = fptr;
        }
        void setHandleControlChange(void (*fptr)(byte channel, byte number, byte value)) {
            _controlChangeCallback = fptr;
        }
        void setHandleProgramChange(void (*fptr)(byte channel, byte number)) {
            _programChangeCallback = fptr;
        }
        void setHandleAfterTouchChannel(void (*fptr)(byte channel, byte pressure)) {
            _afterTouchChannelCallback = fptr;
        }
        void setHandlePitchBend(void (*fptr)(byte channel, int bend)) {
            _pitchBendCallback = fptr;
        }
        void setHandleSysEx(void (*fptr)(const byte * data, uint16_t size)) {
            _sysExCallback = fptr;
        }
        void setHandleTimeCodeQuarterFrame(void (*fptr)(byte data)) {
            _timeCodeQuarterFrameCallback = fptr;
        }
        void setHandleSongPosition(void (*fptr)(unsigned short beats)) {
            _songPositionCallback = fptr;
        }
        void setHandleSongSelect(void (*fptr)(byte songnumber)) {
            _songSelectCallback = fptr;
        }
        void setHandleTuneRequest(void (*fptr)(void)) {
            _tuneRequestCallback = fptr;
        }
        void setHandleClock(void (*fptr)(void)) {
            _clockCallback = fptr;
        }
        void setHandleStart(void (*fptr)(void)) {
            _startCallback = fptr;
        }
        void setHandleContinue(void (*fptr)(void)) {
            _continueCallback = fptr;
        }
        void setHandleStop(void (*fptr)(void)) {
            _stopCallback = fptr;
        }
        void setHandleActiveSensing(void (*fptr)(void)) {
            _activeSensingCallback = fptr;
        }
        void setHandleReset(void (*fptr)(void)) {
            _resetCallback = fptr;
        }
        
    protected:
        
        // Channel messages
        virtual void sendChannelMessage(MidiType type, DataByte data1, DataByte data2, Channel channel)
        {
            // Then test if channel is valid
            if (channel >= MIDI_CHANNEL_OFF  ||
                channel == MIDI_CHANNEL_OMNI ||
                type < 0x80)
            {
                return; // Don't send anything
            }
            
            if (type <= MidiType::PitchBend)
            {
                // Channel messages
                
                // Protection: remove MSBs on data
                data1 &= 0x7f;
                data2 &= 0x7f;
                
                const StatusByte status = getStatus(type, channel);
                
                if (type != MidiType::ProgramChange && type != MidiType::AfterTouchChannel)
                {
                    serialize(status, data1, data2);
                }
                else
                {
                    serialize(status, data1);
                }
            }
            else if (type >= MidiType::Clock && type <= MidiType::SystemReset)
            {
                sendRealTimeMessage(type); // System Real-time and 1 byte.
            }
        }
        
        // SystemCommon message
        virtual void sendSystemCommonMessage(MidiType type, DataByte data1 = 0, DataByte data2 = 0)
        {
            
        }
        
        // RealTime messages
        virtual void sendRealTimeMessage(MidiType type)
        {
            // Do not invalidate Running Status for real-time messages
            // as they can be interleaved within any message.
            
            switch (type)
            {
                case Clock:
                case Start:
                case Stop:
                case Continue:
                case ActiveSensing:
                case SystemReset:
                    serialize(type);
                    break;
                default:
                    // Invalid Real Time marker
                    break;
            }
        }
        
        virtual bool begin(const char*) = 0;
        
        // serialize from the hardware
        virtual void read() = 0;
        
        // serialize towards to hardware
        virtual void serialize(DataByte) = 0;
        virtual void serialize(DataByte, DataByte) = 0;
        virtual void serialize(DataByte, DataByte, DataByte) = 0;
        
    protected:
    };
    
}

