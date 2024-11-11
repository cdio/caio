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

#include <functional>
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

    using ReadObserverCb  = std::function<void(size_t, ReadMode)>;
    using WriteObserverCb = std::function<void(size_t, uint8_t)>;

    /**
     * Set a device read observer.
     * @param cb Observer callback.
     * @see read(size_t, ReadMode)
     */
    void read_observer(const ReadObserverCb& cb);

    /**
     * Set a device write observer.
     * @param cb Observer callback.
     * @see write(size_t, uint8_t)
     */
    void write_observer(const WriteObserverCb& cb);

    /**
     * Return a human readable string representing this device.
     * @return A string representing this device.
     */
    std::string to_string() const override;

    /**
     * Read from an address or device register.
     * If there is a read observer it is called before
     * the actual device read operation through dev_read().
     * @param addr Address to read from;
     * @param mode Read mode (default is ReadMode::Read).
     * @return The data stored at the specified address.
     * @see dev_read(size_t, ReadMode mode)
     * @see ReadMode
     */
    uint8_t read(size_t addr, ReadMode mode = ReadMode::Read);

    /**
     * Read from an address or register without changing the device's internal state.
     * @param addr Address to read from;
     * @return The data stored at the specified address.
     */
    uint8_t peek(size_t addr) const {
        return const_cast<Device*>(this)->read(addr, ReadMode::Peek);
    }

    /**
     * Write a value to an address or register.
     * If there is a write observer it is called before
     * the actual device write operation through dev_write().
     * @param addr Address to write to;
     * @param data Data to write.
     * @see dev_write(size_t, uint8_t)
     */
    void write(size_t addr, uint8_t data);

    /**
     * Reset this device.
     */
    virtual void reset() = 0;

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
    virtual std::ostream& dump(std::ostream& os, size_t base = 0) const;

protected:
    Device(std::string_view type, std::string_view label)
        : Name{type, label} {
    }

    virtual ~Device() {
    }

    /**
     * Read from a device address or register.
     * @param addr Address to read from;
     * @param mode Read mode (default is ReadMode::Read).
     * @return The data stored at the specified address.
     * @see ReadMode
     */
    virtual uint8_t dev_read(size_t addr, ReadMode mode = ReadMode::Read) = 0;

    /**
     * Write a device address or register.
     * @param addr Address to write to;
     * @param data Data to write.
     */
    virtual void dev_write(size_t addr, uint8_t data) = 0;

    ReadObserverCb  _read_cb{};
    WriteObserverCb _write_cb{};
};

}
