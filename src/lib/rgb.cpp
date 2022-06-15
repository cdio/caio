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
#include "rgb.hpp"

#include <cstring>
#include <fstream>

#include "utils.hpp"


namespace caio {

Rgba Rgba::transparent{0};


std::string Rgba::to_string() const
{
    std::ostringstream os{};

    os << std::hex << std::setw(2) << std::setfill('0') << +r
       << std::hex << std::setw(2) << std::setfill('0') << +g
       << std::hex << std::setw(2) << std::setfill('0') << +b
       << std::hex << std::setw(2) << std::setfill('0') << +a;

    return os.str();
}


Rgba operator+(const Rgba &color, int value)
{
    auto r = color.r + value;
    auto g = color.g + value;
    auto b = color.b + value;

    if (r < 0) {
        r = 0;
    } else if (r > 255) {
        r = 255;
    }

    if (g < 0) {
        g = 0;
    } else if (g > 255) {
        g = 255;
    }

    if (b < 0) {
        b = 0;
    } else if (b > 255) {
        b = 255;
    }

    return {
        static_cast<uint8_t>(r),
        static_cast<uint8_t>(g),
        static_cast<uint8_t>(b),
        color.a
    };
}


void RgbaTable::load(const std::string &fname)
{
    std::ifstream is{fname, std::ios::binary | std::ios::in};
    if (!is) {
        throw IOError{"Can't open: " + fname + ": " + Error::to_string()};
    }

    clear();
    std::string line{};
    while (std::getline(is, line)) {
        if (line.empty() || line[0] == '#') {
            continue;
        }

        /*
         * Only the rrggbbaa format is supported.
         */
        if (line.size() != 8) {
            throw IOError{fname + ": Invalid line: " + line};
        }

        try {
            auto value = utils::to_number<uint32_t>(line.c_str());
            push_back(Rgba{value});
        } catch (const InvalidNumber &) {
            throw IOError{"Invalid line: " + line};
        }
    }

    is.close();
}

void RgbaTable::save(const std::string &fname)
{
    std::ofstream os{fname, std::ios::binary | std::ios::out | std::ios::trunc};
    if (!os) {
        throw IOError{"Can't create: " + fname + ": " + Error::to_string()};
    }

    for (const auto &rgb : *this) {
        os << rgb.to_string() << std::endl;
        if (!os) {
            throw IOError{"Can't write: " + fname + ": " + Error::to_string()};
        }
    }

    os.close();
}

}
