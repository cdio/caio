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
#include "ui_sfml_panel.hpp"

#include <sstream>

#include "utils.hpp"


namespace cemu {
namespace ui {
namespace sfml {

extern std::stringstream sfml_err;


sf::Vector2u PanelSfml::size(bool is_visible, unsigned max_width)
{
    return (is_visible ? sf::Vector2u{max_width, HEIGHT} : sf::Vector2u{0, 0});
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
    resize(_size.x);
}

void PanelSfml::resize(unsigned max_width)
{
    _size = PanelSfml::size(is_visible(), (max_width > _desktop_mode.width ? _desktop_mode.width : max_width));
}

sf::Sprite PanelSfml::sprite()
{
    if (!is_visible()) {
        return sf::Sprite{};
    }

    _render_tex.clear();

    unsigned left_x = 0;
    unsigned right_x = _size.x;

    for (const auto &pair : widgets()) {
        auto [widget, just] = pair;

        auto sprite = widget->sprite();

        if (just == Just::LEFT) {
            sprite.move(left_x, 0);
            left_x += widget->WIDTH;
        } else {
            right_x -= widget->WIDTH;
            sprite.move(right_x, 0);
        }

        _render_tex.draw(sprite);
    }

    auto frame = sf::RectangleShape{sf::Vector2f{_size}};
    frame.setOutlineThickness(-static_cast<float>(FRAME_TICKNESS));
    frame.setOutlineColor(sf::Color{FRAME_COLOR.to_host_u32()});
    frame.setFillColor(sf::Color{BACKGROUND_COLOR.to_host_u32()});
    _render_tex.draw(frame);

    _render_tex.display();

    return sf::Sprite{_render_tex.getTexture(), {0, 0, static_cast<int>(_size.x), static_cast<int>(_size.y)}};
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
