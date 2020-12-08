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

#include "ui_panel.hpp"


namespace cemu {
namespace ui {
namespace sfml {

/**
 * SFML implementation of ui::Panel.
 */
class PanelSfml : public ui::Panel {
public:
    constexpr static const unsigned HEIGHT = Widget::HEIGHT;

    /**
     * Create a SFML panel.
     * @parm is_visible Visibility flag (true if visible; false otherwise).
     * @exception UIError
     * @see ui::Panel::Panel()
     */
    PanelSfml(bool is_visible, unsigned max_width = 0);

    virtual ~PanelSfml() {
    }

    /**
     * Set the panel visibility.
     * @param is_visible Panel visibility (true if visible; false otherwise).
     */
    void visible(bool is_visible) override;

    /**
     * Set the panel size.
     * @param max_width Maximum allowed panel width.
     */
    void resize(unsigned max_width);

    /**
     * @return A constant reference to the panel size.
     */
    const sf::Vector2u &size() const {
        return _size;
    }

    /**
     * @return The panel sprite.
     */
    sf::Sprite sprite();

    /**
     * Get the size of a panel.
     * @param is_visible Visibility flag (true if visible; false otherwise).
     * @param max_width Maximum allowed panel width.
     * @return The panel size.
     */
    static sf::Vector2u size(bool is_visible, unsigned max_width);

private:
    sf::VideoMode     _desktop_mode;
    sf::Vector2u      _size;
    sf::RenderTexture _render_tex;
};

}
}
}
