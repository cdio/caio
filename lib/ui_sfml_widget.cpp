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
#include "ui_sfml_widget.hpp"

#include "types.hpp"


namespace cemu {
namespace ui {
namespace sfml {

void WidgetSfml::load(const std::string &fname)
{
    if (!_texture.loadFromFile(fname)) {
        throw UIError{"Can't load texture: " + fname + ": " + sfml_err.str()};
    }

    _texture.setSmooth(true);
}

void WidgetSfml::load(const gsl::span<const uint8_t> &data)
{
    if (!_texture.loadFromMemory(data.data(), data.size())) {
        throw UIError{"Can't load texture from data: " + sfml_err.str()};
    }

    _texture.setSmooth(true);
}

sf::Sprite WidgetSfml::sprite()
{
    auto sprt = make_sprite();

    /*
     * If there is an action defined for this widget let
     * the user know it by increasing the size of the sprite.
     */
    if (_action) {
        if (mouse_on() && _scale < 1.2f) {
            _scale += 0.05f;
        } else if (!mouse_on() && _scale > 1.0f) {
            _scale -= 0.05f;
            sprt.setScale(_scale, _scale);
        }

        sprt.setScale(_scale, _scale);
    }

    return sprt;
}

sf::Sprite WidgetSfml::rect(const sf::Texture &tex, const sf::IntRect &area)
{
    float cx = (area.width - area.left) / 2.0f;
    float cy = (area.height - area.top) / 2.0f;

    if (cx < 0.0f) {
        cx = -cx;
    }

    if (cy < 0.0f) {
        cy = -cy;
    }

    sf::Sprite sprite{tex, area};

    sprite.setOrigin(cx, cy);
    sprite.setPosition(cx, cy);

    return sprite;
}

void WidgetSfml::event(const sf::Event &ev)
{
    switch (ev.type) {
    case sf::Event::MouseButtonPressed:
        _mouse_on = is_widget_area(ev.mouseButton.x, ev.mouseButton.y);
        _mouse_pressed = (_mouse_on && ev.mouseButton.button == ACTION_BUTTON);
        break;

    case sf::Event::MouseButtonReleased:
        _mouse_on = is_widget_area(ev.mouseButton.x, ev.mouseButton.y);
        if (_mouse_pressed && _mouse_on) {
            action();
        }
        _mouse_pressed = false;
        break;

    case sf::Event::MouseMoved:
        _mouse_on = is_widget_area(ev.mouseMove.x, ev.mouseMove.y);
        break;

    case sf::Event::MouseLeft:
        _mouse_on = false;
        _mouse_pressed = false;
        break;
    default:;
    }
}

}
}
}
