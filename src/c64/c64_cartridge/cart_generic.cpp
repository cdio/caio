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
#include "cart_generic.hpp"

#include <sstream>

#include "logger.hpp"

//#define CART_GENERIC_DEBUG

#ifdef CART_GENERIC_DEBUG
#define DEBUG(args...)      log.debug(args)
#else
#define DEBUG(args...)
#endif

namespace caio {
namespace commodore {
namespace c64 {

void CartGeneric::reset()
{
    Cartridge::reset();

    /*
     * Generic Cartridge.
     * Generic cartridges can contain one or two 8K CHIP packets, one for ROML and one for ROMH.
     * ROML start address is always $8000, ROMH start address is either $A000 or $E000 depending
     * on the GAME/EXROM config.
     *
     * Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     * Normal   8K      1       0       $8000   -----   $8000-$9FFF
     * Normal   16K     0       0       $8000   $A000   $8000-$BFFF
     * Ultimax  4K      0       1       $F000   -----   $F000-$F7FF
     * Ultimax  8K      0       1       -----   $E000   $E000-$FFFF
     * Ultimax  16K     0       1       $8000   $E000   $8000-$9FFF and $E000-$FFFF
     *
     * The ROMH and ROML lines are CPU-controlled status lines, used to bank in/out RAM, ROM or I/O,
     * depending on what is needed at the time.
     */
    _roml= {};
    _romh = {};
    _romh_offset = 0;
    _generic_mode = GenericMode::INVISIBLE;

    const auto &cart = crt();

    switch (mode()) {
    case MODE_16K:
        _roml = cart[0].second;
        _romh = _roml;
        _romh_offset = 0x2000;
        _generic_mode = GenericMode::NORMAL_16K;
        break;

    case MODE_8K:
        _roml = cart[0].second;
        _generic_mode = GenericMode::NORMAL_8K;
        break;

    case MODE_ULTIMAX:
        _roml = cart[0].second;
        switch (_roml->size()) {
        case 0x1000:
            _generic_mode = GenericMode::ULTIMAX_4K;
            break;
        case 0x2000:
            _generic_mode = GenericMode::ULTIMAX_8K;
            break;
        case 0x4000:
            _generic_mode = GenericMode::ULTIMAX_16K;
            break;
        default:
            throw InvalidCartridge{type(), "Invalid ROM size for Ultimax mode: " + cart.to_string()};
        }
        break;

    case MODE_INVISIBLE:
        log.warn("%s(\"%s\"): Cartridge in invisible mode\n", type().c_str(), name().c_str());
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    propagate();
}

uint8_t CartGeneric::read(addr_t addr, ReadMode)
{
    /* Addresses from $DE00 to $DFFF */
    return 255;
}

void CartGeneric::write(addr_t addr, uint8_t data)
{
    /* Addresses from $DE00 to $DFFF */
}

std::string CartGeneric::to_string() const
{
    const char* modestr{};

    switch (_generic_mode) {
    case GenericMode::NORMAL_8K:
        modestr = "Normal 8K";
        break;
    case GenericMode::NORMAL_16K:
        modestr = "Normal 16K";
        break;
    case GenericMode::ULTIMAX_4K:
        modestr = "Ultimax 4K";
        break;
    case GenericMode::ULTIMAX_8K:
        modestr = "Ultimax 8K";
        break;
    case GenericMode::ULTIMAX_16K:
        modestr = "Ultimax 16K";
        break;
    case GenericMode::INVISIBLE:
        modestr = "INVISIBLE";
    }

    std::ostringstream os{};
    os << Name::to_string() << ", " << modestr << ", " << _roml->to_string();
    if (_romh && _romh != _roml) {
        os << ", " << _romh->to_string();
    }

    return os.str();
}

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartGeneric::getdev(addr_t addr, bool romh, bool roml)
{
    /*
     * Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     * Normal   8KiB    1       0       $8000   ----    $8000-$9FFF
     * Normal   16KiB   0       0       $8000   $A000   $8000-$BFFF
     * Ultimax  4KiB    0       1       $F000   ----    $F000-$F7FF
     * Ultimax  8KiB    0       1       $E000   ----    $E000-$FFFF
     * Ultimax  16KiB   0       1       $E000   ----    $8000-$9FFF and $E000-$FFFF
     */
    switch (mode()) {
    case GameExromMode::MODE_8K:
        if (roml) {
            return {{_roml, addr - ROML_LOAD_ADDR}, {}};
        }
        break;

    case GameExromMode::MODE_16K:
        if (roml) {
            return {{_roml, addr - ROML_LOAD_ADDR}, {}};
        }
        if (romh) {
            return {{_romh, addr - ROMH_LOAD_ADDR + _romh_offset}, {}};
        }
        break;

    case GameExromMode::MODE_ULTIMAX:
        switch (_generic_mode) {
        case GenericMode::ULTIMAX_16K:
            if (roml) {
                return {{_roml, addr - U16_ROML_LOAD_ADDR}, {}};
            }
            if (romh) {
                return {{_romh, addr - U16_ROMH_LOAD_ADDR + _romh_offset}, {}};
            }
            break;

        case GenericMode::ULTIMAX_8K:
            if (roml) {
                return {{_roml, addr - U8_ROML_LOAD_ADDR}, {}};
            }
            break;

        case GenericMode::ULTIMAX_4K:
            if (roml) {
                return {{_roml, addr - U4_ROML_LOAD_ADDR}, {}};
            }
            break;

        default:;
        }
        break;

    case GameExromMode::MODE_INVISIBLE:
        break;
    }

    return {{}, {}};
}

size_t CartGeneric::cartsize() const
{
    return (_roml->size() + ((_romh && _romh != _roml) ? _romh->size() : 0));
}

}
}
}
