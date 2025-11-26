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

const std::vector<std::string> engines = {
	"classic waveshapes",
	"phase distortion",
	"fm 1",
	"fm 2",
	"fm 3",
	"wave terrain",
	"string machine",
	"arpeggiator",
	"virtual analog",
	"asymmetric triangle",
	"2 sine waves",
	"formants",
	"additive sines",
	"wavetable",
	"chords",
	"speech",
	"sawtooth swarm",
	"filtered noise",
	"dust noise",
	"rings a",
	"rings b",
	"kick",
	"snare",
	"hihat",
};

const std::vector<std::string> pb_directions = {"forward", "backward", "pendulum",
												"random"};

void ui_run(ftxui::ScreenInteractive& screen, std::shared_ptr<flechtbox_dsp> dsp)
{
	std::vector<std::string> tab_values {
		" T1 ", " T2 ", " T3 ", " T4 ", " T5 ", " T6 ", " T7 ", " T8 ", " T9 ", " MT ",
	};

	/////////////
	// TOP BAR //
	/////////////
	int tab_selected = 0;
	auto tab_toggle = Toggle(&tab_values, &tab_selected);
	auto start_btn = Checkbox("run", &dsp->clock.running);
	auto tempo_ctrl = FloatControl(&dsp->clock.tempo, "bpm:", 1.f, 20.f, 250.f,
								   {.horizontal = true, .border = false});
	auto blinkenlight = Light(&dsp->clock.quarter_gate);
	auto transport_ctrls = Container::Horizontal({tempo_ctrl, start_btn, blinkenlight});
	auto top_container = Container::Horizontal({tab_toggle | flex, transport_ctrls});

	////////////////////
	// MAIN CONTAINER //
	////////////////////

	auto track_tabs = Container::Tab({}, &tab_selected);

	// MASTER TRACK
	// pitch sequencer
	auto pitch_sliders_container = Container::Horizontal({});
	for (int s = 0; s < NUM_STEPS; s++) {
		auto slider = StepSliderBipolar(&dsp->pitch_sequence.data[s], s,
										&dsp->pitch_sequence.current_pos,
										&dsp->pitch_sequence.length, 1, -12, 12);
		pitch_sliders_container->Add(slider | flex);
	}
	auto pitch_length_ctrl =
		IntegerControl(&dsp->pitch_sequence.length, "length", 1, 2, 10);
	auto pitch_settings_container = Container::Vertical({
		pitch_length_ctrl,
		Dropdown(&pb_directions, &dsp->pitch_sequence.playback_dir),
	});
	auto master_pitch_container = Container::Horizontal(
		{pitch_sliders_container | flex | border, pitch_settings_container | border});

	// octave sliders
	auto octave_sliders_container = Container::Horizontal({});
	for (int s = 0; s < NUM_STEPS; s++) {
		auto slider = StepSliderBipolar(&dsp->octave_sequence.data[s], s,
										&dsp->octave_sequence.current_pos,
										&dsp->octave_sequence.length, 12, -36, 36);
		octave_sliders_container->Add(slider | flex);
	}
	auto octave_length_ctrl =
		IntegerControl(&dsp->octave_sequence.length, "length", 1, 2, 10);
	auto octave_settings_container = Container::Vertical({
		octave_length_ctrl,
		Dropdown(&pb_directions, &dsp->octave_sequence.playback_dir),
	});
	auto master_octave_container = Container::Horizontal(
		{octave_sliders_container | flex | border, octave_settings_container | border});

	// velocity sliders
	auto velocity_sliders_container = Container::Horizontal({});
	for (int s = 0; s < NUM_STEPS; s++) {
		auto slider = StepSlider(&dsp->velocity_sequence.data[s], s,
								 &dsp->velocity_sequence.current_pos,
								 &dsp->velocity_sequence.length, 10);
		velocity_sliders_container->Add(slider | flex);
	}
	auto velocity_length_ctrl =
		IntegerControl(&dsp->velocity_sequence.length, "length", 1, 2, 10);
	auto velocity_settings_container = Container::Vertical({
		velocity_length_ctrl,
		Dropdown(&pb_directions, &dsp->velocity_sequence.playback_dir),
	});
	auto master_velocity_container =
		Container::Horizontal({velocity_sliders_container | flex | border,
							   velocity_settings_container | border});

	auto master_track_container = Container::Vertical({
		master_pitch_container | flex,
		master_octave_container | flex,
		master_velocity_container | flex,
	});

	// SLAVE TRACKS
	for (int t = 0; t < NUM_TRACKS; t++) {
		auto sliders_container = Container::Horizontal({});

		for (int s = 0; s < NUM_STEPS; s++) {
			auto slider = StepSlider(&dsp->tracks[t].sequencer.data[s], s,
									 &dsp->tracks[t].sequencer.current_pos,
									 &dsp->tracks[t].sequencer.length, 20);
			sliders_container->Add(slider | flex);
		}

		auto lgp_ctrls = Container::Horizontal({
			FloatControl(&dsp->tracks[t].patch.decay, "decay") | flex,
			FloatControl(&dsp->tracks[t].patch.lpg_colour, "color") | flex,
		});

		auto plaitsctrls_container = Container::Vertical({
			FloatControl(&dsp->tracks[t].patch.harmonics, "harmonics"),
			FloatControl(&dsp->tracks[t].patch.timbre, "timbre"),
			FloatControl(&dsp->tracks[t].patch.morph, "morph"),
			lgp_ctrls,
			Dropdown(&engines, &dsp->tracks[t].patch.engine),
		});

		auto trackctrls_container = Container::Vertical(
			{IntegerControl(&dsp->tracks[t].sequencer.length, "sequence length", 1, 2,
							10),
			 Dropdown(&pb_directions, &dsp->tracks[t].sequencer.playback_dir),
			 IntegerControl(&dsp->tracks[t].pitch, "root note", 1, 0, 96.f),
			 Checkbox("mute", &dsp->tracks[t].muted)});

		auto globalctrls_container = Container::Vertical({
			Checkbox("pitch", &dsp->tracks[t].global_pitch_enabled),
			Checkbox("octave", &dsp->tracks[t].global_octave_enabled),
			Checkbox("velocity", &dsp->tracks[t].global_velocity_enabled),
		});

		auto settings_container = Container::Horizontal(
			{plaitsctrls_container | border | flex, trackctrls_container | border | flex,
			 globalctrls_container | border | flex});

		auto track_container =
			Container::Vertical({sliders_container | border | flex, settings_container});

		track_tabs->Add(track_container);
	}

	track_tabs->Add(master_track_container);

	auto main_container = Container::Vertical({top_container, track_tabs});
	auto renderer = Renderer(main_container, [&top_container, &track_tabs] {
		return vbox({
				   top_container->Render(),
				   separator(),
				   track_tabs->Render() | flex,
			   }) |
			   border;
	});

	renderer |= CatchEvent([&](Event event) {
		// start / stop
		if (event == Event::F1) {
			dsp->clock.running = !dsp->clock.running;
			return true;
		}

		// select tabs with keys 1 - 0
		for (char num = '0'; num <= '9'; num++) {
			if (event == Event::Character(num)) {
				tab_selected = (num == '0') ? 9 : (num - '1');
				return true;
			}
		}

		// mute selected track
		if (event == Event::Character('m') && tab_selected <= 9) {
			dsp->tracks[tab_selected].muted = !dsp->tracks[tab_selected].muted;
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
