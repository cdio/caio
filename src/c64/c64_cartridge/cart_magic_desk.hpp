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

#include "../c64_cartridge.hpp"


namespace caio {
namespace commodore {
namespace c64 {

/**
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
class CartMagicDesk : public Cartridge {
public:
    constexpr static const char* TYPE            = "CART_MAGIC_DESK";
    constexpr static const size_t ROM_SIZE       = 8192;
    constexpr static const size_t MAX_BANKS      = 15;
    constexpr static const addr_t ROML_LOAD_ADDR = 0x8000;

    CartMagicDesk(const sptr_t<Crt>& crt)
        : Cartridge{TYPE, crt} {
    }

    virtual ~CartMagicDesk() {
    }

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::to_string()
     */
    std::string to_string() const override;

    /**
     * @see Cartridge::getdev()
     */
    std::pair<ASpace::devmap_t, ASpace::devmap_t> getdev(addr_t addr, bool romh, bool roml) override;

    /**
     * @see Cartridge::cartsize()
     */
    size_t cartsize() const override;

    /**
     * @see Cartridge::reset()
     */
    void reset() override;

private:
    uint8_t                         _bank{};
    size_t                          _banks{};
    std::array<devptr_t, MAX_BANKS> _roms{};
};

}
}
}
