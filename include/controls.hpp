#include <ftxui/component/component.hpp>
#include <ftxui/component/component_base.hpp>
#include <ftxui/component/component_options.hpp>
#include <ftxui/component/screen_interactive.hpp>
#include <ftxui/dom/direction.hpp>
#include <ftxui/dom/elements.hpp>
#include <iomanip>
#include <memory>
#include <sstream>

using namespace ftxui;

inline Component FloatControl(float* value_ptr, float step = 0.01f, float min_value = 0.f,
							  float max_value = 1.f,
							  std::function<std::string(float)> format = nullptr)
{
	// Create a focusable renderer by using the (bool focused) lambda variant.
	// When focused we change the style so it's visible.
	auto renderer = Renderer([value_ptr, format](bool focused) {
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
		auto e = text(s) | hcenter | border;
		// Visual focus indicator: bold + color the border text when focused.
		if (focused) e = e | bold | color(Color::Green);
		return e;
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

inline Component IntegerControl(int* value_ptr, int step = 1, int min_value = 0,
								int max_value = 1,
								std::function<std::string(int)> format = nullptr)
{
	// Create a focusable renderer by using the (bool focused) lambda variant.
	// When focused we change the style so it's visible.
	auto renderer = Renderer([value_ptr, format](bool focused) {
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
		auto e = text(s) | hcenter | border;
		// Visual focus indicator: bold + color the border text when focused.
		if (focused) e = e | bold | color(Color::Green);
		return e;
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
