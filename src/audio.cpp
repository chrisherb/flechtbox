#include <cstdio>
#include <memory>

#include "audio.hpp"
#include "clock.hpp"

const double SAMPLERATE = 48000;
const int BLOCKSIZE = 256;

void audio_run(std::shared_ptr<flechtbox_dsp> dsp) {
  // init dsp
  dsp_init(dsp, SAMPLERATE);

  // init portaudio
  PaStream *stream;
  PaError err;

  err = Pa_Initialize();
  if (err != paNoError)
    goto error;

  /* Open an audio I/O stream. */
  err = Pa_OpenDefaultStream(&stream, 0, /* no input channels */
                             2,          /* stereo output */
                             paFloat32,  /* 32 bit floating point output */
                             SAMPLERATE, BLOCKSIZE, /* frames per buffer */
                             portaudio_callback, &dsp);

  if (err != paNoError)
    goto error;

  err = Pa_StartStream(stream);
  if (err != paNoError)
    goto error;

  while (!dsp->should_quit) {
    Pa_Sleep(100);
  }

  err = Pa_StopStream(stream);
  if (err != paNoError)
    goto error;

  err = Pa_CloseStream(stream);
  if (err != paNoError)
    goto error;

  Pa_Terminate();

  printf("audio terminated\n");

error:
  if (stream) {
    // attempt to be safe, ignore errors
    Pa_StopStream(stream);
    Pa_CloseStream(stream);
  }
  Pa_Terminate();
  if (err != paNoError) { // Only print if error occurred!
    fprintf(stderr, "An error occurred while using the portaudio stream\n");
    fprintf(stderr, "Error number: %d\n", err);
    fprintf(stderr, "Error message: %s\n", Pa_GetErrorText(err));
  }
}

int portaudio_callback(const void *input, void *outputBuffer,
                       unsigned long framesPerBuffer,
                       const PaStreamCallbackTimeInfo *timeInfo,
                       PaStreamCallbackFlags statusFlags, void *userData) {
  /* Cast data passed through stream to our structure. */
  std::shared_ptr<flechtbox_dsp> *dsp =
      (std::shared_ptr<flechtbox_dsp> *)userData;

  float *out = (float *)outputBuffer;

  (void)input; /* Prevent unused variable warning. */

  for (int i = 0; i < framesPerBuffer; i++) {
    //*out++ = data->left_phase;  /* left */
    //*out++ = data->right_phase; /* right */
    clock_tick(dsp->get()->clock);
  }
  return 0;
}
