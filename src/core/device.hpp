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

#include <memory>
#include <ostream>
#include <string_view>

#include "name.hpp"
#include "types.hpp"

namespace caio {

using devptr_t = sptr_t<class Device>;

/**
 * Generic device.
 * This class implements a hardware device that exposes an address
 * (or register) range where data can be written to or read from.
 * This class must be derived by an actual emulated device.
 */
class Device : public Name {
public:
    enum class ReadMode {
        Peek,       /**< A read operation never changes the internal state of the device.   */
        Read        /**< A read operation could change the internal state of the device.    */
    };

    /**
     * Return a human readable string representing this device.
     * @return A string representing this device.
     */
    std::string to_string() const override;

    /**
     * Reset this device.
     */
    virtual void reset() = 0;

    /**
     * Read from an address or device register.
     * @param addr Address to read from;
     * @param mode Read mode (default is ReadMode::Read).
     * @return The data stored at the specified address.
     * @see ReadMode
     */
    virtual uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) = 0;

    /**
     * Read from an address or register without changing the device's internal state.
     * @param addr Address to read from;
     * @return The data stored at the specified address.
     */
    uint8_t peek(addr_t addr) const {
        return const_cast<Device*>(this)->read(addr, ReadMode::Peek);
    }

    /**
     * Write a value to an address or register.
     * @param addr Address to write to;
     * @param data Data to write.
     */
    virtual void write(addr_t addr, uint8_t data) = 0;

    /**
     * Return the number of addresses (or registers) handled by this device.
     * @return The number of addresses handled by this device.
     */
    virtual size_t size() const = 0;

    /**
     * Dump the content of this device to an output stream in a human readable form.
     * @param os   Output stream;
     * @param base Base address.
     * @return The output stream.
     */
    virtual std::ostream& dump(std::ostream& os, addr_t base = 0) const = 0;

protected:
    Device(const std::string_view type, const std::string_view label)
        : Name{type, label} {
    }

    virtual ~Device() {
    }
};

}
