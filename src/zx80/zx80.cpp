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
#include "zx80.hpp"

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>

#include "fs.hpp"
#include "logger.hpp"
#include "types.hpp"
#include "version.hpp"

#include "ofile.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

void ZX80::run()
{
    create_ui();
    make_widgets();

    create_devices();
    connect_devices();
    connect_ui();

    if (_conf.monitor) {
        auto load = [this](const std::string& fname, addr_t start) -> std::pair<addr_t, addr_t> {
            OFile ofile{fname};
            if (start == 0) {
                start = OFile::LOAD_ADDR;
            }
            addr_t addr = start;
            for (uint8_t c : ofile) {
                this->_cpu->write(addr++, c);
            }
            return {start, ofile.size()};
        };

        auto save = [this](const std::string& fname, addr_t start, addr_t end) {
            OFile ofile{};
            for (auto addr = start; addr <= end; ++addr) {
                uint8_t c = this->_cpu->read(addr);
                ofile.push_back(c);
            }
            ofile.save(fname);
        };

        this->_cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO, load, save);
    }

    start();
}

void ZX80::start()
{
    log.info("Starting caio v" + caio::version() + " - Sinclair ZX80\n" + to_string() + "\n");

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

std::string ZX80::rompath(const std::string& fname) const
{
    auto path = fs::search(fname, {_conf.romdir});
    if (path.empty()) {
        throw IOError{"Can't load ROM: " + fname + ": " + Error::to_string(ENOENT)};
    }

    return path;
}

void ZX80::ram_init(uint64_t pattern, gsl::span<uint64_t>& data)
{
    std::for_each(data.begin(), data.end(), [&pattern](uint64_t& value) {
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
    auto ram_init = [this](gsl::span<uint8_t>& data) {
        gsl::span<uint64_t> data64{reinterpret_cast<uint64_t*>(data.data()), data.size() / sizeof(uint64_t)};
        this->ram_init(RAM_INIT_PATTERN, data64);
    };

    size_t ramsiz  = (_conf.ram16 ? EXTERNAL_RAM_SIZE : INTERNAL_RAM_SIZE);
    size_t romsiz  = (_conf.rom8 ? ROM8_SIZE  : ROM4_SIZE);
    auto   romfile = (_conf.rom8 ? ROM8_FNAME : ROM4_FNAME);

    _cpu   = std::make_shared<Z80>(Z80::TYPE, "CPU");
    _ram   = std::make_shared<DeviceRAM>("RAM", ramsiz, ram_init);
    _rom   = std::make_shared<DeviceROM>(rompath(romfile), "ROM", romsiz);
    _video = std::make_shared<ZX80Video>("VID");
    _kbd   = std::make_shared<ZX80Keyboard>("KBD");
    _mmap  = std::make_shared<ZX80ASpace>(_cpu, _ram, _rom, _video, _kbd);
    _clk   = std::make_shared<Clock>("CLK", CLOCK_FREQ, _conf.delay);

    _cpu->init(_mmap);
}

void ZX80::connect_devices()
{
    /*
     * Load the colour palette.
     */
    if (!_conf.palette.empty()) {
        _video->palette(_conf.palette);
    }

    /*
     * Load the keyboard mappings.
     */
    if (!_conf.keymaps.empty()) {
        _kbd->load(_conf.keymaps);
    }

    /*
     * Connect clockable devices to the system clock.
     */
    _clk->add(_cpu);
}

void ZX80::create_ui()
{
    ui::Config uiconf {
        .audio = {
            .enabled    = false,
            .srate      = 0,
            .channels   = 0,
            .samples    = 0
        },
        .video = {
            .title      = _conf.title,
            .width      = ZX80Video::WIDTH,
            .height     = ZX80Video::HEIGHT,
            .fps        = _conf.fps,
            .scale      = _conf.scale,
            .sleffect   = ui::to_sleffect(_conf.scanlines),
            .fullscreen = _conf.fullscreen,
            .sresize    = _conf.sresize,
        },
    };

    _ui = ui::UI::instance(uiconf);
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

    /*
     * Connect the video output.
     */
    _video->render_line([this](unsigned line, const ui::Scanline& scanline) {
        _ui->render_line(line, scanline);
    });

    /*
     * Connect the keyboard.
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
     * This methods is called within the context of the UI thread
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

    os << _conf.to_string()           << std::endl
       << std::endl
       << "Connected devices:"        << std::endl
       << "  " << _clk->to_string()   << std::endl
       << "  " << _cpu->to_string()   << std::endl
       << "  " << _ram->to_string()   << std::endl
       << "  " << _rom->to_string()   << std::endl
       << "  " << _kbd->to_string()   << std::endl
       << "  " << _video->to_string() << std::endl
       << std::endl;

    os << "UI backend: " << _ui->to_string() << std::endl;

    return os.str();
}

}
}
}
