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

#include <array>

#include "aspace.hpp"
#include "device.hpp"


namespace cemu {
namespace c64 {

/**
 * C64 Address Space.
 * The C64 address space implements the mappings of devices connected
 * to the CPU via its address lines. This class implements the PLA functions.
 * @see ASpace
 */
class C64ASpace : public ASpace {
public:
    /**
     * PLA flags (active low).
     */
    constexpr static unsigned LORAM                  = 0x01;
    constexpr static unsigned HIRAM                  = 0x02;
    constexpr static unsigned CHAREN                 = 0x04;
    constexpr static unsigned CASSETTE_DATA_OUT      = 0x08;
    constexpr static unsigned CASSETTE_SWITCH_SENSE  = 0x10;
    constexpr static unsigned CASSETTE_MOTOR_CONTROL = 0x20;
    constexpr static unsigned GAME                   = 0x100;
    constexpr static unsigned EXROM                  = 0x200;

    /**
     * Initialise this C64 address space.
     * @param ram     RAM (64K);
     * @param basic   Basic ROM (8K);
     * @param kernel  Kernal ROM (8K);
     * @param chargen Chargen ROM (4K);
     * @param io      IO device (VIC-II, colour RAM, CIA and SID) (4K);
     * @param cart    Cartridge ROM (8K or 16K) or null.
     * @exception InvalidWriteAddress see reset()
     * @see C64IO
     * @see reset()
     */
    C64ASpace(devptr_t ram, devptr_t basic, devptr_t kernal, devptr_t chargen, devptr_t io, devptr_t cart)
        : ASpace{} {
        C64ASpace::reset(ram, basic, kernal, chargen, io, cart);
    }

    virtual ~C64ASpace() {
    }

    /**
     * Write data into a memory address.
     * Writes to processor's I/O ports are properly handled.
     * @param addr  Address to write to;
     * @param value Value to write.
     * @exception InvalidWriteAddress see ASpace::write()
     * @see ASpace::write()
     */
    void write(addr_t addr, uint8_t value) override;

    /**
     * Write an address into a memory address.
     * Writes to processor's I/O ports are properly handled.
     * @param addr  Address to write to;
     * @param value Value to write.
     * @exception InvalidWriteAddress see ASpace::write_addr()
     * @see ASpace::write_addr()
     */
    void write_addr(addr_t addr, addr_t value) override;

private:
    /**
     * Reset this address space with a new set of devices.
     * Port registers are set to the default values.
     * @param ram     RAM (64K);
     * @param basic   Basic ROM (8K);
     * @param kernel  Kernal ROM (8K);
     * @param chargen Chargen ROM (4K);
     * @param io      IO device (handles VIC-II, colour RAM, CIA ports and SID) (4K);
     * @param cart    Cartridge ROM (8K or 16K) or null.
     * @exception InvalidWriteAddress see ASpace::write()
     * @exception InvalidArgument     see ASpace::reset()
     * @see C64IO
     * @see DeviceRAM
     * @see DeviceROM
     */
    void reset(devptr_t ram, devptr_t basic, devptr_t kernal, devptr_t chargen, devptr_t io, devptr_t cart);

    /**
     * Compact the PLA flags into a 5 bits mode key.
     * The generated key is composed of bits EXROM, GAME,
     * CHARGEN, HIRAM and LORAM and it specifies a mapping mode.
     * @return The generated mode key.
     */
    unsigned getkey() const;

    /**
     * Re-map the address space based on the current value of the port register.
     * @exception InvalidArgument see ASpace::reset()
     */
    void remap();

    std::array<addrmap_t, 32> _rmodes{};    /* There are 32 possible read mappings  */
    std::array<addrmap_t, 32> _wmodes{};    /* There are 32 possible write mappings */

    uint8_t  _port0{};                      /* I/O Port 0 (Direction)               */
    unsigned _port1{};                      /* I/O Port 1 (Bank switching)          */
};

}
}
