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
namespace c64 {

/**
 * Ocean type 1 Cartridge.
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
 *  Batman The Movie    (128K)
 *  Battle Command      (128K)
 *  Double Dragon       (128K)
 *  Navy Seals          (128K)
 *  Pang                (128K)
 *  Robocop 3           (128K)
 *  Space Gun           (128K)
 *  Toki                (128K)
 *  Chase H.Q. II       (256K)
 *  Robocop 2           (256K)
 *  Shadow of the Beast (256K)
 *  Terminator 2        (512K)
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
class CartOceanType1 : public Cartridge {
public:
    constexpr static const char* TYPE              = "CART_OCEAN_TYPE_1";
    constexpr static const size_t ROM_SIZE         = 8192;
    constexpr static const size_t MAX_BANKS        = 64;
    constexpr static const addr_t ROML_LOAD_ADDR   = 0x8000;
    constexpr static const addr_t ROMH_LOAD_ADDR   = 0xA000;

    CartOceanType1(const sptr_t<Crt>&crt)
        : Cartridge{TYPE, crt} {
    }

    virtual ~CartOceanType1() {
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
