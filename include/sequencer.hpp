#pragma once

#include <array>
#include <climits>
#include <cstdlib>
#include <ctime>

#include "clock.hpp"

const int SEQ_NULL = INT_MIN;

enum playback_directions {
	PB_FORWARD,
	PB_BACKWARD,
	PB_PENDULUM,
	PB_RANDOM
};

struct track_seq {
	unsigned int current_pos = 0;
	int length = 10;
	int playback_dir = PB_FORWARD;
	std::array<int, 10> data;
	bool pendulum_forward = true;
	clock_division division = CL_SIXTEENTH;
	int last_value;
};

inline void track_seq_init(track_seq& t) { t.data.fill(0); }

inline int
track_seq_process_step(track_seq& t,
					   const std::array<bool, CL_NUM_CLOCK_DIVISIONS>& clock_states)
{
	// only process if we land on a division;
	if (!clock_states[t.division]) return SEQ_NULL;

	// read current position
	int tmp_pos = t.current_pos;

	// play head advancement
	if (t.playback_dir == PB_FORWARD ||
		(t.playback_dir == PB_PENDULUM && t.pendulum_forward))
		tmp_pos++;
	else if (t.playback_dir == PB_BACKWARD ||
			 (t.playback_dir == PB_PENDULUM && !t.pendulum_forward))
		tmp_pos--;
	else if (t.playback_dir == PB_RANDOM) tmp_pos = rand() % t.length;

	const int max_index = t.length - 1;

	// play head reset
	switch (t.playback_dir) {
	case PB_FORWARD:
		if (tmp_pos > max_index) tmp_pos = 0;
		break;
	case PB_BACKWARD:
		if (tmp_pos < 0) tmp_pos = max_index;
		break;
	case PB_PENDULUM:
		if (t.pendulum_forward && tmp_pos >= max_index) t.pendulum_forward = false;
		else if (!t.pendulum_forward && tmp_pos <= 0) t.pendulum_forward = true;
		break;
	case PB_RANDOM:
		break;
	}

	// write current position
	t.current_pos = tmp_pos;

	t.last_value = t.data[t.current_pos];
	return t.last_value;
}
