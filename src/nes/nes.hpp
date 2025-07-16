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

#include "ricoh_2a03.hpp"
#include "ricoh_2c02.hpp"

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
class NES : public Platform {
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
    sptr_t<ui::widget::Gamepad> _gamepad1{};
    sptr_t<ui::widget::Gamepad> _gamepad2{};
};

}
}
}
