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
#include "cart_simons_basic.hpp"

#include <sstream>

#include "logger.hpp"
#include "utils.hpp"

//#define CART_SIMONS_BASIC_DEBUG

#ifdef CART_SIMONS_BASIC_DEBUG
#define DEBUG(args...)      log.debug(args)
#else
#define DEBUG(args...)
#endif

namespace caio {
namespace commodore {
namespace c64 {

void CartSimonsBasic::reset()
{
    /*
     * Simons' Basic Cartridge.
     *
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          16K     1       0                           $8000-$9FFF (Module 1)
     *                                                      $A000-$BFFF (Module 2)
     *
     * Simons' BASIC permanently uses 16K ($4000) bytes of cartridge memory from $8000-$BFFF.
     * However, through some custom bank-switching logic the upper area ($A000-$BFFF) may be disabled so
     * Simons' BASIC may use it as additional RAM. Writing a value of $01 to address location $DE00 banks in ROM,
     * $00 disables ROM and enables RAM.
     *
     * The CRT file contains two CHIP blocks of length $2000 each, the first block having a start address of $8000,
     * the second block $A000. In the cartridge header, EXROM ($18) is set to 0, GAME ($19) is set to 1 to indicate
     * the RESET/power-up configuration of 8 KiB ROM.
     *
     * CRT "simonbas.crt", name "Simon's Basic", size 64, hwtype $0004, exrom 0, game 1,
     *     chip(size 8208, type $0000, bank $0000, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0000, bank $0000, addr $A000, rsiz 8192)
     *
     * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
     * @see https://vice-emu.sourceforge.io/vice_17.html#SEC399
     *
     * WARNING:
     *      The bank switching mechanism described above is not correct.
     *      Reading from $DE00 sets the 8K mode while writing to $DE00 sets the 16K mode.
     */
    Cartridge::reset();

    _roml = {};
    _romh = {};
    _reg  = 0;

    auto& cart = crt();
    for (size_t entry = 0; entry < cart.chips(); ++entry) {
        const auto chipdev = cart[entry];
        const Crt::Chip& chip = chipdev.first;
        const devptr_t& rom = chipdev.second;

        switch (chip.type) {
        case Crt::CHIP_TYPE_ROM:
        case Crt::CHIP_TYPE_FLASH:
        case Crt::CHIP_TYPE_EEPROM:
            if (chip.rsiz != ROM_SIZE) {
                throw_invalid_cartridge("Invalid ROM size " + std::to_string(chip.rsiz), entry);
            }

            if (chip.addr == ROML_LOAD_ADDR) {
                _roml = rom;
                DEBUG("%s(\"%s\"): Chip entry %d: ROML device, bank %d, load address $%04X, size %d\n", type().c_str(),
                    name().c_str(), entry, chip.bank, chip.addr, rom->size());
                break;
            }

            if (chip.addr == ROMH_LOAD_ADDR) {
                _romh = rom;
                DEBUG("%s(\"%s\"): Chip entry %d: ROMH device, bank %d, load address $%04X, size %d\n", type().c_str(),
                    name().c_str(), entry, chip.bank, chip.addr, rom->size());
                break;
            }

            throw_invalid_cartridge("Invalid chip ", entry);

        default:
            throw_invalid_cartridge("Unrecognised chip type " + std::to_string(chip.type), entry);
        }
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    propagate();
}

uint8_t CartSimonsBasic::read(addr_t addr, ReadMode rmode)
{
    if (addr == 0x0000 && rmode != ReadMode::Peek) {
        /*
         * Reading from $DE00 sets the 8K mode.
         */
        mode(MODE_8K);
    }

    return 0;
}

void CartSimonsBasic::write(addr_t addr, uint8_t data)
{
    if (addr == 0x0000) {
        /*
         * Writing to $DE00 sets the 16K mode.
         */
        mode(MODE_16K);
    }
}

std::string CartSimonsBasic::to_string() const
{
    std::ostringstream os{};

    os << Name::to_string()
       << ", roml " << (_roml ? _roml->to_string() : "")
       << ", romh " << (_romh ? _romh->to_string() : "");

    return os.str();
}

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartSimonsBasic::getdev(addr_t addr, bool romh, bool roml)
{
    if (roml) {
        /*
         * ROML mapped at $8000-$9FFF.
         */
        DEBUG("%s(\"%s\"): ROML for base addr $%04X, offset $%04X\n", type().c_str(), name().c_str(), addr,
            addr - ROML_LOAD_ADDR);
        return {{_roml, addr - ROML_LOAD_ADDR}, {}};
    }

    if (romh) {
        /*
         * ROMH mapped at $A000-$BFFF.
         */
        DEBUG("%s(\"%s\"): ROMH for base addr $%04X, offset $%04X\n", type().c_str(), name().c_str(), addr,
            addr - ROMH_LOAD_ADDR);
        return {{_romh, addr - ROMH_LOAD_ADDR}, {}};
    }

    return {{}, {}};
}

size_t CartSimonsBasic::cartsize() const
{
    return ((_roml ? _roml->size() : 0) + (_romh ? _romh->size() : 0));
}

}
}
}
