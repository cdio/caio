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

#include "nibble_ram.hpp"
#include "ram.hpp"
#include "rom.hpp"

#include "mos_6510.hpp"
#include "mos_6526.hpp"
#include "mos_6569.hpp"
#include "mos_6581.hpp"

#include "c1541_factory.hpp"
#include "c64_bus_controller.hpp"
#include "c64_cartridge.hpp"
#include "c64_config.hpp"
#include "c64_io.hpp"
#include "c64_joystick.hpp"
#include "c64_keyboard.hpp"
#include "c64_params.hpp"
#include "c64_pla.hpp"
#include "c64_vic2_aspace.hpp"

namespace caio {
namespace commodore {
namespace c64 {

using C1541 = commodore::c1541::C1541;

/**
 * Commodore 64 emulator.
 */
class C64 : public Platform {
public:
    constexpr static const char* LABEL = "C64";

    /**
     * Instantiate this C64.
     * This method only sets the configuration parameters.
     * Call the run() method to build and start the actual C64 emulator.
     * @param sec Configuration section.
     * @see C64Config
     * @see run()
     */
    C64(config::Section& sec);

private:
    /**
     * Detect the format of a file.
     * If the specified file is a snapshot image, a CRT file, or
     * a PRG file, set the proper configuration option accordingly.
     * @param fname File to detect.
     * @return true if the specified file is valid; false otherwise.
     * @see Platform::detect_format(const fs::Path&)
     */
    bool detect_format(const fs::Path& fname) override;

    /**
     * @see Platform::init_monitor(int, int)
     */
    void init_monitor(int ifd, int ofd) override;

    /**
     * @see Platform::connect_ui()
     */
    void connect_ui() override;

    /**
     * @see Platform::create_devices()
     */
    void create_devices() override;

    /**
     * @see Platform::connect_devices()
     */
    void connect_devices() override;

    /**
     * @see Platform::reset_devices()
     */
    void reset_devices() override;

    /**
     * @see Platform::to_string_devices()
     */
    std::string to_string_devices() const override;

    /**
     * @see Platform::make_widgets()
     */
    void make_widgets() override;

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
    Config& config() override
    {
        return _conf;
    }

    /**
     * @see Platform::ui_config()
     */
    ui::Config ui_config() override;

    /**
     * Get the full pathname for a ROM file.
     * @param fname ROM file name.
     * @return The full path.
     * @exception IOError if the ROM file is not found.
     */
    fs::Path rompath(const fs::Path& fname);

    /**
     * Attach a cartridge image file.
     * Load a .crt image and associate it to an I/O expansion device.
     * @return An I/O expansion device (Cartridge) attached to the specified image;
     * nullptr if the cartridge is not specified in the configuration.
     * @exception InvalidCartridge if the specified file is not recognised as a cartridge.
     * @exception IOError if the specified file does not exist or cannot be opened.
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
     * @see Platform::serdes(Serializer&)
     */
    void serdes(Serializer& ser) override;

    C64Config                   _conf;
    sptr_t<Clock>               _clk{};
    sptr_t<RAM>                 _ram{};
    sptr_t<ROM>                 _basic{};
    sptr_t<ROM>                 _kernal{};
    sptr_t<ROM>                 _chargen{};
    sptr_t<NibbleRAM>           _vram{};
    sptr_t<C64IO>               _io{};
    sptr_t<PLA>                 _pla{};
    sptr_t<Mos6510>             _cpu{};
    sptr_t<Vic2ASpace>          _vic2_mmap{};
    sptr_t<Mos6569>             _vic2{};
    sptr_t<Mos6581>             _sid{};
    sptr_t<Mos6526>             _cia1{};
    sptr_t<Mos6526>             _cia2{};
    sptr_t<Cartridge>           _ioexp{};
    sptr_t<cbm_bus::Bus>        _bus{};
    sptr_t<C64BusController>    _busdev{};
    sptr_t<C1541>               _unit8{};
    sptr_t<C1541>               _unit9{};
    sptr_t<C64Keyboard>         _kbd{};
    sptr_t<Joystick>            _joy1{};
    sptr_t<Joystick>            _joy2{};
    sptr_t<ui::widget::Floppy>  _floppy8{};
    sptr_t<ui::widget::Floppy>  _floppy9{};
    sptr_t<ui::widget::Gamepad> _gamepad1{};
    sptr_t<ui::widget::Gamepad> _gamepad2{};

    friend Serializer& operator&(Serializer&, C64&);
};

}
}
}
