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

Element blinking_light(std::shared_ptr<flechtbox_dsp> dsp)
{
	// Read the DSP state
	bool light_on = dsp->clock.quarter_gate;
	return light_on ? text("●") | color(Color::Green)
					: text("○") | color(Color::GrayDark);
}

std::shared_ptr<ComponentBase> get_steps(std::shared_ptr<flechtbox_dsp> dsp,
										 int sequencer, int step)
{
	auto options =
		SliderOption<int>({.value = &dsp->track_sequencers[sequencer].data[step],
						   .min = 0,
						   .max = 100,
						   .increment = 10,
						   .direction = Direction::Up});
	return Slider(options);
}

void ui_run(ftxui::ScreenInteractive& screen, std::shared_ptr<flechtbox_dsp> dsp)
{
	std::vector<std::string> tab_values {
		"T1", "T2", "T4", "T5", "T6", "T7", "T8", "T9", "MT",
	};

	/////////////
	// TOP BAR //
	/////////////
	int tab_selected = 0;
	auto track_selector = Toggle(&tab_values, &tab_selected);
	auto start_btn = Checkbox("run", &dsp->clock.running);
	auto btn_container = Container::Horizontal({start_btn});
	auto top_container = Container::Horizontal({track_selector, btn_container});

	////////////////////
	// MAIN CONTAINER //
	////////////////////
	auto steps_container = Container::Horizontal({
		get_steps(dsp, 0, 0) | xflex_grow,
		get_steps(dsp, 0, 1) | xflex_grow,
		get_steps(dsp, 0, 2) | xflex_grow,
		get_steps(dsp, 0, 3) | xflex_grow,
		get_steps(dsp, 0, 4) | xflex_grow,
		get_steps(dsp, 0, 5) | xflex_grow,
		get_steps(dsp, 0, 6) | xflex_grow,
		get_steps(dsp, 0, 7) | xflex_grow,
		get_steps(dsp, 0, 8) | xflex_grow,
		get_steps(dsp, 0, 9) | xflex_grow,
	});

	// put border around steps container
	auto track_container_1 = Renderer(steps_container, [&] {
		return window(text("steps"), steps_container->Render());
	});
	auto track_container_2 = Container::Horizontal({});
	auto track_container_3 = Container::Horizontal({});
	auto track_container_4 = Container::Horizontal({});
	auto track_container_5 = Container::Horizontal({});
	auto track_container_6 = Container::Horizontal({});
	auto track_container_7 = Container::Horizontal({});
	auto track_container_8 = Container::Horizontal({});
	auto track_container_9 = Container::Horizontal({});

	auto track_container = Container::Tab(
		{track_container_1 | flex, track_container_2 | flex, track_container_3 | flex,
		 track_container_4 | flex, track_container_5 | flex, track_container_6 | flex,
		 track_container_7 | flex, track_container_8 | flex, track_container_9 | flex},
		&tab_selected);

	auto main_container = Container::Vertical({top_container, track_container});

	auto renderer = Renderer(main_container, [&] {
		return vbox({top_container->Render(), separator(), track_container->Render(),
					 blinking_light(dsp)});
	});

	// thread to poll dsp metronome for ui redraws
	bool ui_running = true;
	std::thread([&] {
		bool gate_change = false;
		while (ui_running) {
			bool current_gate = dsp.get()->clock.thirtysecond_gate;

			if (current_gate != gate_change) {
				gate_change = current_gate;
				screen.RequestAnimationFrame();
				// screen.PostEvent(Event::Custom);
			}
			std::this_thread::sleep_for(std::chrono::milliseconds(2)); // adjust speed!
		}
	}).detach();

	screen.Loop(renderer);

	ui_running = false;

	printf("ui terminated\n");
}
