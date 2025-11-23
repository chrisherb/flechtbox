#include <array>

struct param_step {
  bool enabled = false;
  int probability = 100;
};

struct param_track {
  std::array<param_step, 10> steps;
  unsigned int length;
};

enum param_scale { T_MAJOR, T_MINOR };

struct param_master {
  std::array<int, 10> pitch_steps;
  unsigned int pitch_length;
  std::array<int, 10> velocity_steps;
  unsigned int velocity_length;
  std::array<int, 10> octave_steps;
  unsigned int octave_length;

  param_scale scale = T_MINOR;
};

struct parameters {
  std::array<param_track, 9> tracks;
  param_master master;
};
