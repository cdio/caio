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
#include "zx80.hpp"

#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>

#include "fs.hpp"
#include "logger.hpp"
#include "types.hpp"
#include "version.hpp"


namespace caio {
namespace zx80 {

ZX80::ZX80(const ZX80Config &conf)
    : _conf{conf}
{
}

ZX80::~ZX80()
{
}

void ZX80::run()
{
    create_ui();
    make_widgets();

    create_devices();
    connect_devices();
    connect_ui();

    if (_conf.monitor) {
        _cpu->init_monitor(std::cin, std::cout);
    }

    start();
}

void ZX80::start()
{
    log.info("Starting caio v" + caio::version() + " - Sinclair ZX-80\n" + to_string() + "\n");

    /*
     * The emulator runs on its own thread.
     */
    std::thread th{[this]() {
        /*
         * System clock loop.
         */
        _clk->run();

        /*
         * The clock was self-terminated: Stop the user interface and exit this thread.
         */
        _ui->stop();
    }};

    if (!th.joinable()) {
        log.error("Can't start the clock thread: " + Error::to_string() + "\n");
        return;
    }

    /*
     * The UI main loop runs in the main thread.
     */
    _ui->run();

    _clk->stop();

    th.join();

    log.info("Terminating " + _conf.title + "\n");
}

void ZX80::reset()
{
    if (!_clk->paused()) {
        /*
         * Pause the clock and wait until it is actually paused
         * (this method runs in the context of the UI thread; see connect_ui()).
         */
        _clk->pause_wait(true);

        /*
         * This method does not emulate a real hardware reset,
         * it re-launches the emulator instead.
         */
        _ram->reset();
        _rom->reset();
        _cpu->reset();
        _kbd->reset();
        _clk->reset();

        _clk->pause(false);
    }
}

std::string ZX80::rompath(const std::string &fname) const
{
    auto path = fs::search(fname, {_conf.romdir});
    if (path.empty()) {
        throw IOError{"Can't load ROM: " + fname + ": " + Error::to_string(ENOENT)};
    }

    return path;
}

inline std::string ZX80::palettepath(const std::string &fname) const
{
    return fs::search(fname, {_conf.palettedir});
}

inline std::string ZX80::keymapspath(const std::string &fname) const
{
    return fs::search(fname, {_conf.keymapsdir});
}

void ZX80::ram_init(uint64_t pattern, std::vector<uint64_t> &data)
{
    std::for_each(data.begin(), data.end(), [&pattern](uint64_t &value) {
        value = pattern;
        pattern ^= static_cast<uint64_t>(-1);

        /* Put some random values */
        if (std::rand() % 100 < 20) {
            reinterpret_cast<uint8_t *>(&value)[std::rand() % 8] = std::rand() % 256;
        }
    });
}

void ZX80::create_devices()
{
    auto ram_init = [this](std::vector<uint8_t> &data) {
        auto &data64 = reinterpret_cast<std::vector<uint64_t>&>(data);
        this->ram_init(RAM_INIT_PATTERN, data64);
    };

    _ram  = std::make_shared<DeviceRAM>("RAM", RAM_SIZE, ram_init);
    _rom  = std::make_shared<DeviceROM>(rompath(ROM_FNAME), "ROM", ROM_SIZE);
    _mmap = std::make_shared<ZX80ASpace>(_ram, _rom);
    _cpu  = std::make_shared<ZilogZ80>(_mmap, ZilogZ80::TYPE, "CPU");
    _clk  = std::make_shared<Clock>("CLK", zx80::CLOCK_FREQ, _conf.delay);
    _kbd  = std::make_shared<ZX80Keyboard>("KBD");
}

void ZX80::connect_devices()
{
    /*
     * Connect CPU's irq and nmi pins to CIA1, CIA2 and VIC2 irq outputs.
     */
#if 0
    auto set_irq = [this](bool active) {
        _cpu->int_pin(active);
    };

    auto set_nmi = [this](bool active) {
        _cpu->nmi_pin(active);
    };
#endif

//XXX    _cia1->irq(set_irq);
//XXX    _cia2->irq(set_nmi);
//XXX    _vic2->irq(set_irq);

    /*
     * Load the colour palette.
     */
//TODO
#if 0
    if (!_conf.palettefile.empty()) {
        const auto ppath = palettepath(_conf.palettefile);
        if (ppath.empty()) {
            throw Error{"Palette file not found: " + _conf.palettefile};
        }

        _vic2->palette(ppath);
    }
#endif

//TODO
#if 0
    /*
     * Connect keyboard and joysticks to the proper CIA1 ports.
     */
    constexpr uint8_t KBD_MASK = 255;

    auto kbd_read = [this](uint8_t addr) -> uint8_t {
        switch (addr) {
        case Mos6526::PRA:
            return (_conf.swapj ? _joy1->port() : _joy2->port());

        case Mos6526::PRB:
            return (_kbd->read() & (_conf.swapj ? _joy2->port() : _joy1->port()));

        default:;
        }

        return 255; /* Pull-ups */
    };

    auto kbd_write = [this](uint8_t addr, uint8_t value, bool _) {
        switch (addr) {
        case Mos6526::PRA:
            /* Keyboard matrix row to scan */
            _kbd->write(value);
            break;

        case Mos6526::PRB:
            if (value & Mos6526::P4) {
                /* Port B4 is connected to the LP edge triggered input */
                _vic2->trigger_lp();
            }
            break;

        default:;
        }
    };

    _cia1->add_ior(kbd_read, KBD_MASK);
    _cia1->add_iow(kbd_write, KBD_MASK);
#endif

    /*
     * Load the keyboard mappings.
     */
    if (!_conf.keymapsfile.empty()) {
        const auto kpath = keymapspath(_conf.keymapsfile);
        if (kpath.empty()) {
            throw Error{"Keymaps file not found: " + _conf.keymapsfile};
        }

        _kbd->load(kpath);
    }

    /*
     * Connect clockable devices to the system clock.
     */
    _clk->add(_cpu);
}

void ZX80::create_ui()
{
    std::string title{_conf.title};
//XXX    if (_ioexp) {
//XXX        title += " - " + _ioexp->name();
//XXX    }


//- fare un mmap con ram e rom e nient'altro
//- fare l'expansion port
//- display: 24 lines x 32 chars (256x192 pixels) 64x48 Dots Block Graphics


    ui::Config uiconf {
        .audio = {
            .enabled       = false,
            .srate         = 0,
            .channels      = 0,
            .samples       = 0
        },
        .video = {
            .title         = title,
            .width         = pal::VISIBLE_WIDTH,
            .height        = pal::VISIBLE_HEIGHT,
            .fps           = _conf.fps,
            .scale         = _conf.scale,
            .sleffect      = ui::to_sleffect(_conf.scanlines),
            .fullscreen    = _conf.fullscreen,
            .smooth_resize = _conf.smooth_resize,
        },
    };

    _ui = std::make_shared<ui::UI>(uiconf);
}

void ZX80::make_widgets()
{
}

void ZX80::connect_ui()
{
    /*
     * Connect Pause and Reset widgets.
     */
    auto do_pause = [this](bool suspend) {
        hotkeys(Keyboard::KEY_PAUSE);
    };

    auto is_paused = [this]() {
        return _clk->paused();
    };

    auto do_reset = [this]() {
        reset();
    };

    _ui->pause(do_pause, is_paused);
    _ui->reset(do_reset);

//TODO
#if 0
    /*
     * Connect the video output.
     */
    _vic2->render_line([this](unsigned line, const ui::Scanline &scanline) {
        _ui->render_line(line, scanline);
    });
#endif

    /*
     * Connect keyboard and joysticks.
     */
    auto hotkeys = [this](Keyboard::Key key) {
        this->hotkeys(key);
    };

    _ui->hotkeys(hotkeys);
    _ui->keyboard(_kbd);
}

void ZX80::hotkeys(Keyboard::Key key)
{
    /*
     * This methods is called in the context of the UI thread
     * (see connect_ui()).
     */
    switch (key) {
    case Keyboard::KEY_ALT_J:
        break;

    case Keyboard::KEY_ALT_M:
        /*
         * Enter monitor on the next clock tick only if it is active.
         */
        if (!_conf.monitor) {
            break;
        }

        /* PASSTHROUGH */

    case Keyboard::KEY_CTRL_C:
        /*
         * Enter monitor on the next clock tick.
         * CTRL-C forces resume from pause.
         */
        _cpu->ebreak();
        if (!_clk->paused()) {
            break;
        }

        /* PASSTHROUGH */

    case Keyboard::KEY_PAUSE:
        log.debug("System %spaused\n", (_ui->paused() ? "un" : ""));
        _clk->pause(_clk->paused() ^ true);
        break;

    default:;
    }
}

std::string ZX80::to_string() const
{
    std::ostringstream os{};

    os << _conf.to_string()         << std::endl
       << std::endl
       << "Connected devices:"      << std::endl
       << "  " << _clk->to_string() << std::endl
       << "  " << _cpu->to_string() << std::endl
       << "  " << _ram->to_string() << std::endl
       << "  " << _rom->to_string() << std::endl
       << "  " << _kbd->to_string() << std::endl
       << std::endl;

    os << "UI backend: " << _ui->to_string() << std::endl;

    return os.str();
}

std::string ZX80::name()
{
    return "ZX80";
}

}
}
