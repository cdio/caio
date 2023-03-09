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

#include <array>
#include <initializer_list>
#include <string>
#include <vector>

#include "endian.hpp"
#include "types.hpp"


namespace caio {

/**
 * RGBA colour.
 */
struct Rgba {
    union {
        uint32_t u32;
        struct {
            uint8_t r;
            uint8_t g;
            uint8_t b;
            uint8_t a;
        };
    };

    /**
     * Initialise this RGBA from its red, green, blue and alpha components.
     * @param red   Red component;
     * @param green Green component;
     * @param blue  Blue component;
     * @param alpha Alpha component (default is 255, no transparency).
     */
    constexpr Rgba(uint8_t red, uint8_t green, uint8_t blue, uint8_t alpha = 255)
        : r{red},
          g{green},
          b{blue},
          a{alpha} {
    }

    /**
     * Initialise an RGBA from a 32 bits value.
     * @param rgba A 32 bits RGBA value (default is black, no transparency).
     */
    Rgba(uint32_t rgba = 0x000000ff)
        : u32{htobe32(rgba)} {
    }

    /**
     * @return The RGBA string formatted as "#rrggbbaa".
     */
    std::string to_string() const;

    /**
     * @return true if this RGBA is a transparent; false otherwise.
     */
    bool is_transparent() const {
        return (a == 0);
    }

    /**
     * Set this RGBA.
     * If the specified colour is transparent this RGBA is not changed.
     * @param color RGBA to copy from.
     * @return This RGBA.
     */
    Rgba &set(const Rgba &color) {
        if (!color.is_transparent()) {
            u32 = color.u32;
        }

        return *this;
    }

    /**
     *  @return This RGBA color enconded as a 32 bits integer in host endian order.
     */
    uint32_t to_host_u32() const {
        return be32toh(u32);
    }

    /**
     * Increment the elements of a colour.
     * The RGB components of a specified colour are incremented with a signed value;
     * the alpha component is left unchanged.
     * @param color Colour to increment;
     * @param value Increment value.
     * @return The new colour.
     */
    friend Rgba operator+(const Rgba &color, int value);

    /**
     * Transparent colour.
     */
    static Rgba transparent;
};

/**
 * RGBA Table.
 * The RGBA table associates colour indexes to RGBA values.
 */
class RgbaTable : public std::vector<Rgba> {
public:
    /**
     * Initialise this RGBA table with values from memory.
     * @param il RGBA values.
     */
    RgbaTable(const std::initializer_list<Rgba> &il);

    /**
     * Initialise this RGBA table with values from disk.
     * @param fname Name of the file containing the colour table.
     * @exception IOError
     */
    explicit RgbaTable(const std::string &fname);

    /**
     * Release this RGBA table.
     */
    virtual ~RgbaTable();

    /**
     * (Re-)Initialise this RGBA table with values from disk.
     * @param fname File name.
     * @exception IOError
     */
    void load(const std::string &fname);

    /**
     * Save this RGBA table to disk.
     * @param fname File name.
     * @exception IOError
     */
    void save(const std::string &fname);
};

/**
 * Container for a fixed number of colours.
 */
template<size_t N>
class RgbaN_ {
public:
    template<typename... Ts>
    RgbaN_(const Ts &... colors)
        : _colors{{colors...}} {
    }

    ~RgbaN_() {
    }

    const Rgba &operator[](size_t index) const {
        return _colors[index];
    }

private:
    std::array<Rgba, N> _colors;
};

using Rgba2 = RgbaN_<2>;
using Rgba4 = RgbaN_<4>;

}
