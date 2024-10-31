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

namespace caio {
namespace nintendo {
namespace nes {

Mapper_001::Mapper_001(const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Cartridge{TYPE, fname, hdr, is}
{
    const size_t chr_size = hdr.chr_size();
    const size_t prg_size = hdr.prg_size();
    const size_t ram_size = hdr.prg_ram_size();

    if (ram_size != 0 && (ram_size & RAM_BANK_MASK) != 0)  {
        throw InvalidCartridge{"{}: Invalid PRG RAM size: {}", fname.c_str(), ram_size};
    }

    if (prg_size < PRG_BANK_SIZE || (prg_size & PRG_BANK_MASK) != 0) {
        throw InvalidCartridge{"{}: Invalid PRG ROM size: {}. It must be a multiple of {}K",
            fname.c_str(), prg_size, PRG_BANK_SIZE / 1024};
    }

    if (chr_size != 0 && (chr_size & CHR_BANK_MASK) != 0) {
        throw InvalidCartridge{"{}: Invalid CHR ROM size: {}. It must be a multiple of {}K",
            fname.string(), chr_size, CHR_BANK_SIZE / 1024};
    }

    load_bit(0, SHIFT_RESET);

    const auto shreg_loader = [this](size_t addr, uint8_t value) {
        if (addr >= PRG_LO_BASE && addr < PPU_OFFSET) {
            load_bit(addr, value);
        }
    };

    write_observer(shreg_loader);
}

Mapper_001::~Mapper_001()
{
}

void Mapper_001::reset()
{
    Cartridge::reset();
    load_bit(0, SHIFT_RESET);
}

void Mapper_001::load_bit(size_t addr, uint8_t data)
{
    if (data & SHIFT_RESET) {
        reg_control(CTRL_PRG_FIXED_C000);

    } else {
        /*
         * Load data (bit-0) into the shift register.
         */
        _shreg |= (data & 1) * _shbit;
        _shbit <<= 1;

        if (_shbit != D5) {
            return;
        }

        /*
         * The shift register is full, copy its value
         * to the destination internal register:
         *
         * CPU      Mapper      Register
         * -----------------------------
         * 8000     4000        Control
         * A000     6000        CHR-0
         * C000     8000        CHR-1
         * E000     A000        PRG
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
     * Control Register:
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
    _mirror = static_cast<MirrorType>(value & 3);

    const auto pmode = (value >> 2) & 3;
    switch (pmode) {
    case 0:
    case 1:
        _prg_mode = PrgMode::Mode_32K;
        _prg_lb.bank(0);
        _prg_hb.bank(1);
        break;
    case 2:
        _prg_mode = PrgMode::Fixed_8000;
        _prg_lb.bank(0);
        break;
    case 3:
        _prg_mode = PrgMode::Fixed_C000;
        _prg_hb.bank(_prg_hb.banks() - 1);
        break;
    }

    _chr_mode = static_cast<ChrMode>((value & D4) >> 4);
}

void Mapper_001::reg_prg(uint8_t value)
{
    /*
     * D4 D3 D2 D1 D0
     *  |  |  |  |  |
     *  |  +--+--+--+-> Select 16 KB PRG ROM bank (low bit ignored in 32 KB mode)
     *  +-------------> MMC1B and later: PRG RAM chip enable (0: enabled; 1: disabled; ignored on MMC1A)
     *                  MMC1A: Bit 3 bypasses fixed bank logic in 16K mode (0: Fixed bank affects A17-A14;
     *                         1: Fixed bank affects A16-A14 and bit 3 directly controls A17)
     */
    const size_t bank = (value & (D3 | D2 | D1 | D0)) | (_prg_A18 * D4);

    switch (_prg_mode) {
    case PrgMode::Mode_32K:
        _prg_lb.bank(bank & ~D0);
        _prg_hb.bank(bank | D0);
        break;
    case PrgMode::Fixed_8000:
        _prg_hb.bank(bank);
        break;
    case PrgMode::Fixed_C000:
        _prg_lb.bank(bank);
    }
}

void Mapper_001::reg_chr(bool hi, uint8_t value)
{
    /*
     * $A000 (lo) and $C000 (hi):
     *
     * D4 D3 D2 D1 D0
     *  |  |  |  |  |
     *  |  |  |  |  +-> CHR A12
     *  |  |  |  +----> CHR A13 if CHR >= 16K
     *  |  |  +-------> CHR A14 if CHR >= 32K; PRG RAM A13 if PRG RAM = 32K
     *  |  +----------> CHR A15 if CHR >= 64K; PRG RAM A13 if PRG RAM = 16K or PRG RAM A14 if PRG RAM = 32K
     *  +-------------> CHR A16 if CHR = 128K; PRG ROM A18 if PRG ROM = 512K
     */
    const uint8_t cmask = D0 |
        ((_chr.size() >= 16384)  * D1) |
        ((_chr.size() >= 32768)  * D2) |
        ((_chr.size() >= 65536)  * D3) |
        ((_chr.size() == 131072) * D4);

    const uint8_t cvalue = value & cmask;

    if (_chr_mode == ChrMode::Mode_4K) {
        if (hi) {
            _chr_hb.bank(cvalue);
        } else {
            _chr_lb.bank(cvalue);
        }

    } else {
        _chr_lb.bank(cvalue & ~D0);
        _chr_hb.bank(cvalue | D0);
    }

    /* PRG RAM bank */
    if (_ram.size() > 8192) {
        const bool is16k = (_ram.size() == 16384);
        const auto shift = 2 + is16k;
        uint8_t rmask = (D3 | (!is16k * D2)) >> shift;
        const uint8_t rvalue = value & rmask;
        _ram_b.bank(rvalue);
    }

    /* PRG ROM bank */
    if (_prg.size() == 524288) {
        /*
         * D4 specifies the status of A18 of PRG ROM when PRG ROM size is 512K.
         */
        const bool a18 = value & D4;
        if (a18 != _prg_A18) {
            if (a18) {
                _prg_lb.bank(_prg_lb.bank() | D4);
                _prg_hb.bank(_prg_hb.bank() | D4);
            } else {
                _prg_lb.bank(_prg_lb.bank() & ~D4);
                _prg_hb.bank(_prg_hb.bank() & ~D4);
            }
            _prg_A18 = a18;
        }
    }
}

}
}
}
