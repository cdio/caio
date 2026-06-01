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
#include "mapper_004.hpp"

#include "logger.hpp"
#include "serializer.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_004::Mapper_004(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    const size_t chr_size = hdr.chr_size();
    const size_t prg_size = hdr.prg_size();

    if (prg_size < MMC3_PRG_BANK_SIZE || prg_size > MMC3_PRG_MAX_SIZE) {
        throw InvalidCartridge{"{}: Invalid PRG ROM size: {}, Maximum: {}, Minimum: {}",
            fname.string(), prg_size, MMC3_PRG_MAX_SIZE, MMC3_PRG_MIN_SIZE};
    }

    if (chr_size != 0 && (chr_size < MMC3_CHR_MIN_SIZE || chr_size > MMC3_CHR_MAX_SIZE)) {
        throw InvalidCartridge{"{}: Invalid CHR ROM size: {}, Maximum: {}, Minimum: {}",
            fname.string(), chr_size, MMC3_CHR_MAX_SIZE, MMC3_CHR_MIN_SIZE};
    }

    using namespace std::placeholders;
    write_observer(std::bind(&Mapper_004::decoder, this, _1, _2));

    Mapper_004::reset();
}

void Mapper_004::reset()
{
    Cartridge::reset();

    /*
     * "... hardwired 4-screen VRAM. In the iNES and
     *  NES 2.0 formats, this can be identified through
     *  bit 3 of byte $06 of the header".
     */
    if (_hdr.alternative_nametable()) {
        _mirror = MirrorType::FourScreen;
    }

    /*
     * PRG banks on MMC3 are 8K and not 16K.
     *
     * "Because the values in R6, R7, and 8000 are unspecified at power on,
     * the reset vector must point into E000-FFFF, and code must initialize
     * these before jumping out of E000-FFFF."
     */
    _prg_mode = PrgMode::Fixed_C000;

    const size_t banks = _prg.size() / MMC3_PRG_BANK_SIZE;
    _prg_banks[0] = ROMBank{_prg, MMC3_PRG_BANK_SIZE, 0};
    _prg_banks[1] = ROMBank{_prg, MMC3_PRG_BANK_SIZE, 1};
    _prg_banks[2] = ROMBank{_prg, MMC3_PRG_BANK_SIZE, banks - 2};
    _prg_banks[3] = ROMBank{_prg, MMC3_PRG_BANK_SIZE, banks - 1};

    /*
     * CHR banks on MMC3 are 1K and not 4K.
     */
    _A12_inv = 0;

    _chr_banks[0] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 0};
    _chr_banks[1] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 1};
    _chr_banks[2] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 2};
    _chr_banks[3] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 3};
    _chr_banks[4] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 4};
    _chr_banks[5] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 5};
    _chr_banks[6] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 6};
    _chr_banks[7] = RAMBank{_chr, MMC3_CHR_BANK_SIZE, 7};
}

uint8_t Mapper_004::ppu_read(size_t addr, ReadMode mode)
{
    if (mode != ReadMode::Peek) {
        a12_tick(addr);
    }

    return Cartridge::ppu_read(addr, mode);
}

void Mapper_004::ppu_write(size_t addr, uint8_t value)
{
    Cartridge::ppu_write(addr, value);
    a12_tick(addr);
}

void Mapper_004::decoder(addr_t addr, uint8_t value)
{
    const size_t cpu_addr = to_cpu_address(addr);
    const size_t reg = cpu_addr & MMC3_REG_MASK;
    const bool cmd = cpu_addr & MMC3_REG_CMD_FLAG;

    switch (reg) {
    case MMC3_BANK_REG:
        bank_select(cmd, value);
        break;
    case MMC3_NAMETABLE_REG:
        nametable_select(cmd, value);
        break;
    case MMC3_IRQ_LATCH_REG:
        irq_latch(cmd, value);
        break;
    case MMC3_IRQ_ENABLE_REG:
        irq_enable(cmd);
        break;
    default:;
    }
}

