#pragma once

#include <array>
#include <atomic>
#include <audio_buffer.h>
#include <memory>
#include <plaits/dsp/dsp.h>
#include <plaits/dsp/voice.h>

#include "clock.hpp"
#include "parameters.hpp"
#include "reverb.hpp"
#include "sequencer.hpp"

const double SAMPLERATE = 48000;
const int BLOCKSIZE = 512;
const int PLAITS_BLOCKSIZE = 16;

const int NUM_TRACKS = 9;
const int NUM_STEPS = 10;

struct flechtbox_track {
	int pitch = 48;

	float harmonics = 0.5f;
	float harmonics_rand_amt = 0.f;
	float harmonics_rand_val = 0.f;
	float timbre = 0.5f;
	float timbre_rand_amt = 0.f;
	float timbre_rand_val = 0.f;
	float morph = 0.5f;
	float morph_rand_amt = 0.f;
	float morph_rand_val = 0.f;

	bool global_pitch_enabled = true;
	bool global_velocity_enabled = true;
	bool global_octave_enabled = true;

	float reverb_send_amt = 0.0f;

	float current_velocity = 1.f;

	plaits::Patch plaits_patch;
	plaits::Modulations plaits_mods;
	plaits::Voice* voice;
	plaits::Voice::Frame* frames;

	char* shared_buffer;
	bool enabled = true;
	bool muted = false;

	float volume = 1.f;

	track_seq sequencer;
};

void flechtbox_track_init(flechtbox_track& p);

struct flechtbox_dsp {
	metronome clock;
	parameters params;
	std::atomic<bool> should_quit {false};

	track_seq pitch_sequence;
	track_seq velocity_sequence;
	track_seq octave_sequence;

	std::array<flechtbox_track, NUM_TRACKS> tracks {};

	clouds_reverb reverb;

	trnr::audio_buffer<float> reverb_buffer;
	trnr::audio_buffer<float> mix_buffer;
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
