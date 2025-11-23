#pragma once

#include <array>
#include <cstdlib>

#include "clock.hpp"

enum playback_directions {
	PB_FORWARD,
	PB_BACKWARD,
	PB_PENDULUM,
	PB_RANDOM
};

struct step_state {
	bool enabled = false;
	int probability = 100;
};

struct track_seq {
	unsigned int current_pos = 0;
	unsigned int length = 10;
	playback_directions playback_dir = PB_FORWARD;
	std::array<step_state, 10> data {};
	bool pendulum_forward = true;
	clock_division division = CL_SIXTEENTH;
};

inline step_state& track_seq_process_step(track_seq& t)
{
	// play head advancement
	if (t.playback_dir == PB_FORWARD ||
		t.playback_dir == PB_PENDULUM && t.pendulum_forward)
		t.current_pos++;
	else if (t.playback_dir == PB_BACKWARD ||
			 t.playback_dir == PB_PENDULUM && !t.pendulum_forward)
		t.current_pos--;
	else if (t.playback_dir == PB_RANDOM) t.current_pos = rand() % t.length;

	// play head reset
	switch (t.playback_dir) {
	case PB_FORWARD:
		if (t.current_pos > t.length - 1) t.current_pos = 0;
		break;
	case PB_BACKWARD:
		if (t.current_pos < 0) t.current_pos = t.length - 1;
		break;
	case PB_PENDULUM:
		if (t.pendulum_forward && t.current_pos >= t.length - 1)
			t.pendulum_forward = false;
		else if (!t.pendulum_forward && t.current_pos <= 0) t.pendulum_forward = true;
		break;
	case PB_RANDOM:
		break;
	}

	return t.data[t.current_pos];
}

inline step_state*
track_seq_process_sample(track_seq& t,
						 const std::array<bool, CL_NUM_CLOCK_DIVISIONS>& clock_states)
{

	if (clock_states[t.division]) return &track_seq_process_step(t);

	return nullptr;
}
