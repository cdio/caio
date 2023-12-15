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
#include "c64.hpp"

#include <unistd.h>
#include <cstdlib>
#include <iostream>
#include <sstream>
#include <thread>

#include "fs.hpp"
#include "p00file.hpp"
#include "prgfile.hpp"
#include "utils.hpp"
#include "version.hpp"

#include "mos_6581.hpp"
#include "mos_6581_resid.hpp"

#include "c64_crt.hpp"
#include "c64_io.hpp"

#include "c1541_factory.hpp"


namespace caio {
namespace commodore {
namespace c64 {

void C64::run()
{
    create_ui();
    make_widgets();

    create_devices();
    connect_devices();
    attach_prg();

    connect_ui();

    if (_conf.monitor) {
        auto load = [this](const std::string& fname, addr_t start) -> std::pair<addr_t, addr_t>{
            PrgFile prog{fname};
            addr_t addr = prog.address();
            if (start != 0) {
                addr = start;
                prog.address(start);
             } else {
                start = addr;
                for (auto c : prog) {
                    this->_cpu->write(addr++, c);
                }
            }
            return {start, prog.size()};
        };

        auto save = [this](const std::string& fname, addr_t start, addr_t end) {
            PrgFile prog{};
            for (auto addr = start; addr <= end; ++addr) {
                uint8_t c = this->_cpu->read(addr);
                prog.push_back(c);
            }
            prog.save(fname, start);
        };

        _cpu->init_monitor(STDIN_FILENO, STDOUT_FILENO, load, save);
    }

    start();
}

void C64::start()
{
    log.info("Starting caio v" + caio::version() + " - Commodore C64\n" + to_string() + "\n");

    /*
     * The emulator runs on its own thread.
     */
    std::thread th{[this]() {
        /* System clock loop. */
        _clk->run();

        /* The clock was self-terminated: Stop the user interface and exit this thread. */
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

void C64::reset()
{
    if (!_clk->paused()) {
        /*
         * Pause the clock and wait until it is actually paused
         * (this method runs in the context of the UI thread; see connect_ui()).
         */
        _clk->pause_wait(true);

        /*
         * This method does not emulate a real hardware reset,
         * it re-launches the C64 emulator instead.
         */
        _ram->reset();
        _basic->reset();
        _kernal->reset();
        _chargen->reset();

//XXX     _bus->reset();
//XXX     _busdev->reset();

        _cpu->write(0, 0);  /* Not necessary, PLA already sets the default mode */
        _cpu->write(1, 0);
        _pla->reset();

        _io->reset();       /* Resets: VIC2, SID, VRAM, CIA1, CIA2 and Cartridge */

        _cpu->reset();      /* The CPU is reset after IO and PLA otherwise it could take the wrong reset vector */

        _kbd->reset();

        if (_unit8) {
            _unit8->reset();
        }

        if (_unit9) {
            _unit9->reset();
        }

        attach_prg();

        _clk->reset();
        _clk->pause(false);
    }
}

std::string C64::rompath(const std::string& fname)
{
    auto path = fs::search(fname, {_conf.romdir});
    if (path.empty()) {
        throw IOError{"Can't load ROM: " + fname + ": " + Error::to_string(ENOENT)};
    }

    return path;
}

sptr_t<Cartridge> C64::attach_cartridge()
{
    if (_conf.cartridge.empty()) {
        return {};
    }

    auto fpath = fs::search(_conf.cartridge);
    if (fpath.empty()) {
        throw IOError{"Can't load Cartridge: " + _conf.cartridge + ": " + Error::to_string(ENOENT)};
    }

    auto cart = Cartridge::create(fpath);
    return cart;
}

void C64::attach_prg()
{
    if (_conf.prgfile.empty()) {
        return;
    }

    std::string prgfile{fs::search(_conf.prgfile)};
    if (prgfile.empty()) {
        throw IOError{"Can't load program: " + _conf.prgfile + ": " + Error::to_string()};
    }

    try {
        PrgFile* prog{};
        const char* format{};

        log.debug("Preloading program: " + prgfile + "\n");

        try {
            prog = new P00File{prgfile};
            format = "P00";
        } catch (const IOError&) {
            prog = new PrgFile{prgfile};
            format = "PRG";
        }

        log.debug("Detected format: %s, start address: $%04X, size: %d ($%04X)\n", format, prog->address(),
            prog->size(), prog->size());

        _cpu->bpadd(BASIC_READY_ADDR, [](Mos6510& cpu, void* arg) {
            /*
             * Load PRG into memory.
             */
            uptr_t<PrgFile> prog{static_cast<PrgFile*>(arg)};
            for (size_t i = 0; i < prog->size(); ++i) {
                cpu.write(prog->address() + i, (*prog)[i]);
            }

            /*
             * If it is visible from BASIC, run it.
             */
            if (prog->address() == BASIC_PRG_START) {
                addr_t end = BASIC_PRG_START + prog->size();
                cpu.write_addr(BASIC_TXTTAB, BASIC_PRG_START);
                cpu.write_addr(BASIC_VARTAB, end);
                cpu.write_addr(BASIC_ARYTAB, end);
                cpu.write_addr(BASIC_STREND, end);

                cpu.write(BASIC_KEYB_BUFF + 0, 'R');
                cpu.write(BASIC_KEYB_BUFF + 1, 'U');
                cpu.write(BASIC_KEYB_BUFF + 2, 'N');
                cpu.write(BASIC_KEYB_BUFF + 3, '\r');
                cpu.write(BASIC_KEYB_BUFF_POS, 4);
            }

            cpu.bpdel(BASIC_READY_ADDR);
        }, prog);

    } catch (const std::exception& err) {
        throw IOError{err};
    }
}

void C64::create_devices()
{
    _ram = std::make_shared<RAM>(RAM_SIZE, RAM_INIT_PATTERN1, RAM::PUT_RANDOM_VALUES, "RAM");
    _basic = std::make_shared<ROM>(rompath(BASIC_FNAME), BASIC_SIZE, "BASIC");
    _kernal = std::make_shared<ROM>(rompath(KERNAL_FNAME), KERNAL_SIZE,  "KERNAL");
    _chargen = std::make_shared<ROM>(rompath(CHARGEN_FNAME), CHARGEN_SIZE, "CHARGEN");
    _vram = std::make_shared<NibbleRAM>(VRAM_SIZE, "VRAM");

    if (_conf.resid) {
        _sid = std::make_shared<Mos6581Resid>(Mos6581Resid::version(), CLOCK_FREQ_PAL);
    } else {
        _sid = std::make_shared<Mos6581>("SID", CLOCK_FREQ_PAL);
    }

    _cia1 = std::make_shared<Mos6526>("CIA1");
    _cia2 = std::make_shared<Mos6526>("CIA2");

    _bus = std::make_shared<cbm_bus::Bus>("BUS");
    _busdev = std::make_shared<C64BusController>(_bus, _cia2);

    auto vic2_mmap = std::make_shared<Vic2ASpace>(_cia2, _ram, _chargen);
    _vic2 = std::make_shared<Mos6569>("VIC2", vic2_mmap, _vram);

    _ioexp = attach_cartridge();
    _io = std::make_shared<C64IO>(_vic2, _sid, _vram, _cia1, _cia2, _ioexp);

    _pla = std::make_shared<PLA>(_ram, _basic, _kernal, _chargen, _io);
    _cpu = std::make_shared<Mos6510>(_pla);

    _clk = std::make_shared<Clock>("CLK", CLOCK_FREQ_PAL, _conf.delay);

    auto unit8 = fs::fix_home(_conf.unit8);
    if (!unit8.empty()) {
        _unit8 = c1541::create(unit8, 8, _bus);
    }

    auto unit9 = fs::fix_home(_conf.unit9);
    if (!unit9.empty()) {
        _unit9 = c1541::create(unit9, 9, _bus);
    }

    _kbd  = std::make_shared<C64Keyboard>("KBD");
    _joy1 = std::make_shared<C64Joystick>("JOY1");
    _joy2 = std::make_shared<C64Joystick>("JOY2");

    if (_conf.vjoy.enabled) {
        _kbd->vjoystick(_conf.vjoy, _joy1);
    }
}

void C64::connect_devices()
{
    /*
     * Connect the CPU ports to the PLA.
     */
    auto cpu_port_read = [this](addr_t addr) -> uint8_t {
        uint8_t data = _pla->mode();
        uint8_t cpuval = ((data & PLA::LORAM)  ? Mos6510::P0 : 0) |
                         ((data & PLA::HIRAM)  ? Mos6510::P1 : 0) |
                         ((data & PLA::CHAREN) ? Mos6510::P2 : 0);

        /* Cassette input pins not implemented */
        cpuval |= (Mos6510::P4 | Mos6510::P5);  /* pull-ups */

        /* P6 and P7 not connected */
        cpuval &= ~(Mos6510::P7 | Mos6510::P6);

        return cpuval;
    };

    auto cpu_port_write = [this](addr_t addr, uint8_t data, bool force) {
        uint8_t plaval = ((data & Mos6510::P0) ? PLA::LORAM  : 0) |
                         ((data & Mos6510::P1) ? PLA::HIRAM  : 0) |
                         ((data & Mos6510::P2) ? PLA::CHAREN : 0);

        _pla->mode(plaval, PLA::LORAM | PLA::HIRAM | PLA::CHAREN, force);

        /* Cassette output pin P3 not implemented */
    };

    _cpu->add_ior(cpu_port_read, Mos6510::PALL);
    _cpu->add_iow(cpu_port_write, Mos6510::P0 | Mos6510::P1 | Mos6510::P2 | Mos6510::P3);

    /*
     * Connect the Expansion port (Cartridge) to the PLA.
     */
    if (_ioexp) {
        /*
         * PLA/Cartridge interaction during memory (re)mapping.
         */
        _pla->extmap([this](addr_t addr, bool romh, bool roml) {
            return _ioexp->getdev(addr, romh, roml);
        });

        /*
         * Connect cartridge output ports EXROM and GAME to the PLA.
         */
        auto cart_port_write = [this](addr_t addr, uint8_t data, bool force) {
            uint8_t plaval = ((data & Cartridge::GAME)  ? PLA::GAME  : 0) |
                             ((data & Cartridge::EXROM) ? PLA::EXROM : 0);

            _pla->mode(plaval, PLA::GAME | PLA::EXROM, force);
        };

        _ioexp->add_iow(cart_port_write, Cartridge::GAME | Cartridge::EXROM);
        _ioexp->reset();
        _cpu->reset();
    }

    /*
     * Connect the CPU irq and nmi pins to CIA1, CIA2 and VIC2 irq outputs.
     */
    auto set_irq = [this](bool active) {
        _cpu->irq_pin(active);
    };

    auto set_nmi = [this](bool active) {
        _cpu->nmi_pin(active);
    };

    _cia1->irq(set_irq);
    _cia2->irq(set_nmi);
    _vic2->irq(set_irq);

    /*
     * Connect the CPU rdy pin to the VIC2 ba pin.
     */
    auto set_rdy = [this](bool active) {
        _cpu->rdy_pin(active);
    };

    _vic2->ba(set_rdy);

    /*
     * Load the VIC2 colour palette.
     */
    if (!_conf.palette.empty()) {
        _vic2->palette(_conf.palette);
    }

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

    auto kbd_write = [this](uint8_t addr, uint8_t value, bool) {
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

    /*
     * Connect the RESTORE key to the CPU nmi pin.
     */
    auto restore_key = [this]() {
        _cpu->nmi_pin(true);
    };

    _kbd->restore_key(restore_key);

    /*
     * Load the keyboard mappings.
     */
    if (!_conf.keymaps.empty()) {
        _kbd->load(_conf.keymaps);
    }

    /*
     * Connect clockable devices to the system clock.
     */
    _clk->add(_vic2);
    _clk->add(_cpu);
    _clk->add(_cia1);
    _clk->add(_cia2);
    _clk->add(_sid);
    _clk->add(_unit8);
    _clk->add(_unit9);
}

void C64::create_ui()
{
    std::string title{_conf.title};
    if (_ioexp) {
        title += " - " + _ioexp->name();
    } else if (!_conf.prgfile.empty()) {
        title += " - " + fs::basename(_conf.prgfile);
    }

    ui::Config uiconf {
        .audio = {
            .enabled    = _conf.audio,
            .srate      = mos_6581::SAMPLING_RATE,
            .channels   = mos_6581::CHANNELS,
            .samples    = mos_6581::SAMPLES
        },
        .video = {
            .title      = title,
            .width      = Mos6569::WIDTH,
            .height     = Mos6569::HEIGHT,
            .fps        = _conf.fps,
            .scale      = _conf.scale,
            .sleffect   = ui::to_sleffect(_conf.scanlines),
            .fullscreen = _conf.fullscreen,
            .sresize    = _conf.sresize
        },
    };

    _ui = ui::UI::instance(uiconf);
}

void C64::make_widgets()
{
    /*
     * Floppy disks presence and idle status.
     */
    auto floppy8 = ui::make_widget<ui::widget::Floppy>(_ui, [this]() {
        using Status = ui::widget::Floppy::Status;
        return (_unit8 ? Status{.is_attached = true, .is_idle = _unit8->is_idle()} :
                         Status{.is_attached = false, .is_idle = true});
    });

    auto floppy9 = ui::make_widget<ui::widget::Floppy>(_ui, [this]() {
        using Status = ui::widget::Floppy::Status;
        return (_unit9 ? Status{.is_attached = true, .is_idle = _unit9->is_idle()} :
                         Status{.is_attached = false, .is_idle = true});
    });

    /*
     * Joystick presence and swap status.
     */
    auto gamepad1 = ui::make_widget<ui::widget::Gamepad>(_ui, [this]() {
        ui::widget::Gamepad::Status st{
            .id = 0,
            .is_connected = (_conf.swapj ? _joy2->is_connected() : _joy1->is_connected()),
            .is_swapped = _conf.swapj
        };
        return st;
    });

    auto gamepad2 = ui::make_widget<ui::widget::Gamepad>(_ui, [this]() {
        ui::widget::Gamepad::Status st{
            .id = 1,
            .is_connected = (_conf.swapj ? _joy1->is_connected() : _joy2->is_connected()),
            .is_swapped = _conf.swapj
        };
        return st;
    });

    auto swapj_action = [this]() {
        /*
         * Click on a gamepad widget swaps joysticks.
         */
        hotkeys(keyboard::KEY_ALT_J);
    };

    gamepad1->action(swapj_action);
    gamepad2->action(swapj_action);

    auto panel = _ui->panel();
    panel->add(floppy8);
    panel->add(floppy9);
    panel->add(gamepad1);
    panel->add(gamepad2);
}

void C64::connect_ui()
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
    _sid->audio_buffer([this]() {
        return _ui->audio_buffer();
    });

    /*
     * Connect the video output.
     */
    _vic2->render_line([this](unsigned line, const ui::Scanline& scanline) {
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

void C64::hotkeys(keyboard::Key key)
{
    /*
     * This methods is called in the context of the UI thread
     * (see connect_ui()).
     */
    switch (key) {
    case keyboard::KEY_ALT_J:
        /*
         * Swap joysticks.
         */
        _conf.swapj ^= true;
        log.debug("Joysticks %sswapped\n", (_conf.swapj ? "" : "un"));
        break;

    case keyboard::KEY_ALT_K:
        /*
         * Toggle virtual joystick active status.
         */
        if (_conf.vjoy.enabled) {
            bool kact = _kbd->active() ^ true;
            _kbd->active(kact);
            log.debug("Keyboard %s\n", (kact ? "active" : "inactive"));
        }
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
        log.debug("System %spaused\n", (_ui->paused() ? "un" : ""));
        _clk->pause(_clk->paused() ^ true);
        break;

    default:;
    }
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
       << "  " << _vram->to_string()    << std::endl
       << "  " << _basic->to_string()   << std::endl
       << "  " << _kernal->to_string()  << std::endl
       << "  " << _chargen->to_string() << std::endl;

    if (_ioexp) {
       os << "  " << _ioexp->to_string() << std::endl;
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
}
