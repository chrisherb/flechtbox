#pragma once

#include <array>
#include <cstdio>

enum clock_division {
	CL_WHOLE,
	CL_HALF,
	CL_QUARTER,
	CL_EIGHTH,
	CL_SIXTEENTH,
	CL_THIRTYSECOND,
	CL_NUM_CLOCK_DIVISIONS
};

struct metronome {
	double samplerate = 48000;
	float tempo = 120.f;
	bool running = false;
	bool quarter_gate = false;
	bool thirtysecond_gate = false;

	std::array<double, CL_NUM_CLOCK_DIVISIONS> phases = {0.0}; // For each clock
	std::array<bool, CL_NUM_CLOCK_DIVISIONS> cur_clock_states = {false};
};

inline void clock_process_block(metronome& c, int frames)
{
	if (!c.running) return;

	// Division multipliers (relative to quarter note)
	static constexpr double division_multiplier[CL_NUM_CLOCK_DIVISIONS] = {
		1.0 / 4.0, // whole note
		1.0 / 2.0, // half note
		1.0,	   // quarter note
		2.0,	   // eighth
		4.0,	   // sixteenth
		8.0		   // thirtysecond
	};

	for (int div = 0; div < CL_NUM_CLOCK_DIVISIONS; ++div)
		c.cur_clock_states[div] = false;

	for (int i = 0; i < frames; ++i) {
		for (int div = 0; div < CL_NUM_CLOCK_DIVISIONS; ++div) {
			// Calculate frequency (beats per second)
			double bps = (c.tempo / 60.0) * division_multiplier[div];
			double phase_inc = bps / c.samplerate;
			c.phases[div] += phase_inc;
			if (c.phases[div] >= 1.0) {
				c.phases[div] -= 1.0;
				c.cur_clock_states[div] = true;
			}
		}

		// gates for visualization
		double quarter_phase = c.phases[CL_QUARTER];
		double thirty_second_phase = c.phases[CL_THIRTYSECOND];
		c.quarter_gate = (quarter_phase < 0.5);
		c.thirtysecond_gate = (thirty_second_phase < 0.5);
	}
}
