#include "dsp.hpp"

float plaits::kSampleRate = SAMPLERATE;
float plaits::kCorrectedSampleRate = SAMPLERATE;
float plaits::a0 = (440.0f / 8.0f) / plaits::kCorrectedSampleRate;

void dsp_init(flechtbox_dsp &dsp, double samplerate) {
  dsp.clock.samplerate = samplerate;
}

void dsp_process_block(flechtbox_dsp &dsp, float *out, int frames) {

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
