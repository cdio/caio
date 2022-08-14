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
#include "c64_vic2_aspace.hpp"


namespace caio {
namespace c64 {

Vic2ASpace::Vic2ASpace(const std::shared_ptr<Mos6526> &cia2, devptr_t &ram, devptr_t &chargen)
    : _cia2{cia2}
{
    /*
     * The 16K address space of a memory bank is subdivided into four 4K mappings.
     */
    addrmap_t bank_0 = {
        { ram,      0x0000 },
        { chargen,  0x0000 },
        { ram,      0x2000 },
        { ram,      0x3000 },
    };

    addrmap_t bank_1 = {
        { ram,      0x4000 },
        { ram,      0x5000 },
        { ram,      0x6000 },
        { ram,      0x7000 },
    };

    addrmap_t bank_2 = {
        { ram,      0x8000 },
        { chargen,  0x0000 },
        { ram,      0xA000 },
        { ram,      0xB000 },
    };

    addrmap_t bank_3 = {
        { ram,      0xC000 },
        { ram,      0xD000 },
        { ram,      0xE000 },
        { ram,      0xF000 },
    };

    _rbanks = {
        bank_3,
        bank_2,
        bank_1,
        bank_0
    };

    _wbanks = _rbanks;

    /*
     * Connect to Port pins PA0 and PA1 of CIA-2.
     */
    auto iow = [this](uint8_t addr, uint8_t value, bool) {
        if (addr == Mos6526::PRA) {
            bank(value);
        }
    };

    _cia2->add_iow(iow, Mos6526::P0 | Mos6526::P1);

    /*
     * Default bank.
     */
    bank(3);
}

}
}
