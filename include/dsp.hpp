#pragma once

#include <array>
#include <plaits/dsp/dsp.h>

#include "clock.hpp"
#include "parameters.hpp"
#include "sequencer.hpp"

const double SAMPLERATE = 48000;
const int BLOCKSIZE = 16;

struct flechtbox_dsp {
  metronome clock;
  parameters params;

  std::array<track_seq, 9> track_sequencers;

  std::atomic<bool> should_quit{false};
};

const int NUM_TRACKS = 9;

void dsp_init(flechtbox_dsp &dsp, double samplerate);

void dsp_process_block(flechtbox_dsp &dsp, float *out, int frames);
