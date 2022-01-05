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

#include <memory>
#include <ostream>
#include <string>

#include "name.hpp"
#include "types.hpp"


namespace cemu {

using devptr_t = std::shared_ptr<class Device>;


/**
 * Generic device.
 * This class implements a hardware device that exposes an
 * address range where data can be written to or read from.
 * This class must be derived by the actual emulated device.
 */
class Device : public Name {
public:
    virtual ~Device();

    /**
     * @return A human readable string representing this device.
     */
    std::string to_string() const override;

    /**
     * Read an address from an address.
     * @param addr  Address to read from.
     * @param is_le true if the emulated architecture is little endian, false otherwise.
     * @return The address stored at the specified address.
     * @exception InvalidReadAddress if the specified address is not handled by this device.
     */
    addr_t read_addr(size_t addr, bool is_le = true) const;

    /**
     * Write an address into an address.
     * @param addr  Address to write;
     * @param data  Address value to write;
     * @param is_le true if the emulated architecture is little endian, false otherwise.
     * @exception InvalidWriteAddress if the specified address is not handled by this device.
     */
    void write_addr(addr_t addr, addr_t data, bool is_le = true);

    /**
     * Read from an address.
     * @param addr Address to read from.
     * @return The data stored at the specified address.
     * @exception InvalidReadAddress if the specified address is not handled by this device.
     */
    virtual uint8_t read(addr_t addr) const = 0;

    /**
     * Write into an address.
     * @param addr Address to write into;
     * @param data Data to write.
     * @exception InvalidWriteAddress if the specified address is not handled by this device.
     */
    virtual void write(addr_t addr, uint8_t data) = 0;

    /**
     * @return The address range of this device.
     */
    virtual size_t size() const = 0;

    /**
     * Dump the content of this device to an output stream in a human readable form.
     * @param os   Output stream;
     * @param base Base address.
     * @return The output stream.
     */
    virtual std::ostream &dump(std::ostream &os, addr_t base = 0) const = 0;

protected:
    Device(const std::string &type, const std::string &label);
};

}
