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
#include "cart_ocean_type_1.hpp"

#include <sstream>

#include "logger.hpp"
#include "utils.hpp"

//#define CART_OCEAN_TYPE_1_DEBUG

#ifdef CART_OCEAN_TYPE_1_DEBUG
#define DEBUG(args...)      log.debug(args)
#else
#define DEBUG(args...)
#endif

namespace caio {
namespace commodore {
namespace c64 {

void CartOceanType1::reset()
{
    /*
     * Ocean Type 1 Cartridge.
     *
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          32K     0       0                           $8000-$9FFF (Banks 0-3)
     *
     *          128K    0       0                           $8000-$9FFF (Banks 0-15)
     *
     *          256K    0       0                           $8000-$9FFF (Banks 0-15)
     *                                                      $A000-$BFFF (Banks 16-31)
     *
     *          512K    0       0                           $8000-$9FFF (Banks 0-63)    TERMINATOR 2
     *
     * 32K, 128K, 256K or 512K sizes (4, 16, 32 or 64 banks of 8K each)
     *
     * Bank switching is done by writing to $DE00.
     * The lower six bits give the bank number (ranging from 0-63), bit 7 is always set.
     *
     * Some known OCEAN cartridges:
     *  Batman The Movie    (128 KiB)
     *  Battle Command      (128 KiB)
     *  Double Dragon       (128 KiB)
     *  Navy Seals          (128 KiB)
     *  Pang                (128 KiB)
     *  Robocop 3           (128 KiB)
     *  Space Gun           (128 KiB)
     *  Toki                (128 KiB)
     *  Chase H.Q. II       (256 KiB)
     *  Robocop 2           (256 KiB)
     *  Shadow of the Beast (256 KiB)
     *  Terminator 2        (512 KiB)
     *
     * CRT "kung_fu_master.crt", name "kungfu", size 64, hwtype $0005, exrom 0, game 1,
     *     chip(size 8208, type $0000, bank $0000, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0000, bank $0001, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0000, bank $0002, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0000, bank $0003, addr $8000, rsiz 8192)
     *
     * @see https://vice-emu.sourceforge.io/vice_17.html#SEC400
     * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
     */
    Cartridge::reset();

    _bank = 0;
    _banks = 0;
    _roms.fill(devptr_t{});

    const auto& cart = crt();
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

            _roms[_banks] = rom;
            ++_banks;

            if (_banks > MAX_BANKS) {
                throw_invalid_cartridge("Max number of banks reached " + std::to_string(_banks));
            }

            DEBUG("%s(\"%s\"): Chip entry %d: ROM device, bank %d, load address $%04X, size %d\n", type().c_str(),
                name().c_str(), entry, chip.bank, chip.addr, rom->size());

            break;

        default:
            throw_invalid_cartridge("Unrecognised chip type " + std::to_string(chip.type), entry);
        }
    }

    switch (cartsize()) {
    case 0x08000:   /* 32K  */
    case 0x20000:   /* 128K */
    case 0x40000:   /* 256K */
    case 0x80000:   /* 512K */
        break;
    default:
        throw_invalid_cartridge("Invalid cartridge size " + std::to_string(cartsize()) +
            ". Allowed sizes are 32K, 128K, 256K, or 512K, " + cart.to_string());
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    propagate();
}

uint8_t CartOceanType1::read(addr_t addr, ReadMode)
{
    if (addr == 0x0000) {
        /*
         * 32K, 128K, 256K or 512K sizes (4, 16, 32 or 64 banks of 8K each)
         * Bank switching is done by writing to $DE00.
         * The lower six bits give the bank number (ranging from 0-63), bit 7 is always set.
         */
        return (_bank | 0x80);
    }

    return 255;
}

void CartOceanType1::write(addr_t addr, uint8_t data)
{
    addr &= IO_ADDR_MASK;
    if (addr == 0x0000) {
        /*
         * 32K, 128K, 256K or 512K sizes (4, 16, 32 or 64 banks of 8K each)
         * Bank switching is done by writing to $DE00.
         * The lower six bits give the bank number (ranging from 0-63), bit 7 is always set.
         */
        uint8_t bank = data & 0x3F;
        if (bank != _bank) {
            _bank = bank;

            /*
             * Force the propagation of GAME/EXROM output pins.
             * This makes the connected devices to update their internal status
             * even if the GAME/EXROM pins are not changed (like in this case).
             */
            propagate(true);
        }
    }
}

std::string CartOceanType1::to_string() const
{
    std::ostringstream os{};

    os << Name::to_string() << ", banks " << _banks << ", size " << cartsize() << " (" << (cartsize() >> 10) << "K)";

    for (const auto& rom : _roms) {
        if (rom) {
            os << ", " << rom->to_string();
        }
    }

    return os.str();
}

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartOceanType1::getdev(addr_t addr, bool romh, bool roml)
{
    /*
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          32K     0       0                           $8000-$9FFF (Banks 0-3)
     *
     *          128K    0       0                           $8000-$9FFF (Banks 0-15)
     *
     *          256K    0       0                           $8000-$9FFF (Banks 0-15)
     *                                                      $A000-$BFFF (Banks 16-31)
     *
     *          512K    0       0                           $8000-$9FFF (Banks 0-63)    TERMINATOR 2
     *
     * 32K, 128K, 256K or 512K sizes (4, 16, 32 or 64 banks of 8K each)
     *
     * Bank switching is done by writing to $DE00.
     * The lower six bits give the bank number (ranging from 0-63), bit 7 is always set.
     */
    switch (cartsize()) {
    case 0x08000:   /* 32K  */
    case 0x20000:   /* 128K */
    case 0x80000:   /* 512K */
        if (roml) {
            return {{_roms[_bank], addr - ROML_LOAD_ADDR}, {}};
        }
        break;

    case 0x40000:   /* 256K */
        if (roml && _bank < 16) {
            return {{_roms[_bank], addr - ROML_LOAD_ADDR}, {}};
        }
        if (romh && _bank > 15) {
            return {{_roms[_bank], addr - ROMH_LOAD_ADDR}, {}};
        }
        break;
    }

    return {{}, {}};
}

size_t CartOceanType1::cartsize() const
{
    return (_banks * ROM_SIZE);
}

}
}
}
