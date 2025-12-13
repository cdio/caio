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

#include "device.hpp"
#include "name.hpp"
#include "serializer.hpp"
#include "types.hpp"

#include <array>
#include <span>
#include <string_view>

namespace caio {

/**
 * Address Space.
 * Address space is the interface used to access the memory mappings of a system,
 * anything that involves the address bus and/or the data bus should be handled
 * by an object derived from this class.
 * <br>
 * The entire range of addresses (2^(8*sizeof(addr_t))) must be divided into a
 * number of fixed size banks, the address range within a bank must be handled
 * by a device.
 * <br>
 * There are two sets of address space mappings, one for reading and another for writing.
 * @see addr_t
 * @see ASpace::devmap_t
 * @see ASpace::addrmap_t
 * @see Device
 */
class ASpace : public Name {
public:
    constexpr static const char* TYPE = "ASPACE";
    constexpr static const char* LABEL = "aspace";

    using ReadMode = Device::ReadMode;

    /**
     * Device mappings type.
     * The device mappings type binds a device to a memory bank.
     * The first element is a pointer to the device and the second element
     * is the start address of the bank within the global address space.
     *
     * The device must handle the entire set of addresses of the specified bank:
     * offset + bank_size <= device.size().
     *
     * When a read or a write operation is delivered to the device, it receives
     * a relative address from the specified bank's start address.
     *
     * @see devptr_t
     */
    using devmap_t = std::pair<devptr_t, addr_t>;

    /**
     * Address mappings type.
     * The address mappings type defines the device mappings for the entire address space.
     * It must not contain holes (unmapped addresses): The entire range of addresses
     * supported by the addr_t type must be mapped (2^(8*sizeof(addr_t))).
     * @see addr_t
     * @see devmap_t
     */
    using addrmap_t = std::span<devmap_t>;

    /**
     * Array of memory banks.
     */
    template<size_t BLOCKS>
    using bank_t = std::array<devmap_t, BLOCKS>;

    /**
     * @see Device::read(size_t, Device::ReadMode)
     */
    virtual uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read);

    /**
     * @see Device::write(size_t, uint8_t)
     */
    virtual void write(addr_t addr, uint8_t value);

    /**
     * Reset this address space.
     */
    virtual void reset()
    {
    }

    /**
     * @see Device::peek(size_t)
     */
    uint8_t peek(addr_t addr) const
    {
        return const_cast<ASpace*>(this)->read(addr, ReadMode::Peek);
    }

    /**
     * Read the content of the data bus.
     * This is usually the last value written to or read from the address space.
     * @return The value present on the data bus.
     */
    uint8_t data_bus() const
    {
        return _data_bus;
    }

    /**
     * Set a value on the data bus.
     * The value on the data bus is the one that was last read from or
     * written to the address space; this value can be overrided by calling
     * this method.
     * @param data Data to set.
     */
    uint8_t data_bus(uint8_t data)
    {
        _data_bus = data;
        return _data_bus;
    }

    /**
     * Read the content of the address bus.
     * This is usually the last address written to or read from the address space.
     * @return The value present on the address bus.
     */
    addr_t address_bus() const
    {
        return _address_bus;
    }

    /**
     * Set a value on the address bus.
     * The value on the address bus is the last address written to
     * or read from the address space; this value can be overrided by
     * calling this method.
     * @param addr Address to set.
     */
    virtual void address_bus(addr_t addr)
    {
        _address_bus = addr;
    }

    /**
     * Dump the contents of the read and write memory mappings as human readable strings.
     * @param os Stream to dump to.
     * @return os.
     */
    virtual std::ostream& dump(std::ostream& os) const;

protected:
    ASpace(std::string_view type = TYPE, std::string_view label = LABEL);

    /**
     * Initialise this address space.
     * @param rmaps Address mappings for read operations (it must have the same size as the write map);
     * @param wmaps Address mappings for write operations (it must have the same size as the read map);
     * @param amask Address space mask (addresses are bitwise ANDed with this mask).
     * @see reset()
     */
    ASpace(const addrmap_t& rmaps, const addrmap_t& wmaps, addr_t amask);

    /**
     * Reset this address space with a new set of mappings.
     * - The size of the address space is 2^(sizeof addr_t);
     * - Both the read an write mappings must have the same size which equals the number of banks;
     * - All banks must have the same size (2^(sizeof addr_t) / banks);
     * - The size of a bank must be a power of 2.
     * @param rmaps Address mappings for read operations;
     * @param wmaps Address mappings for write operations;
     * @param amask Address space mask (addresses are bitwise ANDed with this mask).
     */
    void reset(const addrmap_t& rmaps, const addrmap_t& wmaps, addr_t amask);

private:
    std::pair<addr_t, addr_t> decode(addr_t addr) const;

    addr_t    _amask{};
    addr_t    _bsize{};
    addr_t    _bmask{};
    addr_t    _bshift{};
    addrmap_t _rmaps{};
    addrmap_t _wmaps{};
    uint8_t   _data_bus{};
    addr_t    _address_bus{};

    friend Serializer& operator&(Serializer&, ASpace&);
};

}
