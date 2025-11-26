#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <ftxui/screen/color.hpp>
#include <iomanip>
#include <memory>
#include <sstream>
#include <string>

using namespace ftxui;

inline Component Light(bool* light_on)
{
	auto renderer = Renderer([light_on] {
		// Read the DSP state
		return *light_on ? text(" ● ") | color(Color::Green)
						 : text(" ○ ") | color(Color::GrayDark);
	});

	auto catcher = CatchEvent(renderer, [](Event event) { return false; });

	return Container::Vertical({renderer});
}

struct ControlOptions {
	bool horizontal = false;
	bool border = true;
};

inline Component FloatControl(float* value_ptr, std::string label = "",
							  float step = 0.01f, float min_value = 0.f,
							  float max_value = 1.f, ControlOptions options = {},
							  std::function<std::string(float)> format = nullptr)
{
	// Create a focusable renderer by using the (bool focused) lambda variant.
	// When focused we change the style so it's visible.
	auto renderer = Renderer([value_ptr, label, format, options](bool focused) {
		if (!value_ptr) {
			auto e = text("N/A") | hcenter | border;
			if (focused) e = e | bold | color(Color::Green);
			return e;
		}
		std::string s;
		if (format) {
			s = format(*value_ptr);
		} else {
			// sensible default formatting
			std::ostringstream os;
			os << std::fixed << std::setprecision(2) << *value_ptr;
			s = os.str();
		}
		auto e = text(s) | hcenter;
		if (options.border) e = e | border;
		// Visual focus indicator: bold + color the border text when focused.
		if (focused) e = e | bold | color(Color::Green);

		if (options.horizontal)
			return hbox({text(label) | vcenter, text(" "), e, text(" ")});
		else return vbox({text(label), e});
	});

	auto catcher =
		CatchEvent(renderer, [value_ptr, step, min_value, max_value](Event event) {
			if (!value_ptr) return false;
			if (event == Event::ArrowUp) {
				float next = *value_ptr + step;
				if (next > max_value) next = max_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			if (event == Event::ArrowDown) {
				float next = *value_ptr - step;
				if (next < min_value) next = min_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			return false;
		});

	// Container makes the control focusable when used in a parent container.
	return Container::Vertical({catcher});
}

inline Component IntegerControl(int* value_ptr, std::string label = "", int step = 1,
								int min_value = 0, int max_value = 1,
								ControlOptions options = {},
								std::function<std::string(int)> format = nullptr)
{
	// Create a focusable renderer by using the (bool focused) lambda variant.
	// When focused we change the style so it's visible.
	auto renderer = Renderer([value_ptr, format, label, options](bool focused) {
		if (!value_ptr) {
			auto e = text("N/A") | hcenter | border;
			if (focused) e = e | bold | color(Color::Green);
			return e;
		}
		std::string s;
		if (format) {
			s = format(*value_ptr);
		} else {
			// sensible default formatting
			std::ostringstream os;
			os << std::fixed << *value_ptr;
			s = os.str();
		}
		auto e = text(s) | hcenter;
		if (options.border) e = e | border;
		// Visual focus indicator: bold + color the border text when focused.
		if (focused) e = e | bold | color(Color::Green);

		if (options.horizontal)
			return hbox({text(label) | vcenter, text(" "), e, text(" ")});
		else return vbox({text(label) | vcenter, e});
	});

	auto catcher =
		CatchEvent(renderer, [value_ptr, step, min_value, max_value](Event event) {
			if (!value_ptr) return false;
			if (event == Event::ArrowUp) {
				int next = *value_ptr + step;
				if (next > max_value) next = max_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			if (event == Event::ArrowDown) {
				int next = *value_ptr - step;
				if (next < min_value) next = min_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			return false;
		});

	// Container makes the control focusable when used in a parent container.
	return Container::Vertical({catcher});
}

inline Component StepSlider(int* value_ptr, int step, unsigned int* step_active,
							int increment = 25, int min_value = 0, int max_value = 100)
{
	auto renderer = Renderer([value_ptr, step_active, step, max_value](bool focused) {
		auto g = gaugeUp(*value_ptr / static_cast<float>(max_value));
		auto t = text(std::to_string(step + 1));
		if (focused) {
			g = g | color(Color::Green);
			t = t | color(Color::Green);
		} else if (step == *step_active) {
			g = g | color(Color::Cyan);
			t = t | color(Color::Cyan);
		}
		return vbox(g | flex, t);
	});
	auto catcher =
		CatchEvent(renderer, [value_ptr, increment, min_value, max_value](Event event) {
			if (!value_ptr) return false;
			if (event == Event::ArrowUp) {
				int next = *value_ptr + increment;
				if (next > max_value) next = max_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			if (event == Event::ArrowDown) {
				int next = *value_ptr - increment;
				if (next < min_value) next = min_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			return false;
		});
	return Container::Vertical({catcher | flex});
}

inline Component StepSliderBipolar(int* value_ptr, int step, unsigned int* step_active,
								   int increment = 25, int min_value = 0,
								   int max_value = 100)
{
	auto renderer =
		Renderer([value_ptr, step_active, step, min_value, max_value](bool focused) {
			float val_pos = 0.f;
			float val_neg = 0.f;

			if (*value_ptr > 0) val_pos = *value_ptr / (float)max_value;
			else if (*value_ptr < 0) val_neg = *value_ptr / (float)min_value;

			auto g_up = gaugeUp(val_pos);
			auto g_do = gaugeDown(val_neg);

			auto label = text(std::to_string(step + 1));
			auto divider = text("—");

			if (focused) {
				g_up = g_up | color(Color::Green);
				g_do = g_do | color(Color::Green);
				label = label | color(Color::Green);
				divider = divider | color(Color::Green);
			} else if (step == *step_active) {
				g_up = g_up | color(Color::Cyan);
				g_do = g_do | color(Color::Cyan);
				label = label | color(Color::Cyan);
				divider = divider | color(Color::Cyan);
			}
			return vbox(g_up | flex, divider, g_do | flex, label);
		});

	auto catcher =
		CatchEvent(renderer, [value_ptr, increment, min_value, max_value](Event event) {
			if (!value_ptr) return false;
			if (event == Event::ArrowUp) {
				int next = *value_ptr + increment;
				if (next > max_value) next = max_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			if (event == Event::ArrowDown) {
				int next = *value_ptr - increment;
				if (next < min_value) next = min_value;
				if (next != *value_ptr) *value_ptr = next;
				return true;
			}
			return false;
		});
	return Container::Vertical({catcher | flex});
}
