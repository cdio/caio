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
#include "ui_sdl2/widget.hpp"

#include <SDL_image.h>

#include "ui_sdl2/sdl2.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

Widget* Widget::pressed_widget{nullptr};

Widget::Widget(SDL_Renderer* renderer)
    : _renderer{renderer}
{
}

Widget::~Widget()
{
    if (_texture != nullptr) {
        SDL_DestroyTexture(_texture);
    }
}

void Widget::load(const std::string& fname)
{
    if (_texture != nullptr) {
        SDL_DestroyTexture(_texture);
        _texture = nullptr;
    }

    if (_renderer != nullptr) {
        _texture = IMG_LoadTexture(_renderer, fname.c_str());
        if (_texture == nullptr) {
            throw UIError{"Can't load image: {}: {}", fname, sdl_error()};
        }
    }
}

void Widget::load(const std::span<const uint8_t>& data)
{
    if (_texture != nullptr) {
        SDL_DestroyTexture(_texture);
        _texture = nullptr;
    }

    if (_renderer != nullptr) {
        SDL_RWops* ops = SDL_RWFromConstMem(data.data(), data.size());
        if (ops == nullptr) {
            throw UIError{"Can't load image from memory: {}", sdl_error()};
        }

        _texture = IMG_LoadTexture_RW(_renderer, ops, 1);
        if (_texture == nullptr) {
            throw UIError{"Can't create texture from memory: {}", sdl_error()};
        }
    }
}

void Widget::action(const std::function<void()>& act)
{
    _action = act;
}

void Widget::action()
{
    if (_action) {
        _action();
    }
}

bool Widget::enabled() const
{
    return true;
}

void Widget::event(const SDL_Event& event, const SDL_Rect& rect)
{
    switch (event.type) {
    case SDL_JOYAXISMOTION:
    case SDL_JOYBALLMOTION:
    case SDL_JOYHATMOTION:
    case SDL_JOYBUTTONDOWN:
    case SDL_JOYBUTTONUP:
    case SDL_JOYDEVICEADDED:
    case SDL_JOYDEVICEREMOVED:
    case SDL_CONTROLLERAXISMOTION:
    case SDL_CONTROLLERBUTTONDOWN:
    case SDL_CONTROLLERBUTTONUP:
    case SDL_CONTROLLERDEVICEADDED:
    case SDL_CONTROLLERDEVICEREMOVED:
    case SDL_CONTROLLERDEVICEREMAPPED:
    case SDL_CONTROLLERTOUCHPADDOWN:
    case SDL_CONTROLLERTOUCHPADMOTION:
    case SDL_CONTROLLERTOUCHPADUP:
    case SDL_CONTROLLERSENSORUPDATE:
    case SDL_MOUSEWHEEL:
    case SDL_MOUSEMOTION:
        break;

    case SDL_MOUSEBUTTONDOWN:
        pressed_widget = (event.button.button == ACTION_BUTTON ? this : nullptr);
        break;

    case SDL_MOUSEBUTTONUP:
        if (pressed_widget == this && in_rect(event.button.x, event.button.y, rect)) {
            action();
        }
        pressed_widget = nullptr;
        break;

    default:;
    }
}

void Widget::render(const SDL_Rect& srcrect, const SDL_Rect& dstrect)
{
    if (SDL_RenderCopy(_renderer, _texture, &srcrect, &dstrect) < 0) {
        throw UIError{"Can't SDL_RenderCopy: {}", sdl_error()};
    }
}

void Widget::render(const SDL_Rect& srcrect, const SDL_Rect& dstrect, Rgba colour)
{
    auto prev_colour = color_modulator();
    color_modulator(colour);
    render(srcrect, dstrect);
    color_modulator(prev_colour);
}

void Widget::render(const SDL_Rect& srcrect, const SDL_Rect& dstrect, const SDL_Point& centre, float angle,
    const SDL_RendererFlip& flip)
{
    if (SDL_RenderCopyEx(_renderer, _texture, &srcrect, &dstrect, angle, &centre, flip) < 0) {
        throw UIError{"Can't SDL_RenderCopyEx: {}", sdl_error()};
    }
}

void Widget::render(const SDL_Rect& srcrect, const SDL_Rect& dstrect, const SDL_Point& centre, float angle,
    const SDL_RendererFlip& flip, Rgba colour)
{
    auto prev_colour = color_modulator();
    color_modulator(colour);
    render(srcrect, dstrect, centre, angle, flip);
    color_modulator(prev_colour);
}

Rgba Widget::draw_color() const
{
    Rgba colour{};

    if (SDL_GetRenderDrawColor(_renderer, &colour.r, &colour.g, &colour.b, &colour.a) < 0) {
        throw UIError{"Can't SDL_GetRenderDrawColor: {}", sdl_error()};
    }

    return colour;
}

void Widget::draw_color(Rgba colour)
{
    if (SDL_SetRenderDrawColor(_renderer, colour.r, colour.g, colour.b, colour.a) < 0) {
        throw UIError{"Can't SDL_SetRenderDrawColor: {}", sdl_error()};
    }
}

Rgba Widget::color_modulator() const
{
    Rgba colour{};

    if (SDL_GetTextureColorMod(_texture, &colour.r, &colour.g, &colour.b) < 0 ||
        SDL_GetTextureAlphaMod(_texture, &colour.a) < 0) {
        throw UIError{"Can't SDL_GetTexture{{Color,Alpha}}Mod: {}", sdl_error()};
    }

    return colour;
}

void Widget::color_modulator(Rgba colour)
{
    if (SDL_SetTextureColorMod(_texture, colour.r, colour.g, colour.b) < 0 ||
        SDL_SetTextureAlphaMod(_texture, colour.a) < 0) {
        throw UIError{"Can't SDL_SetTexture{{Color,Alpha}}Mod: {}", sdl_error()};
    }
}

}
}
}
