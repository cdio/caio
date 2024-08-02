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
#include "mos_6569.hpp"

#include "endian.hpp"
#include "logger.hpp"

namespace caio {
namespace mos {

/*
 * https://www.colodore.com
 */
RgbaTable Mos6569::builtin_palette{
    0x000000FF,
    0xFFFFFFFF,
    0x813338FF,
    0x75CEC8FF,
    0x8E3C97FF,
    0x56AC4DFF,
    0x2E2C9BFF,
    0xEDF171FF,
    0x8E5029FF,
    0x553800FF,
    0xC46C71FF,
    0x4A4A4AFF,
    0x7B7B7BFF,
    0xA9FF9FFF,
    0x706DEBFF,
    0xB2B2B2FF
};

const Mos6569::CData Mos6569::idle_cdata{
    .ch     = 0,
    .mcm    = false,
    .colors = []() { return Rgba4{}; }
};

Mos6569::Mos6569(std::string_view label, const sptr_t<ASpace>& mmap, const devptr_t& vcolor)
    : Device{TYPE, label},
      Clockable{},
      _mmap{mmap},
      _vcolor{vcolor},
      _palette{builtin_palette},
      _scanline(WIDTH)
{
    std::fill(std::begin(_cdata), std::end(_cdata), idle_cdata);
}

Mos6569::~Mos6569()
{
}

void Mos6569::render_line(const Renderer& rl)
{
    _render_line = rl;
}

void Mos6569::palette(std::string_view fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }
}

void Mos6569::palette(const RgbaTable& plt)
{
    _palette = plt;
}

void Mos6569::irq(const OutputPinCb& irq_out)
{
    _irq_out = irq_out;
}

void Mos6569::aec(const OutputPinCb& aec_out)
{
    _aec_out = aec_out;
}

void Mos6569::trigger_lp()
{
    if (!_lp_latched) {
        _lp_latched = true;

        _lp_x = static_cast<uint8_t>((_cycle << 3) >> 1);

        _lp_y = ((_raster_counter >= DISPLAY_Y_START && _raster_counter < DISPLAY_Y_END) ?
            static_cast<uint8_t>(_raster_counter - DISPLAY_Y_START) :
            ((_raster_counter >= DISPLAY_Y_END) ? 255 : 0));

        trigger_irq_if(REG_INTERRUPT_LP, true);
    }
}

void Mos6569::reset()
{
    _irq_status = 0;
    irq_out(false);
    aec_out(true);
}

size_t Mos6569::size() const
{
    return Registers::REGMAX;
}

uint8_t Mos6569::dev_read(addr_t addr, ReadMode mode)
{
    uint8_t data{};

    switch (addr) {
    case REG_MOB_0_X:
        return static_cast<uint8_t>(_mob_coord_x[0] & 255);

    case REG_MOB_1_X:
        return static_cast<uint8_t>(_mob_coord_x[1] & 255);

    case REG_MOB_2_X:
        return static_cast<uint8_t>(_mob_coord_x[2] & 255);

    case REG_MOB_3_X:
        return static_cast<uint8_t>(_mob_coord_x[3] & 255);

    case REG_MOB_4_X:
        return static_cast<uint8_t>(_mob_coord_x[4] & 255);

    case REG_MOB_5_X:
        return static_cast<uint8_t>(_mob_coord_x[5] & 255);

    case REG_MOB_6_X:
        return static_cast<uint8_t>(_mob_coord_x[6] & 255);

    case REG_MOB_7_X:
        return static_cast<uint8_t>(_mob_coord_x[7] & 255);

    case REG_MOB_0_Y:
        return _mob_coord_y[0];

    case REG_MOB_1_Y:
        return _mob_coord_y[1];

    case REG_MOB_2_Y:
        return _mob_coord_y[2];

    case REG_MOB_3_Y:
        return _mob_coord_y[3];

    case REG_MOB_4_Y:
        return _mob_coord_y[4];

    case REG_MOB_5_Y:
        return _mob_coord_y[5];

    case REG_MOB_6_Y:
        return _mob_coord_y[6];

    case REG_MOB_7_Y:
        return _mob_coord_y[7];

    case REG_MOBS_MSB_X:
        for (auto mob = 0; mob < 8; ++mob) {
            data |= ((_mob_coord_x[mob] > 255) ? (1 << mob) : 0);
        }
        return data;

    case REG_CONTROL_1:
        data = (_raster_counter > 255 ? REG_CONTROL_1_RC8  : 0) |
               (_ecm_mode             ? REG_CONTROL_1_ECM  : 0) |
               (_bmm_mode             ? REG_CONTROL_1_BMM  : 0) |
               (_den                  ? REG_CONTROL_1_DEN  : 0) |
               (_25_rows              ? REG_CONTROL_1_RSEL : 0) |
               _scroll_y;
        return data;

    case REG_RASTER_COUNTER:
        return static_cast<uint8_t>(_raster_counter & 255);

    case REG_LIGHT_PEN_X:
        return _lp_x;

    case REG_LIGHT_PEN_Y:
        return _lp_y;

    case REG_MOB_ENABLE:
        return _mob_enable;

    case REG_CONTROL_2:
        /*
         * Bits 7 and 6 set to 1 regardless of the written value,
         * Bit 5 (REG_CONTROL_2_RES) ignored.
         * (see C64 Programmer's Reference Guide, page 448).
         */
        data = 0xC0 |
               (_mcm_mode   ? REG_CONTROL_2_MCM  : 0) |
               (_40_columns ? REG_CONTROL_2_CSEL : 0) |
               _scroll_x;
        return data;

    case REG_MOB_Y_EXPANSION:
        return _mob_expand_y;

    case REG_MEMORY_POINTERS:
        /*
         * Bit 0 set to 1 regardless of the written value
         * (see MOS6569 Data Sheet Preliminary, page 14).
         */
        return 1 |
               ((_video_matrix >>  6) & REG_MEMORY_POINTERS_VIDEO) |
               ((_char_base    >> 10) & REG_MEMORY_POINTERS_CHAR);

    case REG_INTERRUPT:
        /*
         * Bits 6, 5 and 4 set to 1 regardless of the written value
         * (see MOS6569 Data Sheet Preliminary, page 14).
         */
        data = 0x70 | _irq_status;
        return data;

    case REG_INTERRUPT_ENABLE:
        /*
         * Bits 7, 6, 5 and 4 set to 1 regardless of the written value
         * (see MOS6569 Data Sheet Preliminary, page 14).
         */
        data = 0xF0 | _irq_enable;
        return data;

    case REG_MOB_DATA_PRI:
        return _mob_data_priority;

    case REG_MOB_MULTICOLOR_SEL:
        return _mob_mcm_sel;

    case REG_MOB_X_EXPANSION:
        return _mob_expand_x;

    case REG_MOB_MOB_COLLISION:
        /*
         * MOB-MOB collision register automatically cleared when read.
         */
        data = _mob_mob_collision;
        if (mode != ReadMode::Peek) {
            const_cast<Mos6569*>(this)->_mob_mob_collision = 0;
        }
        return data;

    case REG_MOB_DATA_COLLISION:
        /*
         * MOB-DATA collision register automatically cleared when read.
         */
        data = _mob_data_collision;
        if (mode != ReadMode::Peek) {
            const_cast<Mos6569*>(this)->_mob_data_collision = 0;
        }
        return data;

    case REG_BORDER_COLOR:
        return (0xF0 | static_cast<uint8_t>(_border_color));

    case REG_BACKGROUND_COLOR_0:
    case REG_BACKGROUND_COLOR_1:
    case REG_BACKGROUND_COLOR_2:
    case REG_BACKGROUND_COLOR_3:
        return (0xF0 | static_cast<uint8_t>(_background_color[addr - REG_BACKGROUND_COLOR_0]));

    case REG_MOB_MULTICOLOR_0:
    case REG_MOB_MULTICOLOR_1:
        return (0xF0 | static_cast<uint8_t>(_mob_mcm[addr - REG_MOB_MULTICOLOR_0]));

    case REG_MOB_0_COLOR:
    case REG_MOB_1_COLOR:
    case REG_MOB_2_COLOR:
    case REG_MOB_3_COLOR:
    case REG_MOB_4_COLOR:
    case REG_MOB_5_COLOR:
    case REG_MOB_6_COLOR:
    case REG_MOB_7_COLOR:
        return (0xF0 | static_cast<uint8_t>(_mob_color[addr - REG_MOB_0_COLOR]));

    default:
        return 0;
    }
}

void Mos6569::dev_write(addr_t addr, uint8_t data)
{
    unsigned prev_stored{};

    switch (addr) {
    case REG_MOB_0_X:
        _mob_coord_x[0] = (_mob_coord_x[0] & 0x0100) | data;
        break;

    case REG_MOB_1_X:
        _mob_coord_x[1] = (_mob_coord_x[1] & 0x0100) | data;
        break;

    case REG_MOB_2_X:
        _mob_coord_x[2] = (_mob_coord_x[2] & 0x0100) | data;
        break;

    case REG_MOB_3_X:
        _mob_coord_x[3] = (_mob_coord_x[3] & 0x0100) | data;
        break;

    case REG_MOB_4_X:
        _mob_coord_x[4] = (_mob_coord_x[4] & 0x0100) | data;
        break;

    case REG_MOB_5_X:
        _mob_coord_x[5] = (_mob_coord_x[5] & 0x0100) | data;
        break;

    case REG_MOB_6_X:
        _mob_coord_x[6] = (_mob_coord_x[6] & 0x0100) | data;
        break;

    case REG_MOB_7_X:
        _mob_coord_x[7] = (_mob_coord_x[7] & 0x0100) | data;
        break;

    case REG_MOB_0_Y:
        _mob_coord_y[0] = data;
        break;

    case REG_MOB_1_Y:
        _mob_coord_y[1] = data;
        break;

    case REG_MOB_2_Y:
        _mob_coord_y[2] = data;
        break;

    case REG_MOB_3_Y:
        _mob_coord_y[3] = data;
        break;

    case REG_MOB_4_Y:
        _mob_coord_y[4] = data;
        break;

    case REG_MOB_5_Y:
        _mob_coord_y[5] = data;
        break;

    case REG_MOB_6_Y:
        _mob_coord_y[6] = data;
        break;

    case REG_MOB_7_Y:
        _mob_coord_y[7] = data;
        break;

    case REG_MOBS_MSB_X:
        for (auto mob = 0; mob < 8; ++mob) {
            _mob_coord_x[mob] = (_mob_coord_x[mob] & 0x00FF) | ((data & (1 << mob)) ? 0x0100 : 0);
        }
        break;

    case REG_CONTROL_1:
        prev_stored    = _stored_raster;
        _stored_raster = (_stored_raster & 0x00FF) | ((data & REG_CONTROL_1_RC8) ? 0x0100 : 0);
        _ecm_mode      = (data & REG_CONTROL_1_ECM);
        _bmm_mode      = (data & REG_CONTROL_1_BMM);
        _mode          = video_mode(_ecm_mode, _bmm_mode, _mcm_mode);
        _den           = (data & REG_CONTROL_1_DEN);
        _scroll_y      = data & REG_CONTROL_1_YSCROLL;
        _25_rows       = (data & REG_CONTROL_1_RSEL);
        if (_25_rows) {
            _uborder_end   = UBORDER_Y_END;
            _bborder_start = BBORDER_Y_START;
        } else {
            _uborder_end   = UBORDER_Y_END + 4;
            _bborder_start = BBORDER_Y_START - 4;
        }
        set_badline();
        trigger_irq_if(REG_INTERRUPT_RST, prev_stored != _stored_raster && _raster_counter == _stored_raster);
        break;

    case REG_RASTER_COUNTER:
        prev_stored = _stored_raster;
        _stored_raster = (_stored_raster & 0x0100) | data;
        trigger_irq_if(REG_INTERRUPT_RST, prev_stored != _stored_raster && _raster_counter == _stored_raster);
        break;

    case REG_LIGHT_PEN_X:
    case REG_LIGHT_PEN_Y:
        break;

    case REG_MOB_ENABLE:
        _mob_enable = data;
        break;

    case REG_CONTROL_2:
        /*
         * Bit 5 (REG_CONTROL_2_RES) ignored (see C64 Programmer's Reference Guide, page 448).
         */
        _mcm_mode   = (data & REG_CONTROL_2_MCM);
        _mode       = video_mode(_ecm_mode, _bmm_mode, _mcm_mode);
        _scroll_x   = data & REG_CONTROL_2_XSCROLL;
        _40_columns = (data & REG_CONTROL_2_CSEL);
        if (_40_columns) {
            _lborder_end   = LBORDER_X_END;
            _rborder_start = RBORDER_X_START;
        } else {
            _lborder_end   = LBORDER_X_END + 8;
            _rborder_start = RBORDER_X_START - 8;
        }
        break;

    case REG_MOB_Y_EXPANSION:
        _mob_expand_y = data;
        break;

    case REG_MEMORY_POINTERS:
        _video_matrix = static_cast<addr_t>((data & REG_MEMORY_POINTERS_VIDEO) << 6);
        _bitmap_base  = static_cast<addr_t>((data & REG_MEMORY_POINTERS_CB13) ? (1 << 13) : 0);
        _char_base    = static_cast<addr_t>((data & REG_MEMORY_POINTERS_CHAR) << 10);
        break;

    case REG_INTERRUPT:
        /*
         * The REG_INTERRUPT register is a read-only status register.
         * By delivering a write to this register the user acknowledges up
         * to four interrupt sources. Only when all interrupt sources are
         * acknowledged the IRQ pin is de-activated.
         */
        data = (~data) & (_irq_status & REG_INTERRUPT_MASK);
        _irq_status = (_irq_status & REG_INTERRUPT_IRQ) | data;
        if ((_irq_status & _irq_enable) == 0) {
            /*
             * No more ACKs pending.
             */
            irq_out(false);
        }
        break;

    case REG_INTERRUPT_ENABLE:
        _irq_enable = data & REG_INTERRUPT_MASK;
        break;

    case REG_MOB_DATA_PRI:
        _mob_data_priority = data;
        break;

    case REG_MOB_MULTICOLOR_SEL:
        _mob_mcm_sel = data;
        break;

    case REG_MOB_X_EXPANSION:
        _mob_expand_x = data;
        break;

    case REG_MOB_MOB_COLLISION:
    case REG_MOB_DATA_COLLISION:
        /* Read-only */
        break;

    case REG_BORDER_COLOR:
        _border_color = static_cast<Color>(data & Color::MASK);
        break;

    case REG_BACKGROUND_COLOR_0:
    case REG_BACKGROUND_COLOR_1:
    case REG_BACKGROUND_COLOR_2:
    case REG_BACKGROUND_COLOR_3:
        _background_color[addr - REG_BACKGROUND_COLOR_0] = static_cast<Color>(data & Color::MASK);
        break;

    case REG_MOB_MULTICOLOR_0:
    case REG_MOB_MULTICOLOR_1:
        _mob_mcm[addr - REG_MOB_MULTICOLOR_0] = static_cast<Color>(data & Color::MASK);
        break;

    case REG_MOB_0_COLOR:
    case REG_MOB_1_COLOR:
    case REG_MOB_2_COLOR:
    case REG_MOB_3_COLOR:
    case REG_MOB_4_COLOR:
    case REG_MOB_5_COLOR:
    case REG_MOB_6_COLOR:
    case REG_MOB_7_COLOR:
        _mob_color[addr - REG_MOB_0_COLOR] = static_cast<Color>(data & Color::MASK);
        break;

    default:;
    }
}

std::ostream& Mos6569::dump(std::ostream& os, addr_t base) const
{
    uint8_t regs[Registers::REGMAX];
    uint8_t r{};
    std::for_each(std::begin(regs), std::end(regs), [this, &r](uint8_t& reg) { reg = peek(r++); });
    return utils::dump(os, regs, base);
}

size_t Mos6569::tick(const Clock& clk)
{
    bool aec_pin = _aec_pin;

    switch (_cycle) {
    case 0:
        ++_raster_counter;
        if (_raster_counter == FRAME_HEIGHT) {
            _raster_counter = 0;
            _lp_latched = false;
            _vc_base = 0;
        }
        _vblank = ((_raster_counter < VISIBLE_Y_START) || (_raster_counter >= VISIBLE_Y_END));
        if (_vblank) {
            _bad_line = false;
            aec_pin = true;
        } else {
            if (_raster_counter == DISPLAY_Y_START) {
                _bl_den = _den;
            }
            set_badline();
            if (_bad_line) {
                _idle_mode = false;
            }
        }
        aec_pin = ps_access(3);
        break;

    case 1:
        break;

    case 2:
        aec_pin = ps_access(4);
        break;

    case 3:
        break;

    case 4:
        aec_pin = ps_access(5);
        break;

    case 5:
        break;

    case 6:
        aec_pin = ps_access(6);
        break;

    case 7:
        break;

    case 8:
        aec_pin = ps_access(7);
        break;

    case 9:
        break;

    case 10:
        aec_pin = true;
        break;

    case 11:
        break;

    case 12:
        trigger_irq_if(REG_INTERRUPT_RST, _stored_raster == _raster_counter);   /* FIXME cycles 0 and 1 */
        break;

    case 13:
        _vc = _vc_base;
        _col = 0;
        if (_bad_line) {
            _rc = 0;
        }
        break;

    case 14:
        if (_raster_counter == _bborder_start) {
            _vertical_border = true;
        } else if (_den && _raster_counter == _uborder_end) {
            _vertical_border = false;
        }
        if (!_vertical_border) {
            _main_border = false;
        }
        aec_pin = c_access();
        break;

    case 15: case 16: case 17:
    case 18: case 19: case 20:
    case 21: case 22: case 23:
    case 24: case 25: case 26:
    case 27: case 28: case 29:
    case 30: case 31: case 32:
    case 33: case 34: case 35:
    case 36: case 37: case 38:
    case 39: case 40: case 41:
    case 42: case 43: case 44:
    case 45: case 46: case 47:
    case 48: case 49: case 50:
    case 51: case 52: case 53:
        g_access();
        aec_pin = c_access();
        break;

    case 54:
        g_access();
        aec_pin = true;
        if (!_40_columns) {
            _main_border = true;
        }
        break;

    case 55:
        break;

    case 56:
        if (_40_columns) {
            _main_border = true;
        }
        paint_mobs();
        paint_borders();
        render_line();
        trigger_irq_if(REG_INTERRUPT_MDC, _mob_data_collision != 0);
        trigger_irq_if(REG_INTERRUPT_MMC, _mob_mob_collision != 0);
        reset_collision_data();
        reset_collision_mob();
        std::for_each(std::begin(_mdata), std::end(_mdata), [this](auto& md) { mob_dma_off(md); });
        paint(0, 0, _palette[_background_color[0]]);    // FIXME
        break;

    case 57:
        if (_rc == 7) {
            _idle_mode = true;
            _vc_base = _vc;
        }
        if (!_idle_mode) {
            ++_rc;
        }
        aec_pin = ps_access(0);
        break;

    case 58:
        break;

    case 59:
        aec_pin = ps_access(1);
        break;

    case 60:
        break;

    case 61:
        aec_pin = ps_access(2);
        break;

    case 62:
        if (_raster_counter == _bborder_start) {
            _vertical_border = true;
        } else if (_den && _raster_counter == _uborder_end) {
            _vertical_border = false;
        }
        break;
    }

    if (aec_pin != _aec_pin) {
        aec_out(aec_pin);
        _aec_pin = aec_pin;
    }

    _cycle = (_cycle + 1) % SCANLINE_CYCLES;
    return 1;
}

inline void Mos6569::set_badline()
{
    _bad_line = _bl_den &&  (_raster_counter >= DISPLAY_Y_START && _raster_counter < DISPLAY_Y_END) &&
        ((_raster_counter & SCROLL_Y_MASK) == _scroll_y);
}

bool Mos6569::c_access()
{
    if (_idle_mode) {
        return true;
    }

    const uint8_t v_data = _vcolor->read(_vc) & Color::MASK;
    const uint8_t c_data = _mmap->read(_video_matrix | _vc);

    CData& cd = _cdata[_col];

    switch (_mode) {
    case VideoMode::TextStandard:
        /*
         * c_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   +--+--+--+--+--+--+--+--> Character code
         *
         * v_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  X  X  +--+--+--+--> Foreground colour
         *
         * Hi-Res 2 colours:
         *  pixel 0 = Background colour 0
         *  pixel 1 = Foreground colour
         */
        cd = CData{
            .ch = c_data,
            .mcm = false,
            .colors = [this, v_data]() {
                return Rgba4{
                    _palette[_background_color[0]],
                    _palette[v_data]
                };
            }
        };
        break;

    case VideoMode::TextMulticolor:
        /*
         * c_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   +--+--+--+--+--+--+--+--> Character code
         *
         * v_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  X  X  |  +--+--+--> Foreground colour
         *               +-----------> Multicolor flag
         *
         * Multicolor flag set: Lo-Res 4 colours
         *  pixel 00 = Background colour 0
         *  pixel 01 = Background colour 1
         *  pixel 10 = Background colour 2
         *  pixel 11 = Foreground colour (limited to 8 colours)
         *
         * Multicolor flag not set: Hi-Res 2 colours
         *  pixel 0 = Background colour 0
         *  pixel 1 = Foreground colour (limited to 8 colours)
         */
        if (v_data & 8) {
            cd = CData{
                .ch = c_data,
                .mcm = true,
                .colors = [this, v_data]() {
                    return Rgba4{
                        _palette[_background_color[0]],
                        _palette[_background_color[1]],
                        _palette[_background_color[2]],
                        _palette[v_data & 7]
                    };
                }
            };
        } else {
            cd = CData{
                .ch = c_data,
                .mcm = false,
                .colors = [this, fg = v_data]() {
                    return Rgba4{
                        _palette[_background_color[0]],
                        _palette[fg]
                    };
                }
            };
        }
        break;

    case VideoMode::TextExtendedColor:
        /*
         * c_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   |  |  +--+--+--+--+--+--> Character code
         *   +--+--------------------> Background selection (0-3)
         *
         * v_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  X  X  +--+--+--+--> Foreground colour
         *
         * Hi-Res 2 colors:
         *  pixel 0 = Background bg colour (bg = background selection)
         *  pixel 1 = Foreground colour
         */
        cd = CData{
            .ch = static_cast<uint8_t>(c_data & 63),
            .mcm = false,
            .colors = [this, fg = v_data, bg = c_data >> 6]() {
                return Rgba4{
                    _palette[_background_color[bg]],
                    _palette[fg]
                };
            }
        };
        break;

    case VideoMode::BitmapStandard:
        /*
         * c_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   |  |  |  |  +--+--+--+--> Pixel 0 colour
         *   +--+--+--+--------------> Pixel 1 colour
         *
         * v_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  X  X  X  X  X  X
         *
         * Hi-Res 2 colours
         */
        cd = CData{
            .mcm = false,
            .colors = [this, px0 = c_data & Color::MASK, px1 = c_data >> 4]() {
                return Rgba4{
                    _palette[px0],
                    _palette[px1]
                };
            }
        };
        break;

    case VideoMode::BitmapMulticolor:
        /*
         * c_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   |  |  |  |  +--+--+--+--> Pixel 10 colour
         *   +--+--+--+--------------> Pixel 01 colour
         *
         * v_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  X  X  +--+--+--+--> Pixel 11 colour
         *
         * Pixel 00 colour = Background colour 0
         * Lo-Res 4 colours
         */
        cd = CData{
            .mcm = true,
            .colors = [this, px01 = c_data >> 4, px10 = c_data & Color::MASK, px11 = v_data]() {
                return Rgba4{
                    _palette[_background_color[0]],
                    _palette[px01],
                    _palette[px10],
                    _palette[px11],
                };
            }
        };
        break;

    case VideoMode::TextInvalid:
        /*
         * c_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  +--+--+--+--+--+--> Character code
         *
         * v_data:
         *  D7 D6 D5 D4 D3 D2 D1 D0
         *   |  |  |  |  |  |  |  |
         *   X  X  X  X  |  X  X  X
         *               +-----------> Multicolor flag
         */
        cd = CData{
            .ch  = static_cast<uint8_t>(c_data & 63),
            .mcm = static_cast<bool>(v_data & 8),
            .colors = [this]() {
                return Rgba4{
                    _palette[Color::BLACK],
                    _palette[Color::BLACK],
                    _palette[Color::BLACK],
                    _palette[Color::BLACK]
                };
            }
        };
        break;

    case VideoMode::BitmapInvalid:
        cd = CData{
            .mcm = false,
            .colors = [this]() {
                return Rgba4{
                    _palette[Color::BLACK],
                    _palette[Color::BLACK],
                    _palette[Color::BLACK],
                    _palette[Color::BLACK]
                };
            }
        };
        break;

    case VideoMode::BitmapMulticolorInvalid:
        cd = CData{
            .mcm = true,
            .colors = [this]() {
                return Rgba4{
                    _palette[Color::BLACK],
                    _palette[Color::BLACK],
                    _palette[Color::BLACK],
                    _palette[Color::BLACK]
                };
            }
        };
        break;
    }

    return !_bad_line;
}

void Mos6569::g_access()
{
    if (_idle_mode) {
#if 0 // FIXME
        const addr_t g_addr = (_ecm_mode ? 0x39FF : 0x3FFF);
        const uint8_t g_data = _mmap->read(g_addr);
        paint_display(g_data, idle_cdata.colors(), idle_cdata.mcm);
#endif
        return;
    }

    /*
     * Display mode.
     */
    const CData& cd = _cdata[_col];
    addr_t g_addr;

    switch (_mode) {
    case VideoMode::TextStandard:
    case VideoMode::TextMulticolor:
    case VideoMode::TextExtendedColor:
    case VideoMode::TextInvalid:
        /*
         * All text modes:
         *  g_addr:
         *   A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
         *    |   |   |   |   |  |  |  |  |  |  |  |  |  |
         *    |   |   |   |   |  |  |  |  |  |  |  +--+--+--> RC
         *    |   |   |   +---+--+--+--+--+--+--+-----------> CDATA
         *    +---+---+-------------------------------------> CB13-CB11
         *
         * TextStandard:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> 8 pixels (1 bit per pixel)
         *
         * TextMulticolor:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> mcm=false: 8 pixels (1 bit per pixel)
         *                              mcm=true:  4 pixels (2 bits per pixel)
         *
         * TextExtendedColor:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> mcm=false: 8 pixels (1 bit per pixel)
         *                              mcm=true:  4 pixels (2 bits per pixel)
         *
         * TextInvalid:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> mcm=false: 8 pixels (1 bit per pixel)
         *                              mcm=true:  4 pixels (2 bits per pixel)
         */
        g_addr = _char_base | (cd.ch << 3) | _rc;
        break;

    case VideoMode::BitmapStandard:
    case VideoMode::BitmapMulticolor:
        /*
         * All valid bitmap modes:
         *  g_addr:
         *   A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
         *    |   |   |   |   |  |  |  |  |  |  |  |  |  |
         *    |   |   |   |   |  |  |  |  |  |  |  +--+--+--> RC
         *    |   +---+---+---+--+--+--+--+--+--+-----------> VC9-VC0
         *    +---------------------------------------------> CB13
         *
         * BitmapStandard:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> 8 pixels (1 bit per pixel)
         *
         * BitmapMulticolor:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> 4 pixels (2 bits per pixel)
         */
        g_addr = _bitmap_base | (_vc << 3) | _rc;
        break;

    case VideoMode::BitmapInvalid:
    case VideoMode::BitmapMulticolorInvalid:
        /*
         * All invalid bitmap modes:
         *  g_addr:
         *   A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
         *    |   |   |   |   |  |  |  |  |  |  |  |  |  |
         *    |   |   |   0   0  |  |  |  |  |  |  +--+--+--> RC
         *    |   |   |          +--+--+--+--+--+-----------> VC5-VC0
         *    |   +---+-------------------------------------> VC9-VC8
         *    +---------------------------------------------> CB13
         *
         * BitmapInvalid:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> 8 pixels (1 bit per pixel)
         *
         * BitmapMulticolorInvalid:
         *  g_data:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    +--+--+--+--+--+--+--+--> 4 pixels (2 bits per pixel)
         */
        g_addr = _bitmap_base | ((_vc & 0b1100111111) << 3) | _rc;
    }

    const uint8_t g_data = _mmap->read(g_addr);
    paint_display(g_data, cd.colors(), cd.mcm);
    ++_vc;
    ++_col;
}

inline bool Mos6569::mob_dma_on(uint8_t mob)
{
    auto& md = _mdata[mob];
    if (!md.dma) {
        const uint8_t sbit = 1 << mob;
        const bool enabled = _mob_enable & sbit;
        const uint16_t line = (_raster_counter + static_cast<uint16_t>(mob < 3)) & 255;
        const uint16_t coord_y = mob_coord_y(mob);
        const bool visible = enabled && (line == coord_y);
        const bool exp_y = _mob_expand_y & sbit;
        if (visible) {
            md = SData{
                .dma     = true,
                .visible = true,
                .exp_y   = exp_y,
                .mc      = 0
            };
        }
    }

    return md.dma;
}

inline void Mos6569::mob_dma_off(SData& md)
{
    if (md.dma) {
        md.dma = (md.mc < MOB_SIZE);
    }
}

bool Mos6569::ps_access(uint8_t mob)
{
    if (!mob_dma_on(mob)) {
        return true;
    }

    auto& md = _mdata[mob];

    /*
     * p-access:
     *  p_addr:
     *   A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
     *    |   |   |   |   |  |  |  |  |  |  |  |  |  |
     *    |   |   |   |   1  1  1  1  1  1  1  +--+--+--> Sprite number
     *    +---+---+---+---------------------------------> VM13-VM10
     *
     *  p_data:
     *   D7 D6 D5 D4 D3 D2 D1 D0
     *    |  |  |  |  |  |  |  |
     *    +--+--+--+--+--+--+--+--> Sprite pointer (MP7-MP0)
     */
    const addr_t p_addr = _video_matrix | 0b1111111000 | mob;
    const addr_t mp = _mmap->read(p_addr);

    /*
     * s-access:
     *  s_addr:
     *   A13 A12 A11 A10 A9 A8 A7 A6 A5 A4 A3 A2 A1 A0
     *    |   |   |   |   |  |  |  |  |  |  |  |  |  |
     *    |   |   |   |   |  |  |  |  +--+--+--+--+--+--> MC5-MC0
     *    +---+---+---+---+--+--+--+--------------------> MP7-MP0
     *
     *  s_data:
     *   D7 D6 D5 D4 D3 D2 D1 D0
     *    |  |  |  |  |  |  |  |
     *    +--+--+--+--+--+--+--+--> 8 pixels (1 bit per pixel) MxMC = 0
     *                              4 pixels (2 bit per pixel) MxMC = 1
     *
     * 1 bit per pixel:
     *  0: Transparent
     *  1: Color REG_MOB_x_COLOR
     *
     * 2 bits per pixel:
     *  00: Transparent
     *  01: Sprite colour REG_MOB_MULTICOLOR_0
     *  10: Sprite colour REG_MOB_x_COLOR
     *  11: Sprite colour REG_MOB_MULTICOLOR_1
     */
    const addr_t s_addr = (mp << 6) | md.mc;
    md.byte1 = _mmap->read(s_addr + 0);
    md.byte2 = _mmap->read(s_addr + 1);
    md.byte3 = _mmap->read(s_addr + 2);
    if (!md.exp_y) {
        md.exp_y = (_mob_expand_y & (1 << mob));
        md.mc += 3;
    } else {
        md.exp_y = false;
    }

    return false;
}

inline void Mos6569::paint_display(uint8_t bitmap, const Rgba4& colors, bool mcm)
{
    const unsigned start = DISPLAY_X_START + (_col << 3) + _scroll_x;

    if (mcm) {
        paint_mcm_byte(start, bitmap, colors);
    } else {
        paint_byte(start, bitmap, colors);
    }

    update_collision_data(start, bitmap);
}

inline void Mos6569::paint_mobs()
{
    /*
     * MOB 0: Highest priority (front);
     * MOB 7: Lowest priority (back).
     */
    for (uint8_t mob = 7; mob != 255; --mob) {
        paint_mob(mob);
    }
}

void Mos6569::paint_mob(uint8_t mob)
{
    auto& md = _mdata[mob];
    if (md.dma && md.visible) {
        const uint8_t sbit   = 1 << mob;
        const bool exp_x     = _mob_expand_x & sbit;
        const uint16_t pos_x = mob_coord_x(mob);
        const bool bg_pri    = _mob_data_priority & sbit;
        const bool mcm       = _mob_mcm_sel & sbit;

        const auto [collision, bitmap, visible_bitmap] =
            mob_bitmap(pos_x, md.byte1, md.byte2, md.byte3, exp_x, mcm, bg_pri);

        if (mcm) {
            /*
             * 4 colours, 2 bits per pixel.
             */
            const Rgba4 colors{
                Rgba::transparent,
                _palette[_mob_mcm[0]],
                _palette[_mob_color[mob]],
                _palette[_mob_mcm[1]]
            };

            paint_mob_mcm(pos_x, visible_bitmap, colors, exp_x);

        } else {
            /*
             * 2 colours, 1 bits per pixel.
             */
            const Rgba4 colors{
                Rgba::transparent,
                _palette[_mob_color[mob]]
            };

            paint_mob(pos_x, visible_bitmap, colors, exp_x);
        }

        if (!_mob_data_collision && collision) {
            /*
             * Collision with the background image.
             */
            _mob_data_collision = sbit;
        }

        const auto cbit = update_collision_mob(mob, pos_x, mcm, bitmap);
        if (!_mob_mob_collision && cbit) {
            /*
             * Collision with another sprite.
             */
            _mob_mob_collision = (sbit | cbit);
        }
    }
}

void Mos6569::paint_mob(unsigned start, uint64_t bitmap, const Rgba4& colors, bool exp_x)
{
    paint_byte(start,      (bitmap >> 56),       colors);
    paint_byte(start + 8,  (bitmap >> 48) & 255, colors);
    paint_byte(start + 16, (bitmap >> 40) & 255, colors);
    if (exp_x) {
        paint_byte(start + 24, (bitmap >> 32) & 255, colors);
        paint_byte(start + 32, (bitmap >> 24) & 255, colors);
        paint_byte(start + 40, (bitmap >> 16) & 255, colors);
    }
}

void Mos6569::paint_mob_mcm(unsigned start, uint64_t bitmap, const Rgba4& colors, bool exp_x)
{
    paint_mcm_byte(start,      (bitmap >> 56),       colors);
    paint_mcm_byte(start + 8,  (bitmap >> 48) & 255, colors);
    paint_mcm_byte(start + 16, (bitmap >> 40) & 255, colors);
    if (exp_x) {
        paint_mcm_byte(start + 24, (bitmap >> 32) & 255, colors);
        paint_mcm_byte(start + 32, (bitmap >> 24) & 255, colors);
        paint_mcm_byte(start + 40, (bitmap >> 16) & 255, colors);
    }
}

void Mos6569::paint_byte(unsigned start, uint8_t bitmap, const Rgba4& colors)
{
    if (start < _scanline.size()) {
        const auto bg_color = colors[0];
        const auto fg_color = colors[1];
        uint8_t bit = 128;
        for (auto it = _scanline.begin() + start; bit != 0 && it != _scanline.end(); ++it, bit >>= 1) {
            auto color = ((bitmap & bit) ? fg_color : bg_color);
            it->set(color);
        }
    }
}

void Mos6569::paint_mcm_byte(unsigned start, uint8_t bitmap, const Rgba4& colors)
{
    if (start + 1 < _scanline.size()) {
        auto it = _scanline.begin() + start;
        for (int shift = 6; shift >= 0; shift -= 2) {
            const size_t index = (bitmap >> shift) & 3;
            const auto color = colors[index];
            it->set(color);
            if (++it == _scanline.end()) {
                break;
            }
            it->set(color);
            if (++it == _scanline.end()) {
                break;
            }
        }
    }
}

void Mos6569::paint_borders()
{
    if (!_bl_den) {
        /*
         * Display not enabled.
         */
        paint(0, 0, _palette[_border_color]);

    } else {
        if (_main_border) {
            /*
             * Left and right borders.
             */
            paint(0, _lborder_end, _palette[_border_color]);
            paint(_rborder_start, 0, _palette[_border_color]);
        }

        if (_vertical_border && (_raster_counter < _uborder_end || _raster_counter >= _bborder_start)) {
            /*
             * Upper and bottom borders.
             */
            paint(_lborder_end, _rborder_start - _lborder_end, _palette[_border_color]);
        }
    }
}

void Mos6569::paint(unsigned start, unsigned width, Rgba color)
{
    if (start < _scanline.size()) {
        if (width == 0 || start + width > _scanline.size()) {
            width = _scanline.size() - start;
        }

        std::fill_n(_scanline.begin() + start, width, color);
    }
}

inline void Mos6569::render_line()
{
    if (!_vblank && _render_line) {
        _render_line(_raster_counter - VISIBLE_Y_START, _scanline);
    }
}

void Mos6569::irq_out(bool active)
{
    const bool irq_pin = (_irq_status & REG_INTERRUPT_IRQ);

    if (irq_pin != active) {
        if (active) {
            _irq_status |= REG_INTERRUPT_IRQ;
        } else {
            _irq_status &= ~REG_INTERRUPT_IRQ;
        }

        if (_irq_out) {
            _irq_out(active);
        }
    }
}

void Mos6569::trigger_irq_if(uint8_t iflag, bool cond)
{
    if (cond) {
        _irq_status |= iflag;
        if ((_irq_enable & iflag) != 0) {
            irq_out(true);
        }
    }
}

void Mos6569::aec_out(bool active)
{
    if (active != _aec_pin) {
        _aec_pin = active;
        if (_aec_out) {
            _aec_out(active);
        }
    }
}

inline uint16_t Mos6569::mob_coord_x(uint8_t mob) const
{
    return (_mob_coord_x[mob] + MOB_X_COORD_OFFSET);
}

inline uint16_t Mos6569::mob_coord_y(uint8_t mob) const
{
    return (_mob_coord_y[mob] + MOB_Y_COORD_OFFSET);
}

std::tuple<bool, uint64_t, uint64_t>
Mos6569::mob_bitmap(unsigned start, uint8_t byte1, uint8_t byte2, uint8_t byte3, bool exp_x, bool mcm, bool bg_pri)
{
    const auto start_byte = start >> 3;
    const auto start_bit = start - (start_byte << 3);

    uint64_t bitmap;
    uint64_t mask;

    if (exp_x) {
        uint64_t w1, w2, w3;
        if (mcm) {
            w1 = utils::expand_dibits(byte1);
            w2 = utils::expand_dibits(byte2);
            w3 = utils::expand_dibits(byte3);
        } else {
            w1 = utils::expand_bits(byte1);
            w2 = utils::expand_bits(byte2);
            w3 = utils::expand_bits(byte3);
        }

        bitmap = (w1 << 48) | (w2 << 32) | (w3 << 16);
        mask = 0xFFFFFFFFFFFF0000ULL;

    } else {
        bitmap = (static_cast<uint64_t>(byte1) << 56) |
                 (static_cast<uint64_t>(byte2) << 48) |
                 (static_cast<uint64_t>(byte3) << 40);

        mask = 0xFFFFFF0000000000ULL;
    }

    const uint64_t background     = be64toh(*reinterpret_cast<uint64_t*>(&_collision_data[start_byte])) << start_bit;
    const uint64_t collision      = mask & background & bitmap;
    const uint64_t visible_bitmap = ((collision && bg_pri) ? (~collision & bitmap) : bitmap);

    return {collision, bitmap, visible_bitmap};
}

uint8_t Mos6569::update_collision_mob(uint8_t mob, unsigned start, bool mcm, uint64_t bitmap)
{
    if (mcm) {
        /*
         * Multicolor sprites use 00 as transparent colour (no content),
         * the following converts 01 and 10 to 11 to generate the collision bitmap.
         */
        std::span data{reinterpret_cast<uint8_t*>(&bitmap), sizeof(bitmap)};
        utils::convert_01_10_to_11(data);
    }

    _mob_bitmaps[mob] = bitmap;

    /*
     * Detect a MOB-MOB collision only if there is no other MOB-MOB collision.
     */
    if (!_mob_mob_collision) {
        for (uint8_t mob_c = 0; mob_c < _mob_bitmaps.size(); ++mob_c) {
            const uint8_t sbit_c = (1 << mob_c);
            if (mob_c != mob && (_mob_enable & sbit_c)) {
                const uint16_t start_c = mob_coord_x(mob_c);

                if (!((start_c + MOB_MAX_X_SIZE) < start || (start + MOB_MAX_X_SIZE) < start_c)) {

                    auto cbitmap = _mob_bitmaps[mob_c];
                    bitmap = _mob_bitmaps[mob];

                    const int shift = static_cast<int>(start) - static_cast<int>(start_c);
                    if (shift < 0) {
                        bitmap <<= -shift;
                    } else {
                        cbitmap <<= shift;
                    }

                    if (cbitmap & bitmap) {
                        /*
                         * MOB-MOB collision detected.
                         */
                        return sbit_c;
                    }
                }
            }
        }
    }

    /*
     * No MOB-MOB collisions.
     */
    return 0;
}

inline void Mos6569::reset_collision_data()
{
    _collision_data.fill(0);
}

inline void Mos6569::reset_collision_mob()
{
    _mob_bitmaps.fill(0);
}

void Mos6569::update_collision_data(unsigned start, uint8_t bitmap)
{
    const auto start_byte   = start >> 3;
    const uint8_t start_bit = start - (start_byte << 3);
    if (start_bit == 0) {
        _collision_data[start_byte] = bitmap;
    } else {
        const uint8_t byte1 = bitmap >> start_bit;
        const uint8_t byte2 = bitmap << (8 - start_bit);
        const uint8_t mask  = 0xFF >> start_bit;
        const uint8_t prev1 = _collision_data[start_byte]     & ~mask;
        const uint8_t prev2 = _collision_data[start_byte + 1] &  mask;
        _collision_data[start_byte]     = byte1 | prev1;
        _collision_data[start_byte + 1] = byte2 | prev2;
    }
}

inline void Mos6569::update_collision_data_mcm(unsigned start, uint8_t bitmap)
{
    /*
     * In multicolor mode 00 and 01 pixels are considered background colours,
     * the following converts 01 to 00 and 10 to 11 to generate the collision bitmap.
     */
    update_collision_data(start, utils::convert_01_10(bitmap));
}

}
}
