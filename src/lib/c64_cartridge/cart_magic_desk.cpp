/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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
#include "cart_magic_desk.hpp"

#include <sstream>

#include "logger.hpp"
#include "utils.hpp"

//#define CART_MAGIC_DESK

#ifdef CART_MAGIC_DESK
#define DEBUG(args...)      log.debug(args)
#else
#define DEBUG(args...)
#endif


namespace caio {
namespace c64 {

CartMagicDesk::CartMagicDesk(const std::shared_ptr<Crt> &crt)
    : Cartridge{TYPE, crt}
{
}

CartMagicDesk::~CartMagicDesk()
{
}

void CartMagicDesk::reset()
{
    /*
     * Magic Desk Cartridge.
     *
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          32K     1       0                           $8000-$9FFF (Banks 0-3)
     *          64K     1       0                           $8000-$9FFF (Banks 0-7)
     *          128K    1       0                           $8000-$9FFF (Banks 0-15)
     *
     * Startup mode 8K
     *
     * This cartridge type is very similar to the OCEAN cart type:
     * ROM memory is organized in 8KiB ($2000) banks located at $8000-$9FFF.
     * Bank switching is done by writing the bank number to $DE00.
     * Deviant from the Ocean type, bit 7 is cleared for selecting one of the ROM banks.
     * If bit 7 is set ($DE00 = $80), the GAME/EXROM lines are disabled,
     * turning on RAM at $8000-$9FFF instead of ROM.
     *
     * In the cartridge header, EXROM ($18) is set to 0, GAME ($19) is set to 1
     * to indicate the RESET/power-up configuration of 8 K ROM.
     *
     * Here is a list of the known cartridges:
     *  Ghosbusters        (HES Australia)   (32 KiB)
     *  Magic Desk         (Commodore)       (32 KiB)
     *  Badlands           (Domark)          (64 KiB)
     *  Vindicators        (Domark)          (64 KiB)
     *  Wonderboy          (HES Australia)   (64 KiB)
     *  Cyberball          (Domark)         (128 KiB)
     *
     * CRT "C64_2016_4K_Games_Collection.crt", name "C64 4 Kb Games", size 64, hwtype $0013, exrom 1, game 0,
     *     chip(size 8208, type $0002, bank $0000, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0001, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0002, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0003, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0004, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0005, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0006, addr $8000, rsiz 8192),
     *     chip(size 8208, type $0002, bank $0007, addr $8000, rsiz 8192)
     *
     * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
     * @see https://vice-emu.sourceforge.io/vice_toc.html#TOC414
     */
    //Cartridge::reset();   /* Invalid mode on the only magic desk cartridge I tested */
    _bank = 0;
    _banks = 0;
    _roms.fill(devptr_t{});

    const auto &cart = crt();
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

    switch (cartsize()) {
    case 0x08000:   /* 32K  */
    case 0x10000:   /* 64K  */
    case 0x20000:   /* 128K */
        break;
    default:
        throw_invalid_cartridge("Invalid cartridge size " + std::to_string(cartsize()) +
            ". Allowed sizes are 32K, 64K, or 128K, " + cart.to_string().c_str());
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    mode(GameExromMode::MODE_8K);   /* Invalid mode on the only magic desk cartridge I tested, force 8K mode */
}

uint8_t CartMagicDesk::read(addr_t addr) const
{
    addr &= IO_ADDR_MASK;
    if (addr == 0x0000) {
        /*
         * 32K, 64K, 128K sizes (4, 8, 16 banks of 8K each)
         * Bank switching is done by writing to $DE00.
         * Bit 7 is cleared for selecting one of the ROM banks.
         * If bit 7 is set ($DE00 = $80), the GAME/EXROM lines are disabled,
         * turning on RAM at $8000-$9FFF instead of ROM.
         */
        return (_bank | (mode() == GameExromMode::MODE_INVISIBLE ? 0x80 : 0x00));
    }

    return 255;
}

void CartMagicDesk::write(addr_t addr, uint8_t data)
{
    addr &= IO_ADDR_MASK;
    if (addr == 0x0000) {
        /*
         * 32K, 64K, 128K sizes (4, 8, 16 banks of 8K each)
         * Bank switching is done by writing to $DE00.
         * Bit 7 is cleared for selecting one of the ROM banks.
         * If bit 7 is set ($DE00 = $80), the GAME/EXROM lines are disabled,
         * turning on RAM at $8000-$9FFF instead of ROM.
         */
        if (data == 0x80) {
            DEBUG("%s(\"%s\"): Setting mode INVISIBLE\n", type().c_str(), name().c_str());
            mode(GameExromMode::MODE_INVISIBLE);
        } else {
            uint8_t bank = data & 0x0F;
            if (mode() == GameExromMode::MODE_INVISIBLE) {
                DEBUG("%s(\"%s\"): Setting mode 8K, bank %d\n", type().c_str(), name().c_str(), bank);
                _bank = bank;
                mode(GameExromMode::MODE_8K);
            } else if (bank != _bank) {
                DEBUG("%s(\"%s\"): Setting bank %d\n", type().c_str(), name().c_str(), bank);
                _bank = bank;
                propagate(true);
            }
        }
    }
}

std::string CartMagicDesk::to_string() const
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

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartMagicDesk::getdev(addr_t addr, bool romh, bool roml)
{
    /*
     * Type     Size    Game    EXROM   ROML        ROMH    LOAD ADDRESS
     * ----------------------------------------------------------------------------
     *          32K     1       0                           $8000-$9FFF (Banks 0-3)
     *          64K     1       0                           $8000-$9FFF (Banks 0-7)
     *          128K    1       0                           $8000-$9FFF (Banks 0-15)
     *
     * Startup mode 8K
     *
     * This cartridge type is very similar to the OCEAN cart type:
     * ROM memory is organized in 8KiB ($2000) banks located at $8000-$9FFF.
     * Bank switching is done by writing the bank number to $DE00.
     * Bit 7 is cleared for selecting one of the ROM banks.
     * If bit 7 is set ($DE00 = $80), the GAME/EXROM lines are disabled,
     * turning on RAM at $8000-$9FFF instead of ROM.
     */
    if (mode() == GameExromMode::MODE_8K) {
        DEBUG("%s(\"%s\"): Setting rom for addr $%04X, roml %d, romh %d\n", type().c_str(), name().c_str(), addr,
            roml, romh);
        if ((roml || romh)) {
            return {{_roms[_bank], addr - ROML_LOAD_ADDR}, {}};
        }
    }

    return {{}, {}};
}

size_t CartMagicDesk::cartsize() const
{
    return (_banks * ROM_SIZE);
}

}
}
