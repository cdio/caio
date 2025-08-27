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

#include "ui_sdl2/sdl2.hpp"

#include "utils.hpp"

#include <algorithm>

namespace caio {
namespace ui {
namespace sdl2 {

Panel::Panel(const sptr_t<::SDL_Renderer>& renderer, const std::string& statusbar)
{
    reset(renderer, statusbar);
}

void Panel::reset(const sptr_t<::SDL_Renderer>& renderer, const std::string& statusbar)
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

        _sb_position = sb_str2pos(statusbar);
    }
}

int Panel::sb_str2pos(const std::string& strpos)
{
    static const std::unordered_map<std::string, int> str2pos{
        { "center",     SB_POSITION_CENTER                      },
        { "north",      SB_POSITION_NORTH                       },
        { "south",      SB_POSITION_SOUTH                       },
        { "east",       SB_POSITION_EAST                        },
        { "west",       SB_POSITION_WEST                        },
        { "north-east", SB_POSITION_NORTH | SB_POSITION_EAST    },
        { "north-west", SB_POSITION_NORTH | SB_POSITION_WEST    },
        { "south-east", SB_POSITION_SOUTH | SB_POSITION_EAST    },
        { "south-west", SB_POSITION_SOUTH | SB_POSITION_WEST    },
        { "none",       SB_DISABLED                             },
    };

    const auto it = str2pos.find(utils::tolow(strpos));
    return (it != str2pos.end() ? it->second : SB_DEFAULT_POSITION);
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
        /*
         * If the panel is not visible render the status bar.
         */
        render_statusbar(width, height);
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
     * Render panel.
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
     * Render widgets.
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

void Panel::render_statusbar(int width, int height)
{
    if (_sb_position == SB_DISABLED) {
        return;
    }

    size_t N{};
    for (const auto& tup : _widgets) {
        N += static_cast<size_t>(!std::get<sptr_t<Widget>>(tup)->is_idle());
    }

    if (N == 0) {
        return;
    }

    const int sb_h = (1.0f + SB_WID_HEIGHT_FACT) *
        ((height * SB_HEIGHT_FACT) < SB_MIN_HEIGHT ? SB_MIN_HEIGHT : height * SB_HEIGHT_FACT);

    const int ww = sb_h * widget::RATIO;
    const int wh = sb_h * SB_WID_HEIGHT_FACT;

    ++N;    /* Space for left and righ borders (the size of each border is half of a widget's width) */

    const int sb_w = N * ww + (N + 1) * SB_WID_SEPARATOR;

    /* Center the status bar and apply modifiers */
    int sb_x = (width - sb_w) / 2;
    int sb_y = height / 2 - sb_h;

    if (_sb_position & SB_POSITION_NORTH) {
        sb_y = 0;
    }

    if (_sb_position & SB_POSITION_SOUTH) {
        sb_y = height - sb_h;
    }

    if (_sb_position & SB_POSITION_EAST) {
        sb_x = width - sb_w;
    }

    if (_sb_position & SB_POSITION_WEST) {
        sb_x = 0;
    }

    ::SDL_Rect sb_rect{sb_x, sb_y, sb_w, sb_h};

    const int delta_w = ww + SB_WID_SEPARATOR;
    const int wx = sb_x + SB_WID_SEPARATOR + delta_w / 2;
    const int right_wx = sb_x + sb_w - ww - SB_WID_SEPARATOR - delta_w / 2;
    const int wy = sb_y;

    ::SDL_Rect wl_rect{
        .x = wx,
        .y = wy,
        .w = ww,
        .h = wh
    };

    ::SDL_Rect wr_rect{
        .x = right_wx,
        .y = wy,
        .w = ww,
        .h = wh
    };

    /*
     * Render the status bar rectangle
     * and the widgets inside it.
     */
    ::SDL_SetRenderDrawColor(_renderer.get(), SB_COLOR.r, SB_COLOR.g, SB_COLOR.b, SB_COLOR.a);
    ::SDL_RenderFillRect(_renderer.get(), &sb_rect);

    for (const auto& [just, wrect, widget] : _widgets) {
        if (!widget->is_idle()) {
            if (just == Just::Left) {
                widget->render(wl_rect);
                wl_rect.x += delta_w;
            } else {
                widget->render(wr_rect);
                wr_rect.x -= delta_w;
            }
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
