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

namespace caio {
namespace nintendo {
namespace nes {

NES::NES(config::Section& sec)
    : Platform{LABEL},
      _conf{sec}
{
}

bool NES::detect_format(const fs::Path& fname)
{
    if (fname.empty()) {
        return false;
    }

    if (Platform::detect_format(fname)) {
        return true;
    }

    try {
        auto [hdr, is] = iNES::load_header(fname);
        if (!_conf.cartridge.empty()) {
            log.warn("Cartridge file overrided. From {} to {}\n", _conf.cartridge, fname.string());
        }

        _conf.cartridge = fname;
        return true;

    } catch (const InvalidCartridge& err) {
        log.error("{}\n", err.what());
    }

    return false;
}

void NES::init_monitor(int ifd, int ofd)
{
    _cpu->init_monitor(ifd, ofd);
}

void NES::reset_devices()
{
    _kbd->reset();
    _ram->reset();
    _cart->reset();
    _ppu_mmap->reset();
    _ppu->reset();
    _cpu_mmap->reset();
    _cpu->reset();
    _kbd->reset();
}

std::string NES::to_string_devices() const
{
    return std::format(
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}",
        _clk->to_string(),
        _cpu->to_string(),
        _ppu->to_string(),
        _ram->to_string(),
        (_cart ? _cart->to_string() : ""),
        _kbd->to_string(),
        _joy1->to_string(),
        _joy2->to_string());
}

void NES::create_devices()
{
    _clk  = std::make_shared<Clock>("clk", PPU_FREQ, _conf.delay);
    _ram  = std::make_shared<RAM>("ram", RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES);
    _cart = Cartridge::instance(_conf.cartridge);

    _ppu_mmap = std::make_shared<NESPPUASpace>("ppu-mmap", _cart);
    _ppu = std::make_shared<RP2C02>("ppu", _ppu_mmap, _conf.ntsc);

    _cpu_mmap = std::make_shared<NESASpace>("cpu-mmap", _ram, _ppu, _cart);
    _cpu = std::make_shared<RP2A03>("cpu", PPU_FREQ, _cpu_mmap);

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
    const auto set_nmi = [this](bool active) {
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
            data = (data & (D7 | D6 | D5 | D4 | D3 | D2)) | sjoy1->data();
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

void NES::make_widgets()
{
    /*
     * Joystick presence and swap status.
     */
    const auto gamepad_status = [this](int id) {
        const auto& [j1, j2] = (id == 0 ? std::pair{_joy1, _joy2} : std::pair{_joy2, _joy1});
        ui::widget::Gamepad::Status st{
            .id = id,
            .is_connected = (_conf.swapj ? j2->is_connected() : j1->is_connected()),
            .is_swapped = _conf.swapj,
            .name = (_conf.swapj ? j2->name() : j1->name()),
        };
        return st;
    };

    const auto swapj_action = [this]() {
        /*
         * Click on a gamepad widget swaps joysticks.
         */
        _conf.swapj ^= true;
        log.debug("Joysticks {}swapped\n", (_conf.swapj ? "" : "un"));
    };

    const auto gamepad1_status = std::bind(gamepad_status, 0);
    const auto gamepad2_status = std::bind(gamepad_status, 1);

    _gamepad1 = ui::make_widget<ui::widget::Gamepad>(ui(), gamepad1_status);
    _gamepad2 = ui::make_widget<ui::widget::Gamepad>(ui(), gamepad2_status);

    _gamepad1->action(swapj_action);
    _gamepad2->action(swapj_action);

    auto panel = ui()->panel();
    panel->add(_gamepad1);
    panel->add(_gamepad2);
}

void NES::connect_ui()
{
    Platform::connect_ui();

    /*
     * Connect the audio output.
     */
    _cpu->audio_buffer([this]() {
        return ui()->audio_buffer();
    });

    /*
     * Connect the video output.
     */
    _ppu->render_line([this](unsigned line, const ui::Scanline& scanline) {
        return ui()->render_line(line, scanline);
    });

    ui()->keyboard(_kbd);
    ui()->joystick({_joy1, _joy2});
}

void NES::hotkeys(keyboard::Key key)
{
    /*
     * Called in the context of the UI thread.
     */
    switch (key) {
    case keyboard::KEY_ALT_J:
        /*
         * Swap joysticks.
         */
        _gamepad1->action();    /* Swap action, gamepad1 visible on status bar */
        _gamepad2->action();    /* Swap action, gamepad2 visible on status bar */
        _gamepad1->action();    /* Swap action, swap value as expected         */
        break;

    case keyboard::KEY_CTRL_C:
        /*
         * Enter monitor on the next clock tick.
         * CTRL-C forces resume from pause.
         */
        _cpu->ebreak();
        if (ui()->paused()) {
            ui()->pause(false);
        }
        break;

    default:;
    }
}

ui::Config NES::ui_config()
{
    std::string title = _conf.title;
    if (_cart) {
        title = std::format("{} - {}", title, _cart->label());
    }

    const ui::Config uiconf {
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
            .aspect         = _conf.aspect,
            .sleffect       = _conf.scanlines,
            .fullscreen     = _conf.fullscreen,
            .sresize        = _conf.sresize,
            .screenshotdir  = _conf.screenshotdir,
            .statusbar      = _conf.statusbar
        }
    };

    return uiconf;
}

void NES::serdes(Serializer& ser)
{
    ser & *this;
}

Serializer& operator&(Serializer& ser, NES& nes)
{
    ser & static_cast<Platform&>(nes)
        & nes._ram
        & nes._cart
        & nes._ppu_mmap
        & nes._ppu
        & nes._cpu_mmap
        & nes._cpu;

    return ser;
}

}
}
}
