/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
#include "ui_sfml_panel.hpp"

#include <sstream>

#include "utils.hpp"


namespace caio {
namespace ui {
namespace sfml {

sf::IntRect PanelSfml::area(bool is_visible, unsigned max_width)
{
    return (is_visible ? sf::IntRect{0, 0, static_cast<int>(max_width), HEIGHT} : sf::IntRect{});
}


PanelSfml::PanelSfml(bool is_visible, unsigned max_width)
    : _is_visible{is_visible},                          /* These two must be initialised before the others */
      _desktop_mode{sf::VideoMode::getDesktopMode()}
{
    resize(max_width);

    if (!_render_tex.create(_desktop_mode.width, HEIGHT)) {
        throw UIError{"Can't create the panel texture: " + sfml_err.str()};
    }
}

void PanelSfml::visible(bool is_visible)
{
    _is_visible = is_visible;
    resize(_area.width);
}

void PanelSfml::resize(unsigned max_width)
{
    auto narea = PanelSfml::area(is_visible(), (max_width > _desktop_mode.width ? _desktop_mode.width : max_width));
    _area.width = narea.width;
    _area.height = narea.height;
}

void PanelSfml::position(int x, int y)
{
    _area.left = x;
    _area.top = y;
}

void PanelSfml::event(const sf::Event &ev)
{
    if (_is_visible) {
        for (auto &pair : _widgets) {
            pair.first->event(ev);
        }
    }
}

sf::Sprite PanelSfml::sprite()
{
    if (!is_visible()) {
        /*
         * Panel not visible: No sprite to render.
         */
        return sf::Sprite{};
    }

    /*
     * Render the panel.
     */
    _render_tex.clear();

    unsigned lx = 0;
    unsigned rx = _area.width;

    for (const auto &pair : _widgets) {
        auto [widget, just] = pair;
        auto sprite = widget->sprite();

        if (just == Just::LEFT) {
            widget->position(_area.left + lx, _area.top);
            sprite.move(lx, 0);
            lx += widget->WIDTH;
        } else {
            rx -= widget->WIDTH;
            widget->position(_area.left + rx, _area.top);
            sprite.move(rx, 0);
        }

        _render_tex.draw(sprite);
    }

    auto frame = sf::RectangleShape{sf::Vector2f{static_cast<float>(_area.width), static_cast<float>(_area.height)}};
    frame.setOutlineThickness(-static_cast<float>(FRAME_TICKNESS));
    frame.setOutlineColor(sf::Color{FRAME_COLOR.to_host_u32()});
    frame.setFillColor(sf::Color{BACKGROUND_COLOR.to_host_u32()});
    _render_tex.draw(frame);

    _render_tex.display();

    return sf::Sprite{_render_tex.getTexture(), {0, 0, _area.width, _area.height}};
}

void PanelSfml::add(const std::shared_ptr<WidgetSfml> &widget, PanelSfml::Just just)
{
    if (find(widget) == _widgets.end()) {
        _widgets.push_back({widget, just});
    }
}

void PanelSfml::del(const std::shared_ptr<WidgetSfml> &widget)
{
    auto it = find(widget);
    if (it != _widgets.end()) {
        _widgets.erase(it);
    }
}

}
}
}
