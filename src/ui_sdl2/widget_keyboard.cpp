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
#include "ui_sdl2/widget_keyboard.hpp"

#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/keyboard_128x2.hpp"

Keyboard::Keyboard(const sptr_t<::SDL_Renderer>& renderer, const std::function<Status()>& upd)
    : Widget{renderer},
      _update{upd}
{
    Widget::load(keyboard_128x2_png);
}

void Keyboard::render(const ::SDL_Rect& dstrect)
{
    auto color = ENABLED_COLOR;

    Status st{};
    if (!_update) {
        color = DISABLED_COLOR;
    } else {
        st = _update();
    }

    Widget::render(::SDL_Rect{128 * (!st.is_enabled), 0, 128, 128}, dstrect, color);
}

}
}
}
}
