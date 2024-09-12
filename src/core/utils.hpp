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
#pragma once

#include <algorithm>
#include <cctype>
#include <chrono>
#include <cstdint>
#include <cstdlib>
#include <iomanip>
#include <limits>
#include <ostream>
#include <span>
#include <sstream>
#include <string>
#include <string_view>
#include <type_traits>
#include <utility>
#include <vector>

#include "types.hpp"
#include "endian.hpp"

namespace caio {
namespace utils {

/**
 * Convert an integer value to floating point.
 * @param value Value to convert;
 * @return The floating point value.
 */
template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
fp_t to_fp(T value)
{
    return static_cast<fp_t>(value) / static_cast<fp_t>(std::numeric_limits<T>::max());
}

/**
 * Convert a floating point value to integer type.
 * @param value Floating point value to convert ([0.0, 1.0] if T is unsigned or [-1.0, 1.0] if T is signed).
 * @return The integer value.
 */
template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
T to_integer(fp_t value)
{
    return static_cast<T>(value * std::numeric_limits<T>::max());
}

/**
 * Convert a floating point value to signed 16 bits.
 * @param value Value to convert (between 0 and 1).
 * @return The 16 bits unsigned value.
 * @see to_integer()
 */
__attribute__((always_inline))
inline int16_t to_i16(fp_t value)
{
    return to_integer<int16_t>(value);
}

/**
 * Align a value to its type size.
 * @param val Value to align.
 * @return The aligned value.
 */
template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
constexpr inline int align(T val)
{
    return ((val + (sizeof(T) - 1)) & ~(sizeof(T) - 1));
}

/**
 * Ceil method as constant expression.
 * @see std::ceil()
 */
template<typename T, std::enable_if_t<std::is_floating_point<T>::value, bool> = true>
constexpr inline int ceil(T fval)
{
    int val = static_cast<int>(fval);

    if (val < fval) {
        ++val;
    }

    return val;
}

/**
 * Tolower a string.
 * @param str String to convert.
 * @return The converted line.
 */
std::string tolow(std::string_view str);

/**
 * Toupper a string.
 * @param str String to convert.
 * @return The converted string.
 */
std::string toup(std::string_view str);

/**
 * Split a string.
 * @param str String to split;
 * @param sep Separator.
 * @return The splitted string without the separator.
 */
std::vector<std::string> split(std::string_view str, char sep);

/**
 * Remove leading and trailing white spaces from a string.
 * @param str The string to trim.
 * @return The trimmed string.
 */
std::string trim(std::string_view str);

/**
 * Convert a byte buffer to string.
 * @param buf Byte buffer to convert.
 * @return The string.
 */
std::string to_string(std::span<const uint8_t> buf);

/**
 * Convert an integer value to uppercase hexadecimal string.
 * The hexadecimal value is prefixed with '0's, depending on its size.
 * @param v Value to convert.
 * @return The generated string.
 */
template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
std::string to_string(T v)
{
    std::stringstream ss{};

    ss << std::hex << std::setfill('0') << std::setw(sizeof(T) * 2) << std::uppercase << +v;

    return ss.str();
}

/**
 * Dump a range of bytes to a stream.
 * Output format:
 *      0000: 00 01 02 03  04 05 06 07  08 09 0A 0B  0C 0D 0E 0F   ................
 *      0010: 10 11 12 13  14 15 16 17  18 19 1A 1B  1C 1D 1E 1F   ................
 *      ...
 * @param os    Output stream;
 * @param begin First position to dump;
 * @param end   Last position to dump (plus 1);
 * @param base  Base address.
 * @return The output stream.
 */
template <typename Iterator>
std::ostream& dump(std::ostream& os, const Iterator begin, const Iterator end, addr_t base = 0)
{
    constexpr static size_t WIDTH = 6 + 11 * 4 + 8;
    constexpr static size_t ELEMS_PER_LINE = 16;
    constexpr static size_t ELEMS_QRT = ELEMS_PER_LINE >> 2;

    std::ostringstream hex{}, str{};

    size_t count = 0;

    for (Iterator it = begin; it != end; ++it, ++count) {
        if (count % ELEMS_PER_LINE == 0) {
            hex << to_string(static_cast<addr_t>(base + count)) << ": ";
        }

        hex << to_string(*it) << (((count + 1) % ELEMS_QRT) == 0 ? "  " : " ");
        str << (std::isprint(*it) ? static_cast<char>(*it) : '.');

        if ((count + 1) % ELEMS_PER_LINE == 0) {
            os << hex.str() << " " << str.str() << "\n";
            hex = {};
            str = {};
        }
    }

    if (count % ELEMS_PER_LINE) {
        os << std::setfill(' ') << std::setw(WIDTH) << std::left << hex.str() << " " << str.str() << "\n";
    }

    return os;
}

/**
 * Dump a byte buffer to a stream.
 * @param os   Output stream;
 * @param cont Container to dump;
 * @param base Base address.
 * @return The output stream.
 */
template<typename C, typename = std::enable_if<is_container<C>::value>>
std::ostream& dump(std::ostream& os, const C& cont, addr_t base = 0)
{
    return dump(os, std::begin(cont), std::end(cont), base);
}

/**
 * Dump a byte buffer to a string.
 * @param cont Container to dump;
 * @param base Base address.
 * @return The string.
 */
template<typename C, typename = std::enable_if<is_container<C>::value>>
std::string dump(const C& cont, addr_t base = 0)
{
    std::ostringstream os{};
    dump(os, cont, base);
    return os.str();
}

/**
 * Convert a string to unsigned long long.
 * If the string is prefixed by '$' an hexadecimal number is considered, decimal numbers must be prefixed with '#'.
 * By default (no prefix) the string is considered to contain an hexadecimal number.
 * @param str String to convert;
 * @param max Maximum value considered valid.
 * @return The converted number.
 * @exception InvalidNumber
 */
unsigned long long to_ulonglong(std::string_view str, size_t max);

/**
 * Convert a string into an integer type.
 * If the string is prefixed by '$' an hexadecimal number is considered, decimal numbers must be prefixed with '#'.
 * By default (no prefix) the string is considered to contain an hexadecimal number.
 * @param str String to convert;
 * @param max Maximum value considered valid.
 * @return The converted number.
 * @exception InvalidNumber
 */
template<typename T, std::enable_if_t<std::is_integral<T>::value, bool> = true>
T to_number(std::string_view str)
{
    return static_cast<T>(to_ulonglong(str, std::numeric_limits<T>::max()));
}

/**
 * Convert a packed BCD to binary number.
 * @param bcd packed BCD.
 * @return The converted value.
 */
constexpr inline uint8_t bcd_to_bin(uint8_t bcd)
{
    return (((bcd >> 4) * 10) + (bcd & 15));
}

/**
 * Convert a binary number to packed BCD.
 * @param bin Binary number.
 * @return The converted packed BCD value.
 */
constexpr inline uint8_t bin_to_bcd(uint8_t bin)
{
    const uint8_t t = bin / 10;
    return ((t << 4) | (bin - t * 10));
}

/**
 * Expand a byte into a 2-bytes with each bit duplicated.
 * 01011001 becomes 00110011'11000011.
 * @param byte Byte to expand.
 * @return The expanded byte.
 */
constexpr inline uint16_t expand_bits(uint8_t byte)
{
    return ((byte & 0x80) ? 0xC000 : 0) |
           ((byte & 0x40) ? 0x3000 : 0) |
           ((byte & 0x20) ? 0x0C00 : 0) |
           ((byte & 0x10) ? 0x0300 : 0) |
           ((byte & 0x08) ? 0x00C0 : 0) |
           ((byte & 0x04) ? 0x0030 : 0) |
           ((byte & 0x02) ? 0x000C : 0) |
           ((byte & 0x01) ? 0x0003 : 0);
}

/**
 * Expand a byte into a 2-bytes with each di-bit duplicated.
 * 01011001 becomes 01010101'10100101.
 * @param byte Byte to expand.
 * @return The expanded byte.
 */
constexpr inline uint16_t expand_dibits(uint8_t byte)
{
    const uint8_t d1 = byte & 0xC0;
    const uint8_t d2 = byte & 0x30;
    const uint8_t d3 = byte & 0x0C;
    const uint8_t d4 = byte & 0x03;

    return (d1 << 8) | (d1 << 6) |
           (d2 << 6) | (d2 << 4) |
           (d3 << 4) | (d3 << 2) |
           (d4 << 2) | d4;
}

/**
 * Convert 01 di-bits to 00 and 10 di-bits to 11.
 * 01011001 becomes 00001100.
 * @param byte Byte to convert.
 * @return The converted byte.
 */
constexpr inline uint8_t convert_01_10(uint8_t byte)
{
    const uint8_t d1 = byte & 0xC0;
    const uint8_t d2 = byte & 0x30;
    const uint8_t d3 = byte & 0x0C;
    const uint8_t d4 = byte & 0x03;

    return ((d1 == 0x00 || d1 == 0x40) ? 0x00 : 0xC0) |
           ((d2 == 0x00 || d2 == 0x10) ? 0x00 : 0x30) |
           ((d3 == 0x00 || d3 == 0x04) ? 0x00 : 0x0C) |
           ((d4 == 0x00 || d4 == 0x01) ? 0x00 : 0x03);
}

/**
 * Replace 01 di-bits with 00 and 10 di-bits with 11.
 * 01011001 becomes 00001100.
 * @param bytes Bytes to convert.
 * @see convert_01_10(uint8_t)
 */
template<typename C, typename = std::enable_if<is_container<C>::value>>
void convert_01_10(C& bytes)
{
    std::for_each(bytes.begin(), bytes.end(), [](uint8_t& byte) {
        byte = convert_01_10(byte);
    });
}

/**
 * Convert 01 and 10 di-bits to 11.
 * 01011000 becomes 11111100.
 * @param byte Byte to convert.
 * @return The converted byte.
 */
constexpr inline uint8_t convert_01_10_to_11(uint8_t byte)
{
    return ((byte & 0xC0) == 0x00 ? 0x00 : 0xC0) |
           ((byte & 0x30) == 0x00 ? 0x00 : 0x30) |
           ((byte & 0x0C) == 0x00 ? 0x00 : 0x0C) |
           ((byte & 0x03) == 0x00 ? 0x00 : 0x03);
}

/**
 * Convert non null di-bits to 11.
 * 01011000 becomes 11111100.
 * @param bytes Bytes to convert.
 */
template<typename C, typename = std::enable_if<is_container<C>::value>>
void convert_01_10_to_11(C& bytes)
{
    std::for_each(bytes.begin(), bytes.end(), [](uint8_t& byte) {
        byte = convert_01_10_to_11(byte);
    });
}

/**
 * Fill a buffer with a pattern and optional random values.
 * The pattern is repeated until the entire destination buffer is filled.
 * @param dst     Destination buffer;
 * @param pattern Pattern;
 * @param random  True to contaminate the destination buffer with some random values.
 */
template<typename T>
void fill(std::span<uint8_t> dst, T pattern, bool random = false)
{
#if __BYTE_ORDER == __LITTLE_ENDIAN
    std::reverse(reinterpret_cast<uint8_t*>(&pattern), reinterpret_cast<uint8_t*>(&pattern) + sizeof(pattern));
#endif
    std::span<uint8_t> pt{reinterpret_cast<uint8_t*>(&pattern), sizeof(T)};
    for (size_t pos = 0; pos < dst.size(); ++pos) {
        dst[pos] = ((random && ((std::rand() % 100) < 2)) ? (std::rand() % 256) : pt[pos % pt.size()]);
    }
}

/**
 * Get the current time.
 * @return The current time in microseconds.
 */
inline uint64_t now()
{
    auto now = std::chrono::steady_clock::now().time_since_epoch();
    return static_cast<uint64_t>(std::chrono::duration_cast<std::chrono::microseconds>(now).count());
}

/**
 * Delay the calling thread by sleeping for a specified amount of time.
 * The actual sleep time depends on the scheduling mechanism
 * used by the running operating system.
 * @param delay Time to sleep (microseconds).
 * @return The actual time this thread slept (microseconds).
 */
uint64_t sleep(uint64_t delay);

/**
 * Calculate the SHA-256 value of a data buffer.
 * @param buf Data buffer.
 * @return The hash value as a string.
 */
std::string sha256(std::span<const uint8_t> buf);

/**
 * Base 64 decoder.
 * @param src Encoded buffer.
 * @return A buffer with the decoded data.
 */
buffer_t base64_decode(const std::span<const uint8_t>& src);

}
}
