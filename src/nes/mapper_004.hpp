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
 * Cartridge Mapper 004 (MMC3/MMC6).
 *
 * ### Characteristics:
 *
 *     - PRG ROM capacity:  512K
 *     - PRG ROM window:    8K + 8K + 16K fixed
 *     - PRG RAM capacity:  8K
 *     - PRG RAM window:    8K
 *     - CHR capacity:      256K
 *     - CHR window:        2Kx2 + 1Kx4
 *
 * ### Banks:
 *
 *     ASIC  Device  Address                   Description
 *     ----------------------------------------------------------------------------------------
 *     MMC3  CPU     6000-7FFF                 8 KB PRG RAM bank (optional)
 *     MMC3  CPU     8000-9FFF (or C000-DFFF)  8 KB switchable PRG ROM bank
 *     MMC3  CPU     A000-BFFF                 8 KB switchable PRG ROM bank
 *     MMC3  CPU     C000-DFFF (or 8000-9FFF)  8 KB PRG ROM bank, fixed to the second-last bank
 *     MMC3  CPU     E000-FFFF                 8 KB PRG ROM bank, fixed to the last bank
 *     MMC3  PPU     0000-07FF (or 1000-17FF)  2 KB switchable CHR bank
 *     MMC3  PPU     0800-0FFF (or 1800-1FFF)  2 KB switchable CHR bank
 *     MMC3  PPU     1000-13FF (or 0000-03FF)  1 KB switchable CHR bank
 *     MMC3  PPU     1400-17FF (or 0400-07FF)  1 KB switchable CHR bank
 *     MMC3  PPU     1800-1BFF (or 0800-0BFF)  1 KB switchable CHR bank
 *     MMC3  PPU     1C00-1FFF (or 0C00-0FFF)  1 KB switchable CHR bank
 *     MMC6  CPU     7000-7FFF                 1 KB PRG RAM, mirrored
 *     MMC6  CPU     8000-9FFF (or C000-DFFF)  8 KB switchable PRG ROM bank
 *     MMC6  CPU     A000-BFFF                 8 KB switchable PRG ROM bank
 *     MMC6  CPU     C000-DFFF (or 8000-9FFF)  8 KB PRG ROM bank, fixed to the second-last bank
 *     MMC6  CPU     E000-FFFF                 8 KB PRG ROM bank, fixed to the last bank
 *     MMC6  PPU     0000-07FF (or 1000-17FF)  2 KB switchable CHR bank
 *     MMC6  PPU     0800-0FFF (or 1800-1FFF)  2 KB switchable CHR bank
 *     MMC6  PPU     1000-13FF (or 0000-03FF)  1 KB switchable CHR bank
 *     MMC6  PPU     1400-17FF (or 0400-07FF)  1 KB switchable CHR bank
 *     MMC6  PPU     1800-1BFF (or 0800-0BFF)  1 KB switchable CHR bank
 *     MMC6  PPU     1C00-1FFF (or 0C00-0FFF)  1 KB switchable CHR bank
 *
 * ### Registers:
 *
 * Both ASICs MMC3 and MMC6 have 4 pairs of registers at:
 *
 *   - 8000-9FFF: Bank Select (even address) / Bank Data (odd address)
 *   - A000-BFFF: Nametable Arrangement (even address) / PRG RAM protect (odd address)
 *   - C000-DFFF: IRQ Latch (even address) / Reload (odd address)
 *   - E000-FFFF: IRQ Disable (even address) / Enable (odd address)
 *
 * ### Bank Select (8000-9FFE, EVEN):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      |  |  |  x  x  |  |  |
 *      |  |  |        +--+--+-> Specify which bank register to update on next write to Bank Data register
 *      |  |  |                  000: R0: Select 2 KB CHR bank at PPU 0000-07FF (or 1000-17FF)
 *      |  |  |                  001: R1: Select 2 KB CHR bank at PPU 0800-0FFF (or 1800-1FFF)
 *      |  |  |                  010: R2: Select 1 KB CHR bank at PPU 1000-13FF (or 0000-03FF)
 *      |  |  |                  011: R3: Select 1 KB CHR bank at PPU 1400-17FF (or 0400-07FF)
 *      |  |  |                  100: R4: Select 1 KB CHR bank at PPU 1800-1BFF (or 0800-0BFF)
 *      |  |  |                  101: R5: Select 1 KB CHR bank at PPU 1C00-1FFF (or 0C00-0FFF)
 *      |  |  |                  110: R6: Select 8 KB PRG ROM bank at 8000-9FFF (or C000-DFFF)
 *      |  |  |                  111: R7: Select 8 KB PRG ROM bank at A000-BFFF
 *      |  |  |
 *      |  |  +----------------> MMC3: Nothing;
 *      |  |                     MMC6: PRG RAM enable.
 *      |  |
 *      |  +-------------------> PRG ROM bank mode:
 *      |                        0: 8000-9FFF swappable, C000-DFFF fixed to second-last bank;
 *      |                        1: C000-DFFF swappable, 8000-9FFF fixed to second-last bank.
 *      |
 *      +----------------------> CHR A12 inversion
 *                               0: Two 2 KB banks at 0000-0FFF, four 1 KB banks at 1000-1FFF;
 *                               1: Two 2 KB banks at 1000-1FFF, four 1 KB banks at 0000-0FFF.
 *
 * #### PRG Banks:
 *
 * Bit 6 of the last value written to 8000 swaps the PRG windows at 8000 and C000.
 * The MMC3 uses one map if bit 6 was cleared to 0 (value & $40 == $00) and another
 * if set to 1 (value & $40 == $40).
 *
 *      PRG map mode  8000.D6=0  8000.D6=1
 *      ----------------------------------
 *      8000-9FFF     R6         (-2)
 *      A000-BFFF     R7         R7
 *      C000-DFFF     (-2)       R6
 *      E000-FFFF     (-1)       (-1)
 *
 * (-1): The last bank.
 * (-2): The second last bank.
 *
 * Because the values in R6, R7, and 8000 are unspecified at power on,
 * the reset vector must point into E000-FFFF, and code must initialize
 * these before jumping out of E000-FFFF.
 *
 * #### CHR Banks:
 *
 *      CHR map mode  8000.D7=0  8000.D7=1
 *      ----------------------------------
 *      0000-03FF     R0         R2
 *      0400-07FF     R0+1       R3
 *      0800-0BFF     R1         R4
 *      0C00-0FFF     R1+1       R5
 *      1000-13FF     R2         R0
 *      1400-17FF     R3         R0+1
 *      1800-1BFF     R4         R1
 *      1C00-1FFF     R5         R1+1
 *
 * 2KB banks may only select even numbered CHR banks. (The lowest bit is ignored.)
 *
 * ### Bank Data (8001-9FFF, ODD):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      +--+--+--+--+--+--+--+-> New bank value (based on last value written to Bank select register).
 *
 * R6 and R7 will ignore the top two bits, as the MMC3 has only 6 PRG ROM address lines.
 * Some romhacks rely on an 8-bit extension of R6/7 for oversized PRG-ROM, but this is
 * deliberately not supported by many emulators. See iNES Mapper 004 below.
 *
 * R0 and R1 ignore the bottom bit, as the value written still counts banks in 1KB units
 * but odd numbered banks can't be selected.
 *
 * ### Nametable arrangement (A000-BFFE, EVEN):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      x  x  x  x  x  x  x  +-> Nametable arrangement:
 *                               0: Horizontal (A10);
 *                               1: Vertical (A11).
 *
 * This bit has no effect on cartridges with hardwired 4-screen VRAM.
 * In the iNES and NES 2.0 formats, this can be identified through
 * bit 3 of byte $06 of the header.
 *
 * ### PRG RAM protect (A001-BFFF, ODD):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      |  |  |  |  x  x  x  x
 *      |  |  |  |
 *      |  |  |  +-------------> MMC3: Nothing
 *      |  |  |                  MMC6: Enable write to 7000-71FF
 *      |  |  |
 *      |  |  +----------------> MMC3: Nothing
 *      |  |                     MMC6: Enable read at 7000-71FF
 *      |  |
 *      |  +-------------------> MMC3: Write protection (0: Allow writes; 1: Deny writes)
 *      |                        MMC6: Enable write to 7200-73FF
 *      |                        Not implemented
 *      |
 *      +----------------------> MMC3: PRG RAM chip enable (0: Disable; 1: Enable)
 *                               MMC6: Enable read at 7200-73FF
 *
 * Disabling PRG RAM through bit 7 causes reads from the PRG RAM region to return open bus.
 * Though these bits are functional on the MMC3, their main purpose is to write-protect
 * save RAM during power-off. Many emulators choose not to implement them as part of
 * iNES Mapper 4 to avoid an incompatibility with the MMC6.
 *
 * ### IRQ Latch (C000-DFFE, EVEN):
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *      |  |  |  |  |  |  |  |
 *      +--+--+--+--+--+--+--+-> IRQ counter reload value.
 *
 * When the IRQ counter is zero (or a reload is requested through C001),
 * this value will be copied to the IRQ counter at the NEXT rising edge
 * of the PPU address, presumably at PPU cycle 260 of the current scanline.
 *
 * ### IRQ Reload (C001-DFFF, ODD):
 *
 * Writing any value to this register clears the MMC3 IRQ counter immediately,
 * and then reloads it at the NEXT rising edge of the PPU address, presumably
 * at PPU cycle 260 of the current scanline.
 *
 * ### IRQ Disable (E000-FFFE, EVEN):
 *
 * Writing any value to this register will disable MMC3 interrupts
 * AND acknowledge any pending interrupts.
 *
 * ### IRQ Enable (E001-FFFF, ODD):
 *
 * Writing any value to this register will enable MMC3 interrupts.
 *
 * @see Cartridge
 * @see https://www.nesdev.org/wiki/MMC3
 * @see https://www.nesdev.org/wiki/MMC6
 */
