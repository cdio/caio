/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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

#include <memory>

#include "ui.hpp"


namespace cemu {
namespace ui {
namespace sfml {

/**
 * Create a SFML user interface.
 * @param conf Configuration parameters;
 * @param icon Icon image.
 * @return The user interface using SFML as backend.
 * @exception UIError
 */
std::shared_ptr<UI> create(const ui::Config &conf, const Image &icon = {});

}
}
}
