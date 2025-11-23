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
	bool quarter_gate = false;		// for the blinkenlights
	bool thirtysecond_gate = false; // for ui redraw

	unsigned long count = 0;

	std::array<bool, CL_NUM_CLOCK_DIVISIONS> cur_clock_states;
};

inline void clock_process_block(metronome& c, int frames)
{
	// reset
	c.cur_clock_states[CL_WHOLE] = false;
	c.cur_clock_states[CL_HALF] = false;
	c.cur_clock_states[CL_QUARTER] = false;
	c.cur_clock_states[CL_EIGHTH] = false;
	c.cur_clock_states[CL_SIXTEENTH] = false;
	c.cur_clock_states[CL_THIRTYSECOND] = false;

	if (!c.running) return;

	for (int i = 0; i < frames; i++) {
		int q_smp = (60.f / c.tempo) * c.samplerate;

		// Prevent division by zero
		if (q_smp) {
			// determine clock states
			if (c.count % (q_smp * 4) == 0) c.cur_clock_states[CL_WHOLE] = true;
			if (c.count % (q_smp * 2) == 0) c.cur_clock_states[CL_HALF] = true;
			if (c.count % q_smp == 0) c.cur_clock_states[CL_QUARTER] = true;
			if (c.count % (q_smp / 2) == 0) c.cur_clock_states[CL_EIGHTH] = true;
			if (c.count % (q_smp / 4) == 0) c.cur_clock_states[CL_SIXTEENTH] = true;
			if (c.count % (q_smp / 8) == 0) c.cur_clock_states[CL_THIRTYSECOND] = true;

			c.quarter_gate = c.count % q_smp < (q_smp / 2);
			c.thirtysecond_gate = c.count % (q_smp / 8) < (q_smp / 16);
		}

		c.count++;
	}
}
