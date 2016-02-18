/*!
 *  @file		AppleMIDI.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino
 *	Version		0.4
 *  @author		lathoub 
 *	@date		13/04/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

// http://www.blitter.com/~russtopia/MIDI/~jglatt/tech/midispec.htm

#include "AppleMidi.h"

#if !(APPLEMIDI_BUILD_INPUT) && !(APPLEMIDI_BUILD_OUTPUT)
#   error To use AppleMIDI, you need to enable at least input or output.
#endif
