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

#include "types.hpp"
#include "device.hpp"


namespace cemu {
namespace c64 {

/**
 * C64 I/O device.
 * This class implements a meta-device that embeds all the devices
 * connected to the C64 I/O space between addresses $D000 and $DFFF.
 *
 * $D000 - $D02E -- $0000 - $002E   VIC2 registers
 * $D02F - $D03F -- $002F - $003F   Fixed #$FF
 * $D040 - $D3FF -- $0040 - $03FF   Previous 2 repeated as 64 bytes blocks
 * $D400 - $D7FF -- $0400 - $07FF   SID registers (repeated as 32 bytes blocks)
 * $D800 - $DBFF -- $0800 - $0BFF   Color RAM (only low nibbles are settable)
 * $DC00 - $DCFF -- $0C00 - $0CFF   CIA #1 registers
 * $DD00 - $DDFF -- $0D00 - $0DFF   CIA #2 registers
 * $DE00 - $DEFF -- $0E00 - $0EFF   Reserved for future I/O expansion
 * $DF00 - $DFFF -- $0F00 - $0FFF   Reserved for future I/O expansion
 */
class C64IO : public Device {
public:
    constexpr static const char *TYPE           = "I/O";
    constexpr static const addr_t START_ADDR    = 0xD000;
    constexpr static const addr_t END_ADDR      = 0xE000;
    constexpr static const size_t SIZE          = 0x1000;

    constexpr static const addr_t VIC2_ADDR     = 0x0000;
    constexpr static const addr_t SID_ADDR      = 0x0400;
    constexpr static const addr_t VCOLOR_ADDR   = 0x0800;
    constexpr static const addr_t CIA1_ADDR     = 0x0C00;
    constexpr static const addr_t CIA2_ADDR     = 0x0D00;
    constexpr static const addr_t RESERVED_ADDR = 0x0E00;

    /**
     * Initialise this C64-IO device.
     * @param ram    The C64 RAM (64K);
     * @param vic2   Video controller device;
     * @param sid    Audio controller device;
     * @param vcolor Video colour RAM;
     * @param cia1   CIA#1 device;
     * @param cia2   CIA#2 device.
     */
    C64IO(devptr_t ram, devptr_t vic2, devptr_t sid, devptr_t vcolor, devptr_t cia1, devptr_t cia2)
        : Device{TYPE, {}},
          _vic2{vic2},
          _sid{sid},
          _vcolor{vcolor},
          _cia1{cia1},
          _cia2{cia2} {
    }

    virtual ~C64IO() {
    }

    /**
     * @see Device::size()
     */
    size_t size() const override {
        return SIZE;
    }

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump();
     */
    std::ostream &dump(std::ostream &os, addr_t base = 0) const override;

private:
    devptr_t _vic2{};
    devptr_t _sid{};
    devptr_t _vcolor{};
    devptr_t _cia1{};
    devptr_t _cia2{};
};

}
}
