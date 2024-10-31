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
#include "nes_cartridge.hpp"

#include "config.hpp"

#include "mapper_000.hpp"
#include "mapper_001.hpp"
#include "mapper_002.hpp"

namespace caio {
namespace nintendo {
namespace nes {

Cartridge::Cartridge(std::string_view type, const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Device{type, fname.stem().string()},
      _fname{fname},
      _hdr{hdr},
      _mirror{hdr.vertical_mirror() ? MirrorType::Vertical : MirrorType::Horizontal},
      _vram{"vram", VRAM_SIZE}
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

    if (_hdr.persistent_ram()) {
        /*
         * Load previously saved data from a persistent RAM.
         */
        _ram_fname = std::format("{}/{}.ram", config::storage_path().c_str(), fs::sha256(fname));
        if (fs::exists(_ram_fname)) {
            _ram = RAM{_ram_fname.string(), _ram_fname, (ram_size == 0 ? RAM_SIZE : ram_size)};
        }
    }

    if (_ram.size() == 0) {
        _ram = RAM{"ram", (ram_size == 0 ? RAM_SIZE : ram_size)};
    }

    _prg = ROM{"prg", is, prg_size};

    if (chr_size == 0) {
        _chr = RAM{"chr", CHR_RAM_SIZE};
    } else {
        _chr = ROM{"chr", is, chr_size};
    }

    _prg_mode = PrgMode::Fixed_C000;
    _prg_lb = ROMBank{_prg, PRG_BANK_SIZE};
    _prg_hb = ROMBank{_prg, PRG_BANK_SIZE};
    _prg_hb.bank(_prg_hb.banks() - 1);

    _chr_mode = ChrMode::Mode_8K;
    _chr_lb = ROMBank{_chr, CHR_BANK_SIZE, 0};
    _chr_hb = ROMBank{_chr, CHR_BANK_SIZE, 1};

    _ram_b = RAMBank{_ram, _ram.size()};
}

Cartridge::~Cartridge()
{
    if (!_ram_fname.empty()) {
        fs::save(_ram_fname, _ram.buffer());
    }
}

void Cartridge::reset()
{
    _prg_mode = PrgMode::Fixed_C000;
    _prg_lb = ROMBank{_prg, PRG_BANK_SIZE};
    _prg_hb = ROMBank{_prg, PRG_BANK_SIZE};
    _prg_hb.bank(_prg_hb.banks() - 1);

    _chr_mode = ChrMode::Mode_8K;
    _chr_lb = ROMBank{_chr, CHR_BANK_SIZE, 0};
    _chr_hb = ROMBank{_chr, CHR_BANK_SIZE, 1};

    _ram_b = RAMBank{_ram, _ram.size()};
}

size_t Cartridge::size() const
{
    return (_ram.size() + _prg.size() + _vram.size() + _chr.size());
}

std::string Cartridge::to_string() const
{
    return std::format("{}: {}", Name::to_string(), iNES::to_string(_hdr));
}

uint8_t Cartridge::dev_read(size_t addr, ReadMode mode)
{
    return (addr < PPU_OFFSET ? cpu_read(addr - CPU_OFFSET, mode) : ppu_read(addr - PPU_OFFSET, mode));
}

void Cartridge::dev_write(size_t addr, uint8_t value)
{
    if (addr < PPU_OFFSET) {
        cpu_write(addr - CPU_OFFSET, value);
    } else {
        ppu_write(addr - PPU_OFFSET, value);
    }
}

inline uint8_t Cartridge::cpu_read(size_t addr, ReadMode mode)
{
    if (addr < RAM_BASE) {
        /*
         * Unmapped area: 0000-1FFF (CPU 4000-5FFF).
         */
        return 0;
    }

    if (addr < PRG_LO_BASE) {
        /*
         * RAM access: 2000-3FFF (CPU 6000-7FFF).
         */
        return _ram_b.read(addr - RAM_BASE, mode);
    }

    if (addr < PRG_HI_BASE) {
        /*
         * PRG LO access: 4000-7FFF (CPU 8000-BFFF)
         */
        return _prg_lb.read(addr - PRG_LO_BASE, mode);
    }

    /*
     * PRG HI access: 8000-BFFF (CPU C000-FFFF)
     */
    return _prg_hb.read(addr - PRG_HI_BASE, mode);
}

void Cartridge::cpu_write(size_t addr, uint8_t data)
{
    if (addr < RAM_BASE) {
        /*
         * Unmapped area: 0000-1FFF (CPU 4000-5FFF).
         */
        return;
    }

    if (addr < PRG_LO_BASE) {
        /*
         * RAM access: 2000-3FFF (CPU 6000-7FFF).
         */
        _ram_b.write(addr - RAM_BASE, data);
    }
}

inline uint8_t Cartridge::ppu_read(size_t addr, ReadMode mode)
{
    if (addr < CHR_HI_BASE) {
        /*
         * CHR LO ROM access: 0000-0FFF (PPU 0000-0FFF).
         */
        return _chr_lb.read(addr - CHR_LO_BASE, mode);
    }

    if (addr < VRAM_BASE) {
        /*
         * CHR HI ROM access: 1000-1FFF (PPU 1000-1FFF).
         */
        return _chr_hb.read(addr - CHR_HI_BASE, mode);
    }

    /*
     * VRAM access: 2000-2C00 (PPU 2000-2C00).
     */
    addr = vram_mirror(addr - VRAM_BASE) & VRAM_MASK;
    return _vram.read(addr, mode);
}

inline void Cartridge::ppu_write(size_t addr, uint8_t value)
{
    if (addr < CHR_HI_BASE) {
        /*
         * CHR LO ROM access: 0000-0FFF (PPU 0000-0FFF).
         */
        _chr_lb.write(addr - CHR_LO_BASE, value);
        return;
    }

    if (addr < VRAM_BASE) {
        /*
         * CHR HI ROM access: 1000-1FFF (PPU 1000-1FFF).
         */
        _chr_hb.write(addr - CHR_HI_BASE, value);
        return;
    }

    /*
     * VRAM access: 2000-2C00 (PPU 2000-2C00).
     */
    addr = vram_mirror(addr - VRAM_BASE) & VRAM_MASK;
    _vram.write(addr, value);
}

inline addr_t Cartridge::vram_mirror(size_t addr) const
{
    return vram_mirror(addr, _mirror);
}

inline addr_t Cartridge::vram_mirror(size_t addr, MirrorType type) const
{
    /*
     * Horizontal mirroring:
     *   2000 -> 2400
     *   2800 -> 2C00
     *
     * 20xx = 24xx
     * 28xx = 2Cxx
     *
     * Physical 2400 must be moved to logical 2800:
     * Access to logical 2000 => Nothing to do
     * Access to logical 2400 => Clear A10 becoming physical 2000
     * Access to logical 2800 => Clear A11 and set A10 becoming physical 2400
     * Access to logical 2C00 => Clear A11 (A10 already set) becoming physical 2400
     *
     * Vertical mirroring:
     *   2000 2400
     *     |   |
     *     v   v
     *   2800 2C00
     *
     * 20xx = 28xx
     * 24xx = 2Cxx
     *
     * Physical 2000 mirrored to logical 2800
     * Physical 2400 mirrored to logical 2C00
     */
    if (addr >= VRAM_END) {
        addr &= ~A12;
    }

    switch (type) {
    case MirrorType::OneScreenLower:
        /*
         * One-screen, lower bank.
         * All mirror 2000.
         */
        return (addr & ~0x0C00);
    case MirrorType::OneScreenUpper:
        /*
         * One-screen, upper bank/
         * All mirror 2400.
         */
        return (addr & ~0x0C00) | 0x0400;
    case MirrorType::Vertical:
        /*
         * Horizontal arrangement (Vertical mirroring)
         */
        break;
    case MirrorType::Horizontal: {
            /*
             * Vertical arrangement (Horizontal mirroring)
             */
            const addr_t bit10 = (addr & A11) >> 1;
            addr = ((addr & ~(A11 | A10)) | bit10);
        }
        break;
    }

    return addr;
}

sptr_t<Cartridge> Cartridge::instance(const fs::Path& fname)
{
    const auto fullpath = fs::search(fname);
    if (fullpath.empty()) {
        if (fname.empty()) {
            throw InvalidCartridge{"Cartridge file not specified"};
        }
        throw InvalidCartridge{"Invalid cartridge file: {}\n", fname.c_str()};
    }

    auto [hdr, is] = iNES::load_header(fullpath);
    const size_t mapper = hdr.mapper();

    switch (mapper) {
    case 0:   /* NROM */
        return std::make_shared<Mapper_000>(fullpath, hdr, is);
    case 1:   /* SxROM */
        return std::make_shared<Mapper_001>(fullpath, hdr, is);
    case 2:   /* UxROM */
        return std::make_shared<Mapper_002>(fullpath, hdr, is);
    default:;
    }

    throw InvalidCartridge{"Can't instantiate cartridge: {}: Mapper not supported: {}\n",
        fullpath.c_str(), iNES::to_string(hdr)};
}

}
}
}
