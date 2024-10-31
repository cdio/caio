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
#include "zxsp.hpp"

#include <unistd.h>

#include <algorithm>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>

#include "version.hpp"
#include "kempston.hpp"

#include "zxsp_params.hpp"
#include "snap_sna.hpp"
#include "snap_z80.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

ZXSpectrum::ZXSpectrum(config::Section& sec)
    : _conf{sec}
{
}

ZXSpectrum::~ZXSpectrum()
{
}

void ZXSpectrum::run(std::string_view pname)
{
    _title = _conf.title;

    autorun(pname);

    create_devices();
    connect_devices();
    attach_prg();

    create_ui();
    make_widgets();
    connect_ui();

    if (_conf.monitor) {
        this->_cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO, {}, {});
    }

    start();
}

void ZXSpectrum::autorun(std::string_view pname)
{
    if (!pname.empty()) {
        if (!_conf.snap.empty()) {
            log.warn("Snapshot file overrided. From {} to {}\n", _conf.snap, pname);
        }
        _conf.snap = pname;
    }
}

void ZXSpectrum::start()
{
    log.info("Starting {} - {}\n{}\n", full_version(), name(), to_string());

    /*
     * The emulator runs on its own thread.
     */
    std::exception_ptr error{};
    std::thread th{[&error, this]() {
        try {
            /*
             * System clock loop.
             */
            _clk->run();
        } catch (...) {
            error = std::current_exception();
        }

        /*
         * The clock was self-terminated: Stop the user interface and exit this thread.
         */
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

    if (error) {
        std::rethrow_exception(error);
    }

    log.info("Terminating {}\n", _title);
}

void ZXSpectrum::reset()
{
    if (!_clk->paused()) {
        /*
         * Pause the clock and wait until it is actually paused
         * (this method runs in the context of the UI thread; see connect_ui()).
         */
        _clk->pause_wait(true);

        _ram->reset();
        _rom->reset();
        _kbd->reset();
        _ula->reset();
        _tape->rewind();

        _cpu->reset();
        attach_prg();

        _clk->reset();
        _clk->pause(false);
    }
}

void ZXSpectrum::reset(const Snapshot& snap)
{
    const auto& regs = snap.regs();
    const auto [imode, iff1, iff2] = snap.interrupt_flags();
    _cpu->reset(regs, imode, iff1, iff2);

    const auto& ram = snap.ram();
    std::copy(ram.begin(), ram.end(), _ram->begin());

    _ula->video()->border_colour(snap.border_colour());
}

std::string ZXSpectrum::rompath(std::string_view fname) const
{
    auto path = fs::search(fname, {_conf.romdir});
    if (path.empty()) {
        throw IOError{"Can't load ROM: {}: {}", fname, Error::to_string(ENOENT)};
    }

    return path;
}

void ZXSpectrum::attach_prg()
{
    if (!_conf.snap.empty()) {
        std::string fname{fs::search(_conf.snap, {"./"})};
        if (fname.empty()) {
            throw IOError{"Can't load snapshot: {}: {}", _conf.snap, Error::to_string()};
        }

        uptr_t<Snapshot> snap{};

        if (SnapSNA::seems_like(fname)) {
            snap = std::make_unique<SnapSNA>(fname);
        } else if (SnapZ80::seems_like(fname)) {
            snap = std::make_unique<SnapZ80>(fname);
        } else {
            throw IOError{"Unrecognised snapshot format: {}", _conf.snap};
        }

        reset(*snap);

        _title = std::format("{} - {}", _conf.title, fs::basename(fname).string());
    }
}

void ZXSpectrum::create_devices()
{
    _clk  = std::make_shared<Clock>("clk", CLOCK_FREQ, _conf.delay);
    _cpu  = std::make_shared<Z80>();
    _ram  = std::make_shared<RAM>("ram", RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES);
    _rom  = std::make_shared<ROM>("rom", rompath(ROM_FNAME), ROM_DIGEST);
    _kbd  = std::make_shared<ZXSpectrumKeyboard>(_conf.keyboard);
    _joy  = std::make_shared<Joystick>("kempston", kempston::joystick_port);
    _tape = std::make_shared<ZXSpectrumTape>(_clk, _conf.itape, _conf.otape, _conf.fastload);
    _ula  = std::make_shared<ULA>(_cpu, _ram, _rom, _kbd, _joy, _tape);

    if (_conf.vjoy.enabled) {
        _kbd->vjoystick(_conf.vjoy, _joy);
    }

    _cpu->init(_ula->mmap());

    auto itape = fs::fix_home(_conf.itape);
    if (fs::exists(itape) && !fs::is_directory(itape)) {
        _title = std::format("{} - {}", _conf.title, fs::basename(itape).string());
    }
}

void ZXSpectrum::connect_devices()
{
    if (!_conf.palette.empty()) {
        _ula->video()->palette(_conf.palette);
    }

    if (!_conf.keymaps.empty()) {
        _kbd->load(_conf.keymaps);
    }

    _clk->add(_cpu);
    _clk->add(_ula->audio());
    _clk->add(_ula->video());
}

void ZXSpectrum::create_ui()
{
    ui::Config uiconf {
        .audio = {
            .enabled        = _conf.audio,
            .srate          = ULAAudio::SAMPLING_RATE,
            .channels       = ULAAudio::CHANNELS,
            .samples        = ULAAudio::SAMPLES
        },
        .video = {
            .title          = _title,
            .width          = ULAVideo::WIDTH,
            .height         = ULAVideo::HEIGHT,
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

void ZXSpectrum::make_widgets()
{
    auto cassette = ui::make_widget<ui::widget::Cassette>(_ui, [this]() {
        return ui::widget::Cassette::Status{
            .is_enabled = true,
            .is_idle = _tape->is_idle()
        };
    });

    auto gamepad = ui::make_widget<ui::widget::Gamepad>(_ui, [this]() {
        return ui::widget::Gamepad::Status{
            .id = 0,
            .is_connected = _joy->is_connected(),
            .is_swapped = false,
            .name = _joy->name()
        };
    });

    auto panel = _ui->panel();
    panel->add(cassette);
    panel->add(gamepad);
}

void ZXSpectrum::connect_ui()
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
    _ula->audio()->audio_buffer([this]() {
        return _ui->audio_buffer();
    });

    /*
     * Connect the video output.
     */
    _ula->video()->render_line([this](unsigned line, const ui::Scanline& scanline) {
        _ui->render_line(line, scanline);
    });

    /*
     * Connect the keyboard.
     */
    auto hotkeys = [this](keyboard::Key key) {
        this->hotkeys(key);
    };

    _ui->hotkeys(hotkeys);
    _ui->keyboard(_kbd);
    _ui->joystick({_joy});
}

void ZXSpectrum::hotkeys(keyboard::Key key)
{
    /*
     * This methods is called within the context of the UI thread
     * (see connect_ui()).
     */
    switch (key) {
    case keyboard::KEY_ALT_J:
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

std::string ZXSpectrum::to_string() const
{
    return std::format("{}\n\nConnected devices:\n"
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
        _ram->to_string(),
        _rom->to_string(),
        _ula->to_string(),
        _kbd->to_string(),
        _joy->to_string(),
        _ui->to_string());
}

}
}
}
