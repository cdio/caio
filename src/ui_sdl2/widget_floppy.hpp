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

class Floppy : public Widget {
public:
    struct Status {
        bool is_attached{};
        bool is_idle{};
    };

    Floppy(::SDL_Renderer* renderer, const std::function<Status()>& upd);

    virtual ~Floppy() {
    }

    void render(const ::SDL_Rect& dstrect) override;

private:
    std::function<Status()> _update;
    int64_t                 _start{};
    int64_t                 _elapsed{};
    bool                    _prev_idle{};
};

}
}
}
}
