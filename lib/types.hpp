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

#define CEMU_STR_(x)    #x
#define CEMU_STR(x)     CEMU_STR_(x)


namespace cemu {

using addr_t = uint16_t;
using saddr_t = int16_t;


void stacktrace(std::ostream &);


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
    Error(const std::string &elem, const std::string &reason)
        : std::exception{},
          _reason{(elem.empty() ? reason : elem + ": " + reason)} {
    }

    /**
     * Initialise an error exception.
     * @param reason Reason for this error.
     */
    Error(const std::string &reason = {})
        : Error{{}, reason} {
    }

    /**
     * Initialise an error exception.
     * @param ex Exception.
     */
    Error(const std::exception &ex)
        : Error{{}, ex.what()} {
    }

    /**
     * @return The reason for this error.
     */
    const char *what() const noexcept override {
        return _reason.c_str();
    }

    /**
     * Set the reason for this error.
     * @param reason Reason for this error.
     * @return A reference to this error.
     */
    Error &operator=(const std::string &reason) {
        _reason = reason;
        return *this;
    }

    /**
     * @return A human readable string of the specified error code.
     */
    static std::string to_string(int err) {
//FIXME
#if 0 /* unsurprisingly, gnu strerror_r() is broken */
        char buf[128];
        strerror_r(err, buf, sizeof(buf));
        return buf;
#else
        return std::strerror(err);
#endif
    }

    /**
     * @return A human readable string of the current errno code.
     */
    static std::string to_string() {
        return to_string(errno);
    }

private:
    std::string _reason{};
};


/**
 * Internal error: When something is not ok within the emulator library itself.
 */
class InternalError : public Error {
    using Error::Error;
};


/**
 * Invalid Argument.
 */
class InvalidArgument : public Error {
    using Error::Error;
};


/**
 * Invalid Cartridge.
 */
class InvalidCartridge : public Error {
    using Error::Error;
};


/**
 * Invalid Expression.
 */
class InvalidExpression : public Error {
    using Error::Error;
};


/**
 * Invalid number conversion from or to string.
 */
class InvalidNumber : public Error {
    using Error::Error;
};


/**
 * Failed I/O operation.
 */
class IOError : public Error {
    using Error::Error;
};


/**
 * Missing Key Error.
 */
class MissingKeyError : public Error {
public:
    MissingKeyError(const std::string &key)
        : Error{"Missing key: " + key} {
    }
};


/**
 * Missing Section Error.
 */
class MissingSectionError : public MissingKeyError {
    using MissingKeyError::MissingKeyError;
};


/**
 * Not implemented.
 */
class NotImplemented : public Error {
    using Error::Error;
};


/**
 * User Interfac Error.
 */
class UIError : public Error {
    using Error::Error;
};

}
