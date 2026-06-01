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

#include "device.hpp"
#include "fs.hpp"
#include "pin.hpp"
#include "ram.hpp"
#include "ram_bank.hpp"
#include "rom.hpp"

#include "ines.hpp"
#include "nes_params.hpp"

#include <fstream>
#include <string>
#include <string_view>

namespace caio {
namespace nintendo {
namespace nes {

/**
 * Instantiate a cartridge.
 * @param fname Cartridge file name (.nes).
 * @return A pointer to the instantiated cartridge.
 * @exception InvalidCartridge
 * @see iNES::load_header(const fs::Path& fname)
 * @see Cartridge
 */
sptr_t<class Cartridge> make_cartridge(const fs::Path& fname);

/**
 * NES Cartridge.
 * The NES Cartridge class is the base for all mappers.
 * It embeds two devices into one: One device accessed
 * by the CPU and another device accessed by the PPU.
 *
 *     Cartridge Address  Accessed by  Mapped at CPU/PPU Address
 *     ---------------------------------------------------------
 *     0000-BFFF          CPU          4000-FFFF
 *     C000-EFFF          PPU          0000-2FFF
 *
 * ### Address range accessed by the CPU:
 *
 *     Mapper Address  CPU Address  Size  Descripton
 *     --------------------------------------------------------
 *     2000-3FFF       6000-7FFF    8K    PRG RAM (optional)
 *     4000-7FFF       8000-BFFF    16K   16K or 32K PRG ROM
 *     8000-BFFF       C000-FFFF    16K   32K PRG ROM high part
 *
 * ### Address range accessed by the PPU:
 *
 *     Mapper Address  PPU Address  Size  Description
 *     -----------------------------------------------------------
 *     C000-CFFF       0000-0FFF    4K    CHR ROM, Pattern table 0
 *     D000-DFFF       1000-1FFF    4K    CHR ROM, Pattern table 1
 *     E000-E3FF       2000-23FF    1K    VRAM, Nametable 0
 *     E400-E7FF       2400-27FF    1K    VRAM, Nametable 1
 *     E800-EBFF       2800-2BFF    1K    VRAM, Nametable 2
 *     EC00-EFFF       2C00-2FFF    1K    VRAM, Nametable 3
 *     F000-F3FF       3000-33FF    1K    Mirror of 2000-23FF
 *     F400-F7FF       3400-37FF    1K    Mirror of 2400-27FF
 *     F800-FBFF       3800-3BFF    1K    Mirror of 2800-2BFF
 *     FC00-FFFF       3800-3FFF    1K    Mirror of 2C00-2FFF
 *
 * ### Implementation Details:
 *
 *     Device   CPU/PPU Address  Size  Banks  Bank Size
 *     ------------------------------------------------
 *     PRG RAM  6000-7FFF        8K    8      1K
 *     PRG ROM  8000-FFFF        32K   4      8K
 *     CHR      0000-1FFF        8K    8      1K
 *     VRAM     2000-2FFF        4K    4      1K
 *
 * @see https://www.nesdev.org/wiki/Mapper
 */
class Cartridge : public Device {
public:
    constexpr static const addr_t CPU_OFFSET        = 0x0000;
    constexpr static const addr_t PPU_OFFSET        = 0xC000;
    constexpr static const size_t CPU_SIZE          = 0xC000;
    constexpr static const size_t PPU_SIZE          = 0x3000;

    constexpr static const size_t DEFAULT_RAM_SIZE  = 8192;
    constexpr static const size_t RAM_BANK_SIZE     = 1024;
    constexpr static const size_t RAM_BANK_MASK     = RAM_BANK_SIZE - 1;
    constexpr static const size_t RAM_BANK_SHIFT    = 10;
    constexpr static const size_t RAM_BANKS         = 8;                    /* 8x 1K banks  */

    constexpr static const size_t PRG_ASPACE_SIZE   = 32768;
    constexpr static const size_t PRG_BANK_SIZE     = 8192;
    constexpr static const size_t PRG_BANK_MASK     = PRG_BANK_SIZE - 1;
    constexpr static const size_t PRG_BANK_SHIFT    = 13;
    constexpr static const size_t PRG_BANKS         = 4;                    /* 4x 8K banks  */

    constexpr static const size_t VRAM_SIZE         = 2 * nes::VRAM_SIZE;
    constexpr static const size_t VRAM_BANK_SIZE    = 1024;
    constexpr static const size_t VRAM_BANK_MASK    = VRAM_BANK_SIZE - 1;
    constexpr static const size_t VRAM_BANK_SHIFT   = 10;
    constexpr static const size_t VRAM_BANKS        = 4;                    /* 4x 1K banks  */

