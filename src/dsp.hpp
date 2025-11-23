#pragma once

#include <array>

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

inline void dsp_init(flechtbox_dsp &dsp, double samplerate) {
  dsp.clock.samplerate = samplerate;
}

inline void dsp_process_block(flechtbox_dsp &dsp, float *out, int frames) {

  for (int i = 0; i < frames; i++) {
    // get clock states for current sample
    // (if sample is on an eigth/sixteenth/etc.)
    auto clock_states = clock_tick(dsp.clock);

    for (int i = 0; i < NUM_TRACKS; i++) {
      auto track_step_state =
          track_seq_process_sample(dsp.track_sequencers[i], clock_states);
    }
  }
}
