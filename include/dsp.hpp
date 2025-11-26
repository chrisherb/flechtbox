#pragma once

#include <array>
#include <atomic>
#include <memory>
#include <plaits/dsp/dsp.h>
#include <plaits/dsp/voice.h>

#include "clock.hpp"
#include "parameters.hpp"
#include "sequencer.hpp"

const double SAMPLERATE = 48000;
const int BLOCKSIZE = 512;
const int PLAITS_BLOCKSIZE = 16;

const int NUM_TRACKS = 9;
const int NUM_STEPS = 10;

struct flechtbox_track {
	int pitch = 48;

	bool global_pitch_enabled = true;
	bool global_velocity_enabled = true;
	bool global_octave_enabled = true;

	float current_velocity = 1.f;

	plaits::Patch patch;
	plaits::Modulations modulations;
	plaits::Voice* voice;
	plaits::Voice::Frame* frames;

	char* shared_buffer;
	bool enabled = true;
	bool muted = false;

	float volume = 1.f;

	track_seq sequencer;
};

void plaits_voice_init(flechtbox_track& p);

struct flechtbox_dsp {
	metronome clock;
	parameters params;

	track_seq pitch_sequence;
	track_seq velocity_sequence;
	track_seq octave_sequence;

	std::array<flechtbox_track, NUM_TRACKS> tracks {};

	std::atomic<bool> should_quit {false};
};

void dsp_init(std::shared_ptr<flechtbox_dsp> dsp);

void dsp_process_block(std::shared_ptr<flechtbox_dsp> dsp, float* out, int frames);

inline float soft_clip(float x)
{
	if (x < -3.f) {
		return -1.f;
	} else if (x > 3.f) {
		return 1.f;
	} else {
		return x * (27.f + x * x) / (27.f + 9.f * x * x);
	}
}
