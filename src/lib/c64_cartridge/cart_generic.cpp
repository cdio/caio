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
#include "cart_generic.hpp"

#include "device_rom.hpp"


namespace cemu {
namespace c64 {

/*
 * Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
 * -------------------------------------------------------------
 * Normal   8K      1       0       $8000   -----   $8000-$9FFF
 * Normal   16K     0       0       $8000   $A000   $8000-$BFFF
 * Ultimax  8K      0       1       -----   $E000   $E000-$FFFF
 * Ultimax  16K     0       1       $8000   $E000   $8000-$9FFF and $E000-$FFFF
 * Ultimax  4K      0       1       $F000   -----   $F000-$F7FF
 */
CartGeneric::CartGeneric(const std::shared_ptr<Crt> &crt)
    : Cartridge{TYPE, crt}
{
    _game_exrom = (crt->game() ? GAME : 0) | (crt->exrom() ? EXROM : 0);

    switch (_game_exrom) {
    case GAME_EXROM_00:
        /*
         * Normal 16K.
         */
        _rom = (*_crt)[0].second;
        _mode = Mode::NORMAL_16K;
        break;

    case GAME_EXROM_10:
        /*
         * Normal 8K.
         */
        _rom = (*_crt)[0].second;
        _mode = Mode::NORMAL_8K;
        break;

    case GAME_EXROM_01:
        /*
         * Ultimax.
         */
        _rom = (*_crt)[0].second;
        switch (_rom->size()) {
        case 0x1000:
            _mode = Mode::ULTIMAX_4K;
            break;
        case 0x2000:
            _mode = Mode::ULTIMAX_8K;
            break;
        case 0x4000:
            _mode = Mode::ULTIMAX_16K;
            break;
        default:
            throw InvalidCartridge{type(), "Invalid ROM size for Ultimax mode: " + _crt->to_string()};
        }
        break;

    case GAME_EXROM_11:
        throw InvalidCartridge{type(), "EXROM/GAME not supported: " + _crt->to_string()};
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    reset();
}

CartGeneric::~CartGeneric()
{
}

uint8_t CartGeneric::read(addr_t addr) const
{
    /* Addresses from $DE00 to DFFF */
    return 255;
}

void CartGeneric::write(addr_t addr, uint8_t data)
{
    /* Addresses from $DE00 to DFFF */
}

std::string CartGeneric::to_string() const
{
    const char *modestr;

    switch (_mode) {
    case Mode::NORMAL_8K:
        modestr = "Normal 8K";
        break;
    case Mode::NORMAL_16K:
        modestr = "Normal 16K";
        break;
    case Mode::ULTIMAX_4K:
        modestr = "Ultimax 4K";
        break;
    case Mode::ULTIMAX_8K:
        modestr = "Ultimax 8K";
        break;
    case Mode::ULTIMAX_16K:
        modestr = "Ultimax 16K";
        break;
    case Mode::NONE:;
        modestr =  "UNK";
    }

    std::ostringstream os{};
    os << Name::to_string() << ", " << modestr << ", rom size " << (_rom ? _rom->size() : 0);
    return os.str();
}

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartGeneric::getdev(addr_t addr, bool romh, bool roml)
{
    /*
     * Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
     * -------------------------------------------------------------
     * Normal   8KiB    1       0       $8000   ----    $8000-$9FFF
     * Normal   16KiB   0       0       $8000   $A000   $8000-$BFFF
     * Ultimax  8KiB    0       1       $E000   ----    $E000-$FFFF
     * Ultimax  16KiB   0       1       $E000   ----    $8000-$9FFF and $E000-$FFFF
     * Ultimax  4KiB    0       1       $F000   ----    $F000-$F7FF
     */
    if (_rom) {
        switch (_mode) {
        case Mode::NORMAL_8K:
            if (addr == 0x8000 || addr == 0x9000) {
                return {{_rom, addr - 0x8000}, {}};
            }
            break;

        case Mode::NORMAL_16K:
            if (addr == 0x8000 || addr == 0x9000) {
                return {{_rom, addr - 0x8000}, {}};
            }
            if (addr == 0xA000 || addr == 0xB000) {
                return {{_rom, addr - 0x8000}, {}};
            }
            break;

        case Mode::ULTIMAX_4K:
            if (addr == 0xF000) {
                return {{_rom, 0x0000}, {}};
            }
            break;

        case Mode::ULTIMAX_8K:
            if (addr == 0xE000 || addr == 0xF000) {
                return {{_rom, addr - 0xE000}, {}};
            }
            break;

        case Mode::ULTIMAX_16K:
            if (addr == 0x8000 || addr == 0x9000) {
                return {{_rom, addr - 0x8000}, {}};
            }
            if (addr == 0xE000 || addr == 0xF000) {
                return {{_rom, (addr - 0xE000) + 0x2000}, {}};
            }
            break;

        default:;
        }
    }

    return {{}, {}};
}

size_t CartGeneric::cartsize() const
{
    return _rom->size();
}

void CartGeneric::reset()
{
    _ioport.iow(0, _game_exrom);
}

}
}
