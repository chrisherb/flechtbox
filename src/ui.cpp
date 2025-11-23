#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <string>
#include <vector>

#include "ui.hpp"

using namespace ftxui;

Element blinking_light(std::shared_ptr<flechtbox_dsp> dsp) {
  // Read the DSP state
  bool light_on = dsp->clock.quarter_gate;
  return light_on ? text("●") | color(Color::Green)
                  : text("○") | color(Color::GrayDark);
}

void ui_run(ftxui::ScreenInteractive &screen,
            std::shared_ptr<flechtbox_dsp> dsp) {
  std::vector<std::string> tab_values{
      "T1", "T2", "T4", "T5", "T6", "T7", "T8", "T9", "MT",
  };

  int tab_selected = 0;
  auto track_selector = Toggle(&tab_values, &tab_selected);

  auto steps_container = Container::Horizontal({});

  auto titled_probability_container = Renderer(steps_container, [&] {
    return window(text("probability") | bold, steps_container->Render());
  });

  auto track_container = Container::Tab(
      {titled_probability_container | flex | flex_grow}, &tab_selected);

  auto main_container = Container::Vertical({
      track_selector,
      track_container,
  });

  auto renderer = Renderer(main_container, [&] {
    return vbox({track_selector->Render(), separator(),
                 track_container->Render(), blinking_light(dsp)}) |
           border;
  });

  // thread to poll dsp metronome for ui redraws
  bool running = true;
  std::thread([&] {
    bool gate_change = false;
    while (running) {
      bool current_gate = dsp.get()->clock.thirtysecond_gate;

      if (current_gate != gate_change) {
        gate_change = current_gate;
        screen.RequestAnimationFrame();
        // screen.PostEvent(Event::Custom);
      }
      std::this_thread::sleep_for(
          std::chrono::milliseconds(2)); // adjust speed!
    }
  }).detach();

  screen.Loop(renderer);

  running = false;

  printf("ui terminated\n");
}
