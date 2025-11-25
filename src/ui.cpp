#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/event.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
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

const std::vector<std::string> engines = {"1",	"2",  "3",	"4",  "5",	"6",  "7",	"8",
										  "9",	"10", "11", "12", "13", "14", "15", "16",
										  "17", "18", "19", "20", "21", "22", "23", "24"};

void ui_run(ftxui::ScreenInteractive& screen, std::shared_ptr<flechtbox_dsp> dsp)
{
	std::vector<std::string> tab_values {
		"T1", "T2", "T3", "T4", "T5", "T6", "T7", "T8", "T9", "MT",
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

	auto track_tabs = Container::Tab({}, &tab_selected);

	// MASTER TRACK
	// pitch sliders
	auto pitch_sliders_container = Container::Horizontal({});
	for (int s = 0; s < NUM_STEPS; s++) {
		auto slider = StepSliderBipolar(&dsp->pitch_sequence.data[s], s,
										&dsp->pitch_sequence.current_pos, 1, -12, 12);
		pitch_sliders_container->Add(slider | flex);
	}
	// octave sliders
	auto octave_sliders_container = Container::Horizontal({});
	for (int s = 0; s < NUM_STEPS; s++) {
		auto slider = StepSliderBipolar(&dsp->octave_sequence.data[s], s,
										&dsp->octave_sequence.current_pos, 1, -3, 3);
		octave_sliders_container->Add(slider | flex);
	}
	// velocity sliders
	auto velocity_sliders_container = Container::Horizontal({});
	for (int s = 0; s < NUM_STEPS; s++) {
		auto slider = StepSlider(&dsp->velocity_sequence.data[s], s,
								 &dsp->velocity_sequence.current_pos, 10);
		velocity_sliders_container->Add(slider | flex);
	}
	auto master_track_container = Container::Vertical({
		pitch_sliders_container | border | flex,
		octave_sliders_container | border | flex,
		velocity_sliders_container | border | flex,
	});

	// SLAVE TRACKS
	for (int t = 0; t < NUM_TRACKS; t++) {
		auto sliders_container = Container::Horizontal({});

		for (int s = 0; s < NUM_STEPS; s++) {
			auto slider = StepSlider(&dsp->tracks[t].sequencer.data[s], s,
									 &dsp->tracks[t].sequencer.current_pos);
			sliders_container->Add(slider | flex);
		}

		auto settings_container = Container::Horizontal(
			{IntegerControl(&dsp->tracks[t].pitch, 1, 0, 96.f),
			 FloatControl(&dsp->tracks[t].patch.harmonics),
			 FloatControl(&dsp->tracks[t].patch.timbre),
			 FloatControl(&dsp->tracks[t].patch.morph),
			 IntegerControl(&dsp->tracks[t].sequencer.length, 1, 2, 10),
			 Dropdown(&engines, &dsp->tracks[t].patch.engine),
			 Checkbox("pitch", &dsp->tracks[t].global_pitch_enabled),
			 Checkbox("octave", &dsp->tracks[t].global_octave_enabled),
			 Checkbox("velocity", &dsp->tracks[t].global_velocity_enabled)});

		auto track_container = Container::Vertical(
			{sliders_container | border | flex, settings_container | flex});

		track_tabs->Add(track_container);
	}

	track_tabs->Add(master_track_container);

	auto main_container = Container::Vertical({top_container, track_tabs});
	auto renderer = Renderer(main_container, [&] {
		return vbox({top_container->Render(), separator(), track_tabs->Render() | flex,
					 separator(), blinking_light(dsp)}) |
			   border;
	});

	renderer |= CatchEvent([&](Event event) {
		// start / stop
		if (event == Event::F1) { dsp->clock.running = !dsp->clock.running; }

		// select tabs with keys 1 - 0
		for (char num = '0'; num <= '9'; num++) {
			if (event == Event::Character(num)) {
				tab_selected = (num == '0') ? 9 : (num - '1');
			}
		}

		return false;
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
