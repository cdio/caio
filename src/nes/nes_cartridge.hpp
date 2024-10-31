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

#include <fstream>
#include <string>
#include <string_view>

#include "device.hpp"
#include "fs.hpp"
#include "ram.hpp"
#include "ram_bank.hpp"
#include "rom.hpp"

#include "ines.hpp"
#include "nes_params.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * NES Cartridge.
 * A NES Cartridge implements the so called mapper.
 * It embeds two devices into one:
 * - One device accessed by the CPU;
 * - Another device accessed by the PPU.
 *
 * The set of addresses exposed by the cartridge is divided
 * into two separated sections as follows:
 *
 *   Cartridge Address  Accessed by     Mapped at CPU/PPU Address
 *   -------------------------------------------------------------
 *   0000-BFFF          CPU             4000-FFFF
 *   C000-EFFF          PPU             0000-2FFF
 *
 * @see https://www.nesdev.org/wiki/Mapper
 */
class Cartridge : public Device {
public:
    constexpr static const addr_t CPU_OFFSET    = 0x0000;
    constexpr static const addr_t PPU_OFFSET    = 0xC000;
    constexpr static const size_t CPU_SIZE      = 0xC000;
    constexpr static const size_t PPU_SIZE      = 0x3000;

    constexpr static const size_t VRAM_SIZE     = nes::VRAM_SIZE;
    constexpr static const size_t VRAM_MASK     = VRAM_SIZE - 1;
    constexpr static const size_t VRAM_BASE     = 0x2000;
    constexpr static const addr_t VRAM_END      = 0x3000;

    constexpr static const size_t RAM_SIZE      = 8192;                 /* Default PRG RAM size         */
    constexpr static const size_t RAM_BASE      = 0x2000;               /* PRG RAM base address $6000   */
    constexpr static const size_t RAM_BANK_SIZE = 8192;
    constexpr static const size_t RAM_BANK_MASK = RAM_BANK_SIZE - 1;

    constexpr static const size_t PRG_LO_BASE   = 0x4000;               /* PRG LO base address $8000    */
    constexpr static const size_t PRG_HI_BASE   = 0x8000;               /* PRG HI base address $C000    */
    constexpr static const size_t PRG_BANK_SIZE = 16384;
    constexpr static const size_t PRG_BANK_MASK = PRG_BANK_SIZE - 1;

    constexpr static const size_t CHR_RAM_SIZE  = 8192;                 /* Default CHR RAM size         */
    constexpr static const size_t CHR_LO_BASE   = 0x0000;               /* CHR LO base address $0000    */
    constexpr static const size_t CHR_HI_BASE   = 0x1000;               /* CHR HI base address $1000    */
    constexpr static const size_t CHR_BANK_SIZE = 4096;
    constexpr static const size_t CHR_BANK_MASK = CHR_BANK_SIZE - 1;

    enum class MirrorType {
        OneScreenLower  = 0,
        OneScreenUpper  = 1,
        Vertical        = 2,
        Horizontal      = 3
    };

    enum class PrgMode {
        Fixed_C000,
        Fixed_8000,
        Mode_32K
    };

    enum class ChrMode {
        Mode_8K,
        Mode_4K
    };

    /**
     * Instantiate a cartridge.
     * @param fname Cartridge file name (.nes).
     * @exception InvalidCartridge
     * @see iNES::load_header(const fs::Path& fname)
     */
    static sptr_t<Cartridge> instance(const fs::Path& fname);

    virtual ~Cartridge();

    /**
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @see Device::size()
     */
    size_t size() const override;

    /**
     * @see Device::to_string()
     */
    std::string to_string() const override;

    /**
     * @see Device::dev_read(addr_t, ReadMode)
     */
    uint8_t dev_read(size_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write(addr_t, uint8_t)
     */
    void dev_write(size_t addr, uint8_t value) override;

protected:
    /**
     * Load a cartridge from an input stream.
     * @param type  Cartridge type name;
     * @param fname Name of the cartridge file;
     * @param hdr   Loaded iNES cartridge header;
     * @param is    Input stream pointing to the cartridge data (file pointer at byte after the header);
     * @exception InvalidCartridge
     */
    Cartridge(std::string_view type, const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is);

    addr_t vram_mirror(size_t addr) const;
    addr_t vram_mirror(size_t addr, MirrorType type) const;

    uint8_t cpu_read(size_t addr, ReadMode mode);
    void    cpu_write(size_t addr, uint8_t value);

    uint8_t ppu_read(size_t addr, ReadMode mode);
    void    ppu_write(size_t addr, uint8_t value);

    fs::Path        _fname;         /* Cartridge file name              */
    iNES::Header    _hdr;           /* Cartridge header                 */
    MirrorType      _mirror;        /* Nametable mirroring type         */
    fs::Path        _ram_fname{};   /* Persistent PRG RAM full path     */
    RAM             _vram{};        /* Board's 2K VRAM                  */
    RAM             _ram{};         /* PRG RAM                          */
    ROM             _prg{};         /* PRG ROM                          */
    RAM             _chr{};         /* CHR ROM/RAM                      */
    RAMBank         _ram_b{};       /* Current PRG RAM bank             */
    ROMBank         _prg_hb{};      /* Current PRG LO ROM bank          */
    ROMBank         _prg_lb{};      /* Current PRG HI ROM bank          */
    ROMBank         _chr_hb{};      /* Current CHR LO RAM/ROM bank      */
    ROMBank         _chr_lb{};      /* Current CHR HI RAM/ROM bank      */
    ChrMode         _chr_mode{};    /* CHR RAM/ROM mode (default 8K)    */
    PrgMode         _prg_mode{};    /* PRG ROM mode (default 16K, C000 fixed to last bank)  */
};

}
}
}
