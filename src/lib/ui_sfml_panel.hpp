/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#pragma once

#include <memory>
#include <utility>
#include <vector>

#include <SFML/Graphics.hpp>

#include "rgb.hpp"
#include "ui_sfml_widget.hpp"


namespace caio {
namespace ui {
namespace sfml {

/**
 * SFML Panel.
 * A panel is a graphical element used to show some emulator information.
 */
class PanelSfml {
public:
    constexpr static const Rgba BACKGROUND_COLOR   = { 0x00, 0x00, 0x00, 0x00 };
    constexpr static const Rgba FRAME_COLOR        = { 0xFF, 0x00, 0x00, 0xFF };

    constexpr static const uint32_t FRAME_TICKNESS = 2; /* Pixels */

    constexpr static const unsigned HEIGHT         = WidgetSfml::HEIGHT;


    enum class Just {
        LEFT,
        RIGHT
    };


    using widget_just_t = std::pair<std::shared_ptr<WidgetSfml>, Just>;


    /**
     * Create a SFML panel.
     * @parm is_visible Visibility flag (true if visible; false otherwise);
     * @parm max_width  Maximum allowd panel width (pixels).
     * @exception UIError
     */
    PanelSfml(bool is_visible, unsigned max_width = 0);

    virtual ~PanelSfml() {
    }

    /**
     * Set the panel visibility.
     * @param is_visible True if visible; false otherwise.
     */
    void visible(bool is_visible);

    /**
     * Set the panel size.
     * @param max_width Maximum allowed panel width (pixels).
     */
    void resize(unsigned max_width);

    /**
     * @return A constant reference to the panel size.
     */
    const sf::IntRect &area() const {
        return _area;
    }

    /**
     * @return True if the panel is visible; false otherwise.
     */
    bool is_visible() const {
        return _is_visible;
    }

    /**
     * Set the position of this panel.
     * @param x X position;
     * @param y Y position.
     */
    void position(int x, int y);

    /**
     * Event received.
     * @param ev Event data.
     */
    void event(const sf::Event &ev);

    /**
     * @return The panel sprite.
     */
    sf::Sprite sprite();

    /**
     * Add a widget into this panel.
     * @param widget Widget to add;
     * @param just   Justification (Just::LEFT or Just::RIGHT).
     * @see Just
     */
    void add(const std::shared_ptr<WidgetSfml> &widget, Just just = Just::LEFT);

    /**
     * Remove a widget from this panel.
     * @param widget Widget to remove.
     */
    void del(const std::shared_ptr<WidgetSfml> &widget);

    /**
     * Get the area covered by a panel.
     * @param is_visible Visibility flag (true if visible; false otherwise).
     * @param max_width  Maximum allowed width.
     * @return The area a panel would cover.
     */
    static sf::IntRect area(bool is_visible, unsigned max_width);

private:
    std::vector<widget_just_t>::const_iterator find(const std::shared_ptr<WidgetSfml> &widget) const {
        return std::find_if(_widgets.begin(), _widgets.end(), [&widget](const widget_just_t &pair) {
            return (widget == pair.first);
        });
    }

    bool                       _is_visible;
    sf::VideoMode              _desktop_mode;
    sf::IntRect                _area{};
    sf::RenderTexture          _render_tex{};
    std::vector<widget_just_t> _widgets{};
};

}
}
}
