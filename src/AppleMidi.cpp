/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub
 *  License		CC BY-SA 4.0
 */

// http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec.htm

#include "AppleMidi.h"

#if !(APPLEMIDI_BUILD_INPUT) && !(APPLEMIDI_BUILD_OUTPUT)
#   error To use AppleMIDI, you need to enable at least input or output.
#endif