class Mapper_004 : public Cartridge {
public:
    constexpr static const char* TYPE                 = "CART_MMC3";

    constexpr static const size_t MMC3_PRG_BANK_SIZE  = 8   * 1024;
    constexpr static const size_t MMC3_PRG_MIN_SIZE   = 16  * 1024;
    constexpr static const size_t MMC3_PRG_MAX_SIZE   = 512 * 1024;
    constexpr static const size_t MMC3_PRG_BANK_MASK  = MMC3_PRG_BANK_SIZE - 1;
    constexpr static const size_t MMC3_PRG_BANK_SHIFT = 13;
    constexpr static const size_t MMC3_PRG_BANKS      = 4;

    constexpr static const size_t MMC3_CHR_BANK_SIZE  = 1   * 1024;
    constexpr static const size_t MMC3_CHR_MIN_SIZE   = 4   * 1024;
    constexpr static const size_t MMC3_CHR_MAX_SIZE   = 256 * 1024;
    constexpr static const size_t MMC3_CHR_BANK_MASK  = MMC3_CHR_BANK_SIZE - 1;
    constexpr static const size_t MMC3_CHR_BANK_SHIFT = 10;
    constexpr static const size_t MMC3_CHR_BANKS      = 8;

    constexpr static const addr_t MMC3_REG_MASK       = 0xE000;
    constexpr static const addr_t MMC3_BANK_REG       = 0x8000;
    constexpr static const addr_t MMC3_NAMETABLE_REG  = 0xA000;
    constexpr static const addr_t MMC3_IRQ_LATCH_REG  = 0xC000;
    constexpr static const addr_t MMC3_IRQ_ENABLE_REG = 0xE000;
    constexpr static const addr_t MMC3_REG_CMD_FLAG   = 0x0001;

