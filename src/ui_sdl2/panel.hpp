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

#include <tuple>
#include <vector>

#include <SDL.h>

#include "rgb.hpp"
#include "ui_sdl2/widget.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

/**
 * Info Panel.
 * A panel is a container for widgets.
 * @see Widget
 */
class Panel {
public:
    /*
     * Info panel.
     */
    constexpr static const Rgba FRAME_COLOR         = { 0xFF, 0x00, 0x00, 0x80 };
    constexpr static const Rgba BG_COLOR            = { 0x00, 0x00, 0x00, 0x80 };
    constexpr static float WIDTH_RATIO              = 0.618f;
    constexpr static float HEIGHT_RATIO             = WIDTH_RATIO / 4.0f;
    constexpr static const float THICKNESS_RATIO    = WIDTH_RATIO / 30.0f;
    constexpr static const int WIDGET_MAGNIFICATION = 5;

    /*
     * Status bar.
     */
    constexpr static const Rgba SB_COLOR            = { 0x40, 0x00, 0x00, 0x80 };
    constexpr static const int SB_MIN_HEIGHT        = 20;       /* Minimum status bar height                         */
    constexpr static const int SB_WID_SEPARATOR     = 5;        /* Horizontal widget separation (px)                 */
    constexpr static const float SB_HEIGHT_FACT     = 0.05f;    /* Panel height = SB_HEIGHT_FACT * window height     */
    constexpr static const float SB_WID_HEIGHT_FACT = 0.9f;     /* Widget heihgt = panel height * SB_WID_HEIGHT_FACT */
    constexpr static const int SB_DISABLED          = -1;
    constexpr static const int SB_POSITION_CENTER   = 0;
    constexpr static const int SB_POSITION_NORTH    = 1;
    constexpr static const int SB_POSITION_SOUTH    = 2;
    constexpr static const int SB_POSITION_EAST     = 4;
    constexpr static const int SB_POSITION_WEST     = 8;
    constexpr static const int SB_DEFAULT_POSITION  = SB_POSITION_SOUTH;

    using Widget = widget::Widget;

    enum class Just {
        Left  = 0,
        Right = 1
    };

    /**
     * Create a Panel.
     * @param renderer  Renderer;
     * @param statusbar Status bar position.
     * @exception UIError
     * @see reset(const sptr_t<::SDL_Renderer>& renderer, const std::string&)
     * @see sb_str2pos(const std::string&);
     */
    Panel(const sptr_t<::SDL_Renderer>& renderer = {}, const std::string& statusbar = {});

    virtual ~Panel();

    /**
     * Reset this panel,
     * @param renderer  Renderer;
     * @param statusbar Status bar position.
     * @exception UIError
     * @see sb_str2pos(const std::string&);
     */
    void reset(const sptr_t<::SDL_Renderer>& renderer = {}, const std::string& statsubar = {});

    /**
     * Set the panel visibility.
     * @param is_visible true if visible; false otherwise.
     */
    void visible(bool is_visible);

    /**
     * Get the panel visibility.
     * @return true if the panel is visible; false otherwise.
     */
    bool visible() const;

    /**
     * Process SDL events.
     * @param event SDL event.
     */
    void event(const ::SDL_Event& event);

    /**
     * Render the panel.
     * If the panel is visible, render it.
     * This method also renders the status bar.
     * @param width  Width of the destination window;
     * @param height Height of the destination window.
     * @see render_statusbar();
     */
    void render(int width, int height);

    /**
     * Add a widget to this panel.
     * @param widget Widget to add;
     * @param just   Justification;
     * @return true if the widget was added, false if there is no room for a new widget in the panel.
     * @see Just
     */
    void add(const sptr_t<Widget>& widget, Just just = Just::Left);

private:
    using JustRectWidget = std::tuple<Just, ::SDL_Rect, sptr_t<Widget>>;

    /**
     * Render the status bar.
     * The status bar is a panel that appears on the screen when
     * at least one widget is not idle.
     * @param width  Width of the destination window;
     * @param height Height of the destination window.
     */
    void render_statusbar(int width, int height);

    /**
     * Status bar position string to value.
     * The following position strings are reconised:
     *   - "center"
     *   - "north"
     *   - "south"
     *   - "east"
     *   - "west"
     *   - "north-east"
     *   - "north-west"
     *   - "south-east"
     *   - "south-west"
     *   - "none"
     * @param strpos Position string.
     * @return The postion value on success;
     *         The default position if the specified string is not recognised.
     * @see SB_DEFAULT_POSITION
     */
    int sb_str2pos(const std::string& strpos);

    /**
     * Find a widget on a specified coordinates.
     * @param x Horizontal position;
     * @param y Vertical position.
     * @return An iterator to the widget or end().
     */
    std::vector<JustRectWidget>::const_iterator find_widget(int x, int y);

    int                         _sb_position{};     /* Status bar position              */
    bool                        _visible{};         /* Panel visibility                 */
    sptr_t<::SDL_Renderer>      _renderer{};        /* Renderer                         */
    ::SDL_Rect                  _ext_rect{};        /* Panel rectangle                  */
    sptr_t<Widget>              _cur_widget{};      /* Widget under the mouse cursor    */
    std::vector<JustRectWidget> _widgets{};         /* Panel widgets                    */
};

}
}
}
