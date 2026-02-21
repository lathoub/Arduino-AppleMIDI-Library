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
	uint32_t low32_;
	uint32_t high32_;

	void Init(uint64_t initialTimeStamp, uint32_t clockRate)
	{
		initialTimeStamp_ = initialTimeStamp;
		clockRate_ = clockRate;

		if (clockRate_ == 0)
		{
			clockRate_ = MIDI_SAMPLING_RATE_DEFAULT;
		}

		low32_ = millis();
		high32_ = 0;
		startTime_ = Ticks();
	}

	/// <summary>
	/// Returns a timestamp value suitable for inclusion in a RTP packet header.
	/// </summary>
	uint64_t Now()
	{
		return CalculateCurrentTimeStamp();
	}

private:
	uint64_t CalculateCurrentTimeStamp()
	{
        return initialTimeStamp_ + (CalculateTimeSpent() * clockRate_) / MSEC_PER_SEC;
	}

	/// <summary>
	///     Returns the time spent since the initial clock timestamp value.
	///     The returned value is expressed in milliseconds.
	/// </summary>
	uint64_t CalculateTimeSpent()
	{
        return Ticks() - startTime_;
	}

    /// <summary>
    ///     millis() as a 64bit (not the default 32bit)
    ///     this prevents wrap around.
	///     Note: rollover tracking is per instance; call Init() before use.
    /// </summary>
	uint64_t Ticks()
	{
        uint32_t new_low32 = millis();
        if (new_low32 < low32_) high32_++;
        low32_ = new_low32;
        return (uint64_t) high32_ << 32 | low32_;
	}


} RtpMidiClock_t;

END_APPLEMIDI_NAMESPACE