void Mapper_004::bank_select(bool bank_data, uint8_t value)
{
    /*
     * D7 D6 D5 D4 D3 D2 D1 D0
     *  |  |  |  |  |  |  |  |
     *  |  |  |  x  x  |  |  |
     *  |  |  |        +--+--+-> Specify which bank register to update on next write to Bank Data register
     *  |  |  |                  000: R0: Select 2 KB CHR bank at PPU 0000-07FF (or 1000-17FF)
     *  |  |  |                  001: R1: Select 2 KB CHR bank at PPU 0800-0FFF (or 1800-1FFF)
     *  |  |  |                  010: R2: Select 1 KB CHR bank at PPU 1000-13FF (or 0000-03FF)
     *  |  |  |                  011: R3: Select 1 KB CHR bank at PPU 1400-17FF (or 0400-07FF)
     *  |  |  |                  100: R4: Select 1 KB CHR bank at PPU 1800-1BFF (or 0800-0BFF)
     *  |  |  |                  101: R5: Select 1 KB CHR bank at PPU 1C00-1FFF (or 0C00-0FFF)
     *  |  |  |                  110: R6: Select 8 KB PRG ROM bank at 8000-9FFF (or C000-DFFF)
     *  |  |  |                  111: R7: Select 8 KB PRG ROM bank at A000-BFFF
     *  |  |  |
     *  |  |  +----------------> MMC3: Nothing;
     *  |  |                     MMC6: PRG RAM enable.
     *  |  |
     *  |  +-------------------> PRG ROM bank mode:
     *  |                        0: 8000-9FFF Swappable,
     *  |                           C000-DFFF Fixed to second-last bank;
     *  |                        1: C000-DFFF Swappable,
     *  |                           8000-9FFF Fixed to second-last bank.
     *  |
     *  +----------------------> CHR A12 inversion
     *                           0: Two 2 KB banks at 0000-0FFF, four 1 KB banks at 1000-1FFF;
     *                           1: Two 2 KB banks at 1000-1FFF, four 1 KB banks at 0000-0FFF.
     */
    constexpr static const uint8_t BANK_REG_MASK = D2 | D1 | D0;
    constexpr static const uint8_t BANK_PRG_MODE = D6;
    constexpr static const uint8_t BANK_CHR_MODE = D7;

    if (!bank_data) {
        /*
         * Register/mode select.
         */
        _mmc3_reg = static_cast<MMC3_Register>(value & BANK_REG_MASK);
        _prg_mode = static_cast<PrgMode>((value & BANK_PRG_MODE) >> 6);
        _A12_inv  = (value & BANK_CHR_MODE);
    } else {
        /*
         * Register data write.
         */
         _mmc3_registers[_mmc3_reg] = value;
         prg_switch();
         chr_switch();
    }
}

void Mapper_004::prg_switch()
{
    /*
     * PRG map mode  Fixed_C000  Fixed_8000
     * ------------------------------------
     * 8000-9FFF     R6          (-2)
     * A000-BFFF     R7          R7
     * C000-DFFF     (-2)        R6
     * E000-FFFF     (-1)        (-1)
     *
     * (-1): The last bank.
     * (-2): The second last bank.
     *
     * "R6 and R7 will ignore the top two bits, as the MMC3 has only 6 PRG ROM address lines.
     * Some romhacks rely on an 8-bit extension of R6/7 for oversized PRG-ROM, but this is
     * deliberately not supported by many emulators. See iNES Mapper 004 below."
     */
    const auto banks = _prg_banks[0].banks();
    const auto r6 = _mmc3_registers[R6] & 0b0011'1111;
    const auto r7 = _mmc3_registers[R7] & 0b0011'1111;

    switch (_prg_mode) {
    case PrgMode::Fixed_C000:
        _prg_banks[0].bank(r6);
        _prg_banks[1].bank(r7);
        _prg_banks[2].bank(banks - 2);
        _prg_banks[3].bank(banks - 1);
        break;
    case PrgMode::Fixed_8000:
        _prg_banks[0].bank(banks - 2);
        _prg_banks[1].bank(r7);
        _prg_banks[2].bank(r6);
        _prg_banks[3].bank(banks - 1);
        break;
    case PrgMode::Mode_32K:
        /* Unused */
        break;
    }
}

