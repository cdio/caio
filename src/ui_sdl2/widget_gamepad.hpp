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

class Gamepad : public Widget {
public:
    struct Status {
        int         id{-1};
        bool        is_connected{};
        bool        is_swapped{};
        std::string name{};

        bool operator!=(const Status& other) const;
    };

    Gamepad(const sptr_t<::SDL_Renderer>& renderer, const std::function<Status()>& upd);

    virtual ~Gamepad() = default;

    void render(const ::SDL_Rect& dstrect) override;

private:
    std::function<Status()> _update;
    Label                   _label;
    Status                  _prev_status{};
};

}
}
}
}
