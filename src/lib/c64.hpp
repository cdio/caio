/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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

#include "clock.hpp"
#include "device.hpp"
#include "device_ram.hpp"
#include "device_rom.hpp"
#include "nibble_ram.hpp"
#include "ui.hpp"

#include "mos_6510.hpp"
#include "mos_6526.hpp"
#include "mos_6569.hpp"
#include "mos_6581_i.hpp"

#include "c64_bus_controller.hpp"
#include "c64_cartridge.hpp"
#include "c64_config.hpp"
#include "c64_joystick.hpp"
#include "c64_keyboard.hpp"
#include "c64_pla.hpp"
#include "c64_vic2_aspace.hpp"


namespace caio {
namespace c64 {

constexpr static const char *KERNAL_FNAME         = "c64_kernal.901227-03.bin";
constexpr static const char *BASIC_FNAME          = "c64_basic.901226-01.bin";
constexpr static const char *CHARGEN_FNAME        = "c64_characters.901225-01.bin";

constexpr static const size_t KERNAL_SIZE         = 8192;
constexpr static const size_t BASIC_SIZE          = 8192;
constexpr static const size_t CHARGEN_SIZE        = 4096;
constexpr static const size_t VCOLOR_SIZE         = 1024;

constexpr static const addr_t BASIC_READY_ADDR    = 0xA474;     /* Basic waiting for user commands              */
constexpr static const addr_t BASIC_PRG_START     = 0x0801;     /* Start address of basic programs              */

/* See https://www.c64-wiki.com/wiki/Zeropage */
constexpr static const addr_t BASIC_TXTTAB        = 0x002B;     /* Pointer to start of basic program            */
constexpr static const addr_t BASIC_VARTAB        = 0x002D;     /* Pointer to end of basic program +1           */
constexpr static const addr_t BASIC_ARYTAB        = 0x002F;     /* Pointer to start of basic array variables    */
constexpr static const addr_t BASIC_STREND        = 0x0031;     /* Pointer to end of basic array variables      */
constexpr static const addr_t BASIC_KEYB_BUFF     = 0x0277;     /* Keyboard buffer used by basic (10 bytes)     */
constexpr static const addr_t BASIC_KEYB_BUFF_POS = 0x00C6;     /* Number of elements in the keyboard buffer    */

/**
 * The C64 emulator.
 */
class C64 {
public:
    /**
     * Initialise this C64.
     * @param conf Configuration parameters.
     * @see C64Config
     */
    C64(const C64Config &conf);

    virtual ~C64();

    /**
     * Reset and start this C64.
     * @see reset()
     * @see start()
     */
    void run();

    /**
     * @return A human-readable string representation of this instance.
     */
    std::string to_string() const;

private:
    /**
     * Get the full pathname for a ROM file.
     * @param fname ROM file name.
     * @return The full path.
     * @exception IOError if the ROM file is not found.
     */
    std::string rompath(const std::string &fname) const;

    /**
     * Get the full pathname for a cartridge file.
     * @param fname Cartridge file name.
     * @return The full path.
     * @exception IOError if the Cartridge file is not found.
     */
    std::string cartpath(const std::string &fname) const;

    /**
     * Get the full pathname for a palette file.
     * @param fname Colour table file name.
     * @return The full path; an empty string if the file is not found.
     */
    std::string palettepath(const std::string &fname) const;

    /**
     * Get the full pathname for a key mappings file.
     * @param fname Colour table file name.
     * @return The full path; an empty string if the file is not found.
     */
    std::string keymapspath(const std::string &fname) const;

    /**
     * Attach a cartridge image file.
     * Load a .crt image and associate it to an I/O expansion device.
     * @return An I/O expansion device (Cartridge) attached to the specified image;
     * nullptr if the cartridge is not specified in the configuration.
     * @exception InvalidCartridge
     * @see Cartridge
     * @see Crt
     */
    std::shared_ptr<Cartridge> attach_cartridge();

    /**
     * Attach A PRG file.
     * If a PRG file is specified in the configuration, load (inject)
     * it into the system RAM and prepare the BASIC and the CPU to run it.
     * @exception IOError if the PRG file cannot be loaded.
     */
    void attach_prg();

    /**
     * Create (instantiate) the devices that emulate the C64 hardware.
     */
    void create_devices();

    /**
     * Create the user interface based on configuration parameters.
     */
    void create_ui();

    /**
     * Create UI panel widgets and connect them to the associated devices.
     */
    void make_widgets();

    /**
     * Connect (wire) all the devices in order to build the C64 hardware.
     */
    void connect_devices();

    /**
     * Reset this C64.
     * All devices are destroyed and recreated.
     */
    void reset();

    /**
     * Start this C64.
     */
    void start();

    C64Config                         _conf{};
    bool                              _paused{};
    devptr_t                          _ram{};
    devptr_t                          _basic{};
    devptr_t                          _kernal{};
    devptr_t                          _chargen{};
    devptr_t                          _vcolor{};
    devptr_t                          _io{};
    std::shared_ptr<PLA>              _pla{};
    std::shared_ptr<Mos6510>          _cpu{};
    std::shared_ptr<Mos6569>          _vic2{};
    std::shared_ptr<Mos6581I>         _sid{};
    std::shared_ptr<Mos6526>          _cia1{};
    std::shared_ptr<Mos6526>          _cia2{};
    std::shared_ptr<Cartridge>        _ioexp{};
    std::shared_ptr<cbm_bus::Bus>     _bus{};
    std::shared_ptr<C64BusController> _busdev{};
    std::shared_ptr<cbm_bus::Device>  _unit8{};
    std::shared_ptr<cbm_bus::Device>  _unit9{};
    std::shared_ptr<Clock>            _clk{};
    std::shared_ptr<Keyboard>         _kbd{};
    std::shared_ptr<Joystick>         _joy1{};
    std::shared_ptr<Joystick>         _joy2{};
    std::shared_ptr<ui::UI>           _ui{};
};

}
}
