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

#include "snapshot.hpp"
#include "ula.hpp"
#include "zxsp_config.hpp"
#include "zxsp_keyboard.hpp"
#include "zxsp_tape.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/**
 * Sinclair ZX-Spectrum 48K emulator.
 */
class ZXSpectrum {
public:
    /**
     * Initialise this ZX-Spectrum.
     * This method only sets the configuration parameters.
     * The run() method must be called to build and start the actual ZX-Spectrum emulator.
     * @param sec Configuration section.
     * @see run()
     * @see ZXSpectrumConfig
     */
    ZXSpectrum(config::Section& sec);

    virtual ~ZXSpectrum();

    /**
     * Build this ZX-Spectrum emulator and start it.
     * This method returns on error or when the user terminates the emulator through the UI.
     * @param pname If not empty, name of the program to launch (its format is auto-detected).
     * @see start()
     */
    void run(const std::string& pname);

    /**
     * Return a human-readable string representation of this ZX-Spectrum.
     * @return A string representation of this ZX-Spectrum.
     */
    std::string to_string() const;

    /**
     * Return the name of this platform.
     * @return The name of this platform.
     */
    constexpr static const char* name() {
        return "Sinclair ZX-Spectrum";
    }

private:
    /**
     * Auto-detect the format of a file to launch and
     * set the configuration options accordingly.
     * @param pname File to launch.
     * @exception IOError
     */
    void autorun(const std::string& pname);

    /**
     * Start this ZX-Spectrum.
     * - Instantiate the UI and run it in the context of the calling thread.
     * - Build a ZX-Spectrum and run it on its own thread.
     * This method returns on error or when the user terminates the emulator through the UI.
     */
    void start();

    /**
     * Restart this ZX-Spectrum.
     * This method is called by the UI when the user clicks on the reset widget
     * (it runs in the context of the UI thread).
     * If the emulator is paused this method does nothing.
     */
    void reset();

    /**
     * Reset this ZX-Spectrum with values from a snapshot file.
     * This method is used to launch a snapshot and it must be called
     * after all devices are successfuly created.
     * @param snap Snapshot to launch.
     * @see Snapshot
     */
    void reset(const Snapshot& snap);

    /**
     * Instantiate the devices needed by a ZX-Spectrum.
     */
    void create_devices();

    /**
     * Connect the devices and buildup a ZX-Spectrum.
     * @see create_devices()
     */
    void connect_devices();

    /**
     * Create the user interface.
     */
    void create_ui();

    /**
     * Create the user interface widgets used by the ZX-Spectrum.
     */
    void make_widgets();

    /**
     * Connect the user interface to the ZX-Spectrum.
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
     * Load a snapshot file.
     * If a snapshot is specified in the configuration, load it.
     * @exception IOError if the specified file cannot be loaded.
     */
    void attach_prg();

    /**
     * Process hot-keys.
     * This method is indirectly called by the user interface.
     */
    void hotkeys(keyboard::Key key);

    ZXSpectrumConfig            _conf;

    std::string                 _title{};
    sptr_t<Clock>               _clk{};
    sptr_t<Z80>                 _cpu{};
    sptr_t<RAM>                 _ram{};
    sptr_t<ROM>                 _rom{};
    sptr_t<ULA>                 _ula{};
    sptr_t<ZXSpectrumKeyboard>  _kbd{};
    sptr_t<ZXSpectrumTape>      _tape{};
    sptr_t<Joystick>            _joy{};
    sptr_t<ui::UI>              _ui{};
};

}
}
}
