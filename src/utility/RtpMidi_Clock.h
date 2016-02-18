/*!
 *  @file		RtpMidi_Clock.h
 *  Project		Arduino AppleMIDI Library
 *	@brief		AppleMIDI Library for the Arduino 
 *	Version		0.3
 *  @author		lathoub 
 *	@date		27/06/14
 *  License		Code is open source so please feel free to do anything you want with it; you buy me a beer if you use this and we meet someday (Beerware license).
 */

#pragma once

#include <limits.h>

BEGIN_APPLEMIDI_NAMESPACE

#define MSEC_PER_SEC 1000
#define USEC_PER_SEC 1000000
#define NSEC_PER_SEC 1000000000


typedef struct RtpMidi_Clock {

	uint32_t clockRate_;

    unsigned long startTime_;
    uint32_t timestamp_;

	void Init(uint32_t initialTimeStamp, uint32_t clockRate)
	{
		timestamp_ = initialTimeStamp;
		clockRate_ = clockRate;
		  
		if( clockRate_ == 0 ) 
			clockRate_ = (USEC_PER_SEC / 1);

        startTime_ = Ticks();
	}

    /// <summary>
    ///     Returns an timestamp value suitable for inclusion in a RTP packet header.
    /// </summary>
    uint32_t Now()
    {
        return CalculateCurrentTimeStamp();
    }

    /// <summary>
    ///     Returns the time spent since the initial clock timestamp value.
    ///     The returned value is expressed in units of "clock pulsations",
    ///     that are equivalent to seconds, scaled by the clock rate.
    ///     i.e: 1 second difference will result in a delta value equals to the clock rate.
    /// </summary>
    uint32_t Delta()
    {
        return CalculateTimeSpent();
    }

	uint32_t CalculateCurrentTimeStamp()
	{
		uint32_t lapse = CalculateTimeSpent();

		// check for potential overflow
		if (timestamp_ + lapse < UINT_MAX )
			return timestamp_ + lapse;

		uint32_t remainder = UINT_MAX  - timestamp_;
		return lapse - remainder;
	}

	uint32_t CalculateTimeSpent()
	{
		unsigned long ticks = millis() - startTime_;
		unsigned long seconds = ticks / TicksPerSecond();

		uint32_t lapse = (uint32_t)(static_cast<double>(seconds) * clockRate_);
		return lapse;
	}

	unsigned long Ticks() const
	{
		return millis();
	}

	unsigned long TicksPerSecond() const
	{
		return 1000;
	}

} RtpMidiClock_t;

END_APPLEMIDI_NAMESPACE
