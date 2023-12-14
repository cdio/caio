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
#include <cstdint>
#include <cstring>
#include <exception>
#include <iomanip>
#include <ostream>
#include <memory>
#include <string>
#include <sstream>

#define CAIO_STR_(x)    #x
#define CAIO_STR(x)     CAIO_STR_(x)


namespace caio {

using addr_t = uint16_t;
using saddr_t = int16_t;

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

template<typename T>
using sptr_t = std::shared_ptr<T>;

template<typename T>
using uptr_t = std::unique_ptr<T>;

std::string stacktrace();
std::ostream& stacktrace(std::ostream& os);

/**
 * Base error.
 */
class Error : public std::exception {
public:
    /**
     * Initialise this error.
     * @param elem   Name of the element that generated this error;
     * @param reason Reason of description of this error.
     */
    Error(const std::string& elem, const std::string& reason)
        : std::exception{},
          _reason{(elem.empty() ? reason : elem + ": " + reason)} {
    }

    /**
     * Initialise this error.
     * @param reason The reason or description of this error.
     */
    Error(const std::string& reason = {})
        : Error{{}, reason} {
    }

    /**
     * Initialise this error.
     * @param ex Standard exception associated to this error.
     */
    Error(const std::exception& ex)
        : Error{{}, ex.what()} {
    }

    /**
     * Return a null terminated string with the description of this error.
     * @return The null terminated string description of this error.
     */
    const char* what() const noexcept override {
        return _reason.c_str();
    }

    /**
     * Set the reason or description of this error.
     * @param reason Reason or description of this error.
     * @return A reference to this error.
     */
    Error& operator=(const std::string& reason) {
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
    static std::string to_string() {
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
ERROR_CLASS(IOError);
ERROR_CLASS(UIError);

}