    constexpr static const size_t CHR_RAM_SIZE      = 8192;
    constexpr static const size_t CHR_BANK_SIZE     = 1024;
    constexpr static const size_t CHR_BANK_MASK     = CHR_BANK_SIZE - 1;
    constexpr static const size_t CHR_BANK_SHIFT    = 10;
    constexpr static const size_t CHR_BANKS         = 8;                    /* 8x 1K banks  */

    constexpr static const size_t RAM_BASE_ADDR     = 0x2000;               /* PRG RAM base address $6000     */
    constexpr static const size_t PRG_BASE_ADDR     = 0x4000;               /* PRG ROM base address $8000     */
    constexpr static const size_t VRAM_BASE_ADDR    = 0x2000;
    constexpr static const size_t VRAM_END_ADDR     = 0x3000;
    constexpr static const size_t CHR_BASE_ADDR     = 0x0000;               /* CHR ROM/RAM base address $0000 */

    constexpr static const size_t VRAM_MASK         = (VRAM_END_ADDR - VRAM_BASE_ADDR) - 1;

    enum class MirrorType {
        OneScreenLower  = 0,
        OneScreenUpper  = 1,
        Vertical        = 2,
        Horizontal      = 3,
        FourScreen      = 4
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

    virtual ~Cartridge();

    /**
     * Get the signature of this cartridge.
     * If the signature of this cartridge is empty, compute it.
     * @return The signature of this cartridge as a string.
     * @see _signature
     */
    virtual std::string signature() const;

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

    /**
     * Set the IRQ output pin callback.
     * @param cb IRQ output pin callback.
     */
    void irq(const OutputPinCb& cb);

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

    /**
     * Apply mirroring to VRAM banks.
     * @param type Mirror type.
     */
    void vram_mirror(MirrorType type);

    /**
     * Read from PRG memory.
     * @param addr CPU memory address (relative to this device's CPU part);
     * @param mode Read mode.
     * @return The value at the specified address.
     * @see dev_read(size_t, ReadMode);
     */
    virtual uint8_t cpu_read(size_t addr, ReadMode mode);

    /**
     * Write to PRG memory.
     * @param addr  CPU memory address (relative to this device);
     * @param value Value to write.
     * @see dev_write(size_t, ReadMode);
     */
    virtual void cpu_write(size_t addr, uint8_t value);

    /**
     * Read from CHR memory.
     * @param addr PPU memory address (relative to this device);
     * @param mode Read mode.
     * @return The value at the specified address.
     * @see dev_read(size_t, ReadMode);
     */
    virtual uint8_t ppu_read(size_t addr, ReadMode mode);

    /**
     * Write to CHR memory.
     * @param addr  PPU memory address (relative to this device);
     * @param value Value to write.
     * @see dev_write(size_t, ReadMode);
     */
    virtual void ppu_write(size_t addr, uint8_t value);

    /**
     * Enable/Diable interrupt requests.
     * If interrupts are disabled any pending IRQ is automatically cleared.
     * @param enable true to enable; false to disable.
     */
    void irq_enable(bool enable);

    /**
     * Set/Clear an IRQ request.
     * If interrupts are enabled, set/clear an IRQ request.
     * @param active true to set; false to clear.
     */
    void irq_out(bool active);

    /**
     * Cartridge to CPU address.
     * @param addr Address relative to this cartridge.
     * @return Address relative to the CPU.
     */
    constexpr static const addr_t to_cpu_address(const addr_t addr)
    {
        return (addr + 0x4000);
    }

    /**
     * Cartridge to PPU address.
     * @param addr Address relative to this cartridge.
     * @return Address relative to the PPU.
     */
    constexpr static const addr_t to_ppu_address(const addr_t addr)
    {
        return (addr - PPU_OFFSET);
    }

    fs::Path        _fname;                 /* Cartridge file name              */
    iNES::Header    _hdr;                   /* Cartridge header                 */
    std::string     _signature{};           /* Cartridge signature              */
    fs::Path        _ram_fname{};           /* Persistent PRG RAM full path     */
    MirrorType      _mirror{};              /* Nametable mirroring type         */
    ChrMode         _chr_mode{};            /* CHR RAM/ROM mode                 */
    PrgMode         _prg_mode{};            /* PRG ROM mode                     */
    bool            _irq_enabled{};         /* Enable/Diable interrupt requests */
    bool            _irq_pin{};             /* State of IRQ output pin          */
    OutputPinCb     _irq_out_cb{};          /* IRQ output pin callback          */

    RAM             _ram{};
    RAMBank         _ram_banks[RAM_BANKS];

    ROM             _prg{};
    ROMBank         _prg_banks[PRG_BANKS];

    RAM             _vram{};
    RAMBank         _vram_banks[VRAM_BANKS];

    RAM             _chr{};
    RAMBank         _chr_banks[CHR_BANKS];

    friend Serializer& operator&(Serializer&, Cartridge&);
};

}
}
}
