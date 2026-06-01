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
#include "mapper_001.hpp"

#include "logger.hpp"
#include "serializer.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Mapper_001::Mapper_001(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    load_bit(0, SHIFT_RESET);

    using namespace std::placeholders;
    write_observer(std::bind(&Mapper_001::shreg_loader, this, _1, _2));
}

void Mapper_001::reset()
{
    Cartridge::reset();
    load_bit(0, SHIFT_RESET);
}

void Mapper_001::shreg_loader(size_t addr, uint8_t value)
{
    /*
     * Shift register loader: Writes to 4000-7FFF (CPU 8000-FFFF).
     */
    if (addr >= PRG_BASE_ADDR && addr < PRG_BASE_ADDR + PRG_ASPACE_SIZE) {
        load_bit(addr, value);
    }
}

void Mapper_001::load_bit(size_t addr, uint8_t value)
{
    /*
     * D7 D6 D5 D4 D3 D2 D1 D0
     *  |  |  |  |  |  |  |  |
     *  |  x  x  x  x  x  x  +-> Data bit loaded into the shift register (LSB first)
     *  +----------------------> 0: No effect
     *                           1: Reset the shift register and write:
     *                              control = control OR $0C
     */
    if (value & SHIFT_RESET) {
        reg_control(CTRL_PRG_FIXED_C000);

    } else {
        /*
         * Load data (bit-0) into the shift register.
         */
        _shreg |= (value & 1) * _shbit;
        _shbit <<= 1;

        if (_shbit != D5) {
            return;
        }

        /*
         * The shift register is full, copy its value
         * to the destination internal register:
         *
         * CPU   Mapper  Register
         * ----------------------
         * 8000  4000    Control
         * A000  6000    CHR-0
         * C000  8000    CHR-1
         * E000  A000    PRG
         */
        const auto reg = addr >> 13;
        switch (reg) {
        case 0x2:
            reg_control(_shreg);
            break;
        case 0x3:
            reg_chr(0, _shreg);
            break;
        case 0x4:
            reg_chr(1, _shreg);
            break;
        case 0x5:
            reg_prg(_shreg);
            break;
        }
    }

    _shbit = D0;
    _shreg = 0;
}

void Mapper_001::reg_control(uint8_t value)
{
    /*
     * Control register:
     *   D4 D3 D2 D1 D0
     *    |  |  |  |  |
     *    |  |  |  +--+-> Nametable arrangement:
     *    |  |  |         00: One-screen, lower bank
     *    |  |  |         01: One-screen, upper bank
     *    |  |  |         10: Vertical mirroring
     *    |  |  |         11: Horizontal mirroring
     *    |  +--+-------> PRG ROM bank mode:
     *    |               00: Switch 32K at 8000, ignoring low bit of bank number
     *    |               01: Same as 00
     *    |               10: Fix first bank at 8000, switch 16K bank at C000
     *    |               11: Fix last bank at C000, switch 16K bank at 8000
     *    +-------------> CHR ROM bank mode:
     *                    0: Switch 8K at a time
     *                    1: Switch two separate 4K banks
     */
    const auto mtype = static_cast<MirrorType>(value & 3);
    vram_mirror(mtype);

    const size_t banks = _prg_banks[0].size();
    const auto pmode = (value >> 2) & 3;

    switch (pmode) {
    case 0:
    case 1:
        _prg_mode = PrgMode::Mode_32K;
        _prg_banks[0].bank(0);
        _prg_banks[1].bank(1);
        _prg_banks[2].bank(2);
        _prg_banks[3].bank(3);
        break;
    case 2:
        _prg_mode = PrgMode::Fixed_8000;
        _prg_banks[0].bank(0);
        _prg_banks[1].bank(1);
        break;
    case 3:
        _prg_mode = PrgMode::Fixed_C000;
        _prg_banks[2].bank(banks - 2);
        _prg_banks[3].bank(banks - 1);
        break;
    }

    _chr_mode = static_cast<ChrMode>((value & D4) >> 4);
}

