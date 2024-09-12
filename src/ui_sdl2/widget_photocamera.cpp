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
#include "ui_sdl2/widget_photocamera.hpp"

#include "signal.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

#include "icons/photocamera_128x1.hpp"

PhotoCamera::PhotoCamera(sptr_t<::SDL_Renderer>& renderer)
    : Widget{renderer}
{
    Widget::load(photocamera_128x1_png);
}

void PhotoCamera::action()
{
    if (_atime >= ANIMATION_TIME) {
        Widget::action();
        _atime = 0.0f;
    }
}

void PhotoCamera::render(const ::SDL_Rect& dstrect)
{
    constexpr static const float w = 2.0f * Pi * 4.0f;
    constexpr static const float MAX_ANGLE = 60.0f;
    constexpr static const float TIME_CONST = 2.0f / 3.0f;
    static const ::SDL_Rect rect{0, 0, 128, 128};

    if (_atime < ANIMATION_TIME) {
        const float angle = signal::exp(0.0f, MAX_ANGLE, _atime, TIME_CONST) * std::sin(w * _atime);
        const auto centre = ::SDL_Point{dstrect.w / 2, dstrect.h / 2};
        Widget::render(rect, dstrect, centre, angle, ::SDL_FLIP_NONE, ENABLED_COLOR);
        _atime += 0.1f;
    } else {
        Widget::render(rect, dstrect, ENABLED_COLOR);
    }
}

}
}
}
}
