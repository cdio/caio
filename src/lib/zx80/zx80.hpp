/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "device_ram.hpp"
#include "device_rom.hpp"
#include "ui.hpp"
#include "zilog_z80.hpp"

#include "zx80_aspace.hpp"
#include "zx80_config.hpp"
#include "zx80_keyboard.hpp"


namespace caio {

/* XXX TODO move to pal.hpp, make 6569 use it too */
namespace pal {
    constexpr static unsigned FRAME_WIDTH    = 504;
    constexpr static unsigned FRAME_HEIGHT   = 312;

    constexpr static unsigned VISIBLE_WIDTH  = 403;
    constexpr static unsigned VISIBLE_HEIGHT = 284;
};

namespace zx80 {

constexpr static const char *ROM_FNAME     = "zx80_rom.bin";
constexpr static const size_t ROM_SIZE     = 4096;
constexpr static const size_t RAM_SIZE     = 1024;
constexpr static const unsigned CLOCK_FREQ = 3250000;

/**
 * Sinclair ZX-80 emulator.
 */
class ZX80 {
public:
    constexpr static const uint64_t RAM_INIT_PATTERN = 0x0000000000000000ULL;

    /**
     * Instantiate this ZX80.
     * This method only sets the specified configuration parameters.
     * Call the run() method to build and start the actual ZX80 emulator.
     * @param conf Configuration parameters.
     * @see ZX80Config
     * @see run()
     */
    ZX80(const ZX80Config &conf);

    virtual ~ZX80();

    /**
     * Build a ZX80 emulator and start it.
     * This method returns on error or when the user terminates the emulator through the UI.
     * @see start()
     */
    void run();

    /**
     * @return A human-readable string representation of this ZX80.
     */
    std::string to_string() const;

    /**
     * @return The name of this platform.
     */
    static std::string name();

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
     * Connect the devices and build a ZX80.
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
     * Get the full pathname for a ROM file.
     * @param fname ROM file name.
     * @return The full path.
     * @exception IOError if the ROM file is not found.
     */
    std::string rompath(const std::string &fname) const;

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
     * Initialise RAM memory using a specific pattern.
     * @param pattern Initialisation patter;
     * @param data    RAM to initialise.
     */
    void ram_init(uint64_t pattern, std::vector<uint64_t> &data);

    /**
     * Process hot-keys.
     * This method is indirectly called by the user interface.
     */
    void hotkeys(Keyboard::Key key);

    ZX80Config                    _conf{};
    devptr_t                      _ram{};
    devptr_t                      _rom{};
    std::shared_ptr<ZX80ASpace>   _mmap{};
    std::shared_ptr<ZilogZ80>     _cpu{};
    std::shared_ptr<Clock>        _clk{};
    std::shared_ptr<ZX80Keyboard> _kbd{};
    std::shared_ptr<ui::UI>       _ui{};
};

}
}
