/*
 * Copyright (C) 2020 Claudio Castiglia
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
#include "rgb.hpp"

#include "utils.hpp"

#include <cstring>
#include <fstream>

namespace caio {

Rgba Rgba::transparent{0};

static inline uint8_t adjust(float c)
{
    return (c > 255.0f ? 255 : static_cast<uint8_t>(c));
}

std::string Rgba::to_string() const
{
    return std::format("{:02X}{:02X}{:02X}{:02X}", r, g, b, a);
}

Rgba operator*(Rgba color, float value)
{
    return Rgba{
        adjust(color.r * value),
        adjust(color.g * value),
        adjust(color.b * value),
        color.a
    };
}

Rgba operator/(Rgba color, float value)
{
    return Rgba{
        adjust(color.r / value),
        adjust(color.g / value),
        adjust(color.b / value),
        color.a
    };
}

Rgba operator+(Rgba color1, Rgba color2)
{
    return Rgba{
        adjust(color1.r + color2.r),
        adjust(color1.g + color2.g),
        adjust(color1.b + color2.b),
        adjust((color1.a + color2.a) / 2)
    };
}

RgbaTable::RgbaTable(const std::initializer_list<Rgba>& il)
    : std::vector<Rgba>(il)
{
}

RgbaTable::RgbaTable(const fs::Path& fname)
{
    load(fname);
}

void RgbaTable::load(const fs::Path& fname)
{
    std::ifstream is{fname, std::ios::binary | std::ios::in};
    if (!is) {
        throw IOError{"Can't open: {}: {}", fname.string(), Error::to_string()};
    }

    clear();
    std::string line{};
    while (std::getline(is, line)) {
        line = utils::trim(line);

        if (line.empty() || line[0] == '#') {
            continue;
        }

        /*
         * Only the rrggbbaa format is supported.
         */
        if (line.size() != 8) {
            throw IOError{"{}: Invalid line: {}", fname.string(), line};
        }

        try {
            auto value = utils::to_number<uint32_t>(line.c_str());
            push_back(Rgba{value});
        } catch (const InvalidNumber&) {
            throw IOError{"Invalid line: {}", line};
        }
    }
}

void RgbaTable::save(const fs::Path& fname)
{
    std::ofstream os{fname, std::ios::binary | std::ios::out | std::ios::trunc};
    if (!os) {
        throw IOError{"Can't create: {}: {}", fname.string(), Error::to_string()};
    }

    for (const auto& rgb : *this) {
        os << rgb.to_string() << "\n";
        if (!os) {
            throw IOError{"Can't write: {}: {}", fname.string(), Error::to_string()};
        }
    }
}

Serializer& operator&(Serializer& ser, RgbaTable &table)
{
    auto buf = std::span<uint32_t>{reinterpret_cast<uint32_t*>(table.data()), table.size()};
    return (ser & buf);
}

}
