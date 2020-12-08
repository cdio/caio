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

#include <SFML/Graphics.hpp>

#include "ui_widget.hpp"


namespace cemu {
namespace ui {
namespace sfml {

/**
 * SFML implementation of ui::Widget.
 */
class WidgetSfml : public ui::Widget {
public:
    using ui::Widget::Widget;

    /**
     * @return The sprite representing the widget current status.
     */
    virtual sf::Sprite sprite() = 0;

    /**
     * @see ui::Widget::load_image(const std::string &)
     */
    void load(const std::string &fname) override;

    /**
     * @see ui::Widget::load(const gsl::span<uint8_t> &)
     */
    void load(const gsl::span<const uint8_t> &data) override;

    /**
     * @see ui::Widget::load()
     */
    void load() override {
    }

protected:
    /**
     * @return A constant reference to this widget's texture.
     */
    const sf::Texture &texture() const {
        return _texture;
    }

private:
    sf::Texture _texture{};
};

}
}
}
