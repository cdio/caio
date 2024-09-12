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
#include "ui_sdl2/panel.hpp"

#include <algorithm>

#include "ui_sdl2/sdl2.hpp"

namespace caio {
namespace ui {
namespace sdl2 {

Panel::Panel(const sptr_t<::SDL_Renderer>& renderer)
{
    reset(renderer);
}

Panel::~Panel()
{
    reset();
}

void Panel::reset(const sptr_t<::SDL_Renderer>& renderer)
{
    _renderer = renderer;

    if (_renderer) {
        int displays = ::SDL_GetNumVideoDisplays();
        if (displays <= 0) {
            throw UIError{"panel: Can't get number of displays: {}", sdl_error()};
        }

        int max_width{};
        ::SDL_DisplayMode dmode{};
        for (int displ = 0; displ < displays; ++displ) {
            if (::SDL_GetDesktopDisplayMode(0, &dmode) < 0) {
                throw UIError{"panel: Can't get desktop display mode for display {}: {}", displ, sdl_error()};
            }

            if (dmode.w > max_width) {
                max_width = dmode.w;
            }
        }
    }
}

void Panel::visible(bool is_visible)
{
    _visible = is_visible;
}

bool Panel::visible() const
{
    return _visible;
}

void Panel::event(const ::SDL_Event& event)
{
    if (!_visible) {
        return;
    }

    int x{}, y{};
    ::SDL_GetMouseState(&x, &y);

    auto it = find_widget(x, y);
    if (it == _widgets.end()) {
        _cur_widget = {};
        return;
    }

    const auto& [_, rect, widget] = *it;
    _cur_widget = widget;
    widget->event(event, rect);
}

void Panel::render(int width, int height)
{
    if (!visible()) {
        return;
    }

    const int N = _widgets.size();
    const int wc_width = WIDTH_RATIO * width;

    /*
     * Width and height of widgets.
     */
    const int ww = wc_width / N;
    const int wh = ww / widget::RATIO;

    /*
     * Starting horizontal position for left and right justified widgets.
     */
    const int left_wx = (width - N * ww) / 2;
    const int right_wx = left_wx + ww * (N - 1);

    /*
     * Vertical position and height of the container panel.
     */
    const int candidate_panel_height = height * HEIGHT_RATIO;
    const int ext_h = (candidate_panel_height < wh ? wh : candidate_panel_height);
    const int ext_y = (height - ext_h) / 2;

    /*
     * Vertical position for all the widgets.
     */
    const int wy = ext_y + (ext_h - wh) / 2;

    /*
     * Panel external rectangle (frame).
     */
    _ext_rect = {
        .x = 0,
        .y = ext_y,
        .w = width,
        .h = ext_h
    };

    const int frame_thickness = ext_h * THICKNESS_RATIO;

    /*
     * Panel internal rectangle.
     */
    ::SDL_Rect int_rect{
        .x = 0,
        .y = ext_y + frame_thickness,
        .w = width,
        .h = ext_h - 2 * frame_thickness
    };

    /*
     * Draw panel.
     */
    ::SDL_SetRenderDrawColor(_renderer.get(), FRAME_COLOR.r, FRAME_COLOR.g, FRAME_COLOR.b, FRAME_COLOR.a);
    ::SDL_RenderFillRect(_renderer.get(), &_ext_rect);

    ::SDL_SetRenderDrawColor(_renderer.get(), BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
    ::SDL_RenderFillRect(_renderer.get(), &int_rect);

    /*
     * Coordinates for the next left justified widget.
     */
    ::SDL_Rect wid_rect{
        .x = left_wx,
        .y = wy,
        .w = ww,
        .h = wh
    };

    /*
     * Coordinates for the next right justified widget.
     */
    ::SDL_Rect wid_rev_rect{
        .x = right_wx,
        .y = wy,
        .w = ww,
        .h = wh
    };

    /*
     * Draw widgets.
     */
    for (auto& tup : _widgets) {
        auto& [just, wrect, widget] = tup;
        if (widget) {
            if (just == Just::Left) {
                wrect = wid_rect;
                wid_rect.x += ww;
            } else {
                wrect = wid_rev_rect;
                wid_rev_rect.x -= ww;
            }

            if (widget == _cur_widget && widget->enabled()) {
                /*
                 * Draw a rectangle around the (enabled) widget under the mouse cursor.
                 */
                ::SDL_Rect erect{
                    .x = wrect.x,
                    .y = _ext_rect.y,
                    .w = wrect.w,
                    .h = _ext_rect.h
                };

                ::SDL_SetRenderDrawColor(_renderer.get(), FRAME_COLOR.r, FRAME_COLOR.g, FRAME_COLOR.b, FRAME_COLOR.a);
                ::SDL_RenderFillRect(_renderer.get(), &erect);

                ::SDL_Rect irect{
                    .x = erect.x + frame_thickness,
                    .y = erect.y + frame_thickness,
                    .w = erect.w - 2 * frame_thickness,
                    .h = erect.h - 2 * frame_thickness
                };

                ::SDL_SetRenderDrawColor(_renderer.get(), BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
                ::SDL_RenderFillRect(_renderer.get(), &irect);

                /*
                 * Increase the size of the (enabled) widget under the mouse cursor.
                 */
                wrect.x -= WIDGET_MAGNIFICATION;
                wrect.y -= WIDGET_MAGNIFICATION;
                wrect.w += 2 * WIDGET_MAGNIFICATION;
                wrect.h += 2 * WIDGET_MAGNIFICATION;
            }

            widget->render(wrect);
        }
    }
}

void Panel::add(const sptr_t<Widget>& widget, Panel::Just just)
{
    _widgets.push_back({just, {}, widget});
}

std::vector<Panel::JustRectWidget>::const_iterator Panel::find_widget(int x, int y)
{
    auto it = std::find_if(_widgets.begin(), _widgets.end(), [this, x, y](const JustRectWidget& tup) {
        const auto& [_, wrect, widget] = tup;
        const ::SDL_Rect rect = {
            .x = wrect.x,
            .y = _ext_rect.y,
            .w = wrect.w,
            .h = _ext_rect.h
        };
        return (widget && in_rect(x, y, rect));
    });

    return it;
}

}
}
}
