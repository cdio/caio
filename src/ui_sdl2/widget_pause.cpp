/*
 * Copyright (C) 2020 Claudio Castiglia
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
#include "ui_sdl2/widget_pause.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/pause_128x2.hpp"


Pause::Pause(::SDL_Renderer* renderer, const std::function<bool()>& upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(pause_128x2_png);
}

void Pause::render(const ::SDL_Rect& dstrect)
{
    bool is_paused{};

    if (_update) {
        is_paused = _update();
    }

    if (_rect.x == -1 || _is_paused != is_paused) {
        _rect = (is_paused ? ::SDL_Rect{128, 0, 128, 128} : ::SDL_Rect{0, 0, 128, 128});
        _is_paused = is_paused;
    }

    Widget::render(_rect, dstrect);
}

}
}
}
}
