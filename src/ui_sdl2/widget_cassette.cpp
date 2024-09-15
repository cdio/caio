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
#include "ui_sdl2/widget_cassette.hpp"

#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/cassette_128x23.hpp"

Cassette::Cassette(const sptr_t<::SDL_Renderer>& renderer, const std::function<Status()>& upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(cassette_128x23_png);
}

void Cassette::update()
{
    if (_update) {
       _status = _update();
    }
}

bool Cassette::is_idle()
{
    update();
    return _status.is_idle;
}

void Cassette::render(const ::SDL_Rect& dstrect)
{
    ::SDL_Rect rect{0, 0, 128, 128};

    update();

    if (_status.is_enabled && !_status.is_idle) {
        auto now = utils::now();
        auto elapsed = now - _start;

        if (elapsed >= 50'000) {
            _start = now;
            _pos = (_pos + 1) % ANIMATED_POSITIONS;
        }

        rect.x = 128 * (_pos + 1);
    }

    Widget::render(rect, dstrect, (_status.is_enabled ? ENABLED_COLOR : DISABLED_COLOR));
}

}
}
}
}
