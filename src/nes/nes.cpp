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
#include "nes.hpp"

#include <thread>

#include "version.hpp"

namespace caio {
namespace nintendo {
namespace nes {

NES::NES(config::Section& sec)
    : _conf{sec}
{
}

NES::~NES()
{
}

void NES::autorun(const fs::Path& pname)
{
    if (!pname.empty()) {
        try {
            auto [hdr, is] = iNES::load_header(pname);
            if (!_conf.cartridge.empty()) {
                log.warn("Cartridge file overrided. From {} to {}\n", _conf.cartridge, pname.string());
            }

            _conf.cartridge = pname;

        } catch (const InvalidCartridge& err) {
            log.error("{}\n", err.what());
        }
    }
}

void NES::run(std::string_view pname)
{
    autorun(pname);

    create_devices();
    connect_devices();

    create_ui();
    make_widgets();
    connect_ui();

    if (_conf.monitor) {
        _cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO);
    }

    start();
}

void NES::start()
{
    log.info("Starting {} - {}\n{}\n", full_version(), _conf.title, to_string());

    /*
     * The emulator runs in its own thread.
     */
    std::thread th{[this]() {
        for (;;) {
            _clk->run();
            if (!_reset) {
                /*
                 * Clock not stopped by a reset request.
                 */
                break;
            }

            _reset = false;
            _clk->pause();
            _clk->reset();
            _kbd->reset();
            _cart->reset();
            _ppu->reset();
            _cpu->reset();
            _clk->pause(false);
        }

        _ui->stop();
    }};

    if (!th.joinable()) {
        log.error("Can't start the clock thread: {}\n", Error::to_string());
        return;
    }

    /*
     * The UI main loop runs in the main thread.
     */
    _ui->run();

    _clk->stop();

    th.join();

    log.info("Terminating {}\n", _conf.title);
}

void NES::reset()
{
    /*
     * This method runs in the context of UI (main) thread.
     * See connect_ui().
     */
    if (!_reset) {
        /*
         * Send a reset request to the clock which is
         * running in the context of the emulator thread.
         * See start().
         */
        _reset = true;
        _clk->stop();
    }
}

void NES::create_devices()
{
    _clk  = std::make_shared<Clock>("clk", PPU_FREQ, _conf.delay);
    _ram  = std::make_shared<RAM>("ram", RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES);
    _cart = Cartridge::instance(_conf.cartridge);

    _ppu_mmap = std::make_shared<NESPPUASpace>(_cart);
    _ppu = std::make_shared<RP2C02>("ppu", _ppu_mmap, _conf.ntsc);

    _cpu_mmap = std::make_shared<NESASpace>(_ram, _ppu, _cart);
    _cpu = std::make_shared<RP2A03>(PPU_FREQ, _cpu_mmap);

    _kbd  = std::make_shared<NESKeyboard>(_conf.keyboard);
    _joy1 = std::make_shared<NESJoystick>("joy1", _conf.buttons);
    _joy2 = std::make_shared<NESJoystick>("joy2", _conf.buttons);

    if (_conf.vjoy.enabled) {
        _kbd->vjoystick(_conf.vjoy, _joy1);
    }
}

void NES::connect_devices()
{
    /*
     * Connect the PPU /IRQ output to the CPU /NMI input.
     */
    auto set_nmi = [this](bool active) {
        _cpu->nmi_pin(active);
    };

    _ppu->irq(set_nmi);

    /*
     * Connect keyboard and controllers to the proper CPU I/O ports.
     */
    constexpr uint8_t IOPORT_WRITE_MASK = D2 | D1 | D0;
    constexpr uint8_t IOPORT_READ_MASK = 255;

    const auto ports_write = [this](uint8_t, uint8_t value, bool) {
        /*
         * Controllers, Family BASIC's Keyboard, and Tape interfaces.
         *
         * IOPORT_OUT ($4016):
         *   D2 D1 D0    Keyboard                       Tape                            Controllers
         *    |  |  |   ------------------------------------------------------------------------------------
         *    |  |  +->  Row reset                      Data out                        Load shift register
         *    |  +---->  Column select/row increment    0: Force read 0, 1: Tape read   x
         *    +------->  0: Disable, 1: Enable          0: Enable, 1: Disable           x
         *
         * TODO: The tape interface is not implemented
         */
        _kbd->write(value);
        if (value) {
            _joy1->load();
            _joy2->load();
        }
    };

    const auto ports_read = [this](uint8_t addr) -> uint8_t {
        /*
         * Controllers, Family BASIC's Keyboard, and Tape interfaces.
         *
         * IOPORT_IN1 ($4016):
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  x  x  x  |  +-> Controller#1 status bit
         *                      +----> Tape input data
         *
         * IOPORT_IN2 ($4017):
         *  Keyboard matrix read:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  |  |  |  |  x
         *             +--+--+--+----> Keyboard matrix column data
         *
         *  Controller #2 read:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    x  x  x  x  x  |  |  +-> Controller #2 status bit
         *                   |  +----> NES: 0, Famicom: Expansion controller status bit
         *                   +-------> NES: 0, Famicom: Microphone status bit
         */
        uint8_t data = _cpu_mmap->data_bus();
        auto& sjoy1 = (_conf.swapj ? _joy2 : _joy1);
        auto& sjoy2 = (_conf.swapj ? _joy1 : _joy2);
        switch (addr) {
        case RP2A03::IOPORT_IN1:
            data = (data & (D7 | D6 | D5)) | sjoy1->data();
            sjoy1->clk();
            break;
        case RP2A03::IOPORT_IN2:
            data = (data & (D7 | D6 | D5)) | (_kbd->read() & (D4 | D3 | D2 | D1)) | sjoy2->data();
            sjoy2->clk();
            break;
        }
        return data;
    };

    _cpu->add_ior(ports_read, IOPORT_READ_MASK);
    _cpu->add_iow(ports_write, IOPORT_WRITE_MASK);

    /*
     * Load the keyboard mappings.
     */
    if (!_conf.keymaps.empty()) {
        _kbd->load(_conf.keymaps);
    }

    /*
     * Load the colour palette.
     */
    if (!_conf.palette.empty()) {
        _ppu->palette(_conf.palette);
    }

    /*
     * Connect clockable devices to the system clock.
     */
    _clk->add(_cpu);
    _clk->add(_ppu);
}

void NES::create_ui()
{
    std::string title = _conf.title;
    if (_cart) {
        title = std::format("{} - {}", title, _cart->label());
    }

    ui::Config uiconf {
        .audio = {
            .enabled        = _conf.audio,
            .srate          = ricoh::rp2a03::AUDIO_SAMPLING_RATE,
            .channels       = ricoh::rp2a03::AUDIO_CHANNELS,
            .samples        = ricoh::rp2a03::AUDIO_SAMPLES
        },
        .video = {
            .title          = title,
            .width          = RP2C02::WIDTH,
            .height         = (_conf.ntsc ? RP2C02::NTSC_HEIGHT : RP2C02::HEIGHT),
            .fps            = _conf.fps,
            .scale          = _conf.scale,
            .sleffect       = ui::to_sleffect(_conf.scanlines),
            .fullscreen     = _conf.fullscreen,
            .sresize        = _conf.sresize,
            .screenshotdir  = _conf.screenshotdir,
            .statusbar      = _conf.statusbar
        }
    };

    _ui = ui::UI::instance(uiconf);
}

void NES::make_widgets()
{
    /*
     * Joystick presence and swap status.
     */
    _gamepad1 = ui::make_widget<ui::widget::Gamepad>(_ui, [this]() {
        ui::widget::Gamepad::Status st{
            .id = 0,
            .is_connected = (_conf.swapj ? _joy2->is_connected() : _joy1->is_connected()),
            .is_swapped = _conf.swapj,
            .name = (_conf.swapj ? _joy2->name() : _joy1->name()),
        };
        return st;
    });

    _gamepad2 = ui::make_widget<ui::widget::Gamepad>(_ui, [this]() {
        ui::widget::Gamepad::Status st{
            .id = 1,
            .is_connected = (_conf.swapj ? _joy1->is_connected() : _joy2->is_connected()),
            .is_swapped = _conf.swapj,
            .name = (_conf.swapj ? _joy1->name() : _joy2->name()),
        };
        return st;
    });

    auto swapj_action = [this]() {
        /*
         * Click on a gamepad widget swaps joysticks.
         */
        _conf.swapj ^= true;
        log.debug("Joysticks {}swapped\n", (_conf.swapj ? "" : "un"));
    };

    _gamepad1->action(swapj_action);
    _gamepad2->action(swapj_action);

    auto panel = _ui->panel();
    panel->add(_gamepad1);
    panel->add(_gamepad2);
}

void NES::connect_ui()
{
    /*
     * Connect Pause and Reset widgets.
     */
    auto do_pause = [this](bool suspend) {
        hotkeys(keyboard::KEY_PAUSE);
    };

    auto is_paused = [this]() {
        return _clk->paused();
    };

    auto do_reset = [this]() {
        reset();
    };

    _ui->pause(do_pause, is_paused);
    _ui->reset(do_reset);

    /*
     * Connect the audio output.
     */
    _cpu->audio_buffer([this]() {
        return _ui->audio_buffer();
    });

    /*
     * Connect the video output.
     */
    _ppu->render_line([this](unsigned line, const ui::Scanline& scanline) {
        _ui->render_line(line, scanline);
    });

    /*
     * Connect keyboard and joysticks.
     */
    auto hotkeys = [this](keyboard::Key key) {
        this->hotkeys(key);
    };

    _ui->hotkeys(hotkeys);
    _ui->keyboard(_kbd);
    _ui->joystick({_joy1, _joy2});
}

void NES::hotkeys(keyboard::Key key)
{
    /*
     * hotkeys() is called in the context of the UI thread
     * (see connect_ui()).
     */
    switch (key) {
    case keyboard::KEY_ALT_J:
        /*
         * Swap joysticks.
         */
        _gamepad1->action();    /* Swap action, gamepad1 visible on status bar */
        _gamepad2->action();    /* Swap action, gamepad2 visible on status bar */
        _gamepad1->action();    /* Swap action, swap value as excepted         */
        break;

    case keyboard::KEY_CTRL_C:
        /*
         * Enter monitor on the next clock tick.
         * CTRL-C forces resume from pause.
         */
        _cpu->ebreak();
        if (!_clk->paused()) {
            break;
        }

        /* PASSTHROUGH */

    case keyboard::KEY_PAUSE:
        log.debug("System {}paused\n", (_ui->paused() ? "un" : ""));
        _clk->pause(_clk->paused() ^ true);
        break;

    default:;
    }
}

std::string NES::to_string() const
{
    return std::format("{}\n\nConnected devices:\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n\n"
        "UI backend: {}\n",
        _conf.to_string(),
        _clk->to_string(),
        _cpu->to_string(),
        _ppu->to_string(),
        _ram->to_string(),
        (_cart ? _cart->to_string() : "<No Cartridge>"),
        _kbd->to_string(),
        _joy1->to_string(),
        _joy2->to_string(),
        _ui->to_string());
}

}
}
}
