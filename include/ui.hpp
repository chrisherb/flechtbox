#pragma once

#include <ftxui/component/screen_interactive.hpp>

#include "audio.hpp"

void ui_run(ftxui::ScreenInteractive &screen,
            std::shared_ptr<flechtbox_dsp> dsp);
