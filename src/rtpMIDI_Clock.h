#pragma once

#include <stdint.h>

#include "AppleMIDI_Namespace.h"

BEGIN_APPLEMIDI_NAMESPACE

#define MSEC_PER_SEC 1000

typedef struct rtpMidi_Clock
{
	uint32_t clockRate_;

	uint64_t startTime_;
	uint64_t initialTimeStamp_;

	void Init(uint64_t initialTimeStamp, uint32_t clockRate)
	{
		initialTimeStamp_ = 0;
		clockRate_ = clockRate;

		if (clockRate_ == 0)
		{
			clockRate_ = MIDI_SAMPLING_RATE_DEFAULT;
		}

		startTime_ = Ticks();
	}

	/// <summary>
	/// Returns an timestamp value suitable for inclusion in a RTP packet header.
	/// </summary>
	uint64_t Now()
	{
		return CalculateCurrentTimeStamp();
	}

private:
	uint64_t CalculateCurrentTimeStamp()
	{
        return  (CalculateTimeSpent() * clockRate_) / MSEC_PER_SEC;
	}

	/// <summary>
	///     Returns the time spent since the initial clock timestamp value.
	///     The returned value is expressed in units of "clock pulsations",
	///     that are equivalent to seconds, scaled by the clock rate.
	///     i.e: 1 second difference will result in a delta value equals to the clock rate.
	/// </summary>
	uint64_t CalculateTimeSpent()
	{
        return Ticks() - startTime_;
	}

    /// <summary>
    ///     millis() as a 64bit (not the default 32bit)
    ///     this prevents wrap around.
    /// </summary>
	uint64_t Ticks() const
	{
        static uint32_t low32, high32;
        uint32_t new_low32 = millis();
        if (new_low32 < low32) high32++;
        low32 = new_low32;
        return (uint64_t) high32 << 32 | low32;
	}


} RtpMidiClock_t;

END_APPLEMIDI_NAMESPACE
