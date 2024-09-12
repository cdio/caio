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
#include "ui_sdl2/widget_volume.hpp"

#include <format>

#include "sdl2.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/volume_128x21.hpp"

Volume::Volume(const sptr_t<::SDL_Renderer>& renderer, const std::function<float()>& getvol,
    const std::function<void(float)>& setvol)
    : Widget{renderer},
      _label{renderer},
      _getvol{getvol},
      _setvol{setvol}
{
    Widget::load(volume_128x21_png);
}

bool Volume::enabled() const
{
    return (_getvol && _setvol);
}

void Volume::event(const ::SDL_Event& event, const ::SDL_Rect& rect)
{
    if (enabled()) {
        switch (event.type) {
        case SDL_MOUSEWHEEL:
            volume(event.wheel.y * (event.wheel.direction == SDL_MOUSEWHEEL_NORMAL ? 1 : -1));
            break;

        case SDL_MOUSEMOTION:
            if (pressed_widget == this && ::SDL_GetMouseState(nullptr, nullptr) == ACTION_BUTTON) {
                volume(event.motion.xrel);
            }
            break;

        case SDL_MOUSEBUTTONDOWN:
        case SDL_MOUSEBUTTONUP:
        default:;
        }
    }

    Widget::event(event, rect);
}

void Volume::render(const ::SDL_Rect& dstrect)
{
    if (_volidx < 0) {
        volume(0);
    }

    Widget::render(_rect, dstrect, (enabled() ? ENABLED_COLOR : DISABLED_COLOR));

    if (_getvol && _volidx != _prev_volidx) {
        const auto str = std::format("{:3d}%", static_cast<unsigned>(_getvol() * 100));
        _label.reset(str);
        _prev_volidx = _volidx;
    }

    _label.render(dstrect);
}

void Volume::volume(int incr)
{
    incr = (incr < 0 ? -1 : 1);
    const int vol = std::max(0, std::min(10, static_cast<int>((_getvol ? _getvol() : 0.0f) * 10.0f) + incr));

    if (_setvol) {
        _setvol(vol / 10.0f);
    }

    const int ivol = vol * (LEVELS - 1) / 10;
    _volidx = (vol == 0 ? 0 : ivol + 1);
    _rect.x = WIDTH * _volidx;
}

}
}
}
}
