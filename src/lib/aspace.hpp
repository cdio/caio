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

#include <vector>

#include "device.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace cemu {

/**
 * Address Space.
 * The Address Space is the interface used to access a system's address space;
 * the entire addr_t set of addresses is separated into a number of fixed sized
 * banks, each address range of a bank is handled by a single device.
 * There are two sets of address space mappings, one for reading and another for writing.
 * @see addr_t
 * @see Device
 */
class ASpace {
public:
    virtual ~ASpace() {
    }

    /**
     * @see Device::read()
     */
    virtual uint8_t read(addr_t addr) const {
        try {
            const auto [bank, offset] = decode(addr);
            auto &dev = _rmaps->at(bank);
            return dev.first->read(dev.second + offset);
        } catch (std::out_of_range &) {
            throw InternalError{"Unmapped address: $" + utils::to_string(addr)};
        }
    }

    /**
     * @see Device::write()
     */
    virtual void write(addr_t addr, uint8_t value) {
        try {
            const auto [bank, offset] = decode(addr);
            auto &dev = _wmaps->at(bank);
            dev.first->write(dev.second + offset, value);
        } catch (std::out_of_range &) {
            throw InternalError{"Unmapped address: $" + utils::to_string(addr)};
        }
    }

    /**
     * @see Device::read_addr()
     */
    virtual addr_t read_addr(addr_t addr) const {
        try {
            const auto [bank, offset] = decode(addr);
            auto &dev = _rmaps->at(bank);
            return dev.first->read_addr(dev.second + offset);
        } catch (std::out_of_range &) {
            throw InternalError{"Unmapped address: $" + utils::to_string(addr)};
        }
    }

    /**
     * @seee Device::write_addr()
     */
    virtual void write_addr(addr_t addr, addr_t value) {
        try {
            const auto [bank, offset] = decode(addr);
            auto &dev = _wmaps->at(bank);
            dev.first->write_addr(dev.second + offset, value);
        } catch (std::out_of_range &) {
            throw InternalError{"Unmapped address: $" + utils::to_string(addr)};
        }
    }

protected:
    /**
     * Device mappings type.
     * The device mappings type binds a device instance to a bank.
     * The first element is the device handling the bank addresses;
     * the second element is the offset within the addr_t address space.
     *
     * Care must be taken when creating a mapping: The device must be able to handle the
     * entire set of addresses of a bank starting at the specified offset:
     * offset + bank_size <= device.size().
     *
     * When a read or a write is delivered the device receives a relative
     * address from the starting offset.
     */
    using devmap_t = std::pair<devptr_t, addr_t>;

    /**
     * Address mappings type.
     * The address mappings type defines the device mappings for an entire address space.
     * It must not contain holes (unmapped addresses): The entire addr_t set must be mapped.
     */
    using addrmap_t = std::shared_ptr<std::vector<devmap_t>>;

    ASpace() {
    }

    /**
     * Initialise this address space.
     * @param rmaps Address mappings for read operations (it must have the same size as the write map);
     * @param wmaps Address mappings for write operations (it must have the same size as the read map).
     * @exception InternalError
     * @see reset()
     */
    ASpace(const addrmap_t &rmaps, const addrmap_t &wmaps) {
        reset(rmaps, wmaps);
    }

    /**
     * Reset this address space with a new set of mappings.
     * - The size of the address space is 2^(sizeof addr_t);
     * - Both the read an write mappings must be the same size which equals the number of banks;
     * - All banks must have the same size (2^(sizeof addr_t) / banks);
     * - The size of a bank must be a power of 2.
     * @param rmaps Address mappings for read operations;
     * @param wmaps Address mappings for write operations;
     * @exception InternalError if one of the parameters is ill formed.
     */
    void reset(const addrmap_t &rmaps, const addrmap_t &wmaps);

private:
    std::pair<addr_t, addr_t> decode(addr_t addr) const {
        addr_t bank = addr >> _bshift;
        addr_t offset = addr & _bmask;
        return {bank, offset};
    }

    addr_t    _bsize{};
    addr_t    _bmask{};
    addr_t    _bshift{};
    addrmap_t _rmaps{};
    addrmap_t _wmaps{};
};

}