void Mapper_001::reg_prg(uint8_t value)
{
    /*
     * PRG bank select register:
     *   D4 D3 D2 D1 D0
     *    |  |  |  |  |
     *    |  +--+--+--+-> Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode)
     *    +-------------> MMC1B and later: PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A)
     *                    MMC1A: Bit 3 bypasses fixed bank logic in 16K mode (0: Fixed bank affects A17-A14;
     *                           1: Fixed bank affects A16-A14 and bit 3 directly controls A17)
     */
    const size_t off = 64 * (_prg_A18);
    const size_t bank = (((value & (D3 | D2 | D1 | D0)) | (_prg_A18 * D4)) << 1) + off;

    switch (_prg_mode) {
    case PrgMode::Mode_32K:
        _prg_banks[0].bank(bank);
        _prg_banks[1].bank(bank + 1);
        _prg_banks[2].bank(bank + 2);
        _prg_banks[3].bank(bank + 3);
        break;
    case PrgMode::Fixed_8000:
        _prg_banks[2].bank(bank);
        _prg_banks[3].bank(bank + 1);
        break;
    case PrgMode::Fixed_C000:
        _prg_banks[0].bank(bank);
        _prg_banks[1].bank(bank + 1);
    }
}

void Mapper_001::reg_chr(bool hi, uint8_t value)
{
    /*
     * CHR bank select register:
     *   A000 (lo) and C000 (hi):
     *
     *   D4 D3 D2 D1 D0
     *    |  |  |  |  |
     *    |  |  |  |  +-> CHR A12
     *    |  |  |  +----> CHR A13 if CHR >= 16K
     *    |  |  +-------> CHR A14 if CHR >= 32K; PRG RAM A13 if PRG RAM = 32K
     *    |  +----------> CHR A15 if CHR >= 64K; PRG RAM A13 if PRG RAM = 16K or PRG RAM A14 if PRG RAM = 32K
     *    +-------------> CHR A16 if CHR = 128K; PRG ROM A18 if PRG ROM = 512K
     *
     * D4 is PRG-RAM disable for SNROM (PRG-ROM size <= 256 KiB, CHR-RAM size = 8 KiB, PRG-RAM size = 8 KiB)
     */
    const size_t cmask = D0 |
        ((_chr.size() >= 16384)  * D1) |
        ((_chr.size() >= 32768)  * D2) |
        ((_chr.size() >= 65536)  * D3) |
        ((_chr.size() == 131072) * D4);

    if (_chr_mode == ChrMode::Mode_4K) {
        const size_t bank = (value & cmask) << 2;
        const size_t r = 4 * hi;
        _chr_banks[r].bank(bank);
        _chr_banks[r + 1].bank(bank + 1);
        _chr_banks[r + 2].bank(bank + 2);
        _chr_banks[r + 3].bank(bank + 3);
    } else {
        const size_t bank = (value & cmask) << 3;
        _chr_banks[0].bank(bank);
        _chr_banks[1].bank(bank + 1);
        _chr_banks[2].bank(bank + 2);
        _chr_banks[3].bank(bank + 3);
        _chr_banks[4].bank(bank + 4);
        _chr_banks[5].bank(bank + 5);
        _chr_banks[6].bank(bank + 6);
        _chr_banks[7].bank(bank + 7);
    }

    /* PRG RAM bank */
    if (_ram.size() > 8192) {
        const bool is16k = (_ram.size() == 16384);
        const size_t shift = 2 + is16k;
        const size_t rmask = (D3 | (!is16k * D2)) >> shift;
        const size_t rvalue = value & rmask;
        const size_t bank = rvalue << 3;
        _ram_banks[0].bank(bank);
        _ram_banks[1].bank(bank + 1);
        _ram_banks[2].bank(bank + 2);
        _ram_banks[3].bank(bank + 3);
        _ram_banks[4].bank(bank + 4);
        _ram_banks[5].bank(bank + 5);
        _ram_banks[6].bank(bank + 6);
        _ram_banks[7].bank(bank + 7);
    }

    /* PRG ROM bank */
    const bool is_D4_A18 = !(_prg.size() < 524288);
    const bool A18 = (value & D4);
    if (is_D4_A18 && A18 != _prg_A18) {
        /*
         * D4 specifies the status of A18 of PRG ROM when PRG ROM size is 512K.
         */
        _prg_A18 = A18;
        int8_t off = 64 | (!A18 * 0x80);
        std::for_each(std::begin(_prg_banks), std::end(_prg_banks), [off](RAMBank& rbank) {
            const size_t b = rbank.bank();
            rbank.bank(b + off);
        });
    }
}

Serializer& operator&(Serializer& ser, Mapper_001& cart)
{
    ser & static_cast<Cartridge&>(cart)
        & cart._shreg
        & cart._shbit
        & cart._prg_A18;

    return ser;
}

}
}
}
