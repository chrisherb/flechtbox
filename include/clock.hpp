#pragma once

#include <array>
#include <atomic>
#include <cstdio>

enum clock_division {
  CL_WHOLE,
  CL_HALF,
  CL_QUARTER,
  CL_EIGHTH,
  CL_SIXTEENTH,
  CL_THIRTYSECOND,
  CL_NUM_CLOCK_DIVISIONS
};

struct metronome {
  double samplerate = 48000;
  float tempo = 120.f;

  std::atomic<bool> running = true;
  std::atomic<bool> quarter_gate = false;      // for the blinkenlights
  std::atomic<bool> thirtysecond_gate = false; // for ui redraw

  unsigned long count = 0;

  std::array<bool, CL_NUM_CLOCK_DIVISIONS> cur_clock_states;
};

inline std::array<bool, CL_NUM_CLOCK_DIVISIONS> &clock_tick(metronome &c) {

  int q_smp = (60.f / c.tempo) * c.samplerate;

  // Prevent division by zero
  if (q_smp) {
    // determine clock states
    c.cur_clock_states[CL_WHOLE] = c.count % (q_smp * 4) == 0;
    c.cur_clock_states[CL_HALF] = c.count % (q_smp * 2) == 0;
    c.cur_clock_states[CL_QUARTER] = c.count % q_smp == 0;
    c.cur_clock_states[CL_EIGHTH] = c.count % (q_smp / 2) == 0;
    c.cur_clock_states[CL_SIXTEENTH] = c.count % (q_smp / 4) == 0;
    c.cur_clock_states[CL_THIRTYSECOND] = c.count % (q_smp / 8) == 0;

    // toggle quarter gate (for blinkenlight)
    c.quarter_gate = c.count % q_smp < (q_smp / 2);
    c.thirtysecond_gate = c.count % (q_smp / 8) < (q_smp / 16);
  }

  c.count++;

  return c.cur_clock_states;
}
