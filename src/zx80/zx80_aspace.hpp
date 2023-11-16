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

#include "aspace.hpp"
#include "device.hpp"
#include "zilog_z80.hpp"

#include "zx80_config.hpp"
#include "zx80_keyboard.hpp"
#include "zx80_video.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * The ZX80 Address Space implements the logic that
 * connect all the devices that make a ZX80 including I/O.
 * @see ASpace
 */
class ZX80ASpace : public ASpace {
public:
    constexpr static const addr_t ADDR_MASK             = 0xFFFF;
    constexpr static const addr_t VIDEO_ADDR_MASK       = A15;
    constexpr static const addr_t KEYBOARD_ADDR_MASK    = A0;
    constexpr static const addr_t INTERRUPT_ADDR_MASK   = A6;
    constexpr static const addr_t BITMAP_ADDR_MASK      = 0x1E00;
    constexpr static const uint8_t VIDEO_RATE_50HZ      = D6;
    constexpr static const uint8_t VIDEO_INVERT         = D7;
    constexpr static const uint8_t VIDEO_HALT           = D6;
    constexpr static const uint8_t CHCODE_MASK          = 0x3F;
    constexpr static const uint8_t CHCODE_BLANK         = 0x00;
    constexpr static const size_t BLOCKS                = 64;

    using bank_t = std::array<devmap_t, BLOCKS>;

    /**
     * Initialise this ZX80 Address Space.
     * @param cpu   CPU;
     * @param ram   Internal RAM (1K or 16K);
     * @param rom   Internal ROM (4K or 8K);
     * @param video Video device;
     * @param kbd   Keyboard device.
     */
    ZX80ASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom, const sptr_t<ZX80Video>& video,
        const sptr_t<ZX80Keyboard>& kbd);

    virtual ~ZX80ASpace() {
    }

    /**
     * @see ASpace::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see ASpace::write()
     */
    void write(addr_t addr, uint8_t value) override;

private:
    void interrupt_req();

    void interrupt_ack();

    uint8_t io_read(addr_t addr);

    /**
     * Retrieve bitmap data.
     * Retrieve the bitmap data that corresponds to the current
     * characater line for the current character code and invert
     * it if necessary.
     * @param base Base address of the character bitmap data within the ROM.
     * @return The bitmap data.
     * @see _chcode
     * @see _counter
     */
    uint8_t character_bitmap(addr_t base);

    sptr_t<Z80>             _cpu;
    devptr_t                _rom;
    sptr_t<ZX80Video>       _video;
    sptr_t<ZX80Keyboard>    _kbd;
    bank_t                  _mmap;
    bool                    _intreq{};      /* Interrupt request flag           */
    addr_t                  _chcode{};      /* Current character code           */
    addr_t                  _counter{};     /* Current character line counter   */
    bool                    _vsync{};       /* VSYNC flag                       */
};

}
}
}
