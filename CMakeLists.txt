cmake_minimum_required(VERSION 3.15)
project(flechtbox LANGUAGES CXX)

# Set C++ standard and warnings
set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_EXPORT_COMPILE_COMMANDS ON)
add_compile_options(-Wall)

# plaits source folders
set(MI_DIR ${PROJECT_SOURCE_DIR}/lib/eurorack)
set(STMLIB_DIR ${MI_DIR}/stmlib)
set(PLAITS_DIR ${MI_DIR}/plaits)

# List main project sources
set(PROJECT_SRCS
  src/main.cpp
  src/audio.cpp
  src/ui.cpp
  src/dsp.cpp
)

set(MI_DIR ${PROJECT_SOURCE_DIR}/lib/eurorack)
set(STMLIB_DIR ${MI_DIR}/stmlib)
set(PLAITS_DIR ${MI_DIR}/plaits)
set(CLOUDS_DIR ${MI_DIR}/clouds)

# Gather all required files in Mutable Instruments and Plaits
set(MI_SRCS
    ${STMLIB_DIR}/stmlib.h
    ${STMLIB_DIR}/utils/buffer_allocator.h
    ${STMLIB_DIR}/utils/dsp.h
    ${STMLIB_DIR}/utils/random.cc
    ${STMLIB_DIR}/utils/random.h
    ${STMLIB_DIR}/dsp/atan.cc
    ${STMLIB_DIR}/dsp/atan.h
    ${STMLIB_DIR}/dsp/units.cc
    ${STMLIB_DIR}/dsp/units.h
    ${STMLIB_DIR}/dsp/filter.h
    ${STMLIB_DIR}/dsp/cosine_oscillator.h
    ${CLOUDS_DIR}/dsp/frame.h
    ${CLOUDS_DIR}/dsp/fx/reverb.h
    ${CLOUDS_DIR}/dsp/fx/fx_engine.h
    ${PLAITS_DIR}/resources.cc
    ${PLAITS_DIR}/resources.h
    ${PLAITS_DIR}/dsp/voice.cc
    ${PLAITS_DIR}/dsp/voice.h
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth.cc
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth.h
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth_controller.cc
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth_controller.h
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth_phonemes.cc
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth_words.cc
    ${PLAITS_DIR}/dsp/speech/lpc_speech_synth_words.h
    ${PLAITS_DIR}/dsp/speech/naive_speech_synth.cc
    ${PLAITS_DIR}/dsp/speech/naive_speech_synth.h
    ${PLAITS_DIR}/dsp/speech/sam_speech_synth.cc
    ${PLAITS_DIR}/dsp/speech/sam_speech_synth.h
    ${PLAITS_DIR}/dsp/drums/analog_bass_drum.h
    ${PLAITS_DIR}/dsp/drums/analog_snare_drum.h
    ${PLAITS_DIR}/dsp/drums/hi_hat.h
    ${PLAITS_DIR}/dsp/drums/synthetic_bass_drum.h
    ${PLAITS_DIR}/dsp/drums/synthetic_snare_drum.h
    ${PLAITS_DIR}/dsp/dsp.h
    ${PLAITS_DIR}/dsp/engine/additive_engine.cc
    ${PLAITS_DIR}/dsp/engine/additive_engine.h
    ${PLAITS_DIR}/dsp/engine/bass_drum_engine.cc
    ${PLAITS_DIR}/dsp/engine/bass_drum_engine.h
    ${PLAITS_DIR}/dsp/engine/chord_engine.cc
    ${PLAITS_DIR}/dsp/engine/chord_engine.h
    ${PLAITS_DIR}/dsp/engine/engine.h
    ${PLAITS_DIR}/dsp/engine/fm_engine.cc
    ${PLAITS_DIR}/dsp/engine/fm_engine.h
    ${PLAITS_DIR}/dsp/engine/grain_engine.cc
    ${PLAITS_DIR}/dsp/engine/grain_engine.h
    ${PLAITS_DIR}/dsp/engine/hi_hat_engine.cc
    ${PLAITS_DIR}/dsp/engine/hi_hat_engine.h
    ${PLAITS_DIR}/dsp/engine/modal_engine.cc
    ${PLAITS_DIR}/dsp/engine/modal_engine.h
    ${PLAITS_DIR}/dsp/engine/noise_engine.cc
    ${PLAITS_DIR}/dsp/engine/noise_engine.h
    ${PLAITS_DIR}/dsp/engine/particle_engine.cc
    ${PLAITS_DIR}/dsp/engine/particle_engine.h
    ${PLAITS_DIR}/dsp/engine/snare_drum_engine.cc
    ${PLAITS_DIR}/dsp/engine/snare_drum_engine.h
    ${PLAITS_DIR}/dsp/engine/speech_engine.cc
    ${PLAITS_DIR}/dsp/engine/speech_engine.h
    ${PLAITS_DIR}/dsp/engine/string_engine.cc
    ${PLAITS_DIR}/dsp/engine/string_engine.h
    ${PLAITS_DIR}/dsp/engine/swarm_engine.cc
    ${PLAITS_DIR}/dsp/engine/swarm_engine.h
    ${PLAITS_DIR}/dsp/engine/virtual_analog_engine.cc
    ${PLAITS_DIR}/dsp/engine/virtual_analog_engine.h
    ${PLAITS_DIR}/dsp/engine/waveshaping_engine.cc
    ${PLAITS_DIR}/dsp/engine/waveshaping_engine.h
    ${PLAITS_DIR}/dsp/engine/wavetable_engine.cc
    ${PLAITS_DIR}/dsp/engine/wavetable_engine.h
    ${PLAITS_DIR}/dsp/engine2/arpeggiator.h
    ${PLAITS_DIR}/dsp/engine2/chiptune_engine.cc
    ${PLAITS_DIR}/dsp/engine2/chiptune_engine.h
    ${PLAITS_DIR}/dsp/engine2/phase_distortion_engine.cc
    ${PLAITS_DIR}/dsp/engine2/phase_distortion_engine.h
    ${PLAITS_DIR}/dsp/engine2/six_op_engine.cc
    ${PLAITS_DIR}/dsp/engine2/six_op_engine.h
    ${PLAITS_DIR}/dsp/engine2/string_machine_engine.cc
    ${PLAITS_DIR}/dsp/engine2/string_machine_engine.h
    ${PLAITS_DIR}/dsp/engine2/virtual_analog_vcf_engine.cc
    ${PLAITS_DIR}/dsp/engine2/virtual_analog_vcf_engine.h
    ${PLAITS_DIR}/dsp/engine2/wave_terrain_engine.cc
    ${PLAITS_DIR}/dsp/engine2/wave_terrain_engine.h
    ${PLAITS_DIR}/dsp/envelope.h
    ${PLAITS_DIR}/dsp/fm/algorithms.cc
    ${PLAITS_DIR}/dsp/fm/algorithms.h
    ${PLAITS_DIR}/dsp/fm/dx_units.cc
    ${PLAITS_DIR}/dsp/fm/dx_units.h
    ${PLAITS_DIR}/dsp/fm/envelope.h
    ${PLAITS_DIR}/dsp/fm/lfo.h
    ${PLAITS_DIR}/dsp/fm/operator.h
    ${PLAITS_DIR}/dsp/fm/patch.h
    ${PLAITS_DIR}/dsp/fm/voice.h
    ${PLAITS_DIR}/dsp/fx/diffuser.h
    ${PLAITS_DIR}/dsp/fx/fx_engine.h
    ${PLAITS_DIR}/dsp/fx/low_pass_gate.h
    ${PLAITS_DIR}/dsp/fx/overdrive.h
    ${PLAITS_DIR}/dsp/fx/sample_rate_reducer.h
    ${PLAITS_DIR}/dsp/noise/clocked_noise.h
    ${PLAITS_DIR}/dsp/noise/dust.h
    ${PLAITS_DIR}/dsp/noise/fractal_random_generator.h
    ${PLAITS_DIR}/dsp/noise/particle.h
    ${PLAITS_DIR}/dsp/noise/smooth_random_generator.h
    ${PLAITS_DIR}/dsp/oscillator/formant_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/grainlet_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/harmonic_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/sine_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/string_synth_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/variable_saw_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/variable_shape_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/vosim_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/wavetable_oscillator.h
    ${PLAITS_DIR}/dsp/oscillator/z_oscillator.h
    ${PLAITS_DIR}/dsp/physical_modelling/delay_line.h
    ${PLAITS_DIR}/dsp/physical_modelling/modal_voice.cc
    ${PLAITS_DIR}/dsp/physical_modelling/modal_voice.h
    ${PLAITS_DIR}/dsp/physical_modelling/resonator.cc
    ${PLAITS_DIR}/dsp/physical_modelling/resonator.h
    ${PLAITS_DIR}/dsp/physical_modelling/string.cc
    ${PLAITS_DIR}/dsp/physical_modelling/string.h
    ${PLAITS_DIR}/dsp/physical_modelling/string_voice.cc
    ${PLAITS_DIR}/dsp/physical_modelling/string_voice.h
    ${PLAITS_DIR}/dsp/chords/chord_bank.cc
    ${PLAITS_DIR}/dsp/chords/chord_bank.h
)

# Combine sources
set(ALL_SRCS ${PROJECT_SRCS} ${MI_SRCS} ${MI_CPP_SRCS})

# Create executable
add_executable(flechtbox ${ALL_SRCS})

# Set include directories
target_include_directories(flechtbox
  PRIVATE
    lib/eurorack
    lib/eurorack/plaits
    lib/eurorack/stmlib
    lib/trnr-lib/util
    include
)

# Link system libraries
target_link_libraries(flechtbox
  PRIVATE
    portaudio
    ftxui-component
    ftxui-dom
    ftxui-screen
)

# If you ever want to build with JACK, add this option:
option(ENABLE_JACK "Enable JACK audio support" OFF)
if(ENABLE_JACK)
  target_link_libraries(flechtbox PRIVATE jack)
endif()

# Installation (optional)
install(TARGETS flechtbox RUNTIME DESTINATION bin)
