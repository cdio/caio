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

#include <cerrno>
#include <cinttypes>
#include <cstdint>
#include <cstring>
#include <exception>
#include <format>
#include <iomanip>
#include <numbers>
#include <ostream>
#include <memory>
#include <span>
#include <string>
#include <string_view>
#include <sstream>
#include <system_error>
#include <type_traits>
#include <vector>

#define CAIO_STR_(x)    #x
#define CAIO_STR(x)     CAIO_STR_(x)

namespace caio {

using fp_t    = float;
using addr_t  = uint16_t;
using saddr_t = int16_t;

template<typename T>
using sptr_t = std::shared_ptr<T>;

template<typename... T>
using uptr_t = std::unique_ptr<T...>;

template<typename T>
using uptrd_t = std::unique_ptr<T, void(*)(T*)>;

using Buffer     = std::vector<uint8_t>;
using Buffer_it  = std::vector<uint8_t>::iterator;
using Buffer_cit = std::vector<uint8_t>::const_iterator;

using samples_fp  = std::span<fp_t>;
using samples_i16 = std::span<int16_t>;

using p_coeffs = std::pair<samples_fp, samples_fp>;

constexpr fp_t Pi = std::numbers::pi_v<fp_t>;

constexpr static const uint8_t D0 = (1 << 0);
constexpr static const uint8_t D1 = (1 << 1);
constexpr static const uint8_t D2 = (1 << 2);
constexpr static const uint8_t D3 = (1 << 3);
constexpr static const uint8_t D4 = (1 << 4);
constexpr static const uint8_t D5 = (1 << 5);
constexpr static const uint8_t D6 = (1 << 6);
constexpr static const uint8_t D7 = (1 << 7);

constexpr static const addr_t A0  = (1 << 0);
constexpr static const addr_t A1  = (1 << 1);
constexpr static const addr_t A2  = (1 << 2);
constexpr static const addr_t A3  = (1 << 3);
constexpr static const addr_t A4  = (1 << 4);
constexpr static const addr_t A5  = (1 << 5);
constexpr static const addr_t A6  = (1 << 6);
constexpr static const addr_t A7  = (1 << 7);
constexpr static const addr_t A8  = (1 << 8);
constexpr static const addr_t A9  = (1 << 9);
constexpr static const addr_t A10 = (1 << 10);
constexpr static const addr_t A11 = (1 << 11);
constexpr static const addr_t A12 = (1 << 12);
constexpr static const addr_t A13 = (1 << 13);
constexpr static const addr_t A14 = (1 << 14);
constexpr static const addr_t A15 = (1 << 15);

template<typename>
struct Array;

template<size_t N>
struct Array<std::array<uint8_t, N>> {
    constexpr static const size_t size = N;
};

template<typename C>
concept is_container_v = std::is_same_v<C, Buffer> ||
                         std::is_same_v<C, std::span<uint8_t>> ||
                         std::is_same_v<C, std::array<uint8_t, Array<C>::size>>;

/**
 * Retrieve the stack trace.
 * @return The stack trace.
 */
std::string stacktrace();

/**
 * Send the stack trace to an output stream.
 * @param os Output stream.
 * @return The output stream.
 */
std::ostream& stacktrace(std::ostream& os);

/**
 * Base error.
 */
class Error : public std::exception {
public:
    /**
     * Initialise this error.
     */
    Error(std::string_view errmsg = {})
        : std::exception{},
          _reason{errmsg}
    {
    }

    /**
     * Initialise this error.
     * @param fmt  Error message format string;
     * @param args Error message format string arguments.
     */
    template<typename... Args>
    Error(std::format_string<Args...> fmt, Args&&... args)
        : std::exception{},
          _reason{std::vformat(fmt.get(), std::make_format_args(args...))}
    {
    }

    /**
     * Initialise this error.
     * @param ex Exception associated to this error.
     */
    Error(const std::exception& ex)
        : std::exception{},
          _reason{ex.what()}
    {
    }

    /**
     * Return a null terminated string with the error message.
     * @return The null terminated string with the error message.
     */
    const char* what() const noexcept override
    {
        return _reason.c_str();
    }

    /**
     * Set the error message.
     * @param reason Error message.
     * @return A reference to this error.
     */
    Error& operator=(std::string_view reason)
    {
        _reason = reason;
        return *this;
    }

    /**
     * Return an error message that corresponds to the specified system error code.
     * @param err System error code.
     * @return The error message.
     */
    static std::string to_string(int err);

    /**
     * Return an error message that corresponds to the current errno value.
     * @return The error message.
     */
    static std::string to_string()
    {
        return to_string(errno);
    }

private:
    std::string _reason{};
};

#define ERROR_CLASS(cname)      class cname : public Error { public: using Error::Error; }

ERROR_CLASS(ConfigError);
ERROR_CLASS(InvalidArgument);
ERROR_CLASS(InvalidCartridge);
ERROR_CLASS(InvalidNumber);
ERROR_CLASS(InvalidPalette);
ERROR_CLASS(IOError);
ERROR_CLASS(LoggerError);
ERROR_CLASS(NotSupported);
ERROR_CLASS(SignalError);
ERROR_CLASS(UIError);

/**
 * Argument check.
 * @param cond  Condition.
 * @param fmt   Message format string;
 * @param args  Format string arguments.
 * @execption InvalidArgument if the specified condition is not met.
 */
template<typename... Args>
void expects(bool cond, std::format_string<Args...> fmt, Args&&... args)
{
    if (!cond)
    {
        throw InvalidArgument{std::vformat(fmt.get(), std::make_format_args(args...))};
    }
}

#ifdef D_DEBUG
#define CAIO_ASSERT(cond)       caio::expects(cond, __PRETTY_FUNCTION__);
#else
#define CAIO_ASSERT(cond)
#endif

}
