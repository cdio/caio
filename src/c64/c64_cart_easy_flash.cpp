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
#include "c64_cart_easy_flash.hpp"

#include <algorithm>
#include <sstream>

#include "device_none.hpp"
#include "logger.hpp"
#include "utils.hpp"

//#define CART_EASY_FLASH

#ifdef CART_EASY_FLASH
#define DEBUG(args...)      log.debug(args)
#else
#define DEBUG(args...)
#endif

namespace caio {
namespace commodore {
namespace c64 {

/*
 * Easy Flash Cartridge.
 * 1M divided in 64 banks of 2 * 8K each.
 *
 * Type     Size    Game    EXROM   ROML    ROMH    LOAD ADDRESS
 * ----------------------------------------------------------------------------
 *          1024K   0       1       $8000   $A000   $8000-$9FFF and $A000-$BFFF
 *                                  $8000   $E000   $8000-$9FFF and $E000-$FFFF
 *
 * EasyFlash is a 1M flash memory plus 256 bytes of RAM (mapped into the I/O-2 range).
 * Control register 1 ($DE00): Bank switching.
 * Control register 2 ($DE02): EasyFlash control:
 *      Bit     Name    Content
 *      -------------------------------------------------------------------------
 *      7       L       LED (1: LED on, 0: LED off)
 *      6..3    0       Reserved (must be 0)
 *      2       M       GAME mode (1: Controlled by bit G, 0: From jumper "boot")
 *      1       X       EXROM state (0: /EXROM high)
 *      0       G       GAME state (if M = 1, 0 = /GAME high)
 *
 * @see https://skoe.de/easyflash/files/devdocs/EasyFlash-ProgRef.pdf
 */
void CartEasyFlash::reset()
{
    Cartridge::reset();

    _bank = 0;
    _romhs = 0;
    _romls = 0;
    _reg2 = 0;
    _ram = {};
    _roms_lo.fill(devptr_t{});
    _roms_hi.fill(devptr_t{});

    /*
     * Load ROMs and RAM.
     */
    const auto& cart = crt();
    for (size_t entry = 0; entry < cart.chips(); ++entry) {
        const auto chipdev = cart[entry];
        const Crt::Chip& chip = chipdev.first;
        const devptr_t& dev = chipdev.second;

        switch (chip.type) {
        case Crt::CHIP_TYPE_ROM:
        case Crt::CHIP_TYPE_FLASH:
            add_rom(entry, chip, dev);
            break;

        case Crt::CHIP_TYPE_RAM:
            add_ram(entry, chip, dev);
            break;

        case Crt::CHIP_TYPE_EEPROM:     /* TODO: File on user's config directory */
            throw_invalid_cartridge(entry, "type EEPROM");

        default:
            throw_invalid_cartridge(entry, "Unrecognised chip type {}", chip.type);
        }
    }

    /*
     * Propagate GAME and EXROM output pins.
     */
    propagate();
}

void CartEasyFlash::add_rom(size_t entry, const Crt::Chip& chip, const devptr_t& rom)
{
    if (chip.rsiz != ROM_SIZE) {
        throw_invalid_cartridge(entry, "Invalid ROM size {}", chip.rsiz);
    }

    if (chip.bank > MAX_BANKS) {
        throw_invalid_cartridge(entry, "Invalid bank {}", chip.bank);
    }

    switch (chip.addr) {
    case ROML_LOAD_ADDR:
        _roms_lo[chip.bank] = rom;
        ++_romls;
        break;

    case ROMH_LOAD_ADDR_1:
    case ROMH_LOAD_ADDR_2:
        _roms_hi[chip.bank] = rom;
        ++_romhs;
        break;

    default:
        throw_invalid_cartridge(entry, "Invalid load address ${}", utils::to_string(chip.addr));

    }

    DEBUG("{}({}): Chip {}: ROM device: {}\n", type(), name(), entry, Crt::to_string(chip));
}

void CartEasyFlash::add_ram(size_t entry, const Crt::Chip& chip, const devptr_t& ram)
{
    _ram = ram;
    DEBUG("{}({}): Chip {}: RAM device: {}\n", type(), name(), entry, Crt::to_string(chip));
}

uint8_t CartEasyFlash::dev_read(addr_t addr, ReadMode mode)
{
    /*
     * 256 bytes of RAM mapped into the I/O-2 range.
     * Control register 1 ($DE00): Bank switching.
     * Control register 2 ($DE02): EasyFlash control:
     *      Bit     Name    Content
     *      -------------------------------------------------------------------------
     *      7       L       LED (1: LED on, 0: LED off)
     *      6..3    0       Reserved (must be 0)
     *      2       M       GAME mode (1: Controlled by bit G, 0: From jumper "boot")
     *      1       X       EXROM state (0: /EXROM high)
     *      0       G       GAME state (if M = 1, 0 = /GAME high)
     */
    addr &= IO_ADDR_MASK;
    if (addr == 0x0000) {
        /*
         * I/O-1 ($DE00-$DEFF)
         */
        switch (addr & 0x0002) {
        case 0x0000:
            /*
             * Control register 1 ($DE00): Bank switching.
             */
            return static_cast<uint8_t>(_bank);

        default:
            /*
             * Control register 2 ($DE02): EasyFlash Control.
             */
            return _reg2;
        }
    }

    /*
     * I/O-2 ($DF00-$DFFF): Cartridge RAM (if present).
     */
    if (_ram && addr > 0x00FF && addr < 0x0200) {
        return _ram->read(addr - 256, mode);
    }

    return 255;
}

void CartEasyFlash::dev_write(addr_t addr, uint8_t data)
{
    /*
     * 256 bytes of RAM mapped into the I/O-2 range.
     * Control register 1 ($DE00): Bank switching.
     * Control register 2 ($DE02): EasyFlash control:
     *      Bit     Name    Content
     *      -------------------------------------------------------------------------
     *      7       L       LED (1: LED on, 0: LED off)
     *      6..3    0       Reserved (must be 0)
     *      2       M       GAME mode (1: Controlled by bit G, 0: From jumper "boot")
     *      1       X       EXROM state (0: /EXROM high)
     *      0       G       GAME state (if M = 1, 0 = /GAME high)
     */
    if (addr < 256) {
        /*
         * I/O-1 ($DE00-$DEFF).
         */
        addr &= 2;
        if (addr == 0x0000) {
            /*
             * Control register 1 ($DE00): Bank switching.
             */
            uint8_t bank = data & BANK_MASK;
            if (bank != _bank) {
                _bank = bank;

                /*
                 * Force the propagation of GAME/EXROM output pins.
                 * This makes the connected devices to update their internal status
                 * even if the GAME/EXROM pins are not changed (this case).
                 */
                propagate(true);
            }

        } else if (addr == 0x0002) {
            /*
             * Control register 2 ($DE02): EasyFlash Control:
             *   MXG    Configuration
             * --------------------------------------------------------------
             * 0 000    GAME from jumper, EXROM high (i.e. Ultimax or Off)
             * 1 001    Reserved, don’t use this
             * 2 010    GAME from jumper, EXROM low (i.e. 16K or 8K)
             * 3 011    Reserved, don’t use this
             * 4 100    Cartridge ROM off (RAM at $DF00 still available)        <= MODE_INVISIBLE
             * 5 101    Ultimax (Low bank at $8000, high bank at $E000)         <= MODE_ULTIMAX
             * 6 110    8k Cartridge (Low bank at $8000)                        <= MODE_8K
             * 7 111    16k cartridge (Low bank at $8000, high bank at $A000)   <= MODE_16K
             */
            _reg2 = data & 7;
            switch (_reg2) {
            case 4:
                mode(MODE_INVISIBLE);
                break;

            case 5:
                mode(MODE_ULTIMAX);
                break;

            case 6:
                mode(MODE_8K);
                break;

            case 7:
                mode(MODE_16K);
                break;

            default:;
            }
        }

    } else {
        /*
         * I/O-2 ($DF00-$DFFF).
         */
        if (_ram && addr > 255 && addr < 512) {
            _ram->write(addr - 256, data);
        }
    }
}

std::string CartEasyFlash::to_string() const
{
    std::ostringstream os{};
    size_t total = _romls + _romhs;
    size_t size = total * ROM_SIZE;

    os << Name::to_string() << ", " << total << " roms, total rom size " << size << " (" << (size >> 10) << "K)";

    for (const auto &rom : _roms_lo) {
        if (rom) {
            os << ", " << rom->to_string();
        }
    }

    for (const auto &rom : _roms_hi) {
        if (rom) {
            os << ", " << rom->to_string();
        }
    }

    if (_ram) {
        os << ", " << _ram->to_string();
    }

    return os.str();
}

std::pair<ASpace::devmap_t, ASpace::devmap_t> CartEasyFlash::getdev(addr_t addr, bool romh, bool roml)
{
    /*
     * MODE_8K:         8K Cartridge (roml at $8000)
     * MODE_16K:        16K Cartridge (roml at $8000, romh at $A000)
     * MODE_ULTIMAX:    Ultimax (roml at $8000, romh at $E000)
     */
    switch (mode()) {
    case GameExromMode::MODE_8K:
        if (roml) {
            /*
             * ROML mapped at $8000-$9FFF.
             */
#if 1
            DEBUG("{}:({}): 8K ROML: {}\n", type(), name(), _roms_hi[_bank]->to_string());
#endif
            return {{_roms_lo[_bank], addr - ROML_LOAD_ADDR}, {}};
        }
        break;

    case GameExromMode::MODE_16K:
        if (roml) {
            /*
             * ROML mapped at $8000-$9FFF.
             */
#if 1
            DEBUG("{}({}): 16K ROML: {}\n", type(), name(), _roms_hi[_bank]->to_string());
#endif
            return {{_roms_lo[_bank], addr - ROML_LOAD_ADDR}, {}};
        }
        if (romh) {
            /*
             * ROMH mapped at $A000-$BFFF.
             */
#if 1
            DEBUG("{}({}): 16K ROMH: {}\n", type(), name(), _roms_hi[_bank]->to_string());
#endif
            return {{_roms_hi[_bank], addr - ROMH_LOAD_ADDR_1}, {}};
        }
        break;

    case GameExromMode::MODE_ULTIMAX:
        if (roml) {
            /*
             * ROML mapped at $8000-$9FFF.
             */
            return {{_roms_lo[_bank], addr - ROML_LOAD_ADDR}, {}};
        }
        if (romh) {
            /*
             * ROMH mapped at $E000-$FFFF.
             */
            return {{_roms_hi[_bank], addr - ROMH_LOAD_ADDR_2}, {}};
        }
        break;

    case GameExromMode::MODE_INVISIBLE:;
    }

    return {{}, {}};
}

size_t CartEasyFlash::cartsize() const
{
    return ((_romls + _romhs) * ROM_SIZE + (_ram ? _ram->size() : 0));
}

}
}
}
