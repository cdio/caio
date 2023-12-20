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
#include "ram.hpp"
#include "rom.hpp"
#include "ui.hpp"
#include "zilog_z80.hpp"

#include "zx80_aspace.hpp"
#include "zx80_config.hpp"
#include "zx80_keyboard.hpp"
#include "zx80_video.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

constexpr static const addr_t ROM4_MAIN_EXEC        = 0x0283;
constexpr static const addr_t ROM8_MAIN_EXEC        = 0x0419;
constexpr static const addr_t ROM8_SLOW_FAST        = 0x0207;
constexpr static const addr_t ROM8_NEXTLINE         = 0x066C;
constexpr static const addr_t ROM8_NEXTLINE_10      = ROM8_NEXTLINE + 10;
constexpr static const addr_t ROM8_SYSVAR_ERR_NR    = 0x4000;
constexpr static const addr_t ROM8_SYSVAR_FLAGS     = 0x4001;
constexpr static const addr_t RAMTOP_16K            = 0x8000;
constexpr static const addr_t RAMTOP_1K             = 0x4000;

/**
 * Sinclair ZX80 emulator.
 */
class ZX80 {
public:
    /**
     * Instantiate this ZX80.
     * This method only sets the configuration parameters.
     * Call the run() method to build and start the actual ZX80 emulator.
     * @param sec Configuration section.
     * @see ZX80Config
     * @see run()
     */
    ZX80(config::Section& sec)
        : _conf{sec} {
    }

    /**
     * Build this ZX80 emulator and start it.
     * This method returns on error or when the user terminates the emulator through the UI.
     * @see start()
     */
    void run();

    /**
     * Return a human-readable string representation of this ZX80.
     * @return A string representation of this ZX80.
     */
    std::string to_string() const;

    /**
     * Return the name of this platform.
     * @return The name of this platform.
     */
    constexpr static const char* name() {
        return "Sinclair ZX80";
    }

private:
    /**
     * Start this ZX80.
     * - Instantiate the UI and run it in the context of the calling thread.
     * - Build a ZX80 and run it on its own thread.
     * This method returns on error or when the user terminates the emulator through the UI.
     */
    void start();

    /**
     * Restart this ZX80.
     * This method is called by the UI when the user clicks on the reset widget
     * (it runs in the context of the UI thread).
     * If the emulator is paused this method does nothing.
     */
    void reset();

    /**
     * Instantiate the devices needed by a ZX80.
     */
    void create_devices();

    /**
     * Connect the devices and buildup a ZX80.
     * @see create_devices()
     */
    void connect_devices();

    /**
     * Create the user interface.
     */
    void create_ui();

    /**
     * Create the user interface widgets used by the ZX80.
     */
    void make_widgets();

    /**
     * Connect the user interface to the ZX80.
     * @see create_ui()
     * @see create_devices()
     * @see make_widgets()
     */
    void connect_ui();

    /**
     * Return the full pathname of a ROM file.
     * @param fname ROM file name.
     * @return The full pathname.
     * @exception IOError if the ROM file is not found.
     */
    std::string rompath(const std::string& fname) const;

    /**
     * Attach a .o program file.
     * If a .o file is specified in the configuration, load (inject)
     * it into the RAM as soon as the basic is ready.
     * @exception IOError if the specified file cannot be loaded.
     */
    void attach_prg();

    /**
     * Process hot-keys.
     * This method is indirectly called by the user interface.
     */
    void hotkeys(keyboard::Key key);

    ZX80Config           _conf;
    devptr_t             _ram{};
    devptr_t             _rom{};
    sptr_t<ZX80ASpace>   _mmap{};
    sptr_t<Z80>          _cpu{};
    sptr_t<ZX80Video>    _video{};
    sptr_t<ZX80Keyboard> _kbd{};
    sptr_t<Clock>        _clk{};
    sptr_t<ui::UI>       _ui{};
};

}
}
}
