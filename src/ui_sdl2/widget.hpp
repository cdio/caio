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

#include <functional>
#include <memory>
#include <span>
#include <string>
#include <tuple>

#include <SDL.h>
#include <SDL_image.h>
#include <SDL_ttf.h>

#include "fs.hpp"
#include "rgb.hpp"

namespace caio {
namespace ui {
namespace sdl2 {
namespace widget {

constexpr static const unsigned WIDTH               = 128;
constexpr static const unsigned HEIGHT              = 128;

constexpr static const unsigned LBORDER_WIDTH       = 14;
constexpr static const unsigned RBORDER_WIDTH       = 14;
constexpr static const unsigned IMAGE_WIDTH         = WIDTH - LBORDER_WIDTH - RBORDER_WIDTH;
constexpr static const unsigned INFO_WIDTH          = IMAGE_WIDTH;

constexpr static const unsigned UBORDER_HEIGHT      = 20;
constexpr static const unsigned SEPARATOR_HEIGHT    = 10;
constexpr static const unsigned BBORDER_HEIGHT      = 0;
constexpr static const unsigned INFO_HEIGHT         = 20;
constexpr static const unsigned IMAGE_HEIGHT        = HEIGHT - UBORDER_HEIGHT - SEPARATOR_HEIGHT -
                                                      INFO_HEIGHT - BBORDER_HEIGHT;

constexpr static const unsigned IMAGE_X             = LBORDER_WIDTH;
constexpr static const unsigned IMAGE_Y             = UBORDER_HEIGHT;
constexpr static const unsigned INFO_X              = IMAGE_X;
constexpr static const unsigned INFO_Y              = IMAGE_Y + IMAGE_HEIGHT + SEPARATOR_HEIGHT;

constexpr static const float RATIO                  = static_cast<float>(WIDTH) / HEIGHT;

/**
 * Convert widget relative coordinates to target relative coordinates.
 * @param widrect Widget relative coordinates;
 * @param dstrect Destination target rectangle.
 * @return The coordinates relative to the destination target rectangle.
 */
::SDL_Rect to_rect(const ::SDL_Rect& widrect, const ::SDL_Rect& dstrect);

/**
 * Widget.
 * A widget is a graphical element that interact with the user through events.
 *
 * Widget dimensions:
 *
 *      |<-------------------------- WIDTH ------------------------->|
 *                                    128
 *
 *      |<- 14  ->|<------------- IMAGE WIDTH ------------->|<- 14 ->|
 *                                    100
 *      +------------------------------------------------------------+   -+-     -+-
 *      |                                                            |    |       |
 *      |                        UPPER BORDER                        |    | 20    |
 *      |                                                            |    |       |
 *      |         +----------------------------------------+         |   -+-      |
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         |               IMAGE AREA               |         |            |
 *      |         |                                        |         |            | HEIGHT
 *      |         |                                        |         |            |  128
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         |                                        |         |            |
 *      |         +----------------------------------------+         |   -+-      |
 *      |                          SEPARATOR                         |    | 10    |
 *      |         +----------------------------------------+         |   -+-      |
 *      |         |                                        |         |    |       |
 *      |         |            INFORMATION AREA            |         |    | 20    |
 *      |         |                                        |         |    |       |
 *      |         +----------------------------------------+         |   -+-      |
 *      |                        BOTTOM BORDER                       |    | 0     |
 *      +------------------------------------------------------------+   -+-     -+-
 */
class Widget {
public:
    constexpr static const uint8_t ACTION_BUTTON        = SDL_BUTTON_LEFT;
    constexpr static const uint64_t ACTION_TIME         = 2'000'000;
    constexpr static const Rgba DISABLED_COLOR          = { 255, 255, 255,  64 };   /* Colour modulators */
    constexpr static const Rgba ENABLED_COLOR           = { 255, 255, 255, 255 };
    constexpr static const Rgba PROGRESS_BORDER_COLOR   = { 255,   0,   0, 255 };
    constexpr static const Rgba PROGRESS_BG_COLOR       = {   0,   0,   0, 128 };
    constexpr static const Rgba PROGRESS_COLOR          = {   0, 255,   0, 255 };

    /**
     * Initialise this widget.
     * @param renderer SDL renderer.
     */
    Widget(const sptr_t<::SDL_Renderer>& renderer = {});

    virtual ~Widget();

    /**
     * Load an image from a file.
     * @param fname Image file name.
     * @exception UIError
     */
    void load(const fs::Path& fname);

    /**
     * Load an image from memory.
     * @param data Image data.
     * @exception UIError
     */
    void load(const std::span<const uint8_t> data);

    /**
     * Tell whether an action period is ongoing.
     * @return True if an action period is ongoing; false otherwise.
     */
    bool is_action_period() const;

    /**
     * Set the action callback.
     * The action callback is called when the ACTION_BUTTON is pressed.
     * @param act Action callback
     * @see ACTION_BUTTON
     */
    void action(const std::function<void()>& act);

    /**
     * Call the action callback.
     * Call the action callback and start the action period.
     * @see start_action_period()
     */
    virtual void action();

    /**
     * Get the status of this widget.
     * @return True (default) if this widget is enabled; false otherwise.
     */
    virtual bool enabled() const;

