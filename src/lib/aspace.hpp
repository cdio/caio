/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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

#include <vector>

#include "device.hpp"
#include "types.hpp"


namespace caio {

/**
 * Address Space.
 * This class implements the interface used to access a system's memory mappings.
 * The address space (addr_t) is separated into a number of fixed size banks,
 * each range of addresses inside a bank must be handled by a single device.
 * There are two sets of address space mappings, one for reading and another for writing.
 * @see addr_t
 * @see Device
 */
class ASpace {
public:
    /**
     * Device mappings type.
     * The device mappings type binds a device instance to a memory bank.
     * The first element is a pointer to the device and the second element
     * is the offset within the addr_t address space of the memory bank.
     *
     * The device must handle the entire set of addresses of a bank starting
     * at the specified offset: offset + bank_size <= device.size().
     *
     * When a read or a write is delivered the device receives a relative
     * address from the starting offset.
     *
     * @see devptr_t
     */
    using devmap_t = std::pair<devptr_t, addr_t>;

    /**
     * Address mappings type.
     * The address mappings type defines the device mappings for an entire address space.
     * It must not contain holes (unmapped addresses): The entire addr_t set must be mapped.
     * TODO Don't use vector
     */
    using addrmap_t = std::vector<devmap_t>;

    virtual ~ASpace() {
    }

    /**
     * @see Device::read()
     */
    virtual uint8_t read(addr_t addr) const;

    /**
     * @see Device::write()
     */
    virtual void write(addr_t addr, uint8_t value);

    /**
     * Dump the contents of the read and write mappings as human readable strings.
     * @param os Stream to dump to;
     */
    std::ostream &dump(std::ostream &os) const;

protected:
    ASpace() {
    }

    /**
     * Initialise this address space.
     * @param rmaps Address mappings for read operations (it must have the same size as the write map);
     * @param wmaps Address mappings for write operations (it must have the same size as the read map);
     * @param amask Address space mask (addresses are ANDed with this mask).
     * @exception InvalidArgument if one of the parameters is ill formed.
     * @see reset()
     */
    ASpace(const addrmap_t &rmaps, const addrmap_t &wmaps, addr_t amask) {
        reset(rmaps, wmaps, amask);
    }

    /**
     * Reset this address space with a new set of mappings.
     * - The size of the address space is 2^(sizeof addr_t);
     * - Both the read an write mappings must be the same size which equals the number of banks;
     * - All banks must have the same size (2^(sizeof addr_t) / banks);
     * - The size of a bank must be a power of 2.
     * @param rmaps Address mappings for read operations;
     * @param wmaps Address mappings for write operations;
     * @param amask Address space mask (addresses are ANDed with this mask).
     * @exception InvalidArgument if one of the parameters is ill formed.
     */
    void reset(const addrmap_t &rmaps, const addrmap_t &wmaps, addr_t amask);

private:
    std::pair<addr_t, addr_t> decode(addr_t addr) const;

    addr_t    _amask{};
    addr_t    _bsize{};
    addr_t    _bmask{};
    addr_t    _bshift{};
    addrmap_t _rmaps{};
    addrmap_t _wmaps{};
};

}
