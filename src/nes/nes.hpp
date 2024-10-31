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
#include "ricoh_2a03.hpp"
#include "ricoh_2c02.hpp"
#include "ui.hpp"

#include "nes_aspace.hpp"
#include "nes_cartridge.hpp"
#include "nes_config.hpp"
#include "nes_joystick.hpp"
#include "nes_keyboard.hpp"
#include "nes_params.hpp"
#include "nes_ppu_aspace.hpp"

namespace caio {
namespace nintendo {
namespace nes {

/**
 * NES (Nintendo Entertainment System) emulator.
 */
class NES {
public:
    /**
     * Instantiate this NES.
     * This method only sets the configuration parameters.
     * Call the run() method to build and start the actual NES emulator.
     * @param sec Configuration section.
     * @see NESConfig
     * @see run()
     */
    NES(config::Section& sec);

    virtual ~NES();

    /**
     * Build this NES emulator and start it.
     * This method returns on error or when the user terminates the emulator through the UI.
     * @param pname If not empty, name of the program to launch (its format is auto-detected).
     * @see start()
     */
    void run(std::string_view pname);

    /**
     * Get a description of this NES.
     * @return A string representation of this NES.
     */
    std::string to_string() const;

    /**
     * Get the name of this platform.
     * @return The name of this platform.
     */
    constexpr static std::string_view name()
    {
        return "NES";
    }

private:
    /**
     * Auto-detect the format of a cartridge to launch
     * and set the configuration parameters accordingly.
     * @param pname File to launch.
     * On error a message is logged and the configuration is not changed.
     */
    void autorun(const fs::Path& pname);

    /**
     * Start this NES.
     * - Instantiate the UI and run it in the context of the calling thread.
     * - Build a NES machine and run it on its own thread.
     * This method returns on error or when the user terminates the emulator through the UI.
     */
    void start();

    /**
     * Restart this NES.
     * This method is called by the UI when the user clicks on the reset widget
     * (it runs in the context of the UI thread).
     * If the emulator is paused this method does nothing.
     */
    void reset();

    /**
     * Instantiate the devices needed by a NES.
     */
    void create_devices();

    /**
     * Connect the devices and build a NES.
     * @see create_devices()
     */
    void connect_devices();

    /**
     * Create the user interface.
     */
    void create_ui();

    /**
     * Create the user interface widgets used by the NES.
     */
    void make_widgets();

    /**
     * Connect the user interface to the NES.
     * @see create_ui()
     * @see create_devices()
     * @see make_widgets()
     */
    void connect_ui();

    /**
     * Process hot-keys.
     * This method is called by the user interface.
     */
    void hotkeys(keyboard::Key key);

    NESConfig                   _conf;
    sptr_t<Clock>               _clk{};
    sptr_t<RAM>                 _ram{};
    sptr_t<Cartridge>           _cart{};
    sptr_t<NESPPUASpace>        _ppu_mmap{};
    sptr_t<RP2C02>              _ppu{};
    sptr_t<NESASpace>           _cpu_mmap{};
    sptr_t<RP2A03>              _cpu{};
    sptr_t<NESKeyboard>         _kbd{};
    sptr_t<NESJoystick>         _joy1{};
    sptr_t<NESJoystick>         _joy2{};
    sptr_t<ui::UI>              _ui{};
    sptr_t<ui::widget::Gamepad> _gamepad1{};
    sptr_t<ui::widget::Gamepad> _gamepad2{};
    std::atomic_bool            _reset{};
};

}
}
}
