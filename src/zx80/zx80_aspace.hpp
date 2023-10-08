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

#include <memory>

#include "aspace.hpp"
#include "device.hpp"
#include "zilog_z80.hpp"

#include "zx80_video.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

constexpr static const size_t ROM4_SIZE         = 4096;
constexpr static const size_t ROM8_SIZE         = 8192;
constexpr static const size_t INTERNAL_RAM_SIZE = 1024;
constexpr static const size_t EXTERNAL_RAM_SIZE = 16384;

/**
 * The ZX80 Address Space implements the memory mappings seen by the cpu on a ZX80.
 * @see ASpace
 */
class ZX80ASpace : public ASpace {
public:
    constexpr static const addr_t ADDR_MASK     = 0x7FFF;
    constexpr static const addr_t VIDEO_ADDRESS = (1 << 15);
    constexpr static const addr_t A7            = (1 << 7);
    constexpr static const addr_t A6            = (1 << 6);
    constexpr static const uint8_t VDATA_MASK   = 0x3F;
    constexpr static const uint8_t VIDEO_INVERT = (1 << 7);
    constexpr static const uint8_t VIDEO_NOP    = (1 << 6);
    constexpr static const size_t BLOCKS        = 64;

    using bank_t = std::array<devmap_t, BLOCKS>;

    /**
     * Initialise this ZX80 Address Space.
     * @param cpu   CPU;
     * @param ram   Internal RAM (1K or 16K);
     * @param rom   Internal ROM (4K or 8K);
     * @param video Video interface.
     * @note RAM is expected to be 1K or 16K.
     * @note ROM is expected to be 4K or 8K.
     */
    ZX80ASpace(const sptr_t<Z80>& cpu, const devptr_t& ram, const devptr_t& rom, const sptr_t<ZX80Video>& video);

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
    sptr_t<Z80>       _cpu;
    sptr_t<ZX80Video> _video;
    bank_t            _mmap;
    bool              _int{};
};

}
}
}
