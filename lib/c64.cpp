/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "c64.hpp"

#include <iostream>
#include <sstream>
#include <thread>

#include "icon.hpp"
#include "fs.hpp"
#include "prg.hpp"
#include "utils.hpp"
#include "ui_sfml.hpp"
#include "version.hpp"

#include "mos_6581.hpp"
#include "mos_6581_resid.hpp"

#include "c64_crt.hpp"
#include "c64_io.hpp"
#include "c64_vic2_aspace.hpp"

#include "c1541_factory.hpp"


namespace cemu {
namespace c64 {

std::string C64::rompath(const std::string &fname) const
{
    auto path = fs::search(fname, {_conf.romdir});
    if (path.empty()) {
        throw IOError{"Can't load ROM: " + fname + ": " + Error::to_string(ENOENT)};
    }

    return path;
}

std::string C64::cartpath(const std::string &fname) const
{
    auto path = fs::search(fname, {_conf.cartdir});
    if (path.empty()) {
        throw IOError{"Can't load Cartridge: " + fname + ": " + Error::to_string(ENOENT)};
    }

    return path;
}

std::string C64::palettepath(const std::string &fname) const
{
    return fs::search(fname, {_conf.palettedir});
}

std::string C64::keymapspath(const std::string &fname) const
{
    return fs::search(fname, {_conf.keymapsdir});
}

bool C64::check_rom_size(const devptr_t &rom) const
{
    if (rom) {
        switch (rom->size()) {
        case 8192:
        case 16384:
            return true;
        default:;
        }
    }

    return false;
}

devptr_t C64::attach_cartridge()
{
    devptr_t rom{};

    if (!_conf.cartfile.empty()) {
        std::string fpath{cartpath(_conf.cartfile)};
        Crt cart{};
        try {
            cart.open(fpath);
        } catch (const InvalidCartridge &) {
            /*
             * The file is not a CRT file, try RAW format.
             */
            log.debug("Cartridge is not a CRT file, trying it as a ROM dump: " + fpath + "\n");
            try {
                rom = std::make_shared<DeviceROM>(fpath);
                _conf.title += " - " + fs::basename(fpath);
            } catch (const IOError &ex) {
                throw InvalidCartridge{ex};
            }
        }

        if (!rom) {
            if (cart.chips() > 1) {
                throw InvalidCartridge{"Cartridges with only a single chip are supported: " + cart.to_string()};
            }

            rom = cart[0].second;
            _conf.title += " - " + cart.name();
        }

        if (!check_rom_size(rom)) {
            throw InvalidCartridge{"Only 8K or 16K ROMs are supported"};
        }
    }

    return rom;
}

void C64::attach_prg()
{
    if (!_conf.prgfile.empty()) {
        try {
            auto *prog = new Prg{fs::fix_home(_conf.prgfile)};
            _conf.title += " - " + fs::basename(_conf.prgfile);

            _cpu->bpadd(BASIC_READY_ADDR, [this](Mos6510 &cpu, void *arg) {
                /*
                 * Load PRG into memory.
                 */
                auto *prog = static_cast<Prg *>(arg);
                for (size_t i = 0; i < prog->size(); ++i) {
                    _mmap->write(prog->addr() + i, (*prog)[i]);
                }

                /*
                 * If it is visible from BASIC, run it.
                 */
                if (prog->addr() == BASIC_PRG_START) {
                    addr_t end = BASIC_PRG_START + prog->size();
                    _mmap->write_addr(BASIC_TXTTAB, BASIC_PRG_START);
                    _mmap->write_addr(BASIC_VARTAB, end);
                    _mmap->write_addr(BASIC_ARYTAB, end);
                    _mmap->write_addr(BASIC_STREND, end);

                    _mmap->write(BASIC_KEYB_BUFF + 0, 'R');
                    _mmap->write(BASIC_KEYB_BUFF + 1, 'U');
                    _mmap->write(BASIC_KEYB_BUFF + 2, 'N');
                    _mmap->write(BASIC_KEYB_BUFF + 3, '\r');
                    _mmap->write(BASIC_KEYB_BUFF_POS, 4);
                }

                cpu.bpdel(BASIC_READY_ADDR);
                delete prog;
            }, prog);

        } catch (const std::exception &ex) {
            throw IOError{ex};
        }
    }
}

void C64::reset()
{
    _ram     = std::make_shared<DeviceRAM>("SYSTEM RAM", 65536);
    _basic   = std::make_shared<DeviceROM>(rompath(BASIC_FNAME), "BASIC", BASIC_SIZE);
    _kernal  = std::make_shared<DeviceROM>(rompath(KERNAL_FNAME), "KERNAL", KERNAL_SIZE);
    _chargen = std::make_shared<DeviceROM>(rompath(CHARGEN_FNAME), "CHARGEN", CHARGEN_SIZE);
    _vcolor  = std::make_shared<NibbleRAM>("COLOR RAM", VCOLOR_SIZE);

    if (_conf.resid) {
        _sid = std::make_shared<Mos6581Resid>("reSID", CLOCK_FREQ_PAL);
    } else {
        _sid = std::make_shared<Mos6581>("SID", CLOCK_FREQ_PAL);
    }

    _cia1    = std::make_shared<Mos6526>("CIA1");
    _cia2    = std::make_shared<Mos6526>("CIA2");

    _bus     = std::make_shared<cbm_bus::Bus>("C64 BUS");
    _busdev  = std::make_shared<C64BusController>(_bus, _cia2);

    auto vic2_mmap = std::make_shared<Vic2ASpace>(_cia2, _ram, _chargen);
    _vic2    = std::make_shared<Mos6569>("VIC-II", vic2_mmap, _vcolor);

    _io      = std::make_shared<C64IO>(_ram, _vic2, _sid, _vcolor, _cia1, _cia2);

    _cart    = attach_cartridge();

    _mmap    = std::make_shared<C64ASpace>(_ram, _basic, _kernal, _chargen, _io, _cart);
    _cpu     = std::make_shared<Mos6510>(_mmap);

    _clk     = std::make_shared<Clock>("SYSTEM CLOCK", CLOCK_FREQ_PAL, _conf.delay);

    attach_prg();

    _clk->add(_vic2);
    _clk->add(_cpu);
    _clk->add(_cia1);
    _clk->add(_cia2);
    _clk->add(_sid);

    if (!_conf.unit8.empty()) {
        _unit8 = c1541::create(_conf.unit8, 8, _bus);
        _clk->add(_unit8);
    }

    if (!_conf.unit9.empty()) {
        _unit9 = c1541::create(_conf.unit9, 9, _bus);
        _clk->add(_unit9);
    }

    ui::Config uiconf {
        .audio = {
            .enabled       = _conf.audio_enabled,
            .srate         = Mos6581::SAMPLING_RATE,
            .channels      = Mos6581::CHANNELS,
            .samples       = Mos6581::SAMPLES
        },
        .video = {
            .title         = _conf.title,
            .width         = Mos6569::WIDTH,
            .height        = Mos6569::HEIGHT,
            .fps           = _conf.fps,
            .scale         = _conf.scale,
            .sleffect      = ui::to_sleffect(_conf.scanlines),
            .fullscreen    = _conf.fullscreen,
            .smooth_resize = _conf.smooth_resize,
        }
    };

    _ui = ui::sfml::create(uiconf, icon32());

    auto trigger_irq = [this](bool active) {
        _cpu->trigger_irq(active);
    };

    auto trigger_nmi = [this](bool active) {
        _cpu->trigger_nmi(active);
    };

    auto set_rdy = [this](bool active) {
        _cpu->set_rdy(active);
    };

    auto vsync = [this](unsigned wait_cycles) {
        _clk->sync(wait_cycles);
    };

    _vic2->irq(trigger_irq);
    _vic2->aec(set_rdy);
    _vic2->vsync(vsync);

    if (!_conf.palettefile.empty()) {
        const auto ppath = palettepath(_conf.palettefile);
        if (ppath.empty()) {
            throw Error{"Palette file not found: " + _conf.palettefile};
        }

        _vic2->palette(ppath);
    }

    _vic2->ui(_ui);

    _cia1->irq(trigger_irq);
    _cia2->irq(trigger_nmi);

    _sid->ui(_ui);

    /*
     * Keyboard and joysticks.
     */
    constexpr uint8_t KBD_MASK = 255;

    auto kbd_read = [this](uint8_t addr) -> uint8_t {
        switch (addr) {
        case Mos6526::PRA:
            return (_conf.swapj ? _joy1->position() : _joy2->position());

        case Mos6526::PRB:
            return (_kbd->read() & (_conf.swapj ? _joy2->position() : _joy1->position()));

        default:;
        }

        return 255; /* Pull-ups */
    };

    auto kbd_write = [this](uint8_t addr, uint8_t value) {
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

    auto hotkeys = [this](Keyboard::Key key) {
        switch (key) {
        case Keyboard::KEY_ALT_J:
            /* Swap joysticks */
            _conf.swapj ^= 1;
            break;

        case Keyboard::KEY_ALT_M:
            /* Enter monitor on the next clock tick only if it is active */
            if (!_conf.monitor) {
                break;
            }
            /* PASSTHROUGH */

        case Keyboard::KEY_CTRL_C:
            /* Enter monitor on the next clock tick */
            _cpu->ebreak();
            break;

        case Keyboard::KEY_PAUSE:
            _paused ^= true;
            _clk->toggle_suspend();
            if (_paused) {
                _ui->audio_pause();
                _ui->title(_conf.title + " (PAUSED)");
            } else {
                _ui->audio_play();
                _ui->title(_conf.title);
            }
            break;

        default:;
        }
    };

    auto restore_key = [this]() {
        _cpu->trigger_nmi(true);
    };

    _kbd  = std::make_shared<C64Keyboard>("C64 KBD", restore_key);
    _joy1 = std::make_shared<C64Joystick>("C64 JOY1");
    _joy2 = std::make_shared<C64Joystick>("C64 JOY2");

    _cia1->add_ior(kbd_read, KBD_MASK);
    _cia1->add_iow(kbd_write, KBD_MASK);

    if (!_conf.keymapsfile.empty()) {
        const auto kpath = keymapspath(_conf.keymapsfile);
        if (kpath.empty()) {
            throw Error{"Keymaps file not found: " + _conf.keymapsfile};
        }

        _kbd->load(kpath);
    }

    _ui->keyboard(_kbd);
    _ui->joystick({_joy1, _joy2});
    _ui->hotkeys(hotkeys);
}

void C64::run()
{
    reset();

    if (_conf.monitor) {
        _cpu->init_monitor(std::cin, std::cout);
    }

    start();
}

void C64::start()
{
    log.info("Starting CEMU " + cemu::version() + " - C64\n" + to_string() + "\n");

    /*
     * The emulator runs on its own thread.
     */
    std::thread th{[this]() {
        /*
         * System clock loop.
         */
        _clk->run();

        /*
         * The clock was self-terminated.
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

std::string C64::to_string() const
{
    std::ostringstream os{};

    os << _conf.to_string()             << std::endl
       << std::endl
       << "Connected devices:"          << std::endl
       << "  " << _clk->to_string()     << std::endl
       << "  " << _cpu->to_string()     << std::endl
       << "  " << _vic2->to_string()    << std::endl
       << "  " << _cia1->to_string()    << std::endl
       << "  " << _cia2->to_string()    << std::endl
       << "  " << _sid->to_string()     << std::endl
       << "  " << _ram->to_string()     << std::endl
       << "  " << _vcolor->to_string()  << std::endl
       << "  " << _basic->to_string()   << std::endl
       << "  " << _kernal->to_string()  << std::endl
       << "  " << _chargen->to_string() << std::endl;

    if (_cart) {
       os << "  " << _cart->to_string() << std::endl;
    }

    os << "  " << _kbd->to_string()     << std::endl
       << "  " << _joy1->to_string()    << std::endl
       << "  " << _joy2->to_string()    << std::endl
       << "  " << _bus->to_string()     << std::endl
       << std::endl;

    os << "UI backend: " << _ui->to_string() << std::endl;

    return os.str();
}

}
}
