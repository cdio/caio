/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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

#include <functional>
#include <memory>
#include <string>

#include <gsl/span>
#include <SDL.h>

#include "rgb.hpp"


namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Widget.
 * A widget is a graphical element that interact with the user through events.
 */
class Widget {
public:
    constexpr static const unsigned WIDTH         = 128;
    constexpr static const unsigned HEIGHT        = 128;
    constexpr static const float    RATIO         = static_cast<float>(WIDTH) / HEIGHT;
    constexpr static const uint8_t  ACTION_BUTTON = SDL_BUTTON_LEFT;
    constexpr static const Rgba DISABLED_COLOR    = { 255, 255, 255, 64  };     /* Color modulators */
    constexpr static const Rgba ENABLED_COLOR     = { 255, 255, 255, 255 };

    Widget(SDL_Renderer *renderer = nullptr);

    virtual ~Widget();

    /**
     * Load an image from a file.
     * @param fname Image file name.
     * @exception UIError
     */
    void load(const std::string &fname);

    /**
     * Load an image from memory.
     * @param data Image data.
     * @exception UIError
     */
    void load(const gsl::span<const uint8_t> &data);

    /**
     * Set the action callback.
     * The action callback is called when the ACTION_BUTTON is pressed.
     * @param act Action callback
     * @see ACTION_BUTTON
     */
    void action(const std::function<void()> &act);

    /**
     * Call the action callback.
     */
    void action();

    /**
     * @return True (default) if this widget is enabled; false otherwise.
     */
    virtual bool enabled() const;

    /**
     * Process SDL events.
     * @param event SDL event;
     * @param rect  Coordinates of this widget.
     */
    virtual void event(const SDL_Event &event, const SDL_Rect &rect);

    /**
     * Render the widget.
     * @param dstrect Destination rectangle.
     * @exception UIError
     */
    virtual void render(const SDL_Rect &dstrect) = 0;

protected:
    void render(const SDL_Rect &srcrect, const SDL_Rect &dstrect);

    void render(const SDL_Rect &srcrect, const SDL_Rect &dstrect, const Rgba &colour);

    void render(const SDL_Rect &srcrect, const SDL_Rect &dstrect, const SDL_Point &centre, float angle,
        const SDL_RendererFlip &flip);

    void render(const SDL_Rect &srcrect, const SDL_Rect &dstrect, const SDL_Point &centre, float angle,
        const SDL_RendererFlip &flip, const Rgba &colour);

    Rgba draw_color() const;

    void draw_color(const Rgba &color);

    Rgba color_modulator() const;

    void color_modulator(const Rgba &colour);

    SDL_Renderer          *_renderer;
    SDL_Texture           *_texture{nullptr};
    std::function<void()>  _action{};

    static Widget         *pressed_widget;  /* Last widget that received a button press event */
};

}
}
}
