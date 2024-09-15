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
#pragma once

#include "ui_sdl2/widget.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

class Volume : public Widget {
public:
    constexpr static int LEVELS = 20;

    /**
     * Initialise this widget.
     * @param renderer SDL renderer;
     * @param getvol   Callback to retrieve the current volume value;
     * @param setvol   Callback to set the current volume value.
     * @exception UIError
     * @see Widget::load(const std::span<const uint8_t>&)
     */
    Volume(const sptr_t<::SDL_Renderer>& renderer, const std::function<float()>& getvol = {},
        const std::function<void(float)>& setvol = {});

    virtual ~Volume() {
    }

    bool enabled() const override;

    void render(const ::SDL_Rect& dstrect) override;

    void event(const ::SDL_Event& event, const ::SDL_Rect& rect) override;

private:
    void volume(int incr);

    Label                       _label;
    std::function<float()>      _getvol;
    std::function<void(float)>  _setvol;
    int                         _volidx{-1};
    int                         _prev_volidx{};
    ::SDL_Rect                  _rect{0, 0, 128, 128};
};

}
}
}
}
