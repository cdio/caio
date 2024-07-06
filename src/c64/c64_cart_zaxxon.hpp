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

#include "rom.hpp"
#include "types.hpp"

#include "c64_cartridge.hpp"

namespace caio {
namespace commodore {
namespace c64 {

/**
 * Cartridge Zaxxon, Super Zaxxon (SEGA)
 *
 * Type     Size    Game    EXROM   LOAD ADDRESS
 * ----------------------------------------------------------------------
 *          20KiB   0       0       $8000-8FFF (Mirrored in $9000-$9FFF)
 *                                  $A000-BFFF
 *
 * The (Super) Zaxxon carts use a 4Kb ($1000) ROM at $8000-$8FFF (mirrored
 * in $9000-$9FFF) along with two 8Kb ($2000) cartridge banks  located  at
 * $A000-$BFFF. One of the two banks is selected by doing a read access to
 * either the $8000-$8FFF area (bank 0 is selected) or to $9000-$9FFF area
 * (bank 1 is selected). EXROM ($18 = $00) and GAME ($19 = $00) lines  are
 * always pulled to GND to select the 16 kB ROM configuration.
 *
 * The CRT file includes three CHIP blocks:
 * a) bank = 0, load address = $8000, size = $1000
 * b) bank = 0, load address = $A000, size = $2000
 * c) bank = 1, load address = $A000, size = $2000
 *
 * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 */
class CartZaxxon : public Cartridge {
public:
    constexpr static const char* TYPE              = "CART_ZAXXON";
    constexpr static const size_t ROML_SIZE        = 4096;
    constexpr static const size_t ROMH_SIZE        = 8192;
    constexpr static const addr_t ROML_BASE_ADDR   = 0x8000;
    constexpr static const addr_t ROMH_BASE_ADDR   = 0xA000;
    constexpr static const size_t ROMH_BANKS       = 2;
    constexpr static const size_t ROML_MASK        = 0x0FFF;
    constexpr static const size_t ROMH_MASK        = 0x1FFF;
    constexpr static const size_t TOTAL_ROMS       = 3;

    CartZaxxon(const sptr_t<Crt>& crt)
        : Cartridge{TYPE, crt} {
    }

    virtual ~CartZaxxon() {
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
    void roml_read_observer(addr_t addr, ReadMode mode);

    sptr_t<ROM>                         _roml{};
    size_t                              _bank{};
    std::array<sptr_t<ROM>, ROMH_BANKS> _romsh{};
};

}
}
}
