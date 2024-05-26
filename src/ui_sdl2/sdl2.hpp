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

#include <string>
#include <vector>

#include <SDL.h>

#include "keyboard.hpp"
#include "rgb.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Scanline.
 * A Scanline represents a single line of an emulated screen;
 * it is filled by a video controller device with RGBa pixel data.
 * When a scanline is fully filled it must be sent to the user interface
 * to be rendered.
 */
using Scanline = std::vector<Rgba>;

/**
 * Get a string with the version number of the SDL library in use.
 * @return A string showing the SDL library version.
 */
constexpr const char* sdl_version()
{
    return "SDL2-" CAIO_STR(SDL_MAJOR_VERSION) "." \
                   CAIO_STR(SDL_MINOR_VERSION) "." \
                   CAIO_STR(SDL_PATCHLEVEL);
}

/**
 * Get the last SDL error.
 * @return Last SDL error.
 */
extern std::string sdl_error();

/**
 * Convert a SDL scan code to keyboard::Key code.
 * @param code SDL scan code.
 * @return Keyboard::Key code.
 * @see sdl_to_key
 */
extern keyboard::Key to_key(::SDL_Scancode code);

/**
 * Detect whether a coordinate belongs to a rectangle.
 * @param x    X coordinate;
 * @param y    Y coordinate;
 * @param rect Rectangle.
 * @return True if the coordinates fall inside the rectangle; false otherwise.
 */
extern bool in_rect(int x, int y, const ::SDL_Rect& rect);

}
}
}
