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

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <type_traits>

#include <gsl/span>

#include "rgb.hpp"


namespace cemu {
namespace ui {

/**
 * Widget.
 * Widgets are graphical elements.
 */
class Widget {
public:
    constexpr static const Rgba COLOR      = {0x00, 0x00, 0x00, 0xFF};
    constexpr static const unsigned WIDTH  = 64;
    constexpr static const unsigned HEIGHT = 64;

    using update_cb_t = std::function<uint64_t()>;

    /**
     * Create a widget.
     * @param upd Update callabck.
     * @see update(const update_cb_t &)
     */
    Widget(const update_cb_t &upd = {}) {
        update(upd);
    }

    virtual ~Widget() {
    }

    /**
     * Load widget images from a file.
     * @param fname Image file name.
     * @exception UIError
     */
    virtual void load(const std::string &fname) = 0;

    /**
     * Load widget images from memory.
     * @param data Image data.
     * @exception UIError
     */
    virtual void load(const gsl::span<const uint8_t> &data) = 0;

    /**
     * Load widget using builtin images.
     * @exception UIError
     */
    virtual void load() = 0;

    /**
     * Set the update method.
     * @param upd Update callabck.
     * @see update()
     */
    void update(const update_cb_t &upd) {
        _update = upd;
    }

    /**
     * Generic widget factory.
     * @param upd Update callback.
     * @see Widget(const update_cb_t &);
     */
    template<typename W>
    static std::shared_ptr<Widget> create(const Widget::update_cb_t &upd) {
        auto widget = std::make_shared<W>(upd);
        widget->load();
        return widget;
    }

protected:
    /**
     * Call the update method.
     * This method is used to update internal widget status and
     * it is usually called by the implementation at frame rate.
     * @return Some value required by the widget implementation.
     */
    uint64_t update() {
        return (_update ? _update() : 0);
    }

private:
    std::function<uint64_t()> _update{};
};

}
}
