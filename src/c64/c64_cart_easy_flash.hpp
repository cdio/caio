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

#include "ram.hpp"
#include "rom.hpp"

#include "c64_cartridge.hpp"

#include <array>

namespace caio {
namespace commodore {
namespace c64 {

/**
 * EasyFlash Cartridge.
 *
 * ### Memory mappings:
 *
 * 1M divided in 64 banks of 2 * 8K each.
 *
 *     Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
 *     ----------------------------------------------------------------------------
 *              1024K   0       1       $8000   $A000   $8000-$9FFF and $A000-$BFFF
 *                                      $8000   $E000   $8000-$9FFF and $E000-$FFFF
 *
 * EasyFlash is a 1M or 2x512K flash memory (one called ROML and the other ROMH)
 * plus 256 bytes of RAM (mapped into the I/O-2 range).
 *
 * - Control register 1 ($DE00): Bank switching.
 *
 * - Control register 2 ($DE02): EasyFlash control:
 *
 *     Bit     Name    Content
 *     -------------------------------------------------------------------------
 *     7       L       LED (1: LED on, 0: LED off)
 *     6..3    0       Reserved (must be 0)
 *     2       M       GAME mode (1: Controlled by bit G, 0: From jumper "boot")
 *     1       X       EXROM state (0: /EXROM high)
 *     0       G       GAME state (if M = 1, 0 = /GAME high)
 *
 * @see https://skoe.de/easyflash/files/devdocs/EasyFlash-ProgRef.pdf
 */
class CartEasyFlash : public Cartridge {
public:
    constexpr static const char* TYPE              = "CART_EASY_FLASH";
    constexpr static const size_t MAX_BANKS        = 64;
    constexpr static const size_t BANK_MASK        = 63;
    constexpr static const size_t ROM_SIZE         = 8192;
    constexpr static const addr_t ROML_LOAD_ADDR   = 0x8000;
    constexpr static const addr_t ROMH_LOAD_ADDR_1 = 0xA000;
    constexpr static const addr_t ROMH_LOAD_ADDR_2 = 0xE000;
    constexpr static const uint8_t REG2_MODE       = 0x04;
    constexpr static const uint8_t REG2_EXROM      = 0x02;
    constexpr static const uint8_t REG2_GAME       = 0x01;

    CartEasyFlash(const sptr_t<Crt>& crt);

    virtual ~CartEasyFlash() = default;

    /**
     * @see Device::dev_read()
     */
    uint8_t dev_read(size_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write()
     */
    void dev_write(size_t addr, uint8_t data) override;

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
    using ROM_Array = std::array<sptr_t<ROM>, MAX_BANKS>;

    void add_rom(size_t entry, const Crt::Chip& chip, const sptr_t<ROM>& rom);
    void add_ram(size_t entry, const Crt::Chip& chip, const sptr_t<RAM>& ram);

    size_t      _bank{};        /* Current ROM bank             */
    uint8_t     _reg2{};        /* Control register at $DE02    */
    size_t      _romls{};       /* Number of ROMLs              */
    size_t      _romhs{};       /* Number of ROMHs              */
    sptr_t<RAM> _ram{};         /* 256 bytes RAM (if present)   */
    ROM_Array   _roms_lo{};     /* ROMLs                        */
    ROM_Array   _roms_hi{};     /* ROMHs                        */

    friend Serializer& operator&(Serializer&, CartEasyFlash&);
};

}
}
}
