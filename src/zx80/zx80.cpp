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
#include "zx80_params.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

void ZX80::run(std::string_view pname)
{
    autorun(pname);

    create_ui();
    make_widgets();

    create_devices();
    connect_devices();
    attach_prg();
    connect_ui();

    if (_conf.monitor) {
        _cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO, {}, {});
    }

    start();
}

void ZX80::autorun(std::string_view pname)
{
    if (!pname.empty()) {
        if (!_conf.prgfile.empty()) {
            log.warn("Program file overrided. From {} to {}\n", _conf.prgfile, pname);
        }
        _conf.prgfile = pname;
    }
}

void ZX80::start()
{
    log.info("Starting {} - {}\n{}\n", full_version(), _conf.title, to_string());

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

void ZX80::reset()
{
    if (!_clk->paused()) {
        /*
         * Pause the clock and wait until it is actually paused
         * (this method runs in the context of the UI thread; see connect_ui()).
         */
        _clk->pause_wait(true);

        _ram->reset();
        _rom->reset();
        _cpu->reset();
        _kbd->reset();
        _mmap->reset();
        _cpu->reset();

        attach_prg();

        _clk->reset();
        _clk->pause(false);
    }
}

std::string ZX80::rompath(std::string_view fname) const
{
    auto path = fs::search(fname, {_conf.romdir});
    if (path.empty()) {
        throw IOError{"Can't load ROM: {}: {}", fname, Error::to_string(ENOENT)};
    }

    return path;
}

void ZX80::attach_prg()
{
    if (_conf.prgfile.empty()) {
        return;
    }

    std::string fname{fs::search(_conf.prgfile)};
    if (fname.empty()) {
        throw IOError{"Can't load program: {}: {}", _conf.prgfile, Error::to_string()};
    }

    log.debug("Opening program: {}\n", fname);

    OFile* prog{};
    addr_t bpaddr{};

    if (_rom->size() == ROM_SIZE) {
        prog = new OFile{};
        bpaddr = ROM4_MAIN_EXEC;
    } else {
        prog = new PFile{};
        bpaddr = ROM8_MAIN_EXEC;
    }

    prog->load(fname);

    log.debug("Loading program: {}, load address: ${:04X}, size: {} (${:04X})\n", fname, prog->load_address(),
        prog->size(), prog->size());

    _cpu->bpadd(bpaddr, [this, bpaddr](Z80& cpu, void* arg) {
        /*
         * Inject .o or .p into memory.
         */
        uptr_t<OFile> prog{static_cast<OFile*>(arg)};
        addr_t addr = prog->load_address();

        if (addr == PFile::LOAD_ADDR) {
            addr_t ramtop = (_conf.ram16 ? RAMTOP_16K : RAMTOP_1K);
            addr_t stackp = ramtop - 4;
            cpu.regs().SP = stackp;
            cpu.regs().PC = ROM8_SLOW_FAST;
            cpu.write(ROM8_SYSVAR_ERR_NR, 0xFF);
            cpu.write(ROM8_SYSVAR_FLAGS, 0x80);
            cpu.write_addr(stackp, ROM8_NEXT_LINE_10);
        }

        for (auto value : *prog) {
            cpu.write(addr++, value);
        }

        cpu.bpdel(bpaddr);

    }, prog);
}

void ZX80::create_devices()
{
    _ram = (_conf.ram16 ?
        std::make_shared<RAM>(EXTERNAL_RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES, "RAM16") :
        std::make_shared<RAM>(INTERNAL_RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES, "RAM1"));

    _rom = (_conf.rom8 ?
        std::make_shared<ROM>(rompath(ROM8_FNAME), ROM8_DIGEST, "ROM8") :
        std::make_shared<ROM>(rompath(ROM_FNAME), ROM_DIGEST, "ROM4"));

    _clk   = std::make_shared<Clock>("CLK", CLOCK_FREQ, _conf.delay);
    _cpu   = std::make_shared<Z80>(Z80::TYPE, "CPU");
    _video = std::make_shared<ZX80Video>(_clk, _conf.rvideo, "VID");
    _kbd   = std::make_shared<ZX80Keyboard>("KBD");

    _cass = (_conf.rom8 ?
        std::make_shared<ZX80CassetteP>(_clk, _conf.cassdir) :
        std::make_shared<ZX80CassetteO>(_clk, _conf.cassdir));

    _mmap  = std::make_shared<ZX80ASpace>(_cpu, _ram, _rom, _video, _kbd, _cass);

    _cpu->init(_mmap);
}

void ZX80::connect_devices()
{
    if (!_conf.palette.empty()) {
        _video->palette(_conf.palette);
    }

    if (!_conf.keymaps.empty()) {
        _kbd->load(_conf.keymaps);
    }

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
    auto cassette = ui::make_widget<ui::widget::Cassette>(_ui, [this]() {
        using Status = ui::widget::Cassette::Status;
        return Status{ .is_enabled = true, .is_idle = _cass->is_idle() };
    });

    auto panel = _ui->panel();
    panel->add(cassette);
}

void ZX80::connect_ui()
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
     * Connect the video output.
     */
    _video->render_line([this](unsigned line, const ui::Scanline& scanline) {
        _ui->render_line(line, scanline);
    });

    _video->clear_screen([this](const Rgba& color) {
        _ui->clear_screen(color);
    });

    /*
     * Connect the keyboard.
     */
    auto hotkeys = [this](keyboard::Key key) {
        this->hotkeys(key);
    };

    _ui->hotkeys(hotkeys);
    _ui->keyboard(_kbd);
}

void ZX80::hotkeys(keyboard::Key key)
{
    /*
     * This methods is called within the context of the UI thread
     * (see connect_ui()).
     */
    switch (key) {
    case keyboard::KEY_ALT_J:
        break;

    case keyboard::KEY_ALT_M:
        /*
         * Enter monitor on the next clock tick only if it is active.
         */
        if (!_conf.monitor) {
            break;
        }

        /* PASSTHROUGH */

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

std::string ZX80::to_string() const
{
    return std::format("{}\n\nConnected devices:\n"
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
        _kbd->to_string(),
        _video->to_string(),
        _ui->to_string());
}

}
}
}
