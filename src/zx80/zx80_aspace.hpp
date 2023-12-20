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
 * The ZX80 address space implements the logic that
 * connects all the devices that make a ZX80.
 * @see ASpace
 * @see https://problemkaputt.de/zx80-sch.gif
 * @see https://problemkaputt.de/zxdocs.htm
 * @see http://searle.x10host.com/zx80/zx80.html
 */
class ZX80ASpace : public ASpace {
public:
    constexpr static const addr_t ROM_MASK          = ROM_SIZE - 1;
    constexpr static const addr_t ROM8_MASK         = ROM8_SIZE - 1;
    constexpr static const addr_t INTERNAL_RAM_MASK = INTERNAL_RAM_SIZE - 1;
    constexpr static const addr_t EXTERNAL_RAM_MASK = EXTERNAL_RAM_SIZE - 1;
    constexpr static const addr_t VRAM_ADDR_MASK    = A15;
    constexpr static const addr_t RAM_ADDR_MASK     = A14;
    constexpr static const addr_t BITMAP_ADDR_MASK  = 0x1E00;
    constexpr static const addr_t KBD_SCAN_MASK     = ZX80Keyboard::COLUMN_MASK;
    constexpr static const uint8_t ENABLE_BLANK     = D6;
    constexpr static const uint8_t BITMAP_INVERT    = D7;
    constexpr static const uint8_t VIDEO_RATE_50HZ  = D6;
    constexpr static const uint8_t CHCODE_MASK      = 0x3F;
    constexpr static const uint8_t CAS_IN           = D7;

    /**
     * Initialise this ZX80 address space.
     * @param cpu   CPU;
     * @param ram   RAM (1K or 16K);
     * @param rom   ROM (4K);
     * @param video Video interface;
     * @param kbd   Keyboard.
     * @warning All the parameters must be valid otherwise the process is terminated.
     */
    ZX80ASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom, const sptr_t<ZX80Video>& video,
        const sptr_t<ZX80Keyboard>& kbd);

    virtual ~ZX80ASpace();

    /**
     * @see ASpace::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see ASpace::write()
     */
    void write(addr_t addr, uint8_t value) override;

    /**
     * @see ASpace::reset()
     */
    void reset() override;

    /**
     * @see ASpace::address_bus()
     */
    void address_bus(addr_t addr) override;

    /**
     * @see ASpace::dump()
     */
    std::ostream& dump(std::ostream& os) const override;

private:
    enum class AccessType {
        RAM,
        ROM,
        IO
    };

    AccessType access_type(addr_t addr) const;

    void vsync(bool on);

    void hsync();

    void rfsh_cycle();

    uint8_t io_read(addr_t port);

    sptr_t<Z80>             _cpu;
    devptr_t                _ram;
    addr_t                  _ram_mask;
    devptr_t                _rom;
    addr_t                  _rom_mask;
    sptr_t<ZX80Video>       _video;
    sptr_t<ZX80Keyboard>    _kbd;

    addr_t                  _chcode{};      /* Current character code           */
    addr_t                  _counter{};     /* Current character line counter   */
    bool                    _blank{};       /* Blank scanline                   */
    bool                    _intpin{};      /* Status of /INT pin               */
    bool                    _intreq{};      /* Interrupt request flag           */
    bool                    _vsync{};       /* VSYNC flag                       */
};

}
}
}
