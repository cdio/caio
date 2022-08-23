/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of caio.
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
#include "ui_sdl2/widget_gamepad.hpp"


namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/gamepad_128x3.hpp"


Gamepad::Gamepad(SDL_Renderer *renderer, const std::function<Status()> &upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(gamepad_128x3_png);
}

Gamepad::~Gamepad()
{
}

void Gamepad::render(const SDL_Rect &dstrect)
{
    static const SDL_Rect normal_rect{0, 0, 128, 128};
    static const SDL_Rect swapped_0_rect{128, 0, 128, 128};
    static const SDL_Rect swapped_1_rect{256, 0, 128, 128};

    Status st{};
    if (_update) {
        st = _update();
    }

    const Rgba &colour = (st.is_connected ? GAMEPAD_PRESENT_COLOR : GAMEPAD_MISSING_COLOR);
    const auto &rect = (!st.is_swapped ? normal_rect : ((st.id % 2) == 0 ? swapped_0_rect : swapped_1_rect));
    Widget::render(rect, dstrect, colour);
}

}
}
}
}
