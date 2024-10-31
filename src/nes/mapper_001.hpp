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

#include "nes_cartridge.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * Cartridge Mapper 001 (MMC1).
 *
 * Address range accessed by the CPU:
 *
 *   Mapper Address     CPU Address     Descripton
 *   --------------------------------------------------------------------------------------------------
 *   2000-3FFF          6000-7FFF       8K PRG RAM (optional)
 *   4000-7FFF          8000-BFFF       16K PRG ROM (fixed to the first bank or switchable)
 *   8000-BFFF          C000-FFFF       16K PRG ROM (fixed to the last bank or switchable)
 *
 * Address range accessed by the PPU:
 *
 *   Mapper Address     PPU Address     Descripton
 *   --------------------------------------------------------------------------------------------------
 *   C000-CFFF          0000-0FFF       4K CHR ROM (switchable)
 *   D000-DFFF          1000-1FFF       4K CHR ROM (switchable)
 *   E000-E7FF          2000-27FF       2K VRAM
 *   E800-FFFF          2800-2FFF       2K VRAM (Nametable mirroring)
 *   F000-F7FF          3000-37FF       Mirror of 2000-27FF
 *   F800-FFFF          3800-3FFF       Mirror of 2800-2FFF
 *
 * Bank Switching and Registers:
 * The bank switching is done through a 5-bit shift register accessed by
 * writing to 8000-FFFF, the written address selects the internal register.
 *
 * - Load Register (8000-FFFF):
 *   D7 D6 D5 D4 D3 D2 D1 D0
 *    |  |  |  |  |  |  |  |
 *    |  x  x  x  x  x  x  +-> Data bit loaded into the shift register (LSB first)
 *    +----------------------> 0: No effect
 *                             1: Reset the shift register and write:
 *                                control = control OR $0C
 *
 * - Control Register (internal, 8000-9FFF):
 *   D4 D3 D2 D1 D0
 *    |  |  |  |  |
 *    |  |  |  +--+-> Nametable arragement:
 *    |  |  |         00: One-screen, lower bank
 *    |  |  |         01: One-screen, upper bank
 *    |  |  |         10: Horizontal arrangement (Vertical mirroring)
 *    |  |  |         11: Vertical arrangement (Horizontal mirroring)
 *    |  +--+-------> PRG ROM bank mode:
 *    |               00: Switch 32K at 8000, ignoring low bit of bank number
 *    |               01: Switch 32K at 8000, ignoring low bit of bank number
 *    |               10: Fix first bank at 8000, switch 16K bank at C000
 *    |               11: Fix last bank at C000, switch 16K bank at 8000
 *    +-------------> CHR ROM bank mode:
 *                    0: Switch 8K at a time
 *                    1: Switch two separated 4K banks
 *
 * - CHR banks 0 and 1 (internal A000-BFFF and C000-DFFF)
 *
 * + CHR bank 0 (internal, A000-BFFF);
 *   D4 D3 D2 D1 D0
 *    |  |  |  |  |
 *    +--+--+--+--+-> Select 4K or 8K CHR bank at PPU 0000 (low bit ignored on 8K mode)
 *
 * + CHR bank 1 (internal, C000-DFFF):
 *   D4 D3 D2 D1 D0
 *    |  |  |  |  |
 *    +--+--+--+--+-> Select 4K CHR bank at PPU 1000 (ignored in 8K mode)
 *
 * Mapper-001 CHR banks 0 and 1 should be assigned as follows:
 *
 *   D4 D3 D2 D1 D0
 *    |  |  |  |  |
 *    |  |  |  |  +-> CHR A12
 *    |  |  |  +----> CHR A13 if CHR >= 16K
 *    |  |  +-------> CHR A14 if CHR >= 32K; PRG RAM A13 if PRG RAM = 32K
 *    |  +----------> CHR A15 if CHR >= 64K; PRG RAM A13 if PRG RAM = 16K or PRG RAM A14 if PRG RAM = 32K
 *    +-------------> CHR A16 if CHR = 128K; PRG ROM A18 if PRG ROM = 512K
 *
 * - PRG bank (internal, E000-FFFF):
 *   D4 D3 D2 D1 D0
 *    |  |  |  |  |
 *    |  +--+--+--+-> Select 16K PRG ROM bank (low bit ignored in 32K mode)
 *    +-------------> MMC1B and later: PRG RAM chip enable (0: enabled, 1: disabled. Ignored in MMC1A)
 *                    MMC1A: Bit 3 bypasses fixed bank logic in 16K mode
 *                           0: Fixed bank affects A17-A14
 *                           1: Fixed bank affects A16-A14 and bit-3 directly controls A17
 *
 * @see https://www.nesdev.org/wiki/MMC1
 */
class Mapper_001 : public Cartridge {
public:
    constexpr static const char* TYPE                   = "CART_SxROM";
    constexpr static const uint8_t SHIFT_RESET          = 0x80;
    constexpr static const uint8_t CTRL_PRG_FIXED_C000  = 0x0C;
    constexpr static const size_t LOAD_REGISTER         = 0x8000 - 0x4000;
    constexpr static const size_t CONTROL_REGISTER      = 0x8000 - 0x4000;
    constexpr static const size_t CHR_BANK_0_REGISTER   = 0xA000 - 0x4000;
    constexpr static const size_t CHR_BANK_1_REGISTER   = 0xC000 - 0x4000;
    constexpr static const size_t PRG_BANK_REGISTER     = 0xE000 - 0x4000;

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_001(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

    virtual ~Mapper_001();

    void reset() override;

private:
    void load_bit(size_t addr, uint8_t data);
    void reg_control(uint8_t value);
    void reg_prg(uint8_t value);
    void reg_chr(bool hi, uint8_t value);

    uint8_t _shreg{};       /* 5-bit shift register         */
    uint8_t _shbit{};       /* 5-bit shift register counter */
    bool    _prg_A18{};     /* PRG A18 line status          */
};

}
}
}
