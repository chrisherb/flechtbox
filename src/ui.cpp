#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <memory>
#include <string>
#include <vector>

#include "controls.hpp"
#include "dsp.hpp"
#include "ui.hpp"

using namespace ftxui;
using namespace std;

Element blinking_light(std::shared_ptr<flechtbox_dsp> dsp)
{
	// Read the DSP state
	bool light_on = dsp->clock.quarter_gate;
	return light_on ? text("●") | color(Color::Green)
					: text("○") | color(Color::GrayDark);
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
	auto tab_toggle = Toggle(&tab_values, &tab_selected);
	auto start_btn = Checkbox("run", &dsp->clock.running);
	auto transport_ctrls = Container::Horizontal({start_btn});
	auto top_container = Container::Horizontal({tab_toggle, transport_ctrls});

	////////////////////
	// MAIN CONTAINER //
	////////////////////

	const std::vector<std::string> engines = {
		"1",  "2",	"3",  "4",	"5",  "6",	"7",  "8",	"9",  "10", "11", "12",
		"13", "14", "15", "16", "17", "18", "19", "20", "21", "22", "23", "24"};

	auto track_tabs = Container::Tab({}, &tab_selected);

	// SLAVE TRACKS
	for (int t = 0; t < NUM_TRACKS; t++) {
		auto sliders_container = Container::Horizontal({});

		for (int s = 0; s < NUM_STEPS; s++) {
			auto options =
				new SliderOption<int>({.value = &dsp->track_sequencers[t].data[s],
									   .min = 0,
									   .max = 100,
									   .increment = 20,
									   .direction = Direction::Up,
									   .color_active = Color::Green,
									   .color_inactive = Color::GrayDark});
			auto slider = Slider(*options);
			sliders_container->Add(slider | flex);
		}

		auto settings_container = Container::Horizontal({
			FloatControl(&dsp->plaits_voices[t].patch.note, 1.f, 0.f, 96.f),
			FloatControl(&dsp->plaits_voices[t].patch.harmonics),
			FloatControl(&dsp->plaits_voices[t].patch.timbre),
			FloatControl(&dsp->plaits_voices[t].patch.morph),
			Dropdown(&engines, &dsp->plaits_voices[t].patch.engine),
		});

		auto track_container = Container::Vertical(
			{sliders_container | border | flex, settings_container | flex});

		track_tabs->Add(track_container);
	}

	auto main_container = Container::Vertical({top_container, track_tabs});

	auto renderer = Renderer(main_container, [&] {
		return vbox({top_container->Render(), separator(), track_tabs->Render() | flex,
					 separator(), blinking_light(dsp)});
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
