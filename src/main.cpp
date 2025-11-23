#include <csignal>
#include <ftxui/component/screen_interactive.hpp>
#include <memory>
#include <thread>

#include "audio.hpp"
#include "ui.hpp"

ftxui::ScreenInteractive *screen_ptr = nullptr;
std::shared_ptr<flechtbox_dsp> dsp;

void sig_int_handler(int) {
  if (screen_ptr) {
    screen_ptr->Exit();
  }
  if (dsp) {
    dsp->should_quit = true;
  }
}

int main() {
  dsp = std::make_shared<flechtbox_dsp>();
  auto screen = ftxui::ScreenInteractive::Fullscreen();
  screen_ptr = &screen;

  std::signal(SIGINT, sig_int_handler);

  // create audio thread
  std::thread audio_thread(audio_run, dsp);

  // run ui on main thread
  ui_run(*screen_ptr, dsp);

  audio_thread.join();
  return 0;
}
