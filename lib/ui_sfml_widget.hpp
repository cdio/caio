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
#pragma once

#include <memory>
#include <string>
#include <gsl/span>

#include <SFML/Graphics.hpp>

#include "rgb.hpp"


namespace cemu {
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


/**
 * SFML Widget.
 */
class WidgetSfml {
public:
    constexpr static const Rgba COLOR      = { 0x00, 0x00, 0x00, 0xFF };
    constexpr static const unsigned WIDTH  = 64;
    constexpr static const unsigned HEIGHT = 64;

    WidgetSfml() {
    }

    virtual ~WidgetSfml() {
    }

    /**
     * @see ui::Widget::load()
     */
    virtual void load() = 0;

    /**
     * @return The sprite representing the widget current status.
     */
    virtual sf::Sprite sprite() = 0;

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
    const sf::Texture &texture() const {
        return _texture;
    }

private:
    sf::Texture _texture{};
};

}   /* naemspace sfml */
}   /* namespace ui */
}   /* namespace cemu */
