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

#include "clock.hpp"
#include "device.hpp"
#include "nibble_ram.hpp"
#include "ram.hpp"
#include "rom.hpp"
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
namespace commodore {
namespace c64 {

constexpr static const char* KERNAL_FNAME         = "c64_kernal.901227-03.bin";
constexpr static const char* KERNAL_DIGEST        = "83C60D47047D7BEAB8E5B7BF6F67F80DAA088B7A6A27DE0D7E016F6484042721";

constexpr static const char* BASIC_FNAME          = "c64_basic.901226-01.bin";
constexpr static const char* BASIC_DIGEST         = "89878CEA0A268734696DE11C4BAE593EAAA506465D2029D619C0E0CBCCDFA62D";

constexpr static const char* CHARGEN_FNAME        = "c64_characters.901225-01.bin";
constexpr static const char* CHARGEN_DIGEST       = "FD0D53B8480E86163AC98998976C72CC58D5DD8EB824ED7B829774E74213B420";

constexpr static const size_t KERNAL_SIZE         = 8192;
constexpr static const size_t BASIC_SIZE          = 8192;
constexpr static const size_t CHARGEN_SIZE        = 4096;
constexpr static const size_t VRAM_SIZE           = 1024;
constexpr static const size_t RAM_SIZE            = 65536;

constexpr static const addr_t BASIC_READY_ADDR    = 0xA474;     /* Basic waiting for user commands              */
constexpr static const addr_t BASIC_PRG_START     = 0x0801;     /* Start address of basic programs              */

/* See https://www.c64-wiki.com/wiki/Zeropage */
constexpr static const addr_t BASIC_TXTTAB        = 0x002B;     /* Pointer to start of basic program            */
constexpr static const addr_t BASIC_VARTAB        = 0x002D;     /* Pointer to end of basic program +1           */
constexpr static const addr_t BASIC_ARYTAB        = 0x002F;     /* Pointer to start of basic array variables    */
constexpr static const addr_t BASIC_STREND        = 0x0031;     /* Pointer to end of basic array variables      */
constexpr static const addr_t BASIC_KEYB_BUFF     = 0x0277;     /* Keyboard buffer used by basic (10 bytes)     */
constexpr static const addr_t BASIC_KEYB_BUFF_POS = 0x00C6;     /* Number of elements in the keyboard buffer    */

/*
 * RAM initialisation patterns.
 * See the comments here: https://csdb.dk/forums/?roomid=11&topicid=116800&showallposts=1
 */
constexpr static const uint64_t RAM_INIT_PATTERN1 = 0x00FF00FF00FF00FFULL;
constexpr static const uint64_t RAM_INIT_PATTERN2 = 0xFFFFFFFF00000000ULL;
constexpr static const uint64_t RAM_INIT_PATTERN3 = 0x0000FFFFFFFF0000ULL;

/**
 * The C64 emulator.
 */
class C64 {
public:
    /**
     * Instantiate this C64.
     * This method only sets the configuration parameters.
     * Call the run() method to build and start the actual C64 emulator.
     * @param sec Configuration section.
     * @see C64Config
     * @see run()
     */
    C64(config::Section& sec)
        : _conf{sec} {
    }

    /**
     * Build this C64 emulator and start it.
     * This method returns on error or when the user terminates the emulator through the UI.
     * @see start()
     */
    void run();

    /**
     * Get a string describing the components that build this C64.
     * @return A string representation of this C64.
     */
    std::string to_string() const;

    /**
     * Get the name of this platform.
     * @return The name of this platform.
     */
    constexpr static const char* name() {
        return "C64";
    }

private:
    /**
     * Start this C64.
     * - Instantiate the UI and run it in the context of the calling thread.
     * - Build a C64 and run it on its own thread.
     * This method returns on error or when the user terminates the emulator through the UI.
     */
    void start();

    /**
     * Restart this C64.
     * This method is called by the UI when the user clicks on the reset widget
     * (it runs in the context of the UI thread).
     * If the emulator is paused this method does nothing.
     */
    void reset();

    /**
     * Instantiate the devices needed by a C64.
     */
    void create_devices();

    /**
     * Connect the devices and build a C64.
     * @see create_devices()
     */
    void connect_devices();

    /**
     * Create the user interface.
     */
    void create_ui();

    /**
     * Create the user interface widgets used by the C64.
     */
    void make_widgets();

    /**
     * Connect the user interface to the C64.
     * @see create_ui()
     * @see create_devices()
     * @see make_widgets()
     */
    void connect_ui();

    /**
     * Get the full pathname for a ROM file.
     * @param fname ROM file name.
     * @return The full path.
     * @exception IOError if the ROM file is not found.
     */
    std::string rompath(const std::string& fname);

    /**
     * Attach a cartridge image file.
     * Load a .crt image and associate it to an I/O expansion device.
     * @return An I/O expansion device (Cartridge) attached to the specified image;
     * nullptr if the cartridge is not specified in the configuration.
     * @exception InvalidCartridge if the specified file is not recognised as a cartrdige.
     * @exception IOError if the specified file does not exists or cannot be opened.
     * @see Cartridge
     * @see Crt
     */
    sptr_t<Cartridge> attach_cartridge();

    /**
     * Attach a PRG file.
     * If a PRG file is specified in the configuration, load (inject)
     * it into the system RAM and prepare the BASIC and the CPU to run it.
     * @exception IOError if the PRG file cannot be loaded.
     */
    void attach_prg();

    /**
     * Process hot-keys.
     * This method is indirectly called by the user interface.
     */
    void hotkeys(keyboard::Key key);

    C64Config                   _conf;
    devptr_t                    _ram{};
    devptr_t                    _basic{};
    devptr_t                    _kernal{};
    devptr_t                    _chargen{};
    devptr_t                    _vram{};
    devptr_t                    _io{};
    sptr_t<PLA>                 _pla{};
    sptr_t<Mos6510>             _cpu{};
    sptr_t<Mos6569>             _vic2{};
    sptr_t<Mos6581_>            _sid{};
    sptr_t<Mos6526>             _cia1{};
    sptr_t<Mos6526>             _cia2{};
    sptr_t<Cartridge>           _ioexp{};
    sptr_t<cbm_bus::Bus>        _bus{};
    sptr_t<C64BusController>    _busdev{};
    sptr_t<cbm_bus::Device>     _unit8{};
    sptr_t<cbm_bus::Device>     _unit9{};
    sptr_t<Clock>               _clk{};
    sptr_t<C64Keyboard>         _kbd{};
    sptr_t<Joystick>            _joy1{};
    sptr_t<Joystick>            _joy2{};
    sptr_t<ui::UI>              _ui{};
};

}
}
}
