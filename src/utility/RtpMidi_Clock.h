/*!
 *	@brief		AppleMIDI Library for the Arduino
 *  @author		lathoub, hackmancoltaire
 */

#pragma once

#include <stdint.h>

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

		if (clockRate_ == 0) {
			clockRate_ = MIDI_SAMPLING_RATE_DEFAULT;
		}

		startTime_ = Ticks();
	}

	/// <summary>
	///     Returns an timestamp value suitable for inclusion in a RTP packet header.
	/// </summary>
	uint32_t Now()
	{
		return CalculateCurrentTimeStamp();
	}

private:
	uint32_t CalculateCurrentTimeStamp()
	{
		uint32_t lapse = CalculateTimeSpent();

		// check for potential overflow
		if (lapse < UINT32_MAX - timestamp_)
			return timestamp_ + lapse;

		uint32_t remainder = UINT32_MAX - timestamp_;
		return lapse - remainder;
	}

	/// <summary>
	///     Returns the time spent since the initial clock timestamp value.
	///     The returned value is expressed in units of "clock pulsations",
	///     that are equivalent to seconds, scaled by the clock rate.
	///     i.e: 1 second difference will result in a delta value equals to the clock rate.
	/// </summary>
	uint32_t CalculateTimeSpent()
	{
		uint32_t ticks = Ticks() - startTime_;
		uint32_t lapse = (ticks * clockRate_) / MSEC_PER_SEC;
		return lapse;
	}

	inline unsigned long Ticks() const
	{
		return millis();
	}

} RtpMidiClock_t;

END_APPLEMIDI_NAMESPACE
