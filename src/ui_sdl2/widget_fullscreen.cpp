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
#include "ui_sdl2/widget_fullscreen.hpp"


namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/fullscreen_128x2.hpp"

Fullscreen::Fullscreen(SDL_Renderer* renderer, const std::function<bool()>& upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(fullscreen_128x2_png);
}

void Fullscreen::render(const SDL_Rect& dstrect)
{
    bool is_fullscreen{};

    if (_update) {
        is_fullscreen = _update();
    }

    if (_rect.x == -1 || _is_fullscreen != is_fullscreen) {
        _rect = (is_fullscreen ? SDL_Rect{128, 0, 128, 128} : SDL_Rect{0, 0, 128, 128});
        _is_fullscreen = is_fullscreen;
    }

    Widget::render(_rect, dstrect);
}

}
}
}
}
