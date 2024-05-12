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
#include "c64_pla.hpp"

#include "logger.hpp"
#include "device_none.hpp"

namespace caio {
namespace commodore {
namespace c64 {

PLA::PLA(const devptr_t& ram, const devptr_t& basic, const devptr_t& kernal, const devptr_t& chargen,
    const devptr_t& io)
{
    /*
     * Fixed mappings to accelerate bank switching.
     * See https://www.c64-wiki.com/wiki/Bank_Switching.
     *
     * The 64K address space is subdivided into 32 modes implemented
     * as memory banks, each of these modes is subdivided into 16 4K blocks.
     */
    bank_t mode_00 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { ram,          0xA000 },
        { ram,          0xB000 },
        { ram,          0xC000 },
        { ram,          0xD000 },
        { ram,          0xE000 },
        { ram,          0xF000 }
    }};

    auto& mode_01 = mode_00;

    bank_t mode_02 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { device_none,  0x2000 },
        { device_none,  0x3000 },
        { ram,          0xC000 },
        { chargen,      0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    bank_t mode_03 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { device_none,  0x0000 },
        { device_none,  0x1000 },
        { device_none,  0x2000 },
        { device_none,  0x3000 },
        { ram,          0xC000 },
        { chargen,      0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    auto& mode_04 = mode_00;

    bank_t mode_05 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { ram,          0xA000 },
        { ram,          0xB000 },
        { ram,          0xC000 },
        { io,           0x0000 },
        { ram,          0xE000 },
        { ram,          0xF000 },
    }};

    bank_t mode_06 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { device_none,  0x2000 },
        { device_none,  0x3000 },
        { ram,          0xC000 },
        { io,           0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    bank_t mode_07 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { device_none,  0x0000 },
        { device_none,  0x1000 },
        { device_none,  0x2000 },
        { device_none,  0x3000 },
        { ram,          0xC000 },
        { io,           0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    auto& mode_08 = mode_00;

    bank_t mode_09 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { ram,          0xA000 },
        { ram,          0xB000 },
        { ram,          0xC000 },
        { chargen,      0x0000 },
        { ram,          0xE000 },
        { ram,          0xF000 },
    }};