    enum MMC3_Register {
        R0 = 0,
        R1 = 1,
        R2 = 2,
        R3 = 3,
        R4 = 4,
        R5 = 5,
        R6 = 6,
        R7 = 7,

        MAX
    };

    /**
     * @see Cartridge::Cartridge(std::string_view, const fs::Path&, const iNES::Header& std::ifstream&)
     */
    Mapper_004(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

    /**
     * @see Device::reset()
     */
    void reset() override;

private:
    /**
     * @see Cartridge::ppu_read(size_t, ReadMode)
     */
    uint8_t ppu_read(size_t addr, ReadMode mode) override;

    /**
     * @see Cartridge::ppu_write(size_t, uint8_t)
     */
    void ppu_write(size_t addr, uint8_t value) override;

    /**
     * Latch the current state of the A12 line and eventually tick it.
     * @param addr Currently accessed PPU address.
     */
    void a12_tick(addr_t addr);

    /**
     * Tick the IRQ counter.
     * Tick the the IRQ counter and when it reaches 0
     * generate an interrupt request.
     * @see Cartridge::irq_out(bool)
     */
    void a12_tick();

    void decoder(addr_t addr, uint8_t value);

    void bank_select(bool bank_data, uint8_t value);

    void prg_switch();

    void chr_switch();

    void nametable_select(bool ram_protect, uint8_t value);

    void irq_latch(bool latch_reload, uint8_t value);

    std::array<uint8_t, MMC3_Register::MAX> _mmc3_registers{};
    MMC3_Register                           _mmc3_reg{};                /* Selected bank register   */
    bool                                    _A12_inv{};                 /* CHR A12 inversion flag   */
    uint8_t                                 _irq_latch{};               /* IRQ latch value          */
    uint8_t                                 _irq_counter{};             /* IRQ latch counter        */
    bool                                    _prev_A12{};                /* Previous A12 state       */

    friend Serializer& operator&(Serializer&, Mapper_004&);
};

}
}
}
