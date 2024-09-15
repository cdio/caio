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
#include "ui_sdl2/widget_floppy.hpp"

#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/floppy_128x2.hpp"

Floppy::Floppy(const sptr_t<::SDL_Renderer>& renderer, const std::function<Status()>& upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(floppy_128x2_png);
}

void Floppy::update()
{
    if (_update) {
       _status = _update();
    }
}

bool Floppy::is_idle()
{
    update();
    return _status.is_idle;
}

void Floppy::render(const ::SDL_Rect& dstrect)
{
    ::SDL_Rect rect{0, 0, 128, 128};

    update();

    if (_status.is_idle) {
        _prev_idle = true;

    } else {
        if (_prev_idle != _status.is_idle) {
            _start = utils::now();
            _prev_idle = _status.is_idle;
        }

        _elapsed = utils::now() - _start;
        rect = (_elapsed < 500000 ? ::SDL_Rect{0, 0, 128, 128} : ::SDL_Rect{128, 0, 128, 128});
        if (_elapsed >= 1000000) {
            _start = utils::now();
        }
    }

    auto color = (_status.is_attached ? ENABLED_COLOR : DISABLED_COLOR);
    Widget::render(rect, dstrect, color);

    if (!_status.is_idle && _status.progress >= 0.0f) {
        render_progress(_status.progress, dstrect);
    }
}

}
}
}
}
