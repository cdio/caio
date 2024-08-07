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

#include <array>

#include "aspace.hpp"
#include "device.hpp"
#include "mos_6526.hpp"

namespace caio {
namespace commodore {
namespace c64 {

/**
 * The VIC2 Address Space implements the memory mappings seen by the MOS6569 on a C64.
 * Since the MOS6569 controller has an address space of 16K, the memory of the C64 is divided
 * into 4 banks of 16K each. The video controller can see only one of these banks at a time.
 * The selection of the bank is defined by ports PA0 and PA1 of the CIA-2 device.
 * @see ASpace
 * @see Mos6526
 */
class Vic2ASpace : public ASpace {
public:
    constexpr static const size_t BANKS     = 4;
    constexpr static const size_t BLOCKS    = 4;
    constexpr static const addr_t ADDR_MASK = 0x3FFF;

    using bank_t = ASpace::bank_t<BLOCKS>;

    /**
     * Initialise this VIC2 Address Space.
     * @param cia2    CIA#2 Device;
     * @param ram     RAM (64K);
     * @param chargen Chargen ROM (4K).
     */
    Vic2ASpace(const sptr_t<Mos6526>& cia2, const devptr_t& ram, const devptr_t& chargen);

    virtual ~Vic2ASpace() {
    }

private:
    /**
     * Set the memory bank seen by the video controller.
     */
    void bank(size_t bank);

    sptr_t<Mos6526>           _cia2{};
    size_t                    _bank{};
    std::array<bank_t, BANKS> _rbanks{};
    std::array<bank_t, BANKS> _wbanks{};
};

}
}
}