    bank_t mode_10 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { ram,          0xA000 },
        { ram,          0xB000 },
        { ram,          0xC000 },
        { chargen,      0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    bank_t mode_11 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { device_none,  0x0000 },
        { device_none,  0x1000 },
        { basic,        0x0000 },
        { basic,        0x1000 },
        { ram,          0xC000 },
        { chargen,      0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    auto& mode_12 = mode_00;
    auto& mode_13 = mode_05;

    bank_t mode_14 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { ram,          0xA000 },
        { ram,          0xB000 },
        { ram,          0xC000 },
        { io,           0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    bank_t mode_15 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { device_none,  0x0000 },
        { device_none,  0x1000 },
        { basic,        0x0000 },
        { basic,        0x1000 },
        { ram,          0xC000 },
        { io,           0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    bank_t mode_16 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x1000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { io,           0x0000 },
        { device_none,  0x2000 },
        { device_none,  0x3000 },
    }};

    auto& mode_17 = mode_16;
    auto& mode_18 = mode_16;
    auto& mode_19 = mode_16;
    auto& mode_20 = mode_16;
    auto& mode_21 = mode_16;
    auto& mode_22 = mode_16;
    auto& mode_23 = mode_16;
    auto& mode_24 = mode_00;
    auto& mode_25 = mode_09;
    auto& mode_26 = mode_10;

    bank_t mode_27 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { basic,        0x0000 },
        { basic,        0x1000 },
        { ram,          0xC000 },
        { chargen,      0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    auto& mode_28 = mode_00;
    auto& mode_29 = mode_05;
    auto& mode_30 = mode_14;

    bank_t mode_31 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { ram,          0x2000 },
        { ram,          0x3000 },
        { ram,          0x4000 },
        { ram,          0x5000 },
        { ram,          0x6000 },
        { ram,          0x7000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { basic,        0x0000 },
        { basic,        0x1000 },
        { ram,          0xC000 },
        { io,           0x0000 },
        { kernal,       0x0000 },
        { kernal,       0x1000 },
    }};

    _rmodes = std::array<bank_t, BANKS>{
        mode_00, mode_01, mode_02, mode_03, mode_04, mode_05, mode_06, mode_07,
        mode_08, mode_09, mode_10, mode_11, mode_12, mode_13, mode_14, mode_15,
        mode_16, mode_17, mode_18, mode_19, mode_20, mode_21, mode_22, mode_23,
        mode_24, mode_25, mode_26, mode_27, mode_28, mode_29, mode_30, mode_31
    };

    auto& wmode_00 = mode_00;
    auto& wmode_01 = mode_00;
    auto& wmode_02 = mode_00;
    auto& wmode_03 = mode_00;
    auto& wmode_04 = mode_00;
    auto& wmode_05 = mode_13;
    auto& wmode_06 = mode_13;
    auto& wmode_07 = mode_13;
    auto& wmode_08 = mode_00;
    auto& wmode_09 = mode_00;
    auto& wmode_10 = mode_00;
    auto& wmode_11 = mode_00;
    auto& wmode_12 = mode_00;
    auto& wmode_13 = mode_13;
    auto& wmode_14 = mode_13;
    auto& wmode_15 = mode_13;

    bank_t wmode_16 = {{
        { ram,          0x0000 },
        { ram,          0x1000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { ram,          0x8000 },
        { ram,          0x9000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { device_none,  0x0000 },
        { io,           0x0000 },
        { ram,          0xE000 },
        { ram,          0xF000 },
    }};

    auto& wmode_17 = wmode_16;
    auto& wmode_18 = wmode_16;
    auto& wmode_19 = wmode_16;
    auto& wmode_20 = wmode_16;
    auto& wmode_21 = wmode_16;
    auto& wmode_22 = wmode_16;
    auto& wmode_23 = wmode_16;
    auto& wmode_24 = mode_00;
    auto& wmode_25 = mode_00;
    auto& wmode_26 = mode_00;
    auto& wmode_27 = mode_00;
    auto& wmode_28 = mode_00;
    auto& wmode_29 = mode_13;
    auto& wmode_30 = mode_13;
    auto& wmode_31 = mode_13;

    _wmodes = std::array<bank_t, BANKS>{
        wmode_00, wmode_01, wmode_02, wmode_03, wmode_04, wmode_05, wmode_06, wmode_07,
        wmode_08, wmode_09, wmode_10, wmode_11, wmode_12, wmode_13, wmode_14, wmode_15,
        wmode_16, wmode_17, wmode_18, wmode_19, wmode_20, wmode_21, wmode_22, wmode_23,
        wmode_24, wmode_25, wmode_26, wmode_27, wmode_28, wmode_29, wmode_30, wmode_31
    };

    /*
     * Default mappings.
     */
    PLA::reset();
}

void PLA::reset()
{
    /*
     * Default mappings.
     */
    _mode = LORAM | HIRAM | CHAREN | GAME | EXROM;
    remap();
}

void PLA::mode(uint8_t pins, uint8_t mask, bool force)
{
    mask &= MASK;
    uint8_t data = (_mode & ~mask) | (pins & mask);
    if (_mode != data || force) {
        _mode = data;
        remap();
    }
}

uint8_t PLA::mode() const
{
    return _mode;
}

void PLA::extmap(const extmap_cb_t& extmap)
{
    _extmap = extmap;
}

inline bool PLA::romh(addr_t addr) const
{
    /*
     * p21 <= n_hiram and a15 and not a14 and a13 and not n_aec and rd and not n_exrom and not n_game
     * p22 <= a15 and a14 and a13 and not n_aec and n_exrom and not n_game
     * See "The C64 PLA Dissected" from Thomas 'skoe' Giesel.
     */
    bool romh = ((_mode & (HIRAM | EXROM | GAME)) == HIRAM && (addr & (A15 | A14 | A13)) == (A15 | A13)) ||
        ((_mode & (EXROM | GAME)) == EXROM && (addr & (A15 | A14 | A13)) == (A15 | A14 | A13));

    return romh;
}

inline bool PLA::roml(addr_t addr) const
{
    /*
     * p19 <= n_loram and n_hiram and a15 and not a14 and not a13 and not n_aec and rd and not n_exrom
     * p20 <= a15 and not a14 and not a13 and not n_aec and n_exrom and not n_game
     * See "The C64 PLA Dissected" from Thomas 'skoe' Giesel.
     */
    bool roml = ((_mode & (LORAM | HIRAM | EXROM)) == (LORAM | HIRAM) && (addr & (A15 | A14 | A13)) == A15) ||
        ((_mode & (EXROM | GAME)) == EXROM && (addr & (A15 | A14 | A13)) == A15);

    return roml;
}

inline void PLA::remap()
{
    _rmaps = _rmodes[_mode];
    _wmaps = _wmodes[_mode];

    if (_extmap) {
        for (size_t bank = 0; bank < _rmaps.size(); ++bank) {
            addr_t addr = bank * 0x1000;
            bool rh = romh(addr);
            bool rl = roml(addr);
            auto [rdevmap, wdevmap] = _extmap(addr, rh, rl);

            if (rdevmap.first) {
#if 0
                log.debug("PLA: Mode: %d ($%02X), New read device for addr $%04X, offset $%04X, dev %s\n",
                    _mode,
                    _mode,
                    addr,
                    rdevmap.second,
                    rdevmap.first->to_string().c_str());
#endif

                _rmaps[bank] = rdevmap;
            } else {
//                log.debug("PLA: Mode: %d, addr $%04X not mapped for read\n", _mode, addr);
            }

            if (wdevmap.first) {
                log.debug("PLA: Mode: %d ($%02X), New write device for addr $%04X, offset $%04X, dev %s\n",
                    _mode,
                    _mode,
                    addr,
                    wdevmap.second,
                    wdevmap.first->to_string().c_str());

                _wmaps[bank] = wdevmap;
            } else {
//                log.debug("PLA: Mode: %d, addr $%04X not mapped for write\n", _mode, addr);
            }
        }
    }

#if 0
    log.debug("PLA: Mappings for mode: %d ($%02X)\n", _mode, _mode);
    for (size_t bank = 0; bank < rmaps.size(); ++bank) {
        auto &devmap = _rmaps[bank];
        log.debug("  bank %d, device %s, addr $%04X, offset $%04X\n", bank, devmap.first->to_string().c_str(),
            bank * 0x1000, devmap.second);
    }
#endif

    ASpace::reset(_rmaps, _wmaps, ADDR_MASK);
}

}
}
}
