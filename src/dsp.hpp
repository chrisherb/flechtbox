#pragma once

#include <array>

#include "clock.hpp"
#include "parameters.hpp"
#include "sequencer.hpp"

struct flechtbox_dsp {
  double samplerate;
  metronome clock;
  parameters params;

  std::array<track_seq, 9> track_sequencers;
};

const int NUM_TRACKS = 9;

inline void dsp_processblock(flechtbox_dsp &dsp, float **outputs, int frames) {

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
