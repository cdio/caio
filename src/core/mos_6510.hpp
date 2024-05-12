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

#include "gpio.hpp"
#include "mos_6502.hpp"

namespace caio {

/**
 * MOS 6510 emulator.
 */
class Mos6510 : public Mos6502 {
public:
    constexpr static const char* TYPE    = "MOS6510";

    constexpr static const addr_t PORT_0 = 0x0000;  /* Data direction register  */
    constexpr static const addr_t PORT_1 = 0x0001;  /* I/O port                 */

    constexpr static const uint8_t P0    = 0x01;
    constexpr static const uint8_t P1    = 0x02;
    constexpr static const uint8_t P2    = 0x04;
    constexpr static const uint8_t P3    = 0x08;
    constexpr static const uint8_t P4    = 0x10;
    constexpr static const uint8_t P5    = 0x20;
    constexpr static const uint8_t P6    = 0x40;
    constexpr static const uint8_t P7    = 0x80;
    constexpr static const uint8_t PALL  = 0xFF;

    using breakpoint_cb_t = std::function<void(Mos6510&, void*)>;

    using ior_t = Gpio::ior_t;
    using iow_t = Gpio::iow_t;

    /**
     * Initialise this CPU.
     * @param mmap  System mappings;
     * @param type  CPU type;
     * @param label CPU label.
     * @see ASpace
     */
    Mos6510(const sptr_t<ASpace>& mmap, const std::string& type = TYPE, const std::string& label = LABEL)
        : Mos6502{mmap, type, label} {
    }

    virtual ~Mos6510() {
    }

    /**
     * Add an input callback.
     * @param ior  Input callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_ior()
     */
    void add_ior(const ior_t& ior, uint8_t mask) {
        _ioport.add_ior(ior, mask);
    }

    /**
     * Add an ouput callback.
     * @param iow  Output callback;
     * @param mask Bits used by the callback.
     * @see Gpio::add_iow()
     */
    void add_iow(const iow_t& iow, uint8_t mask) {
        _ioport.add_iow(iow, mask);
    }

    /**
     * Add a breakpoint on a memory address.
     * @see Mos6502::bpadd()
     */
    void bpadd(addr_t addr, const breakpoint_cb_t& cb, void* arg) {
        Mos6502::bpadd(addr, *reinterpret_cast<const Mos6502::breakpoint_cb_t*>(&cb), arg);
    }

    /**
     * @see Mos6502::read()
     */
    uint8_t read(addr_t addr, Device::ReadMode mode = Device::ReadMode::Read);

    /**
     * @see Mos6502::write()
     */
    void write(addr_t addr, uint8_t data);

private:
    uint8_t _iodir{};
    Gpio    _ioport{};
};

}
