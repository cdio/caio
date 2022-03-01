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
#include "gpio.hpp"
#include "mos_6510.hpp"


namespace cemu {
namespace c64 {

/**
 * The PLA programmed for the C64.
 */
class PLA : public ASpace {
public:
    constexpr static const uint8_t LORAM    = Mos6510::P0;
    constexpr static const uint8_t HIRAM    = Mos6510::P1;
    constexpr static const uint8_t CHAREN   = Mos6510::P2;
    constexpr static const uint8_t GAME     = 0x08;
    constexpr static const uint8_t EXROM    = 0x10;
    constexpr static const uint8_t CPU_MASK = LORAM | HIRAM | CHAREN;
    constexpr static const uint8_t EXT_MASK = GAME | EXROM;
    constexpr static const uint8_t MASK     = CPU_MASK | EXT_MASK;

    constexpr static const uint8_t ROMH     = 0x01;
    constexpr static const uint8_t ROML     = 0x02;

    PLA(const devptr_t &ram, const devptr_t &basic, const devptr_t &kernal, const devptr_t &chargen,
        const devptr_t &io, const devptr_t &cart);

    virtual ~PLA();

    /**
     * @return The status of the input pins (bitwise OR combination of LORAM, HIRAM, CHAREN, GAME, EXROM).
     */
    uint8_t mode() const;

    /**
     * Set the input pins.
     * @param value Value to set (bitwise OR of combination LORAM, HIRAM, CHAREN, GAME, EXROM).
     */
    void mode(uint8_t value);

    /**
     * Set the CPU input pins.
     * @param value Value to set (bitwise OR of combination LORAM, HIRAM, CHAREN).
     */
    void cpu_pins(uint8_t value);

    /**
     * Set the EXTernal input pins.
     * @param value Value to set (bitwise OR of combination GAME, EXROM).
     */
    void ext_pins(uint8_t value);

    /**
     * Add an ouput callback.
     * Registered callbacks are called each time the status of the ROMH/ROML lines are chagned.
     * @param iow  Output callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_iow()
     */
    void add_iow(const Gpio::iow_t &iow, uint8_t mask);

private:
    Gpio    _ioport{};                      /* Output ports         */
    uint8_t _romhl{};                       /* ROMH and ROML lines  */
    uint8_t _mode{};                        /* Current mapping mode */

    std::array<addrmap_t, 32> _rmodes{};    /* Read mapping modes   */
    std::array<addrmap_t, 32> _wmodes{};    /* Write mapping modes  */
};

}
}
