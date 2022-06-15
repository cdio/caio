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
#pragma once

#include <functional>
#include <memory>
#include <string>
#include <gsl/span>

#include <SFML/Graphics.hpp>

#include "rgb.hpp"


namespace caio {
namespace ui {
namespace sfml {

namespace widget {

template<typename W, class... A>
std::shared_ptr<W> create(const A&... args)
{
    auto widget = std::make_shared<W>(args...);
    widget->load();
    return widget;
}

}   /* namespace widget */


extern std::stringstream sfml_err;


/**
 * SFML Widget.
 */
class WidgetSfml {
public:
    constexpr static const Rgba COLOR         = { 0x00, 0x00, 0x00, 0xFF };

    constexpr static const unsigned WIDTH     = 64;
    constexpr static const unsigned HEIGHT    = 64;

    constexpr static const auto ACTION_BUTTON = sf::Mouse::Button::Left;


    WidgetSfml() {
    }

    virtual ~WidgetSfml() {
    }

    /**
     * Initialise this widget.
     */
    virtual void load() {
    }

    /**
     * @return The widgets's sprite at the current moment.
     * @see make_sprite()
     */
    sf::Sprite sprite();

    /**
     * Set the action callback.
     * The action callback is called when this widget is selected (mouse click).
     * @param act Action callback
     */
    void action(const std::function<void()> &act) {
        _action = act;
    }

    /**
     * Set the position of this widget.
     * @param x X position;
     * @param y Y position.
     */
    void position(int x, int y) {
        _area = sf::IntRect{x, y, WIDTH, HEIGHT};
    }

    /**
     * Event received.
     * @param ev Event data.
     */
    void event(const sf::Event &ev);

    /**
     * Load an image from a file.
     * The image is stored in the widget's texture.
     * @param fname Image file name.
     * @exception UIError
     * @see _texture.
     */
    void load(const std::string &fname);

    /**
     * Load an image from memory.
     * The image is stored in the widget's texture.
     * @param data Image data.
     * @exception UIError
     * @see _texture.
     */
    void load(const gsl::span<const uint8_t> &data);

protected:
    /**
     * @return The widget sprite.
     */
    virtual sf::Sprite make_sprite() = 0;

    /**
     * Evaluate if a point belongs to the area covered by this widget.
     * @param x X position;
     * @param y Y position.
     * @return True if the specified point belogns to the widget area; false otherwise.
     */
    bool is_widget_area(int x, int y) const {
        return (x >= _area.left && x < _area.left + _area.width && y >= _area.top  && y < _area.top + _area.height);
    }

    /**
     * @return True if the mouse cursor is located in the area of this widget.
     */
    bool mouse_on() const {
        return _mouse_on;
    }

    /**
     * Call the action method.
     */
    void action() {
        if (_action) {
            _action();
        }
    }

    /**
     * @return The texture of this widget.
     */
    sf::Texture &texture() {
        return _texture;
    }

    /**
     * Create a sprite from the specified texture area.
     * Create the sprite on the specified texture area and set its
     * position and origin to the area's centre.
     * The widget's texture must be valid before calling this method.
     * @param area Area.
     * @return The sprite.
     * @see load()
     * @see _texture
     */
    sf::Sprite rect(const sf::IntRect &area) {
        return rect(_texture, area);
    }

    /**
     * Create a sprite from the specified texture and texture area.
     * Create the sprite on the specified texture area and set its
     * position and origin to the area's centre.
     * @param tex  Texture;
     * @param area Area.
     * @return The sprite.
     */
    static sf::Sprite rect(const sf::Texture &tex, const sf::IntRect &r);

private:
    std::function<void()> _action{};
    sf::IntRect           _area{};
    bool                  _mouse_on{};
    bool                  _mouse_pressed{};
    sf::Texture           _texture{};
    float                 _scale{1.0f};
};

}   /* naemspace sfml */
}   /* namespace ui */
}   /* namespace caio */
