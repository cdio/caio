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
#include "cart_c64_game_system_3.hpp"

#include <sstream>

#include "logger.hpp"
#include "utils.hpp"

//#define CART_C64_GAME_SYSTEM_3

#ifdef CART_C64_GAME_SYSTEM_3
#define DEBUG(args...)      log.debug(args)
#else
#define DEBUG(args...)
#endif


namespace caio {
namespace c64 {

void CartC64GameSystem3::reset()
{
    /**
     * C64 Game System, System 3 Cartridge.
     *
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          512K    1       0                           $8000-$9FFF (Banks 0-3)
     *
     * 64 banks of 8K each.
     *
     * List of the known cartridges:
     *  C64GS 4-in-1      (Commodore)  (512 KiB)
     *  Last Ninja Remix  (System 3)   (512 KiB)
     *  Myth              (System 3)   (512 KiB)
     *
     * ROM memory is organized in 8K banks located at $8000-$9FFF.
     * Bank switching is done by writing to address $DE00+X, where X is the bank number (STA $DE00,X).
     * For instance, to read from bank 3, address $DE03 is written to.
     * Reading from anywhere in the I/O-1 range will disable the cart.
     *
     * The CRT file contains a string of CHIP blocks, each block with a start address of $8000,
     * length $2000 and the bank number in the bank field. In the cartridge header,
     * EXROM ($18) is set to 0, GAME ($19) is set to 1 to enable the 8 K ROM configuration.
     *
     * @see https://vice-emu.sourceforge.io/vice_17.html#SEC400
     * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
     */
    Cartridge::reset();

    const auto &cart = crt();

    if (mode() != GameExromMode::MODE_8K) {
        throw_invalid_cartridge("Invalid GAME/EXROM mode $" + utils::to_string(static_cast<uint16_t>(mode())) +
            ", " + cart.to_string().c_str());
    }

    _bank = 0;
    _banks = 0;
    _roms.fill(devptr_t{});

    for (size_t entry = 0; entry < cart.chips(); ++entry) {
        const auto chipdev = cart[entry];
        const Crt::Chip &chip = chipdev.first;
        const devptr_t &rom = chipdev.second;

        switch (chip.type) {
        case Crt::CHIP_TYPE_ROM:
        case Crt::CHIP_TYPE_FLASH:
        case Crt::CHIP_TYPE_EEPROM:
            if (chip.rsiz != ROM_SIZE) {
                throw_invalid_cartridge("Invalid ROM size " + std::to_string(chip.rsiz), entry);
            }

            _roms[_banks] = rom;
            ++_banks;

            if (_banks > MAX_BANKS) {
                throw_invalid_cartridge("Max number of banks reached %d\n", _banks);
            }

            DEBUG("%s(\"%s\"): Chip entry %d: ROM device, bank %d, load address $%04X, size %d\n", type().c_str(),
                name().c_str(), entry, chip.bank, chip.addr, rom->size());

            break;

        default:
            throw_invalid_cartridge("Unrecognised chip type " + std::to_string(chip.type), entry);
        }
    }

    if (cartsize() != 0x80000 /* 512K */) {
        throw_invalid_cartridge("Invalid cartridge size " + std::to_string(cartsize()) +
            ". it must be 512K, " + cart.to_string().c_str());
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    propagate();
}

uint8_t CartC64GameSystem3::read(addr_t addr, ReadMode rmode)
{
    addr &= IO_ADDR_MASK;
    if (addr < 255 && rmode != ReadMode::Peek) {
        /*
         * Reading from anywhere in the I/O-1 range will disable the cart.
         */
        mode(GameExromMode::MODE_INVISIBLE);
    }

    return 255;
}

void CartC64GameSystem3::write(addr_t addr, uint8_t data)
{
    addr &= IO_ADDR_MASK;
    if (addr < 64) {
        /*
         * Bank switching is done by writing to address $DE00+X,
         * where X is the bank number (STA $DE00,X).
         * For instance, to read from bank 3, address $DE03 is written to.
         */
        _bank = addr;
        if (mode() == GameExromMode::MODE_8K) {
            propagate(true);
        } else {
            mode(GameExromMode::MODE_8K);
        }
    }
}

std::string CartC64GameSystem3::to_string() const
{
    std::ostringstream os{};

    os << Name::to_string() << ", banks " << _banks << ", size " << cartsize() << " (" << (cartsize() >> 10) << "K)";

    for (const auto &rom : _roms) {
        if (rom) {
            os << ", " << rom->to_string();
        }
    }

    return os.str();
}

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartC64GameSystem3::getdev(addr_t addr, bool romh, bool roml)
{
    /*
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          512K    0       0                           $8000-$9FFF (Banks 0-63)
     * 64 banks of 8K each.
     */
    if (roml) {
        return {{_roms[_bank], addr - ROM_LOAD_ADDR}, {}};
    }

    return {{}, {}};
}

size_t CartC64GameSystem3::cartsize() const
{
    return (_banks * ROM_SIZE);
}

}
}
