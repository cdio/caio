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
#pragma once

#include <array>

#include "../c64_cartridge.hpp"


namespace cemu {
namespace c64 {

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
class CartC64GameSystem3 : public Cartridge {
public:
    constexpr static const char *TYPE           = "CART_C64_GAME_SYSTEM_3";
    constexpr static const size_t ROM_SIZE      = 8192;
    constexpr static const size_t MAX_BANKS     = 64;
    constexpr static const addr_t ROM_LOAD_ADDR = 0x8000;

    CartC64GameSystem3(const std::shared_ptr<Crt> &crt);

    virtual ~CartC64GameSystem3();

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Device::write();
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
