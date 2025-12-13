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

#include "p00file.hpp"
#include "prgfile.hpp"
#include "mos_6581.hpp"
#include "serializer.hpp"

#include "c64_crt.hpp"
#include "c1541_factory.hpp"

namespace caio {
namespace commodore {
namespace c64 {

C64::C64(config::Section& sec)
    : Platform{LABEL},
      _conf{sec}
{
}

bool C64::detect_format(const fs::Path& fname)
{
    if (fname.empty()) {
        return false;
    }

    if (Platform::detect_format(fname)) {
        return true;
    }

    if (Crt::is_crt(fname)) {
        if (!_conf.cartridge.empty()) {
            log.warn("Cartridge file overrided. From {} to {}\n", _conf.cartridge, fname.string());
        }

        _conf.cartridge = fname;
        return true;
    }

    /*
     * PRG files do not have any signature.
     * Assume it is a PRG file.
     */
    if (!_conf.prgfile.empty()) {
        log.warn("Program file overrided. From {} to {}\n", _conf.prgfile, fname.string());
    }

    _conf.prgfile = fname;
    return true;
}

void C64::init_monitor(int ifd, int ofd)
{
    const auto load = [this](const fs::Path& fname, addr_t start) -> std::pair<addr_t, addr_t>{
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

    const auto save = [this](const fs::Path& fname, addr_t start, addr_t end) {
        PrgFile prog{};
        for (auto addr = start; addr <= end; ++addr) {
            uint8_t c = this->_cpu->read(addr);
            prog.push_back(c);
        }
        prog.save(fname, start);
    };

    _cpu->init_monitor(ifd, ofd, load, save);
}

void C64::reset_devices()
{
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
}

std::string C64::to_string_devices() const
{
    return std::format(
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
        "  {}\n"
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
        _vic2->to_string(),
        _cia1->to_string(),
        _cia2->to_string(),
        _sid->to_string(),
        _ram->to_string(),
        _vram->to_string(),
        _basic->to_string(),
        _kernal->to_string(),
        _chargen->to_string(),
        (_ioexp ? _ioexp->to_string() : ""),
        _kbd->to_string(),
        _joy1->to_string(),
        _joy2->to_string(),
        _bus->to_string());
}

void C64::create_devices()
{
    _clk = std::make_shared<Clock>("clk", CLOCK_FREQ, _conf.delay);

    _ram = std::make_shared<RAM>("ram", RAM_SIZE, RAM_INIT_PATTERN1, RAM::PUT_RANDOM_VALUES);
    _basic = std::make_shared<ROM>("basic", rompath(BASIC_FNAME), BASIC_DIGEST);
    _kernal = std::make_shared<ROM>("kernal", rompath(KERNAL_FNAME), KERNAL_DIGEST);
    _chargen = std::make_shared<ROM>("chargen", rompath(CHARGEN_FNAME), CHARGEN_DIGEST);
    _vram = std::make_shared<NibbleRAM>("vram", VRAM_SIZE);
    _sid = std::make_shared<Mos6581>("sid", CLOCK_FREQ);
    _cia1 = std::make_shared<Mos6526>("cia1");
    _cia2 = std::make_shared<Mos6526>("cia2");
    _bus = std::make_shared<cbm_bus::Bus>("bus");
    _busdev = std::make_shared<C64BusController>(_bus, _cia2);

    _vic2_mmap = std::make_shared<Vic2ASpace>(_cia2, _ram, _chargen);
    _vic2 = std::make_shared<Mos6569>("vic2", _vic2_mmap, _vram);

    _ioexp = attach_cartridge();
    _io = std::make_shared<C64IO>(_vic2, _sid, _vram, _cia1, _cia2, _ioexp);

    _pla = std::make_shared<PLA>(_ram, _basic, _kernal, _chargen, _io);
    _cpu = std::make_shared<Mos6510>(_pla);

    if (const auto unit8 = fs::fix_home(_conf.unit8); !unit8.empty()) {
        _unit8 = c1541::instance(unit8, 8, _bus);
    }

    if (const auto unit9 = fs::fix_home(_conf.unit9); !unit9.empty()) {
        _unit9 = c1541::instance(unit9, 9, _bus);
    }

    _kbd  = std::make_shared<C64Keyboard>(_conf.keyboard);
    _joy1 = std::make_shared<Joystick>("joy1", joystick_port());
    _joy2 = std::make_shared<Joystick>("joy2", joystick_port());

    if (_conf.vjoy.enabled) {
        _kbd->vjoystick(_conf.vjoy, _joy1);
    }
}

void C64::connect_devices()
{
    /*
     * Connect the CPU ports to the PLA.
     */
    const auto cpu_port_read = [this](addr_t addr) -> uint8_t {
        uint8_t data = _pla->mode();
        uint8_t cpuval = ((data & PLA::LORAM)  ? Mos6510::P0 : 0) |
                         ((data & PLA::HIRAM)  ? Mos6510::P1 : 0) |
                         ((data & PLA::CHAREN) ? Mos6510::P2 : 0);

        /* Cassette input not implemented */
        cpuval |= Mos6510::P4 | Mos6510::P5;
        return cpuval;
    };

    const auto cpu_port_write = [this](addr_t addr, uint8_t data, bool force) {
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
        const auto cart_port_write = [this](addr_t addr, uint8_t data, bool force) {
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
    const auto set_irq = [this](bool active) {
        _cpu->irq_pin(active);
    };

    const auto set_nmi = [this](bool active) {
        _cpu->nmi_pin(active);
    };

    _cia1->irq(set_irq);
    _cia2->irq(set_nmi);
    _vic2->irq(set_irq);

    /*
     * Connect the CPU rdy pin and the PLA to the VIC2 aec pin.
     */
    const auto set_aec = [this](bool active) {
        _cpu->rdy_pin(active);
    };

    _vic2->aec(set_aec);

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

    const auto kbd_read = [this](uint8_t addr) -> uint8_t {
        switch (addr) {
        case Mos6526::PRA:
            return ~(_conf.swapj ? _joy1->position() : _joy2->position());

        case Mos6526::PRB:
            return (_kbd->read() & ~(_conf.swapj ? _joy2->position() : _joy1->position()));

        default:;
        }

        return 255; /* Pull-ups */
    };

    const auto kbd_write = [this](uint8_t addr, uint8_t value, bool) {
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
    const auto restore_key = [this]() {
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

    attach_prg();
}

void C64::make_widgets()
{
    /*
     * Floppy disks presence and idle status.
     */
    const auto floppy_status = [](const sptr_t<C1541>& unit) {
        using Status = ui::widget::Floppy::Status;
        return (unit ? Status{ .is_attached = true,  .is_idle = unit->is_idle(), .progress = unit->progress()} :
                       Status{ .is_attached = false, .is_idle = true,            .progress = -1.0f});
    };

    const auto unit8_status = std::bind(floppy_status, _unit8);
    const auto unit9_status = std::bind(floppy_status, _unit9);

    _floppy8 = ui::make_widget<ui::widget::Floppy>(ui(), unit8_status);
    _floppy9 = ui::make_widget<ui::widget::Floppy>(ui(), unit9_status);

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
    panel->add(_floppy8);
    panel->add(_floppy9);
    panel->add(_gamepad1);
    panel->add(_gamepad2);
}

void C64::connect_ui()
{
    Platform::connect_ui();

    /*
     * Connect the audio output.
     */
    _sid->audio_buffer([this]() {
        return ui()->audio_buffer();
    });

    /*
     * Connect the video output.
     */
    _vic2->render_line([this](unsigned line, const ui::Scanline& scanline) {
        ui()->render_line(line, scanline);
    });

    /*
     * Connect keyboard and joysticks.
     */
    ui()->keyboard(_kbd);
    ui()->joystick({_joy1, _joy2});
}

void C64::hotkeys(keyboard::Key key)
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
        _gamepad1->action();    /* Swap action, swap value as excepted         */
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

fs::Path C64::rompath(const fs::Path& fname)
{
    const auto path = fs::search(fname, { _conf.romdir });
    if (path.empty()) {
        throw IOError{"Can't load ROM: {}: {}", fname.string(), Error::to_string(ENOENT)};
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
        throw IOError{"Can't load Cartridge: {}: {}", _conf.cartridge, Error::to_string(ENOENT)};
    }

    auto cart = Cartridge::instance(fpath);
    return cart;
}

void C64::attach_prg()
{
    if (_conf.prgfile.empty()) {
        return;
    }

    fs::Path prgfile{fs::search(_conf.prgfile)};
    if (prgfile.empty()) {
        throw IOError{"Can't load program: {}: {}", _conf.prgfile, Error::to_string()};
    }

    try {
        PrgFile* prog{};
        const char* format{};

        log.debug("Preloading program: {}\n", prgfile.string());

        try {
            prog = new P00File{prgfile};
            format = "P00";
        } catch (const IOError&) {
            prog = new PrgFile{prgfile};
            format = "PRG";
        }

        log.debug("Detected format: {}, start address: ${:04X}, size: {} (${:04X})\n", format, prog->address(),
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

ui::Config C64::ui_config()
{
    std::string title = _conf.title;
    std::string session = LABEL;

    if (!_conf.prgfile.empty()) {
        const auto& basename = fs::basename(_conf.prgfile);
        title = std::format("{} - {}", title, basename.string());
        session = (basename.extension().empty() ? basename : basename.stem());

    } else if (_ioexp) {
        const auto& basename = fs::basename(_ioexp->name());
        title = std::format("{} - {}", title, basename.string());
        session = (basename.extension().empty() ? basename : basename.stem());
    }

    const ui::Config uiconf {
        .name               = session,
        .snapshotdir        = _conf.snapshotdir,
        .audio = {
            .enabled        = _conf.audio,
            .srate          = mos::mos_6581::SAMPLING_RATE,
            .channels       = mos::mos_6581::CHANNELS,
            .samples        = mos::mos_6581::SAMPLES
        },
        .video = {
            .title          = title,
            .width          = Mos6569::WIDTH,
            .height         = Mos6569::HEIGHT,
            .scale          = _conf.scale,
            .aspect         = _conf.aspect,
            .sleffect       = _conf.scanlines,
            .fullscreen     = _conf.fullscreen,
            .sresize        = _conf.sresize,
            .statusbar      = _conf.statusbar,
            .screenshotdir  = _conf.screenshotdir
        }
    };

    return uiconf;
}

void C64::serdes(Serializer& ser)
{
    ser & *this;
}

Serializer& operator&(Serializer& ser, C64& c64)
{
    ser & static_cast<Platform&>(c64)
        & c64._ram
        & c64._vram
        & c64._sid
        & c64._cia1
        & c64._cia2
        & c64._bus
        & c64._busdev
        & c64._vic2_mmap
        & c64._vic2
        & c64._ioexp
        & c64._io
        & c64._pla
        & c64._cpu;

    return ser;
}

}
}
}
