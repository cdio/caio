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
#include "ui_sdl2/widget_gamepad.hpp"

#include "sdl2.hpp"
#include "utils.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/gamepad_128x2.hpp"

bool Gamepad::Status::operator!=(const Status& other) const
{
    return ((id != other.id) ||
            (is_connected != other.is_connected) ||
            (is_swapped != other.is_swapped) ||
            (name != other.name));
}

Gamepad::Gamepad(const sptr_t<::SDL_Renderer>& renderer, const std::function<Status()>& upd)
    : Widget{renderer},
      _update{upd},
      _label{renderer}
{
    Widget::load(gamepad_128x2_png);
}

void Gamepad::render(const ::SDL_Rect& dstrect)
{
    Status st{};
    if (_update) {
        st = _update();
    }

    static const ::SDL_Rect enabled_rect{0, 0, 128, 128};
    static const ::SDL_Rect disabled_rect{128, 0, 128, 128};

    if (st.is_connected) {
        Widget::render(enabled_rect, dstrect, ENABLED_COLOR);
    } else {
        Widget::render(disabled_rect, dstrect, DISABLED_COLOR);
    }

    if (st != _prev_status) {
        if (std::string_view{st.name}.empty()) {
            const auto str = std::format("#{}", st.id + 1);
            _label.reset(str, DISABLED_COLOR);
        } else {
            const auto str = utils::trim(std::format("{:.{}}", st.name, Label::LABEL_MAX_COLS));
            _label.reset(str, ENABLED_COLOR);
        }
        _prev_status = st;
    }

    _label.render(dstrect);
}

}
}
}
}
