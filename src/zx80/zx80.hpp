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

#include "zx80_aspace.hpp"
#include "zx80_cassette.hpp"
#include "zx80_config.hpp"
#include "zx80_keyboard.hpp"
#include "zx80_params.hpp"
#include "zx80_video.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * Sinclair ZX-80 emulator.
 */
class ZX80 : public Platform {
public:
    /**
     * Instantiate this ZX80.
     * This method only sets the configuration parameters.
     * Call the run() method to build and start the actual ZX80 emulator.
     * @param sec Configuration section.
     * @see ZX80Config
     * @see run()
     */
    ZX80(config::Section& sec);

    virtual ~ZX80();

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
     * Return the full pathname of a ROM file.
     * @param fname ROM file name.
     * @return The full pathname.
     * @exception IOError if the ROM file is not found.
     */
    std::string rompath(std::string_view fname) const;

    /**
     * Attach a .o (4K ROM) or a .p (8K ROM) program file.
     * If a .o (.p) file is specified in the configuration, load (inject)
     * it into RAM as soon as the basic is started.
     * @exception IOError if the specified file cannot be loaded.
     */
    void attach_prg();

    ZX80Config              _conf;
    sptr_t<Clock>           _clk{};
    sptr_t<RAM>             _ram{};
    sptr_t<ROM>             _rom{};
    sptr_t<ZX80ASpace>      _mmap{};
    sptr_t<Z80>             _cpu{};
    sptr_t<ZX80Video>       _video{};
    sptr_t<ZX80Keyboard>    _kbd{};
    sptr_t<ZX80Cassette>    _cass{};
};

}
}
}
