/*!
 *  @file       AppleMidi_Settings.h
 *  Project     Arduino AppleMIDI Library
 *  @brief      AppleMIDI Library for the Arduino
 *	Version		0.3
 *  @author		lathoub 
 *	@date		04/04/14
 *  license     GPL 
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

#define RELEASE_BUILD              1
#define DEBUG_BUILD

#if defined(RELEASE_BUILD)
#undef APPLEMIDI_DEBUG
#undef APPLEMIDI_DEBUG_VERBOSE
#undef DEBUG_BUILD
#endif

#if defined(DEBUG_BUILD)
#define APPLEMIDI_DEBUG			   1
#define APPLEMIDI_DEBUG_VERBOSE    1
#endif

#define APPLEMIDI_BUILD_INPUT      1
#define APPLEMIDI_BUILD_OUTPUT     1

#define APPLEMIDI_USE_CALLBACKS    1
#define APPLEMIDI_USE_EVENTS       1

// -----------------------------------------------------------------------------

// Rtp UDP ports

#define CONTROL_PORT     5004         
#define RTP_PORT         5005   

// -----------------------------------------------------------------------------

// Create a MIDI object automatically on the port defined with MIDI_SERIAL_PORT.
// You can turn this off by adding #define APPLEMIDI_AUTO_INSTANCIATE 0 just before
// including <MIDI.h> in your sketch.
#ifndef APPLEMIDI_AUTO_INSTANCIATE
#   ifdef ARDUINO
#       define APPLEMIDI_AUTO_INSTANCIATE    1
#   else
#       define APPLEMIDI_AUTO_INSTANCIATE    0   ///< @see APPLEMIDI_CREATE_INSTANCE
#   endif
#endif

// -----------------------------------------------------------------------------
// Default configuration (if APPLEMIDI_AUTO_INSTANCIATE is set)

// Set the default port to use for MIDI.
#if APPLEMIDI_AUTO_INSTANCIATE
#   ifdef ARDUINO
#       include "Arduino.h"
#   else
#       error Auto-instanciation disabled. Use APPLEMIDI_CREATE_INSTANCE macro.
#   endif
#endif

// -----------------------------------------------------------------------------
// Misc. options

// Running status enables short messages when sending multiple values
// of the same type and channel.
// Set to 0 if you have troubles controlling your hardware.
#define APPLEMIDI_USE_RUNNING_STATUS         0

// Setting this to 1 will make MIDI.read parse only one byte of data for each
// call when data is available. This can speed up your application if receiving
// a lot of traffic, but might induce MIDI Thru and treatment latency.
#define APPLEMIDI_USE_1BYTE_PARSING          1

#define MIDI_SYSEX_ARRAY_SIZE           255     // Maximum size is 65535 bytes.

#define MAX_SESSIONS 10

// -----------------------------------------------------------------------------

BEGIN_APPLEMIDI_NAMESPACE

END_APPLEMIDI_NAMESPACE
