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
#include "ui_sdl2/widget_floppy.hpp"

#include "utils.hpp"


namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/floppy_128x2.hpp"


Floppy::Floppy(SDL_Renderer *renderer, const std::function<Status()> &upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(floppy_128x2_png);
}

Floppy::~Floppy()
{
}

void Floppy::render(const SDL_Rect &dstrect)
{
    Status st{};
    if (_update) {
       st = _update();
    }

    SDL_Rect rect{0, 0, 128, 128};

    if (!st.is_idle) {
        if (_prev_idle != st.is_idle) {
            _start = utils::now();
        }

        _elapsed = utils::now() - _start;
        rect = (_elapsed < 500000 ? SDL_Rect{0, 0, 128, 128} : SDL_Rect{128, 0, 128, 128});
        if (_elapsed >= 1000000) {
            _start = utils::now();
        }
    }

    const Rgba &colour = (st.is_attached ? ENABLED_COLOR : DISABLED_COLOR);
    Widget::render(rect, dstrect, colour);
}

}
}
}
}
