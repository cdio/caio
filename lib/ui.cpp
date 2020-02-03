/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation; either version 2 of the License, or
 * (at your option) any later version.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License along
 * with this program; if not, see http://www.gnu.org/licenses/
 */
#include "ui.hpp"

#include <csignal>


namespace cemu {
namespace ui {

std::atomic<Keyboard::Key> UI::signal_key{Keyboard::KEY_NONE};


void UI::signal_handler(int signo)
{
    switch (signo) {
    case SIGINT:
        UI::signal_key = Keyboard::KEY_CTRL_C;
        break;

    default:;
    }
}

void UI::run()
{
    auto old_handler = std::signal(SIGINT, signal_handler);
    if (old_handler == SIG_ERR) {
        throw UIError{"Can't set signal handler: " + Error::to_string()};
    }

    audio_play();

    while (!_stop && process_events()) {
        if (signal_key != Keyboard::KEY_NONE) {
            hotkey(signal_key);
            signal_key = Keyboard::KEY_NONE;
        }
    }

    audio_stop();

    std::signal(SIGINT, old_handler);
}

}
}
