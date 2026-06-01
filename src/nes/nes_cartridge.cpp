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

namespace caio {
namespace nintendo {
namespace nes {

Cartridge::Cartridge(std::string_view type, const fs::Path& fname, const iNES::Header& hdr, std::ifstream& is)
    : Device{type, fname.stem().string()},
      _fname{fname},
      _hdr{hdr}
{
    const size_t ram_size = hdr.prg_ram_size();
    const size_t prg_size = hdr.prg_size();
    const size_t chr_size = hdr.chr_size();

    if (ram_size != 0 && (ram_size & RAM_BANK_MASK) != 0)  {
        throw InvalidCartridge{"{}: Invalid PRG RAM size: {}. It must be a multiple of {}",
            fname.c_str(), ram_size, RAM_BANK_SIZE};
    }

    if (prg_size < PRG_BANK_SIZE || (prg_size & PRG_BANK_MASK) != 0) {
        throw InvalidCartridge{"{}: Invalid PRG ROM size: {}. It must be a multiple of {}",
            fname.c_str(), prg_size, PRG_BANK_SIZE};
    }

    if (chr_size != 0 && ((chr_size < CHR_BANK_SIZE) || (chr_size & CHR_BANK_MASK) != 0)) {
        throw InvalidCartridge{"{}: Invalid CHR ROM size: {}. Minimum size: {}. It must be a multiple of {}",
            fname.c_str(), chr_size, CHR_RAM_SIZE, CHR_BANK_SIZE};
    }

    const size_t rsize = (ram_size == 0 ? DEFAULT_RAM_SIZE : ram_size);

    if (_hdr.persistent_ram()) {
        /*
         * Load previously saved data from a persistent RAM.
         */
        _ram_fname = std::format("{}/{}.ram", config::storage_path().c_str(), fs::sha256(fname));
        if (fs::exists(_ram_fname)) {
            log.debug("{}: Loading PRG RAM data: {}, size: {}\n", Name::to_string(), _ram_fname.c_str(), rsize);
            _ram = RAM{_ram_fname.string(), _ram_fname, rsize};
        }
    }

    if (_ram.size() == 0) {
        log.debug("{}: Initializing PRG RAM, size: {}\n", Name::to_string(), rsize);
        _ram = RAM{"ram", rsize};
    }

    log.debug("{}: Loading PRG ROM, size: {}\n", Name::to_string(), prg_size);
    _prg = ROM{"prg", is, prg_size};

    log.debug("{}: Initializing VRAM, size: {}\n", Name::to_string(), VRAM_SIZE);
    _vram = RAM{"vram", VRAM_SIZE};

    if (chr_size == 0) {
        log.debug("{}: Initializing CHR RAM, size: {}\n", Name::to_string(), CHR_RAM_SIZE);
        _chr = RAM{"chr", CHR_RAM_SIZE};
    } else {
        log.debug("{}: Loading CHR ROM, size: {}\n", Name::to_string(), chr_size);
        _chr = ROM{"chr", is, chr_size};
    }

    signature();

    Cartridge::reset();
}

Cartridge::~Cartridge()
{
    if (!_ram_fname.empty()) {
        try {
            fs::save(_ram_fname, _ram.buffer());
        } catch (const std::exception& err) {
            log.error("{}: Can't save RAM content: {}: {}\n", Name::to_string(), _ram_fname.string(), err.what());
        }
    }
}

void Cartridge::reset()
{
    _ram_banks[0] = RAMBank{_ram, RAM_BANK_SIZE, 0};
    _ram_banks[1] = RAMBank{_ram, RAM_BANK_SIZE, 1};
    _ram_banks[2] = RAMBank{_ram, RAM_BANK_SIZE, 2};
    _ram_banks[3] = RAMBank{_ram, RAM_BANK_SIZE, 3};
    _ram_banks[4] = RAMBank{_ram, RAM_BANK_SIZE, 4};
    _ram_banks[5] = RAMBank{_ram, RAM_BANK_SIZE, 5};
    _ram_banks[6] = RAMBank{_ram, RAM_BANK_SIZE, 6};
    _ram_banks[7] = RAMBank{_ram, RAM_BANK_SIZE, 7};

    _prg_mode = PrgMode::Fixed_C000;
    const size_t banks = _prg.size() / PRG_BANK_SIZE;
    _prg_banks[0] = ROMBank{_prg, PRG_BANK_SIZE, 0};
    _prg_banks[1] = ROMBank{_prg, PRG_BANK_SIZE, 1};
    _prg_banks[2] = ROMBank{_prg, PRG_BANK_SIZE, banks - 2};
    _prg_banks[3] = ROMBank{_prg, PRG_BANK_SIZE, banks - 1};

    _vram_banks[0] = RAMBank{_vram, VRAM_BANK_SIZE, 0};
    _vram_banks[1] = RAMBank{_vram, VRAM_BANK_SIZE, 1};
    _vram_banks[2] = RAMBank{_vram, VRAM_BANK_SIZE, 2};
    _vram_banks[3] = RAMBank{_vram, VRAM_BANK_SIZE, 3};

    const MirrorType mirror = (_hdr.alternative_nametable() ? MirrorType::FourScreen :
        (_hdr.vertical_mirror() ? MirrorType::Vertical : MirrorType::Horizontal));

    log.debug("{}: Applying VRAM mirror type: {}\n", Name::to_string(), static_cast<int>(mirror));
    vram_mirror(mirror);

    _chr_mode = ChrMode::Mode_8K;
    _chr_banks[0] = RAMBank{_chr, CHR_BANK_SIZE, 0};
    _chr_banks[1] = RAMBank{_chr, CHR_BANK_SIZE, 1};
    _chr_banks[2] = RAMBank{_chr, CHR_BANK_SIZE, 2};
    _chr_banks[3] = RAMBank{_chr, CHR_BANK_SIZE, 3};
    _chr_banks[4] = RAMBank{_chr, CHR_BANK_SIZE, 4};
    _chr_banks[5] = RAMBank{_chr, CHR_BANK_SIZE, 5};
    _chr_banks[6] = RAMBank{_chr, CHR_BANK_SIZE, 6};
    _chr_banks[7] = RAMBank{_chr, CHR_BANK_SIZE, 7};
}

size_t Cartridge::size() const
{
    return (_ram.size() + _prg.size() + _vram.size() + _chr.size());
}

std::string Cartridge::to_string() const
{
    return std::format("{}: {}", Name::to_string(), iNES::to_string(_hdr));
}

std::string Cartridge::signature() const
{
    if (_signature.empty()) {
        const auto concat = iNES::signature(_hdr) + _prg.signature() +
            (_hdr.chr_size() == 0 ? "" : static_cast<const ROM&>(_chr).signature());

        const auto buf = std::span<const uint8_t>{reinterpret_cast<const uint8_t*>(concat.data()), concat.size()};
        const_cast<std::string&>(_signature) = utils::sha256(buf);
    }

    return _signature;
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

uint8_t Cartridge::cpu_read(size_t addr, ReadMode mode)
{
    if (addr < RAM_BASE_ADDR) {
        /*
         * Unmapped area: 0000-1FFF (CPU 4000-5FFF).
         */
        return 0;
    }

    if (addr < PRG_BASE_ADDR) {
        /*
         * RAM access: 2000-3FFF (CPU 6000-7FFF).
         */
        const size_t raddr = addr - RAM_BASE_ADDR;
        const size_t rbank = raddr >> RAM_BANK_SHIFT;
        const size_t rbase = raddr & RAM_BANK_MASK;
        return _ram_banks[rbank].read(rbase, mode);
    }

    /*
     * PRG access: 4000-BFFF (CPU 8000-FFFF)
     */
    const size_t raddr = addr - PRG_BASE_ADDR;
    const size_t rbank = raddr >> PRG_BANK_SHIFT;
    const size_t rbase = raddr & PRG_BANK_MASK;
    return _prg_banks[rbank].read(rbase, mode);
}

void Cartridge::cpu_write(size_t addr, uint8_t value)
{
    if (addr < RAM_BASE_ADDR) {
        /*
         * Unmapped area: 0000-1FFF (CPU 4000-5FFF).
         */
        return;
    }

    if (addr < PRG_BASE_ADDR) {
        /*
         * RAM access: 2000-3FFF (CPU 6000-7FFF).
         */
        const size_t raddr = addr - RAM_BASE_ADDR;
        const size_t rbank = raddr >> RAM_BANK_SHIFT;
        const size_t rbase = raddr & RAM_BANK_MASK;
        _ram_banks[rbank].write(rbase, value);
    }
}

uint8_t Cartridge::ppu_read(size_t addr, ReadMode mode)
{
    if (addr < VRAM_BASE_ADDR) {
        /*
         * CHR access: PPU 0000-1FFF.
         */
        const size_t raddr = addr - CHR_BASE_ADDR;
        const size_t rbank = raddr >> CHR_BANK_SHIFT;
        const size_t rbase = raddr & CHR_BANK_MASK;
        return _chr_banks[rbank].read(rbase, mode);
    }

    /*
     * VRAM access: PPU 2000-3FFF.
     */
    const size_t raddr = (addr - VRAM_BASE_ADDR) & VRAM_MASK;
    const size_t rbank = raddr >> VRAM_BANK_SHIFT;
    const size_t rbase = raddr & VRAM_BANK_MASK;
    return _vram_banks[rbank].read(rbase, mode);
}

void Cartridge::ppu_write(size_t addr, uint8_t value)
{
    if (addr < VRAM_BASE_ADDR) {
        /*
         * CHR access: PPU 0000-1FFF.
         */
        const size_t raddr = addr - CHR_BASE_ADDR;
        const size_t rbank = raddr >> CHR_BANK_SHIFT;
        const size_t rbase = raddr & CHR_BANK_MASK;
        _chr_banks[rbank].write(rbase, value);

    } else {
        /*
         * VRAM access: PPU 2000-3FFF.
         */
        const size_t raddr = (addr - VRAM_BASE_ADDR) & VRAM_MASK;
        const size_t rbank = raddr >> VRAM_BANK_SHIFT;
        const size_t rbase = raddr & VRAM_BANK_MASK;
        return _vram_banks[rbank].write(rbase, value);
    }
}

void Cartridge::vram_mirror(MirrorType type)
{
    _mirror = type;

    switch (_mirror) {
    case MirrorType::OneScreenLower:
        /*
         * One-screen, lower bank.
         * All mirror 2000-23FF.
         */
        _vram_banks[0].bank(0);
        _vram_banks[1].bank(0);
        _vram_banks[2].bank(0);
        _vram_banks[3].bank(0);
        break;

    case MirrorType::OneScreenUpper:
        /*
         * One-screen, upper bank.
         * All mirror 2400-27FF.
         */
        _vram_banks[0].bank(1);
        _vram_banks[1].bank(1);
        _vram_banks[2].bank(1);
        _vram_banks[3].bank(1);
        break;

    case MirrorType::Vertical:
        /*
         * Horizontal arrangement (Vertical mirroring).
         * 2000 2400
         *   |   |
         *   v   v
         * 2800 2C00
         *
         * 20xx = 28xx
         * 24xx = 2Cxx
         *
         * Physical 2000 mirrored to logical 2800
         * Physical 2400 mirrored to logical 2C00
         */
        _vram_banks[0].bank(0);
        _vram_banks[1].bank(1);
        _vram_banks[2].bank(0);
        _vram_banks[3].bank(1);
        break;

    case MirrorType::Horizontal:
        /*
         * Vertical arrangement (Horizontal mirroring).
         * 2000 -> 2400
         * 2800 -> 2C00
         *
         * 20xx = 24xx
         * 28xx = 2Cxx
         *
         * Physical 2400 must be moved to logical 2800:
         * Access to logical 2000 => Nothing to do
         * Access to logical 2400 => Clear A10 becoming physical 2000
         * Access to logical 2800 => Clear A11 and set A10 becoming physical 2400
         * Access to logical 2C00 => Clear A11 (A10 already set) becoming physical 2400
         */
        _vram_banks[0].bank(0);
        _vram_banks[1].bank(0);
        _vram_banks[2].bank(1);
        _vram_banks[3].bank(1);
        break;

    case MirrorType::FourScreen:
        /*
         * Four-Screen mirroring:
         * Each page has its own data.
         */
        _vram_banks[0].bank(0);
        _vram_banks[1].bank(1);
        _vram_banks[2].bank(2);
        _vram_banks[3].bank(3);
        break;
    }
}

void Cartridge::irq(const OutputPinCb& cb)
{
    _irq_out_cb = cb;
}

void Cartridge::irq_enable(bool enable)
{
    if (!enable) {
        irq_out(false);
    }

    _irq_enabled = enable;
}

void Cartridge::irq_out(bool active)
{
    if (_irq_enabled && _irq_pin != active) {
        _irq_pin = active;
        if (_irq_out_cb) {
            _irq_out_cb(_irq_pin);
        }
    }
}

Serializer& operator&(Serializer& ser, Cartridge& cart)
{
    std::string sign{cart.signature()};

    ser & static_cast<Device&>(cart)
        & sign
        & cart._mirror
        & cart._chr_mode
        & cart._prg_mode
        & cart._ram
        & cart._prg
        & cart._vram
        & cart._chr
        & cart._ram_banks[0]
        & cart._ram_banks[1]
        & cart._ram_banks[2]
        & cart._ram_banks[3]
        & cart._ram_banks[4]
        & cart._ram_banks[5]
        & cart._ram_banks[6]
        & cart._ram_banks[7]
        & cart._prg_banks[0]
        & cart._prg_banks[1]
        & cart._prg_banks[2]
        & cart._prg_banks[3]
        & cart._chr_banks[0]
        & cart._chr_banks[1]
        & cart._chr_banks[2]
        & cart._chr_banks[3]
        & cart._chr_banks[4]
        & cart._chr_banks[5]
        & cart._chr_banks[6]
        & cart._chr_banks[7]
        & cart._irq_enabled
        & cart._irq_pin;

    if (ser.is_deserializer() && sign != cart.signature()) {
        throw IOError{"Invalid cartridge signature"};
    }

    return ser;
}

}
}
}
