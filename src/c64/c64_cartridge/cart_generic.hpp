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

#include "../c64_cartridge.hpp"


namespace caio {
namespace commodore {
namespace c64 {

/**
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
 *
 * @see https://ist.uwaterloo.ca/~schepers/formats/CRT.TXT
 * @see https://vice-emu.sourceforge.io/vice_17.html#SEC395
 */
class CartGeneric : public Cartridge {
public:
    constexpr static const char* TYPE                = "CART_GENERIC";
    constexpr static const addr_t ROML_LOAD_ADDR     = 0x8000;
    constexpr static const addr_t ROMH_LOAD_ADDR     = 0xA000;
    constexpr static const addr_t U4_ROML_LOAD_ADDR  = 0xF000;
    constexpr static const addr_t U8_ROML_LOAD_ADDR  = 0xE000;
    constexpr static const addr_t U16_ROML_LOAD_ADDR = 0x8000;
    constexpr static const addr_t U16_ROMH_LOAD_ADDR = 0xE000;

    enum class GenericMode {
        NORMAL_8K,
        NORMAL_16K,
        ULTIMAX_4K,
        ULTIMAX_8K,
        ULTIMAX_16K,
        INVISIBLE
    };

    CartGeneric(const sptr_t<Crt>& crt)
        : Cartridge{TYPE, crt} {
    }

    virtual ~CartGeneric() {
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
    GenericMode _generic_mode{GenericMode::INVISIBLE};
    devptr_t    _roml{};
    devptr_t    _romh{};
    addr_t      _romh_offset{};
};

}
}
}
