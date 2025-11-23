#pragma once

#include <memory>
#include <portaudio.h>

#include "dsp.hpp"

// struct flechtbox_dsp {
//   metronome clock;
//   std::atomic<bool> should_quit{false};
// };
//
// inline void dsp_init(std::shared_ptr<flechtbox_dsp> dsp, double samplerate) {
//   dsp->clock.samplerate = samplerate;
// }

// called by portaudio when audio is needed.
int portaudio_callback(const void *input, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags, void *userData);

void audio_run(std::shared_ptr<flechtbox_dsp> dsp);