void Mapper_004::chr_switch()
{
    /*
     * CHR map mode  8000.D7=0  8000.D7=1
     * ----------------------------------
     * 0000-03FF     R0         R2
     * 0400-07FF     R0+1       R3
     * 0800-0BFF     R1         R4
     * 0C00-0FFF     R1+1       R5
     * 1000-13FF     R2         R0
     * 1400-17FF     R3         R0+1
     * 1800-1BFF     R4         R1
     * 1C00-1FFF     R5         R1+1
     */
    const auto r0 = _mmc3_registers[R0] & ~1;
    const auto r1 = _mmc3_registers[R1] & ~1;
    const auto r2 = _mmc3_registers[R2];
    const auto r3 = _mmc3_registers[R3];
    const auto r4 = _mmc3_registers[R4];
    const auto r5 = _mmc3_registers[R5];

    if (_A12_inv) {
        _chr_banks[0].bank(r2);
        _chr_banks[1].bank(r3);
        _chr_banks[2].bank(r4);
        _chr_banks[3].bank(r5);
        _chr_banks[4].bank(r0);
        _chr_banks[5].bank(r0 | 1);
        _chr_banks[6].bank(r1);
        _chr_banks[7].bank(r1 | 1);
    } else {
        _chr_banks[0].bank(r0);
        _chr_banks[1].bank(r0 | 1);
        _chr_banks[2].bank(r1);
        _chr_banks[3].bank(r1 | 1);
        _chr_banks[4].bank(r2);
        _chr_banks[5].bank(r3);
        _chr_banks[6].bank(r4);
        _chr_banks[7].bank(r5);
    }
}

void Mapper_004::nametable_select(bool ram_protect, uint8_t value)
{
    if (!ram_protect) {
        /*
         * D7 D6 D5 D4 D3 D2 D1 D0
         *  |  |  |  |  |  |  |  |
         *  x  x  x  x  x  x  x  +-> Nametable arrangement:
         *                           0: Horizontal (A10);
         *                           1: Vertical (A11).
         *
         * This bit has no effect on cartridges with hardwired 4-screen VRAM.
         * In the iNES and NES 2.0 formats, this can be identified through
         * bit 3 of byte $06 of the header.
         */
        if (_mirror != MirrorType::FourScreen) {
            const bool varr = (value & 1);
            const auto type = (varr ? MirrorType::Horizontal : MirrorType::Vertical);
            vram_mirror(type);
        }

    } else {
        /* RAM protect not implemented (MMC3 <-> MMC6 compatibility) */
    }
}

void Mapper_004::irq_latch(bool latch_reload, uint8_t value)
{
    if (latch_reload) {
        /*
         * Latch value.
         */
        _irq_latch = value;
    } else {
        /*
         * Force counter reload at next rising edge of PPU address
         * (that would be cycle 260 of current scanline).
         */
        _irq_counter = 0;
    }
}

void Mapper_004::a12_tick(addr_t addr)
{
    /*
     * PPU $1000: A12 => Pattern table
     */
    const bool a12 = addr & A12;
    if (a12 != _prev_A12) {
        _prev_A12 = a12;
        if (a12) {
            a12_tick();
        }
    }
}

void Mapper_004::a12_tick()
{
    if (_irq_counter == 0) {
        _irq_counter = _irq_latch;
        irq_out(true);
    } else {
        --_irq_counter;
    }
}

Serializer& operator&(Serializer& ser, Mapper_004& cart)
{
    ser & static_cast<Cartridge&>(cart)
        & cart._mmc3_registers
        & cart._mmc3_reg
        & cart._A12_inv
        & cart._irq_latch
        & cart._irq_counter
        & cart._prev_A12;

    return ser;
}

}
}
}