    /**
     * Get the idle status.
     * @return True if an action period is ongoing; false otherwise.
     * @see is_action_period()
     */
    virtual bool is_idle();

    /**
     * Process SDL events.
     * @param event SDL event;
     * @param rect  Coordinates of this widget.
     */
    virtual void event(const ::SDL_Event& event, const ::SDL_Rect& rect);

    /**
     * Render the widget.
     * @param dstrect Destination rectangle.
     * @exception UIError
     */
    virtual void render(const ::SDL_Rect& dstrect) = 0;

protected:
    /**
     * Get the renderer drawing colour.
     * @return The current drawing colour.
     */
    Rgba draw_color() const;

    /**
     * Set the renderer drawing colour.
     * @param color Colour.
     */
    void draw_color(Rgba color);

    /**
     * Get the renderer colour modulator.
     * @return The current colour modulator.
     */
    Rgba color_modulator() const;

    /**
     * Set the renderer colour modulator.
     * @param color Colour.
     */
    void color_modulator(Rgba color);

    /**
     * Render part of this widget.
     * Render part of this widget and, if the action period is started, update it.
     * @param srcrect Part of this widget to render;
     * @param dstrect Destination coordinates.
     * @see update_action_period()
     */
    void render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect);

    /**
     * Render part of this widget.
     * Render part of this widget and, if the action period is started, update it.
     * @param srcrect Part of this widget to render;
     * @param dstrect Destination coordinates;
     * @param color   Colour modulator.
     * @see color_modulator(Rgba)
     * @see update_action_period()
     */
    void render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect, Rgba color);

    /**
     * Render part of this widget rotated and or flipped.
     * Render part of this widget and, if the action period is started, update it.
     * @param srcrect Part of this widget to render;
     * @param dstrect Destination coordinates;
     * @param centre  Rotation centre;
     * @param angle   Rotation angle (in degrees);
     * @param flip    Type of flip.
     * @see update_action_period()
     */
    void render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect, const ::SDL_Point& centre, float angle,
        ::SDL_RendererFlip flip);

    /**
     * Render part of this widget rotated and or flipped.
     * Render part of this widget and, if the action period is started, update it.
     * @param srcrect Part of this widget to render;
     * @param dstrect Destination coordinates;
     * @param centre  Rotation centre;
     * @param angle   Rotation angle (in degrees);
     * @param flip    Type of flip;
     * @param color   Colour modulator.
     * @see color_modulator(Rgba)
     * @see update_action_period()
     */
    void render(const ::SDL_Rect& srcrect, const ::SDL_Rect& dstrect, const ::SDL_Point& centre, float angle,
        ::SDL_RendererFlip flip, Rgba color);

    /**
     * Render a progress bar in the widget's information area.
     * @param progress Progress value (between 0 and 1).
     * @param dstrect  Destination rectangle.
     */
    void render_progress(float progress, const ::SDL_Rect& dstrect);

    /**
     * Render a filled rectangle.
     * @param rect  Rectangle coordinates;
     * @param color Colour.
     */
    void render_rect(const ::SDL_Rect& rect, Rgba color);

    /**
     * Start the action period.
     * The action period is started when the action method is called.
     * The widget implementation decides how to be rendered during this time.
     */
    void start_action_period();

    /**
     * Update the action period.
     * The action period terminates when its elapsed time reaches ACTION_TIME us.
     * @see ACTION_TIME
     */
    void update_action_period();

    sptr_t<::SDL_Renderer>  _renderer{};
    uptrd_t<::SDL_Texture>  _texture{nullptr, nullptr};
    std::function<void()>   _action{};
    bool                    _is_action_period{};
    uint64_t                _action_time{};

    static Widget* pressed_widget;  /* Last widget that received a button press event */
};

/**
 * Label texture.
 * The label is placed inside a widget's information area.
 */
class Label {
public:
    constexpr static const int LABEL_MAX_COLS       = 8;    /* Label Max characters         */
    constexpr static const int LABEL_MIN_FONT_SIZE  = 5;    /* Label min font size (pixels) */
    constexpr static const Rgba LABEL_COLOR         = { 255, 255, 255, 0 };
    constexpr static const ::SDL_Rect LABEL_RECT    = { INFO_X, INFO_Y, INFO_WIDTH, INFO_HEIGHT };

    Label(const sptr_t<::SDL_Renderer>& renderer = {});

    void reset(const std::string& label, Rgba color = LABEL_COLOR);

    void render(const ::SDL_Rect& dstrect);

private:
    using Texture = std::tuple<uptrd_t<::SDL_Texture>, int, int>;

    /**
     * Generate a text message ready to be rendered.
     * @param msg   Message to print
     * @param color Colour;
     * @param width Maximum width in pixels (0 means no limit).
     * @return A texture.
     * @see Texture
     */
    Texture print(const std::string& msg, Rgba color, unsigned width = 0);

    sptr_t<::SDL_Renderer>  _renderer{};
    std::string             _label{};
    uptrd_t<::SDL_Texture>  _texture{nullptr, nullptr};
    float                   _ratio{};

    /*
     * Font used on all labels.
     */
    class Font : public uptrd_t<::TTF_Font> {
    public:
        Font();
        Font& init();
    private:
        buffer_t _ttf{};
    };

    static Font font;
};

}
}
}
}
