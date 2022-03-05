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


namespace cemu {
namespace c64 {

C64::C64(const C64Config &conf)
    : _conf{conf}
{
}

C64::~C64()
{
}

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

inline std::string C64::palettepath(const std::string &fname) const
{
    return fs::search(fname, {_conf.palettedir});
}

inline std::string C64::keymapspath(const std::string &fname) const
{
    return fs::search(fname, {_conf.keymapsdir});
}

std::shared_ptr<Cartridge> C64::attach_cartridge()
{
    if (!_conf.cartfile.empty()) {
        std::string fpath{cartpath(_conf.cartfile)};
        auto cart = Cartridge::create(fpath);
        if (cart) {
            _conf.title += " - " + cart->name();
        }

        return cart;
    }

    return {};
}

void C64::attach_prg()
{
    if (!_conf.prgfile.empty()) {
        try {
            PrgFile *prog;
            const char *format;

            log.debug("Preloading program: " + _conf.prgfile + "\n");

            try {
                prog = new P00File{fs::fix_home(_conf.prgfile)};
                format = "P00";
            } catch (const IOError &) {
                prog = new PrgFile{fs::fix_home(_conf.prgfile)};
                format = "PRG";
            }

            log.debug("Detected format: %s, start address: $%04X, size: %d ($%04X)\n", format, prog->address(),
                prog->size(), prog->size());

            _conf.title += " - " + fs::basename(_conf.prgfile);

            _cpu->bpadd(BASIC_READY_ADDR, [](Mos6510 &cpu, void *arg) {
                /*
                 * Load PRG into memory.
                 */
                auto *prog = static_cast<PrgFile *>(arg);
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
                delete prog;
            }, prog);

        } catch (const std::exception &ex) {
            throw IOError{ex};
        }
    }
}

void C64::create_devices()
{
    _ram = std::make_shared<DeviceRAM>("SYSTEM RAM", 65536);
    _basic = std::make_shared<DeviceROM>(rompath(BASIC_FNAME), "BASIC", BASIC_SIZE);
    _kernal = std::make_shared<DeviceROM>(rompath(KERNAL_FNAME), "KERNAL", KERNAL_SIZE);
    _chargen = std::make_shared<DeviceROM>(rompath(CHARGEN_FNAME), "CHARGEN", CHARGEN_SIZE);
    _vcolor = std::make_shared<NibbleRAM>("COLOR RAM", VCOLOR_SIZE);

    if (_conf.resid) {
        _sid = std::make_shared<Mos6581Resid>("reSID", CLOCK_FREQ_PAL);
    } else {
        _sid = std::make_shared<Mos6581>("SID", CLOCK_FREQ_PAL);
    }

    _cia1 = std::make_shared<Mos6526>("CIA1");
    _cia2 = std::make_shared<Mos6526>("CIA2");

    _bus = std::make_shared<cbm_bus::Bus>("C64 BUS");
    _busdev = std::make_shared<C64BusController>(_bus, _cia2);

    auto vic2_mmap = std::make_shared<Vic2ASpace>(_cia2, _ram, _chargen);
    _vic2 = std::make_shared<Mos6569>("VIC-II", vic2_mmap, _vcolor);

    _ioexp = attach_cartridge();
    _io = std::make_shared<C64IO>(_vic2, _sid, _vcolor, _cia1, _cia2, _ioexp);

    _pla = std::make_shared<PLA>(_ram, _basic, _kernal, _chargen, _io);
    _cpu = std::make_shared<Mos6510>(_pla);

    _clk = std::make_shared<Clock>("SYSTEM CLOCK", CLOCK_FREQ_PAL, _conf.delay);

    if (!_conf.unit8.empty()) {
        _unit8 = c1541::create(_conf.unit8, 8, _bus);
    }

    if (!_conf.unit9.empty()) {
        _unit9 = c1541::create(_conf.unit9, 9, _bus);
    }

    _kbd  = std::make_shared<C64Keyboard>("C64 KBD");
    _joy1 = std::make_shared<C64Joystick>("C64 JOY1");
    _joy2 = std::make_shared<C64Joystick>("C64 JOY2");
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
        return cpuval;
    };

    auto cpu_port_write = [this](addr_t addr, uint8_t data) {
        uint8_t plaval = ((data & Mos6510::P0) ? PLA::LORAM  : 0) |
                         ((data & Mos6510::P1) ? PLA::HIRAM  : 0) |
                         ((data & Mos6510::P2) ? PLA::CHAREN : 0);
        _pla->mode(plaval, PLA::LORAM | PLA::HIRAM | PLA::CHAREN);
    };

    _cpu->add_ior(cpu_port_read, Mos6510::P0 | Mos6510::P1 | Mos6510::P2);
    _cpu->add_iow(cpu_port_write, Mos6510::P0 | Mos6510::P1 | Mos6510::P2);

