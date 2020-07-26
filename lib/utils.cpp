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
#include "utils.hpp"


namespace cemu {
namespace utils {

unsigned long long to_ulonglong(const std::string &str, size_t max)
{
    if (str.empty()) {
        throw InvalidNumber{str};
    }

    int base = 16;
    int pos = 0;
    switch (str[0]) {
    case '#':
        base = 10;
        /* FALLTHROUGH */
    case '$':
        ++pos;
        break;
    default:;
    }

    char *err;
    unsigned long long val = std::strtoull(str.c_str() + pos, &err, base);
    if (*err != '\0' || val > max) {
        throw InvalidNumber{str};
    }

    return val;
}

std::string to_string(const std::vector<uint8_t> &buf)
{
    std::string str{};

    for (auto &value : buf) {
        str.push_back(static_cast<char>(value));
    }

    return str;
}

}
}
