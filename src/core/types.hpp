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

template<typename T>
using sptr_t = std::shared_ptr<T>;

template<typename T>
using uptr_t = std::unique_ptr<T>;

std::string stacktrace();
std::ostream& stacktrace(std::ostream& os);

/**
 * Base error.
 * All the exceptions thrown by any emulator component have error as base class.
 */
class Error : public std::exception {
public:
    /**
     * Initialise and error excpetion.
     * @param elem   Name of the element that generated this error;
     * @param reason Reason for this error.
     */
    Error(const std::string& elem, const std::string& reason)
        : std::exception{},
          _reason{(elem.empty() ? reason : elem + ": " + reason)} {
    }

    /**
     * Initialise an error exception.
     * @param reason Reason for this error.
     */
    Error(const std::string& reason = {})
        : Error{{}, reason} {
    }

    /**
     * Initialise an error exception.
     * @param ex Exception.
     */
    Error(const std::exception& ex)
        : Error{{}, ex.what()} {
    }

    /**
     * @return The reason for this error.
     */
    const char* what() const noexcept override {
        return _reason.c_str();
    }

    /**
     * Set the reason for this error.
     * @param reason Reason for this error.
     * @return A reference to this error.
     */
    Error& operator=(const std::string& reason) {
        _reason = reason;
        return *this;
    }

    /**
     * @return A human readable string of the specified error code.
     */
    static std::string to_string(int err);

    /**
     * @return A human readable string of the current errno code.
     */
    static std::string to_string() {
        return to_string(errno);
    }

private:
    std::string _reason{};
};

#define ERROR_CLASS(cname)      class cname : public Error { using Error::Error; }

ERROR_CLASS(ConfigError);
ERROR_CLASS(InvalidArgument);
ERROR_CLASS(InvalidCartridge);
ERROR_CLASS(InvalidNumber);
ERROR_CLASS(IOError);
ERROR_CLASS(UIError);

}