    /*
     * Connect the Cartridge to the PLA.
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
        auto cart_port_write = [this](addr_t addr, uint8_t data) {
            uint8_t plaval = ((data & Cartridge::GAME)  ? PLA::GAME  : 0) |
                             ((data & Cartridge::EXROM) ? PLA::EXROM : 0);
            _pla->mode(plaval, PLA::GAME | PLA::EXROM);
        };

        _ioexp->add_iow(cart_port_write, Cartridge::GAME | Cartridge::EXROM);
        _ioexp->reset();
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

    /*
     * Configure the VIC2 video controller.
     */
    auto set_irq = [this](bool active) {
        _cpu->irq_pin() = active;
    };

    auto set_nmi = [this](bool active) {
        _cpu->nmi_pin() = active;
    };

    auto set_rdy = [this](bool active) {
        _cpu->rdy_pin() = active;
    };

    auto vsync = [this](unsigned wait_cycles) {
        _clk->sync(wait_cycles);
    };

    _vic2->irq(set_irq);
    _vic2->ba(set_rdy);
    _vic2->vsync(vsync);

    if (!_conf.palettefile.empty()) {
        const auto ppath = palettepath(_conf.palettefile);
        if (ppath.empty()) {
            throw Error{"Palette file not found: " + _conf.palettefile};
        }

        _vic2->palette(ppath);
    }

    _vic2->render_line([this](unsigned line, const ui::Scanline &scanline) {
        _ui->render_line(line, scanline);
    });

    /*
     * Connect CIA 1/2 IRQ output pins to the CPU IRQ and NMI input pins.
     */
    _cia1->irq(set_irq);
    _cia2->irq(set_nmi);

    /*
     * Configure the audio chip.
     */
    _sid->audio_buffer([this]() {
        return _ui->audio_buffer();
    });

    /*
     * Configure keyboard and joysticks and connect them to CIA1.
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

    _cia1->add_ior(kbd_read, KBD_MASK);
    _cia1->add_iow(kbd_write, KBD_MASK);

    if (!_conf.keymapsfile.empty()) {
        const auto kpath = keymapspath(_conf.keymapsfile);
        if (kpath.empty()) {
            throw Error{"Keymaps file not found: " + _conf.keymapsfile};
        }

        _kbd->load(kpath);
    }

    auto restore_key = [this]() {
        _cpu->nmi_pin() = true;
    };

    static_pointer_cast<C64Keyboard>(_kbd)->restore_key(restore_key);

    /*
     * Connect the keyboard and joysticks to the UI.
     */
    auto hotkeys = [this](Keyboard::Key key) {
        switch (key) {
        case Keyboard::KEY_ALT_J:
            /* Swap joysticks */
            _conf.swapj ^= true;
            log.debug("Joysticks %sswapped\n", (_conf.swapj ? "" : "un"));
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
            if (!_paused) {
                break;
            }

            /* CTRL+C forces resume from pause */
            /* PASSTHROUGH */

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

    _ui->keyboard(_kbd);
    _ui->joystick({_joy1, _joy2});
    _ui->hotkeys(hotkeys);
}

void C64::create_ui()
{
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
            .panel         = _conf.panel,
        },
    };

    _ui = std::make_shared<ui::UI>(uiconf);
}

void C64::make_widgets()
{
    /*
     * Floppy disks presence and idle status.
     */
    auto floppy8 = ui::widget::create<ui::widget::Floppy>([this]() {
        ui::widget::Floppy::Status st{};
        if (_unit8) {
            st.is_idle = _unit8->is_idle();
            st.is_attached = true;
        } else {
            st.is_idle = true;
            st.is_attached = false;
        }

        return st;
    });

    auto floppy9 = ui::widget::create<ui::widget::Floppy>([this]() {
        ui::widget::Floppy::Status st{};
        if (_unit9) {
            st.is_idle = _unit9->is_idle();
            st.is_attached = true;
        } else {
            st.is_idle = true;
            st.is_attached = false;
        }

        return st;
    });

    /*
     * Joystick presence and swap status.
     */
    auto gamepad1 = ui::widget::create<ui::widget::Gamepad>([this]() {
        ui::widget::Gamepad::Status st{};
        st.is_swapped = _conf.swapj;
        st.is_connected = (_conf.swapj ? _joy2->is_connected() : _joy1->is_connected());
        return st;
    });

    auto gamepad2 = ui::widget::create<ui::widget::Gamepad>([this]() {
        ui::widget::Gamepad::Status st{};
        st.is_swapped = _conf.swapj;
        st.is_connected = (_conf.swapj ? _joy1->is_connected() : _joy2->is_connected());
        return st;
    });

    auto swapj_action = [this]() {
        /*
         * Let the user swap joysticks by clicking one of the gamepad widgets.
         */
        _conf.swapj ^= true;
    };

    gamepad1->action(swapj_action);
    gamepad2->action(swapj_action);

    auto panel = _ui->panel();
    panel->add(floppy8);
    panel->add(floppy9);
    panel->add(gamepad1);
    panel->add(gamepad2);
}

void C64::reset()
{
    create_devices();
    create_ui();

    connect_devices();
    make_widgets();

    attach_prg();
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
    log.info("Starting CEMU v" + cemu::version() + " - C64\n" + to_string() + "\n");

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
