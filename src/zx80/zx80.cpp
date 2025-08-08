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

#include "logger.hpp"
#include "types.hpp"

#include "ofile.hpp"
#include "zx80_params.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

ZX80::ZX80(config::Section& sec)
    : Platform{},
      _conf{sec}
{
}

ZX80::~ZX80()
{
}

std::string_view ZX80::name() const
{
    return "Sinclair ZX80";
}

void ZX80::detect_format(const fs::Path& pname)
{
    if (!pname.empty()) {
        if (!_conf.prgfile.empty()) {
            log.warn("Program file overrided. From {} to {}\n", _conf.prgfile, pname.string());
        }
        _conf.prgfile = pname;
    }
}

void ZX80::init_monitor(int ifd, int ofd)
{
    _cpu->init_monitor(ifd, ofd, {}, {});
}

void ZX80::reset_devices()
{
    _ram->reset();
    _rom->reset();
    _cpu->reset();
    _kbd->reset();
    _mmap->reset();
    _cpu->reset();

    attach_prg();
}

std::string ZX80::to_string_devices() const
{
    return std::format(
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}",
        _clk->to_string(),
        _cpu->to_string(),
        _ram->to_string(),
        _rom->to_string(),
        _kbd->to_string(),
        _video->to_string());
}

void ZX80::create_devices()
{
    _ram = (_conf.ram16 ?
        std::make_shared<RAM>("ram16", EXTERNAL_RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES) :
        std::make_shared<RAM>("ram1", INTERNAL_RAM_SIZE, RAM_INIT_PATTERN, RAM::PUT_RANDOM_VALUES));

    _rom = (_conf.rom8 ?
        std::make_shared<ROM>("rom8", rompath(ROM8_FNAME), ROM8_DIGEST) :
        std::make_shared<ROM>("rom4", rompath(ROM_FNAME), ROM_DIGEST));

    _clk   = std::make_shared<Clock>("clk", CLOCK_FREQ, _conf.delay);
    _cpu   = std::make_shared<Z80>();
    _video = std::make_shared<ZX80Video>("vid", _clk, _conf.rvideo);
    _kbd   = std::make_shared<ZX80Keyboard>(_conf.keyboard);

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

    attach_prg();
}

void ZX80::make_widgets()
{
    const auto cassette = ui::make_widget<ui::widget::Cassette>(ui(), [this]() {
        using Status = ui::widget::Cassette::Status;
        return Status{
            .is_enabled = true,
            .is_idle    = _cass->is_idle()
        };
    });

    auto panel = ui()->panel();
    panel->add(cassette);
}

void ZX80::connect_ui()
{
    Platform::connect_ui();

    /*
     * Connect the video output.
     */
    _video->render_line([this](unsigned line, const ui::Scanline& scanline) {
        ui()->render_line(line, scanline);
    });

    _video->clear_screen([this](const Rgba& color) {
        ui()->clear_screen(color);
    });

    /*
     * Connect the keyboard.
     */
    const auto hotkeys = [this](keyboard::Key key) {
        this->hotkeys(key);
    };

    ui()->hotkeys(hotkeys);
    ui()->keyboard(_kbd);
}

void ZX80::hotkeys(keyboard::Key key)
{
    /*
     * This methods is called within the context of the UI thread.
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
        if (ui()->paused()) {
            ui()->pause(false);
        }
        break;

    default:;
    }
}

ui::Config ZX80::ui_config()
{
    const ui::Config uiconf {
        .audio = {
            .enabled        = false,
            .srate          = 0,
            .channels       = 0,
            .samples        = 0
        },
        .video = {
            .title          = _conf.title,
            .width          = ZX80Video::WIDTH,
            .height         = ZX80Video::HEIGHT,
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

}
}
}
