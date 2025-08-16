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
#include <tuple>

#include "sha2.h"

namespace caio {
namespace utils {

constexpr static const char BASE64_PAD = '=';

std::string tolow(std::string_view str)
{
    std::string lstr{str};

    std::transform(lstr.begin(), lstr.end(), lstr.begin(), [](char c) {
        return std::tolower(c);
    });

    return lstr;
}

std::string toup(std::string_view str)
{
    std::string ustr{str};

    std::transform(ustr.begin(), ustr.end(), ustr.begin(), [](char c) {
        return std::toupper(c);
    });

    return ustr;
}

std::vector<std::string> split(std::string_view str, char sep)
{
    std::vector<std::string> v{};
    size_t pos{}, ipos{};

    do {
        pos = str.find(sep, ipos);
        v.push_back(std::string{str.substr(ipos, pos - ipos)});
        ipos = pos + 1;
    } while (pos != std::string::npos);

    return v;
}

std::string trim(std::string_view str)
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

    return std::string{str.substr(begin, end)};
}

unsigned long long to_ulonglong(std::string_view str, size_t max)
{
    if (str.empty()) {
        throw InvalidNumber{"Empty string"};
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

    char* err{};
    unsigned long long val = std::strtoull(str.data() + pos, &err, base);
    if (*err != '\0' || val > max) {
        throw InvalidNumber{str};
    }

    return val;
}

std::string to_string(std::span<const uint8_t> buf)
{
    std::string str{};

    for (uint8_t value : buf) {
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

std::string sha256(std::span<const uint8_t> buf)
{
    SHA2_CTX ctx{};
    uint8_t md[SHA256_DIGEST_LENGTH];
    SHA256Init(&ctx);
    SHA256Update(&ctx, buf.data(), buf.size());
    SHA256Final(md, &ctx);

    std::ostringstream os{};
    for (uint8_t value : md) {
        os << to_string(value);
    }

    return os.str();
}

Buffer base64_decode(std::span<const uint8_t> src)
{
    static const uint8_t decode_table[256] = {
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x00 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x10 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x3E, 0x00, 0x00, 0x00, 0x3F, /* 0x20 */
        0x34, 0x35, 0x36, 0x37, 0x38, 0x39, 0x3A, 0x3B,  0x3C, 0x3D, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x30 */
        0x00, 0x00, 0x01, 0x02, 0x03, 0x04, 0x05, 0x06,  0x07, 0x08, 0x09, 0x0A, 0x0B, 0x0C, 0x0D, 0x0E, /* 0x40 */
        0x0F, 0x10, 0x11, 0x12, 0x13, 0x14, 0x15, 0x16,  0x17, 0x18, 0x19, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x50 */
        0x00, 0x1A, 0x1B, 0x1C, 0x1D, 0x1E, 0x1F, 0x20,  0x21, 0x22, 0x23, 0x24, 0x25, 0x26, 0x27, 0x28, /* 0x60 */
        0x29, 0x2A, 0x2B, 0x2C, 0x2D, 0x2E, 0x2F, 0x30,  0x31, 0x32, 0x33, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x70 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x80 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0x90 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xA0 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xB0 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xC0 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xD0 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, /* 0xE0 */
        0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00,  0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00, 0x00  /* 0xF0 */
    };

    static auto decode_block = [](uint8_t e1, uint8_t e2, uint8_t e3, uint8_t e4) -> std::tuple<int, int, int> {
        const auto ee1 = decode_table[e1];
        const auto ee2 = decode_table[e2];
        const auto ee3 = decode_table[e3];
        const auto ee4 = decode_table[e4];

        const int d1 = (ee1 << 2) | (ee2 >> 4);
        const int d2 = (ee2 << 4) | ((ee3 >> 2) & 15);
        const int d3 = (ee3 << 6) | ee4;

        if (e1 == BASE64_PAD || e2 == BASE64_PAD) {
            return {-1, -1, -1};
        }

        if (e3 == BASE64_PAD) {
            return {d1, -1, -1};
        }

        if (e4 == BASE64_PAD) {
            return {d1, d2, -1};
        }

        return {d1, d2, d3};
    };

    Buffer dst{};
    size_t i{};
    do {
        uint8_t e1{BASE64_PAD}, e2{BASE64_PAD}, e3{BASE64_PAD}, e4{BASE64_PAD};

        while (i < src.size() && (e1 = src[i++]) == '\n') e1 = BASE64_PAD;
        while (i < src.size() && (e2 = src[i++]) == '\n') e2 = BASE64_PAD;
        while (i < src.size() && (e3 = src[i++]) == '\n') e3 = BASE64_PAD;
        while (i < src.size() && (e4 = src[i++]) == '\n') e4 = BASE64_PAD;

        if (e1 == BASE64_PAD || e2 == BASE64_PAD) break;
        if (e3 == BASE64_PAD) e4 = BASE64_PAD;

        const auto [d1, d2, d3] = decode_block(e1, e2, e3, e4);
        if (d1 >= 0) dst.push_back(d1);
        if (d2 >= 0) dst.push_back(d2);
        if (d3 >= 0) dst.push_back(d3);

    } while (i < src.size());

    return dst;
}

Buffer base64_encode(std::span<const uint8_t> src)
{
    static const std::array<char, 64> encode_table = {
        'A', 'B', 'C', 'D', 'E', 'F', 'G', 'H',  'I', 'J', 'K', 'L', 'M', 'N', 'O', 'P',
        'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X',  'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f',
        'g', 'h', 'i', 'j', 'k', 'l', 'm', 'n',  'o', 'p', 'q', 'r', 's', 't', 'u', 'v',
        'w', 'x', 'y', 'z', '0', '1', '2', '3',  '4', '5', '6', '7', '8', '9', '+', '/'
    };

    static const auto encode_block =
        [](uint8_t b1, uint8_t b2, uint8_t b3) -> std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> {
            const int e1 = b1 >> 2;
            const int e2 = ((b1 << 4) & 0x30) | (b2 >> 4);
            const int e3 = ((b2 << 2) & 0x3C) | (b3 >> 6);
            const int e4 = b3 & 63;
            return {encode_table[e1], encode_table[e2], encode_table[e3], encode_table[e4]};
    };

    std::tuple<uint8_t, uint8_t, uint8_t, uint8_t> enc{};
    Buffer dst{};
    size_t i{};

    const auto size = src.size() & ~3;

    for (i = 0; i < size; i += 3) {
        enc = encode_block(src[i], src[i + 1], src[i + 2]);
        dst.push_back(std::get<0>(enc));
        dst.push_back(std::get<1>(enc));
        dst.push_back(std::get<2>(enc));
        dst.push_back(std::get<3>(enc));
    }

    switch (size & 3) {
    case 0:
        break;
    case 1:
        enc = encode_block(src[i + 1], 0, 0);
        dst.push_back(std::get<0>(enc));
        dst.push_back(BASE64_PAD);
        dst.push_back(BASE64_PAD);
        dst.push_back(BASE64_PAD);
        break;
    case 2:
        enc = encode_block(src[i + 1], src[i + 2], 0);
        dst.push_back(std::get<0>(enc));
        dst.push_back(std::get<1>(enc));
        dst.push_back(BASE64_PAD);
        dst.push_back(BASE64_PAD);
        break;
    default:;
    }

    return dst;
}

}
}
