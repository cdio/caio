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

#include "platform.hpp"
#include "ram.hpp"
#include "rom.hpp"
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
class ZXSpectrum : public Platform {
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
     * @see Platform::name()
     */
    std::string_view name() const override;

private:
    /**
     * @see Platform::detect_format(const fs::Path&)
     */
    void detect_format(const fs::Path& pname) override;

    /**
     * @see Platform::init_monitor(int, int)
     */
    void init_monitor(int ifd, int ofd) override;

    /**
     * @see Platform::reset_devices()
     */
    void reset_devices() override;

    /**
     * @see Platform::to_string_devices()
     */
    std::string to_string_devices() const override;

    /**
     * @see Platform::create_devices()
     */
    void create_devices() override;

    /**
     * @see Platform::connect_devices()
     */
    void connect_devices() override;

    /**
     * @see Platform::make_widgets()
     */
    void make_widgets() override;

    /**
     * @see Platform::connect_ui()
     */
    void connect_ui() override;

    /**
     * @see Platform::hostkeys(keyboard::Key)
     */
    void hotkeys(keyboard::Key key) override;

    /**
     * @see Platform::clock()
     */
    Clock& clock() override
    {
        return (*_clk);
    }

    /**
     * @see Platform::config()
     */
    const Config& config() const override
    {
        return _conf;
    }

    /**
     * @see Platform::ui_config()
     */
    ui::Config ui_config() override;

    /**
     * Reset this ZX-Spectrum with values from a snapshot file.
     * This method is used to launch a snapshot and it must be called
     * after all devices are successfuly created.
     * @param snap Snapshot to launch.
     * @see Snapshot
     */
    void reset(const Snapshot& snap);

    /**
     * Return the full pathname of a ROM file.
     * @param fname ROM file name.
     * @return The full pathname.
     * @exception IOError if the ROM file is not found.
     */
    fs::Path rompath(const fs::Path& fname) const;

    /**
     * Load a snapshot file.
     * If a snapshot is specified in the configuration, load it.
     * @exception IOError if the specified file cannot be loaded.
     */
    void attach_prg();

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
};

}
}
}
