/*!
 *  @brief      AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include "utility/AppleMidi_Namespace.h"

// -----------------------------------------------------------------------------

// Here are a few settings you can change to customize
// the library for your own project. You can for example
// choose to compile only parts of it so you gain flash
// space and optimise the speed of your sketch.

// -----------------------------------------------------------------------------

// Compilation flags. Set them to 1 to build the associated feature
// (MIDI in, out, thru), or to 0 to disable the feature and save space.
// Note that thru can work only if input and output are enabled.

//#define DEBUG
#define RELEASE

#if defined(RELEASE)
#define RELEASE_BUILD
#undef DEBUG_BUILD
#endif

#if defined(DEBUG)
#define DEBUG_BUILD
#undef RELEASE_BUILD
#endif


#if defined(RELEASE_BUILD)
#undef APPLEMIDI_DEBUG
#undef APPLEMIDI_DEBUG_VERBOSE
#endif

#if defined(DEBUG_BUILD)
#define APPLEMIDI_DEBUG			   1
#undef APPLEMIDI_DEBUG_VERBOSE
#define APPLEMIDI_DEBUG_PARSING
#endif

#define APPLEMIDI_BUILD_INPUT      1
#define APPLEMIDI_BUILD_OUTPUT     1

#define APPLEMIDI_USE_CALLBACKS    1

// -----------------------------------------------------------------------------
// If you the device is *not* initiating sessions, 
// put this switch off (0). It saves computing cycles and memory (over 2000 bytes)

#define APPLEMIDI_REMOTE_SESSIONS
//#undef APPLEMIDI_REMOTE_SESSIONS

// -----------------------------------------------------------------------------
// Rtp UDP ports

#define CONTROL_PORT     5004

// -----------------------------------------------------------------------------
// Misc. options

// Running status enables short messages when sending multiple values
// of the same type and channel.
// Set to 0 if you have troubles controlling your hardware.
#define APPLEMIDI_USE_RUNNING_STATUS         0

#define MIDI_SYSEX_ARRAY_SIZE           255

#define MIDI_SYSEX_ARRAY_SIZE_CONTENT   (MIDI_SYSEX_ARRAY_SIZE - 2)

// Max size of dissectable packet
#define PACKET_MAX_SIZE 350

// -----------------------------------------------------------------------------

BEGIN_APPLEMIDI_NAMESPACE

struct DefaultSettings
{
    /*! Running status enables short messages when sending multiple values
     of the same type and channel.\n
     Warning: does not work with some hardware, enable with caution.
     */
    static const bool UseRunningStatus = false;

    /*! NoteOn with 0 velocity should be handled as NoteOf.\n
     Set to true  to get NoteOff events when receiving null-velocity NoteOn messages.\n
     Set to false to get NoteOn  events when receiving null-velocity NoteOn messages.
     */
    static const bool HandleNullVelocityNoteOnAsNoteOff = true;
    
    /*! Maximum size of SysEx receivable. Decrease to save RAM if you don't expect
     to receive SysEx, or adjust accordingly.
     */
    static const unsigned SysExMaxSize = 128;
};

END_APPLEMIDI_NAMESPACE
