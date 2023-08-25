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

Panel::Panel(SDL_Renderer* renderer)
{
    reset(renderer);
}

Panel::~Panel()
{
    reset();
}

void Panel::reset(SDL_Renderer* renderer)
{
    _renderer = renderer;

    if (_tex != nullptr) {
        SDL_DestroyTexture(_tex);
    }

    if (_renderer != nullptr) {
        int displays = SDL_GetNumVideoDisplays();
        if (displays <= 0) {
            throw_sdl_uierror("panel: Can't get number of displays");
        }

        int max_width{};
        SDL_DisplayMode dmode{};
        for (int displ = 0; displ < displays; ++displ) {
            if (SDL_GetDesktopDisplayMode(0, &dmode) < 0) {
                throw_sdl_uierror("panel: Can't get desktop display mode for display: " + std::to_string(displ));
            }

            if (dmode.w > max_width) {
                max_width = dmode.w;
            }
        }

        _tex = SDL_CreateTexture(_renderer, SDL_PIXELFORMAT_RGBA8888, SDL_TEXTUREACCESS_STREAMING,
            max_width, max_width * HEIGHT_RATIO);

        if (_tex == nullptr) {
            throw_sdl_uierror("panel: Can't create texture");
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

void Panel::event(const SDL_Event& event)
{
    if (!_visible) {
        return;
    }

    int x{}, y{};
    SDL_GetMouseState(&x, &y);

    auto it = find_widget(x, y);
    if (it == _widgets.end()) {
        _cur_widget = {};
        return;
    }

    auto& rect = std::get<1>(*it);
    auto& widget = std::get<2>(*it);
    widget->event(event, rect);
    _cur_widget = widget;
}

void Panel::render(int width, int height)
{
    if (visible()) {
        int N = _widgets.size();
        int wc_width = WIDTH_RATIO * width;

        int ww = wc_width / N;
        if (ww > Widget::WIDTH) {
            ww = Widget::WIDTH;
        }

        int wh = ww / Widget::RATIO;
        int wx = (width - N * ww) / 2;

        int ext_h = height * HEIGHT_RATIO;
        if (wh > ext_h) {
            ext_h = wh;
        }

        int ext_y = (height - ext_h) / 2;
        int wy = ext_y + (ext_h - wh) / 2;
        int rev_wx = wx + ww * (N - 1);

        /*
         * Panel external rectangle (frame).
         */
        _ext_rect = {
            .x = 0,
            .y = ext_y,
            .w = width,
            .h = ext_h
        };

        int frame_thickness = ext_h * THICKNESS_RATIO;

        /*
         * Panel internal rectangle.
         */
        SDL_Rect int_rect{
            .x = 0,
            .y = ext_y + frame_thickness,
            .w = width,
            .h = ext_h - 2 * frame_thickness
        };

        /*
         * Draw panel.
         */
        SDL_SetRenderDrawColor(_renderer, FRAME_COLOR.r, FRAME_COLOR.g, FRAME_COLOR.b, FRAME_COLOR.a);
        SDL_RenderFillRect(_renderer, &_ext_rect);

        SDL_SetRenderDrawColor(_renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
        SDL_RenderFillRect(_renderer, &int_rect);

        /*
         * Left justified widget rectangle.
         */
        SDL_Rect wid_rect{
            .x = wx,
            .y = wy,
            .w = ww,
            .h = wh
        };

        /*
         * Right justified widget rectangle.
         */
        SDL_Rect wid_rev_rect{
            .x = rev_wx,
            .y = wy,
            .w = ww,
            .h = wh
        };

        /*
         * Draw widgets.
         */
        for (auto& tup : _widgets) {
            auto just = std::get<0>(tup);
            auto& rect = std::get<1>(tup);
            auto& widget = std::get<2>(tup);
            if (widget) {
                if (just == Just::LEFT) {
                    rect = wid_rect;
                    wid_rect.x += ww;
                } else {
                    rect = wid_rev_rect;
                    wid_rev_rect.x -= ww;
                }

                if (widget == _cur_widget && widget->enabled()) {
                    /*
                     * Draw a rectangle around the widget under the mouse cursor.
                     */
                    SDL_SetRenderDrawColor(_renderer, FRAME_COLOR.r, FRAME_COLOR.g, FRAME_COLOR.b, FRAME_COLOR.a);
                    SDL_RenderFillRect(_renderer, &rect);

                    auto irect = rect;
                    irect.x += frame_thickness;
                    irect.y += frame_thickness;
                    irect.w -= 2 * frame_thickness;
                    irect.h -= 2 * frame_thickness;
                    SDL_SetRenderDrawColor(_renderer, BG_COLOR.r, BG_COLOR.g, BG_COLOR.b, BG_COLOR.a);
                    SDL_RenderFillRect(_renderer, &irect);
                }

                widget->render(rect);
            }
        }
    }
}

void Panel::add(const sptr_t<Widget>& widget, Panel::Just just)
{
    _widgets.push_back({just, {}, widget});
}

std::vector<Panel::just_rect_widget_t>::const_iterator Panel::find_widget(int x, int y)
{
    auto it = std::find_if(_widgets.begin(), _widgets.end(), [x, y](const just_rect_widget_t& tup) {
        auto& rect = std::get<1>(tup);
        auto& widget = std::get<2>(tup);
        return (widget && in_rect(x, y, rect));
    });

    return it;
}

}
}
}
