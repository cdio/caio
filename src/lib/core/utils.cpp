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
#include "utils.hpp"

#include <thread>


namespace caio {
namespace utils {

std::string tolow(const std::string& str)
{
    std::string lstr{str};

    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](char c) {
        return std::tolower(c);
    });

    return lstr;
}

std::string toup(const std::string& str)
{
    std::string ustr{str};

    std::transform(ustr.begin(), ustr.end(), ustr.begin(), [](char c) {
        return std::toupper(c);
    });

    return ustr;
}

std::vector<std::string> split(const std::string& str, char sep)
{
    std::vector<std::string> v{};
    size_t pos{}, ipos{};

    do {
        pos = str.find(sep, ipos);
        v.push_back(str.substr(ipos, pos - ipos));
        ipos = pos + 1;
    } while (pos != std::string::npos);

    return v;
}

std::string trim(const std::string& str)
{
    size_t len = str.length();
    size_t begin = str.find_first_not_of(" \t");
    size_t end = str.find_last_not_of(" \t");

    if (begin == std::string::npos) {
        begin = 0;
    }

    if (end == std::string::npos) {
        end = len - 1;
    } else {
        ++end;
    }

    return (str.substr(begin, end));
}

unsigned long long to_ulonglong(const std::string& str, size_t max)
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

    char* err;
    unsigned long long val = std::strtoull(str.c_str() + pos, &err, base);
    if (*err != '\0' || val > max) {
        throw InvalidNumber{str};
    }

    return val;
}

std::string to_string(const std::vector<uint8_t>& buf)
{
    std::string str{};

    for (auto& value : buf) {
        str.push_back(static_cast<char>(value));
    }

    return str;
}

uint64_t sleep(uint64_t delay)
{
    uint64_t start = now();
    std::this_thread::sleep_for(std::chrono::microseconds{delay});
    return (now() - start);
}

}
}
