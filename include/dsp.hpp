#pragma once

#include <array>
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

struct plaits_voice {
	plaits::Patch patch;
	plaits::Modulations modulations;
	plaits::Voice* voice;
	plaits::Voice::Frame* frames;

	char* shared_buffer;
};

void plaits_voice_init(plaits_voice& p);

struct flechtbox_dsp {
	metronome clock;
	parameters params;

	std::array<track_seq, NUM_TRACKS> track_sequencers {};
	std::array<plaits_voice, NUM_TRACKS> plaits_voices {};

	std::atomic<bool> should_quit {false};
};

void dsp_init(std::shared_ptr<flechtbox_dsp> dsp);

void dsp_process_block(std::shared_ptr<flechtbox_dsp> dsp, float* out, int frames);
