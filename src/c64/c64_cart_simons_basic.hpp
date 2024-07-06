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

#include "c64_cartridge.hpp"

namespace caio {
namespace commodore {
namespace c64 {

/**
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
 * the RESET/power-up configuration of 8K ROM.
 *
 * CRT "simonbas.crt", name "Simon's Basic", size 64, hwtype $0004, exrom 0, game 1,
 *     chip(size 8208, type $0000, bank $0000, addr $8000, rsiz 8192),
 *     chip(size 8208, type $0000, bank $0000, addr $A000, rsiz 8192)
 *
 * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 * @see https://vice-emu.sourceforge.io/vice_16.html#SEC435
 */
class CartSimonsBasic : public Cartridge {
public:
    constexpr static const char* TYPE              = "CART_SIMONS_BASIC";
    constexpr static const size_t ROM_SIZE         = 8192;
    constexpr static const addr_t ROML_LOAD_ADDR   = 0x8000;
    constexpr static const addr_t ROMH_LOAD_ADDR   = 0xA000;

    CartSimonsBasic(const sptr_t<Crt>& crt)
        : Cartridge{TYPE, crt} {
    }

    virtual ~CartSimonsBasic() {
    }

    /**
     * @see Device::dev_read()
     */
    uint8_t dev_read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write()
     */
    void dev_write(addr_t addr, uint8_t data) override;

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
    devptr_t _roml{};
    devptr_t _romh{};
    uint8_t  _reg{};
};

}
}
}
