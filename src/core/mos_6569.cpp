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

#include <gsl/assert>

#include "endian.hpp"
#include "logger.hpp"
#include "utils.hpp"


namespace caio {

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

Mos6569::Mos6569(const std::string& label, const sptr_t<ASpace>& mmap, const devptr_t& vcolor)
    : Device{TYPE, label},
      Clockable{},
      _mmap{mmap},
      _vcolor{vcolor},
      _palette{builtin_palette},
      _scanline(WIDTH)
{
}

void Mos6569::render_line(const std::function<void(unsigned, const ui::Scanline&)>& rl)
{
    _render_line = rl;
}

void Mos6569::palette(const std::string& fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }
}

void Mos6569::palette(const RgbaTable& plt)
{
    _palette = plt;
}

void Mos6569::vsync(const std::function<void(unsigned)>& cb)
{
    _vsync = cb;
}

void Mos6569::irq(const OutputPinCb& irq_out)
{
    _irq_out = irq_out;
}

void Mos6569::ba(const OutputPinCb& ba_out)
{
    _ba_out = ba_out;
}

void Mos6569::trigger_lp()
{
    if (!_light_pen_latched) {
        _light_pen_latched = true;

        _light_pen_x = static_cast<uint8_t>((_cycle << 3) >> 1);

        _light_pen_y = ((_raster_counter >= DISPLAY_Y_START && _raster_counter < DISPLAY_Y_END) ?
            static_cast<uint8_t>(_raster_counter - DISPLAY_Y_START) : ((_raster_counter >= DISPLAY_Y_END) ? 255 : 0));

        if ((_irq_status & REG_INTERRUPT_LP) == 0) {
            _irq_status |= REG_INTERRUPT_LP;
            if ((_irq_enable & REG_INTERRUPT_LP) != 0) {
                irq_out(true);
            }
        }
    }
}

void Mos6569::reset()
{
    irq_out(false);
    ba_out(true);
}

size_t Mos6569::size() const
{
    return Registers::REGMAX;
}

uint8_t Mos6569::read(addr_t addr, ReadMode mode)
{
    using namespace gsl;
    Expects(addr < REGMAX);

    uint8_t data{};

    switch (addr) {
    case REG_MIB_0_X:
        return static_cast<uint8_t>(_mib_coord_x[0] & 255);

    case REG_MIB_1_X:
        return static_cast<uint8_t>(_mib_coord_x[1] & 255);

    case REG_MIB_2_X:
        return static_cast<uint8_t>(_mib_coord_x[2] & 255);

    case REG_MIB_3_X:
        return static_cast<uint8_t>(_mib_coord_x[3] & 255);

    case REG_MIB_4_X:
        return static_cast<uint8_t>(_mib_coord_x[4] & 255);

    case REG_MIB_5_X:
        return static_cast<uint8_t>(_mib_coord_x[5] & 255);

    case REG_MIB_6_X:
        return static_cast<uint8_t>(_mib_coord_x[6] & 255);

    case REG_MIB_7_X:
        return static_cast<uint8_t>(_mib_coord_x[7] & 255);

    case REG_MIB_0_Y:
        return _mib_coord_y[0];

    case REG_MIB_1_Y:
        return _mib_coord_y[1];

    case REG_MIB_2_Y:
        return _mib_coord_y[2];

    case REG_MIB_3_Y:
        return _mib_coord_y[3];

    case REG_MIB_4_Y:
        return _mib_coord_y[4];

    case REG_MIB_5_Y:
        return _mib_coord_y[5];

    case REG_MIB_6_Y:
        return _mib_coord_y[6];

    case REG_MIB_7_Y:
        return _mib_coord_y[7];

    case REG_MIBS_MSB_X:
        for (auto mib = 0; mib < 8; ++mib) {
            data |= ((_mib_coord_x[mib] > 255) ? (1 << mib) : 0);
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
        return _light_pen_x;

    case REG_LIGHT_PEN_Y:
        return _light_pen_y;

    case REG_MIB_ENABLE:
        return _mib_enable;

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

    case REG_MIB_Y_EXPANSION:
        return _mib_expand_y;

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

    case REG_MIB_DATA_PRI:
        return _mib_data_priority;

    case REG_MIB_MULTICOLOR_SEL:
        return _mib_multicolor_sel;

    case REG_MIB_X_EXPANSION:
        return _mib_expand_x;

    case REG_MIB_MIB_COLLISION:
        /*
         * MIB-MIB collision register automatically cleared when read.
         */
        data = _mib_mib_collision;
        if (mode != ReadMode::Peek) {
            const_cast<Mos6569*>(this)->_mib_mib_collision = 0;
        }
        return data;

    case REG_MIB_DATA_COLLISION:
        /*
         * MIB-DATA collision register automatically cleared when read.
         */
        data = _mib_data_collision;
        if (mode != ReadMode::Peek) {
            const_cast<Mos6569*>(this)->_mib_data_collision = 0;
        }
        return data;

    case REG_BORDER_COLOR:
        return static_cast<uint8_t>(_border_color);

    case REG_BACKGROUND_COLOR_0:
    case REG_BACKGROUND_COLOR_1:
    case REG_BACKGROUND_COLOR_2:
    case REG_BACKGROUND_COLOR_3:
        return static_cast<uint8_t>(_background_color[addr - REG_BACKGROUND_COLOR_0]);

    case REG_MIB_MULTICOLOR_0:
    case REG_MIB_MULTICOLOR_1:
        return static_cast<uint8_t>(_mib_multicolor[addr - REG_MIB_MULTICOLOR_0]);

    case REG_MIB_0_COLOR:
    case REG_MIB_1_COLOR:
    case REG_MIB_2_COLOR:
    case REG_MIB_3_COLOR:
    case REG_MIB_4_COLOR:
    case REG_MIB_5_COLOR:
    case REG_MIB_6_COLOR:
    case REG_MIB_7_COLOR:
        return static_cast<uint8_t>(_mib_color[addr - REG_MIB_0_COLOR]);

    default:
        return 0;
    }
}

void Mos6569::write(addr_t addr, uint8_t data)
{
    using namespace gsl;
    Expects(addr < REGMAX);

    switch (addr) {
    case REG_MIB_0_X:
        _mib_coord_x[0] = (_mib_coord_x[0] & 0x0100) | data;
        break;

    case REG_MIB_1_X:
        _mib_coord_x[1] = (_mib_coord_x[1] & 0x0100) | data;
        break;

    case REG_MIB_2_X:
        _mib_coord_x[2] = (_mib_coord_x[2] & 0x0100) | data;
        break;

    case REG_MIB_3_X:
        _mib_coord_x[3] = (_mib_coord_x[3] & 0x0100) | data;
        break;

    case REG_MIB_4_X:
        _mib_coord_x[4] = (_mib_coord_x[4] & 0x0100) | data;
        break;

    case REG_MIB_5_X:
        _mib_coord_x[5] = (_mib_coord_x[5] & 0x0100) | data;
        break;

    case REG_MIB_6_X:
        _mib_coord_x[6] = (_mib_coord_x[6] & 0x0100) | data;
        break;

    case REG_MIB_7_X:
        _mib_coord_x[7] = (_mib_coord_x[7] & 0x0100) | data;
        break;

    case REG_MIB_0_Y:
        _mib_coord_y[0] = data;
        break;

    case REG_MIB_1_Y:
        _mib_coord_y[1] = data;
        break;

    case REG_MIB_2_Y:
        _mib_coord_y[2] = data;
        break;

    case REG_MIB_3_Y:
        _mib_coord_y[3] = data;
        break;

    case REG_MIB_4_Y:
        _mib_coord_y[4] = data;
        break;

    case REG_MIB_5_Y:
        _mib_coord_y[5] = data;
        break;

    case REG_MIB_6_Y:
        _mib_coord_y[6] = data;
        break;

    case REG_MIB_7_Y:
        _mib_coord_y[7] = data;
        break;

    case REG_MIBS_MSB_X:
        for (auto mib = 0; mib < 8; ++mib) {
            _mib_coord_x[mib] = (_mib_coord_x[mib] & 0x00FF) | ((data & (1 << mib)) ? 0x0100 : 0);
        }
        break;

    case REG_CONTROL_1:
        _stored_raster = (_stored_raster & 0x00FF) | ((data & REG_CONTROL_1_RC8) ? 0x0100 : 0);
        _ecm_mode      = (data & REG_CONTROL_1_ECM);
        _bmm_mode      = (data & REG_CONTROL_1_BMM);
        _den           = (data & REG_CONTROL_1_DEN);
        _25_rows       = (data & REG_CONTROL_1_RSEL);
        _scroll_y      = data & REG_CONTROL_1_YSCROLL;

        if (_25_rows) {
            _uborder_end   = UBORDER_Y_END;
            _bborder_start = BBORDER_Y_START;
        } else {
            _uborder_end   = UBORDER_Y_END + 4;
            _bborder_start = BBORDER_Y_START - 4;
        }

        break;

    case REG_RASTER_COUNTER:
        _stored_raster = (_stored_raster & 0x0100) | data;
        break;

    case REG_LIGHT_PEN_X:
    case REG_LIGHT_PEN_Y:
        break;

    case REG_MIB_ENABLE:
        _mib_enable = data;
        break;

    case REG_CONTROL_2:
        /*
         * Bit 5 (REG_CONTROL_2_RES) ignored (see C64 Programmer's Reference Guide, page 448).
         */
        _mcm_mode   = (data & REG_CONTROL_2_MCM);
        _40_columns = (data & REG_CONTROL_2_CSEL);
        _scroll_x   = data & REG_CONTROL_2_XSCROLL;

        if (_40_columns) {
            _lborder_end   = LBORDER_X_END;
            _rborder_start = RBORDER_X_START;
        } else {
            _lborder_end   = LBORDER_X_END + 8;
            _rborder_start = RBORDER_X_START - 8;
        }

        break;

    case REG_MIB_Y_EXPANSION:
        _mib_expand_y = data;
        break;

    case REG_MEMORY_POINTERS:
        _video_matrix = static_cast<addr_t>(data & REG_MEMORY_POINTERS_VIDEO) << 6;
        _bitmap_base  = ((data & REG_MEMORY_POINTERS_CB13) ? (1 << 13) : 0);
        _char_base    = static_cast<addr_t>(data & REG_MEMORY_POINTERS_CHAR) << 10;
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

    case REG_MIB_DATA_PRI:
        _mib_data_priority = data;
        break;

    case REG_MIB_MULTICOLOR_SEL:
        _mib_multicolor_sel = data;
        break;

    case REG_MIB_X_EXPANSION:
        _mib_expand_x = data;
        break;

    case REG_MIB_MIB_COLLISION:
    case REG_MIB_DATA_COLLISION:
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

    case REG_MIB_MULTICOLOR_0:
    case REG_MIB_MULTICOLOR_1:
        _mib_multicolor[addr - REG_MIB_MULTICOLOR_0] = static_cast<Color>(data & Color::MASK);
        break;

    case REG_MIB_0_COLOR:
    case REG_MIB_1_COLOR:
    case REG_MIB_2_COLOR:
    case REG_MIB_3_COLOR:
    case REG_MIB_4_COLOR:
    case REG_MIB_5_COLOR:
    case REG_MIB_6_COLOR:
    case REG_MIB_7_COLOR:
        _mib_color[addr - REG_MIB_0_COLOR] = static_cast<Color>(data & Color::MASK);
        break;

    default:;
    }
}

std::ostream& Mos6569::dump(std::ostream& os, addr_t base) const
{
    std::array<uint8_t, Registers::REGMAX> regs = {
        peek(REG_MIB_0_X),
        peek(REG_MIB_0_Y),
        peek(REG_MIB_1_X),
        peek(REG_MIB_1_Y),
        peek(REG_MIB_2_X),
        peek(REG_MIB_2_Y),
        peek(REG_MIB_3_X),
        peek(REG_MIB_3_Y),
        peek(REG_MIB_4_X),
        peek(REG_MIB_4_Y),
        peek(REG_MIB_5_X),
        peek(REG_MIB_5_Y),
        peek(REG_MIB_6_X),
        peek(REG_MIB_6_Y),
        peek(REG_MIB_7_X),
        peek(REG_MIB_7_Y),
        peek(REG_MIBS_MSB_X),
        peek(REG_CONTROL_1),
        peek(REG_RASTER_COUNTER),
        peek(REG_LIGHT_PEN_X),
        peek(REG_LIGHT_PEN_Y),
        peek(REG_MIB_ENABLE),
        peek(REG_CONTROL_2),
        peek(REG_MIB_Y_EXPANSION),
        peek(REG_MEMORY_POINTERS),
        peek(REG_INTERRUPT),
        peek(REG_INTERRUPT_ENABLE),
        peek(REG_MIB_DATA_PRI),
        peek(REG_MIB_MULTICOLOR_SEL),
        peek(REG_MIB_X_EXPANSION),
        peek(REG_MIB_MIB_COLLISION),
        peek(REG_MIB_DATA_COLLISION),
        peek(REG_BORDER_COLOR),
        peek(REG_BACKGROUND_COLOR_0),
        peek(REG_BACKGROUND_COLOR_1),
        peek(REG_BACKGROUND_COLOR_2),
        peek(REG_BACKGROUND_COLOR_3),
        peek(REG_MIB_MULTICOLOR_0),
        peek(REG_MIB_MULTICOLOR_1),
        peek(REG_MIB_0_COLOR),
        peek(REG_MIB_1_COLOR),
        peek(REG_MIB_2_COLOR),
        peek(REG_MIB_3_COLOR),
        peek(REG_MIB_4_COLOR),
        peek(REG_MIB_5_COLOR),
        peek(REG_MIB_6_COLOR),
        peek(REG_MIB_7_COLOR)
    };

    return utils::dump(os, regs, base);
}

size_t Mos6569::tick(const Clock& clk)
{
    switch (_cycle) {
    case 0:
        ++_raster_counter;

        if (_raster_counter == FRAME_HEIGHT) {
            _raster_counter = 0;
            if (_vsync) {
                /*
                 * Vertical sync started.
                 */
                _vsync(FRAME_CYCLES);
            }

            _light_pen_latched = false;
            _bad_line = false;
            _video_counter = 0;
        }

        _vblank = ((_raster_counter < VISIBLE_Y_START) || (_raster_counter >= VISIBLE_Y_END));

        if (_vblank) {
            _bad_line = false;
            ba_out(true);

        } else {
#if 0
            if (_raster_counter == _stored_raster) {
                _irq_status |= REG_INTERRUPT_RST;
                if ((_irq_enable & REG_INTERRUPT_RST) != 0) {
                    irq_out(true);
                }
            }
#endif
            if (_raster_counter == DISPLAY_Y_START) {
                _bl_den = _den;
            }

            _bad_line = _bl_den && (_raster_counter >= DISPLAY_Y_START) && (_raster_counter < DISPLAY_Y_END) &&
                ((_raster_counter & SCROLL_Y_MASK) == _scroll_y);

            if (_bad_line) {
                _idle_mode = false;
            }

            ba_out(!is_mib_visible(_raster_counter, 3));
        }

        break;

    case 1:
#if 0
        if (_raster_counter == 0 && _stored_raster == 0) {
            _irq_status |= REG_INTERRUPT_RST;
            if ((_irq_enable & REG_INTERRUPT_RST) != 0) {
                irq_out(true);
            }
        }
#endif
        break;

    case 2:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter, 4));
        }
        break;

    case 3:
        break;

    case 4:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter, 5));
        }
        break;

    case 5:
        break;

    case 6:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter, 6));
        }
        break;

    case 7:
        break;

    case 8:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter, 7));
        }
        break;

    case 9:
        break;

    case 10:
#if 1
        if (_raster_counter == _stored_raster) {    /* FIXME: This must be done on cycles 0 and 1 */
            _irq_status |= REG_INTERRUPT_RST;
            if ((_irq_enable & REG_INTERRUPT_RST) != 0) {
                irq_out(true);
            }
        }
#endif
        break;

    case 11:
        if (!_vblank) {
            ba_out(!_bad_line);
        }
        break;

    case 12:
        break;

    case 13:
        if (_bad_line) {
            _row_counter = 0;
        }
        break;

    case 14:
        break;

    case 15:
        if (!_vblank && _40_columns) {
            if (_raster_counter == _bborder_start) {
                _vertical_border = true;
            } else if (_raster_counter == _uborder_end && _bl_den) {
                _vertical_border = false;
            }

            if (!_vertical_border) {
                _main_border = false;
            }
        }
        break;

    case 16:
        if (!_vblank && !_40_columns) {
            if (_raster_counter == _bborder_start) {
                _vertical_border = true;
            } else if (_raster_counter == _uborder_end && _bl_den) {
                _vertical_border = false;
            }

            if (!_vertical_border) {
                _main_border = false;
            }
        }

        /* PASSTHROUGH */

    case 17:
    case 18:
    case 19:
    case 20:
    case 21:
    case 22:
    case 23:
    case 24:
    case 25:
    case 26:
    case 27:
    case 28:
    case 29:
    case 30:
    case 31:
    case 32:
    case 33:
    case 34:
    case 35:
    case 36:
    case 37:
    case 38:
    case 39:
    case 40:
    case 41:
    case 42:
    case 43:
    case 44:
    case 45:
    case 46:
    case 47:
    case 48:
    case 49:
    case 50:
    case 51:
    case 52:
    case 53:
    case 54:
        if (!_vblank) {
            unsigned x = (_cycle - 16) << 3;
            paint_display_cycle(x);
        }
        break;

    case 55:
        if (!_vblank) {
            ba_out(true);
            if (!_40_columns) {
                _main_border = true;
            }

            unsigned x = (_cycle - 16) << 3;
            paint_display_cycle(x);
        }
        break;

    case 56:
        if (!_vblank) {
            if (_40_columns) {
                _main_border = true;
            }

            if (_row_counter == 7) {
                _idle_mode = true;
                _video_counter += 8;
                _row_counter = 0;
            }

            if (!_idle_mode) {
                ++_row_counter;
            }
        }
        break;

    case 57:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter + 1, 0));
        }
        break;

    case 58:
        break;

    case 59:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter + 1, 1));
        }
        break;

    case 60:
        break;

    case 61:
        if (!_vblank) {
            ba_out(!is_mib_visible(_raster_counter + 1, 2));
        }
        break;

    case 62:
        if (!_vblank) {
            if (_raster_counter >= MIB_Y_START) {
                paint_sprites();
            }

            if ((_irq_status & REG_INTERRUPT_MDC) == 0 && _mib_data_collision != 0) {
                _irq_status |= REG_INTERRUPT_MDC;
                if ((_irq_enable & REG_INTERRUPT_MDC) != 0) {
                    irq_out(true);
                }
            }

            if ((_irq_status & REG_INTERRUPT_MMC) == 0 && _mib_mib_collision != 0) {
                _irq_status |= REG_INTERRUPT_MMC;
                if ((_irq_enable & REG_INTERRUPT_MMC) != 0) {
                    irq_out(true);
                }
            }

            if (_raster_counter == _bborder_start) {
                _vertical_border = true;
            } else if (_bl_den && _raster_counter == _uborder_end) {
                _vertical_border = false;
            }

            paint_borders();

            render_line(_raster_counter - VISIBLE_Y_START);

            reset_collision_data();
            reset_collision_mib();
        }

        _cycle = 0;
        return 1;
    }

    ++_cycle;
    return 1;
}

void Mos6569::paint_borders()
{
    if (_main_border) {
        /*
         * Left and right borders.
         */
        paint(0, _lborder_end, _palette[_border_color]);
        paint(_rborder_start, 0, _palette[_border_color]);
    }

    if (_vertical_border) {
        /*
         * Upper and bottom borders.
         */
        if (_raster_counter < _uborder_end || _raster_counter >= _bborder_start) {
            paint(_lborder_end, _rborder_start - _lborder_end, _palette[_border_color]);
        }
    }
}

void Mos6569::paint_display_cycle(unsigned x)
{
    if (_idle_mode) {
        paint(0, 0, {0, 0, 0});
        return;
    }

    if (!_bl_den) {
        /*
         * Display not enabled: Entire screen painted in border colour.
         */
        paint(0, 0, _palette[_border_color]);
        return;
    }

    unsigned dline = _video_counter + _row_counter;
    if (dline < DISPLAY_HEIGHT) {
        /*
         * Display mode.
         */
        if (!_bmm_mode) {
            /*
             * Text mode.
             */
            if (!(_ecm_mode && _mcm_mode)) {
                paint_char_mode(dline, x);

            } else {
                /*
                 * Invalid text mode.
                 */
                paint(0, 0, {0, 0, 0});
            }

        } else {
            /*
             * Bitmap mode.
             */
            if (!_ecm_mode) {
                paint_bitmap_mode(dline, x);

            } else {
                /*
                 * Invalid bitmap mode.
                 */
                paint(0, 0, {0, 0, 0});
            }
        }
    }
}

inline void Mos6569::render_line(unsigned line)
{
    if (_render_line) {
        _render_line(line, _scanline);
    }
}

void Mos6569::irq_out(bool active)
{
    bool irq_pin = (_irq_status & REG_INTERRUPT_IRQ);

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

inline void Mos6569::ba_out(bool active)
{
    if (_ba_out) {
        _ba_out(active);
    }
}

inline addr_t Mos6569::char_base(uint8_t ch) const
{
    return _char_base + (static_cast<addr_t>(ch) << 3);
}

inline addr_t Mos6569::mib_base(unsigned mib) const
{
    addr_t addr = static_cast<addr_t>(_mmap->read(_video_matrix + MIB_POINTER_OFFSET + mib)) << 6;
    return addr;
}

inline unsigned Mos6569::mib_x(unsigned mib) const
{
    return  (_mib_coord_x[mib] + MIB_X_COORD_OFFSET);
}

inline unsigned Mos6569::mib_y(unsigned mib) const
{
    return (_mib_coord_y[mib] + MIB_Y_COORD_OFFSET);
}

inline bool Mos6569::is_mib_visible(unsigned line, uint8_t mib)
{
    return (std::get<0>(mib_visibility_y(line, mib)) != static_cast<unsigned>(-1));
}

std::tuple<unsigned, unsigned, bool> Mos6569::mib_visibility_y(unsigned line, uint8_t mib)
{
    uint8_t sbit = (1 << mib);

    if (_mib_enable & sbit) {
        bool exp_y = (_mib_expand_y & sbit);
        unsigned pos_y = mib_y(mib);
        unsigned max_y = pos_y + (MIB_HEIGHT << (exp_y ? 1 : 0));
        if (line >= pos_y && line < max_y) {
            return {pos_y, max_y, exp_y};
        }
    }

    return {static_cast<unsigned>(-1), {}, {}};
}

Mos6569::Color Mos6569::video_color_code(unsigned x, unsigned y) const
{
    addr_t addr = static_cast<addr_t>(x + y * CHARMODE_COLUMNS);
    return static_cast<Color>(_vcolor->read(addr) & Color::MASK);
}

void Mos6569::paint(unsigned start, unsigned width, const Rgba &color)
{
    if (start < _scanline.size()) {
        if (width == 0 || start + width > _scanline.size()) {
            width = _scanline.size() - start;
        }

        std::fill_n(_scanline.begin() + start, width, color);
    }
}

void Mos6569::paint_byte(unsigned start, uint8_t bitmap, const Rgba4& colors)
{
    if (start < _scanline.size()) {
        const auto &bg_color = colors[0];
        const auto &fg_color = colors[1];
        uint8_t bit = 128;

        for (auto it = _scanline.begin() + start; bit != 0 && it != _scanline.end(); ++it, bit >>= 1) {
            it->set((bitmap & bit) ? fg_color : bg_color);
        }
    }
}

void Mos6569::paint_mcm_byte(unsigned start, uint8_t bitmap, const Rgba4& colors)
{
    if (start + 1 < _scanline.size()) {
        auto it = _scanline.begin() + start;

        for (int shift = 6; shift >= 0; shift -= 2) {
            uint8_t index = (bitmap >> shift) & 3;
            const auto &color = colors[index];
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

void Mos6569::paint_char_mode(unsigned line, unsigned x)
{
    unsigned row   = line >> 3;
    unsigned col   = x >> 3;
    addr_t ch_addr = _video_matrix + row * CHARMODE_COLUMNS + col;
    Color fg_code  = video_color_code(col, row);
    int bg         = 0;
    addr_t ch      = _mmap->read(ch_addr);

    if (_ecm_mode) {
        bg = ch >> 6;
        ch &= 63;
    }

    uint8_t ch_row_data = _mmap->read(char_base(ch) + (line & 7));
    unsigned start = DISPLAY_X_START + (col << 3);

    if (_mcm_mode && fg_code > 7) {
        /*
         * Multi-color mode.
         */
        const Rgba4 colors = {
            _palette[_background_color[0]],
            _palette[_background_color[1]],
            _palette[_background_color[2]],
            _palette[fg_code & 7]
        };

        paint_mcm_byte(start + _scroll_x, ch_row_data, colors);
        update_collision_data_mcm(start + _scroll_x, ch_row_data);

    } else {
        /*
         * Hi-Res or extended-background-color mode.
         */
        Rgba4 colors = {
            _palette[_background_color[bg]],
            _palette[fg_code]
        };

        paint_byte(DISPLAY_X_START + (col << 3) + _scroll_x, ch_row_data, colors);
        update_collision_data(start + _scroll_x, ch_row_data);
    }
}

void Mos6569::paint_bitmap_mode(unsigned line, unsigned x)
{
    unsigned row           = line >> 3;
    unsigned col           = x >> 3;
    addr_t color_code_addr = _video_matrix + row * CHARMODE_COLUMNS + col;
    uint8_t color_code     = _mmap->read(color_code_addr);
    const Rgba &fg_color   = _palette[color_code >> 4];
    const Rgba &bg_color   = _palette[color_code & Color::MASK];

    uint8_t byte = _mmap->read(_bitmap_base + row * DISPLAY_WIDTH + (col << 3) + (line & 7));
    unsigned start = DISPLAY_X_START + (col << 3);

    if (_mcm_mode) {
        /*
         * Multi-color bitmap mode.
         */
        const Rgba4 colors = {
            _palette[_background_color[0]],
            fg_color,
            bg_color,
            _palette[video_color_code(col, row)]
        };

        paint_mcm_byte(start + _scroll_x, byte, colors);
        update_collision_data_mcm(start + _scroll_x, byte);

    } else {
        /*
         * Hi-Res bitmap mode.
         */
        paint_byte(start + _scroll_x, byte, {bg_color, fg_color});
        update_collision_data(start + _scroll_x, byte);
    }
}

void Mos6569::paint_sprite_line(unsigned start, uint64_t bitmap, const Rgba4& colors, bool expand)
{
    paint_byte(start,      (bitmap >> 56),       colors);
    paint_byte(start + 8,  (bitmap >> 48) & 255, colors);
    paint_byte(start + 16, (bitmap >> 40) & 255, colors);

    if (expand) {
        paint_byte(start + 24, (bitmap >> 32) & 255, colors);
        paint_byte(start + 32, (bitmap >> 24) & 255, colors);
        paint_byte(start + 40, (bitmap >> 16) & 255, colors);
    }
}

void Mos6569::paint_sprite_line_mcm(unsigned start, uint64_t bitmap, const Rgba4& colors, bool expand)
{
    paint_mcm_byte(start,      (bitmap >> 56),       colors);
    paint_mcm_byte(start + 8,  (bitmap >> 48) & 255, colors);
    paint_mcm_byte(start + 16, (bitmap >> 40) & 255, colors);

    if (expand) {
        paint_mcm_byte(start + 24, (bitmap >> 32) & 255, colors);
        paint_mcm_byte(start + 32, (bitmap >> 24) & 255, colors);
        paint_mcm_byte(start + 40, (bitmap >> 16) & 255, colors);
    }
}

void Mos6569::paint_sprite(unsigned line, uint8_t mib)
{
    auto [pos_y, max_y, exp_y] = mib_visibility_y(line, mib);
    if (pos_y == static_cast<unsigned>(-1)) {
        /*
         * Sprite not visible or not enabled.
         */
        return;
    }

    uint8_t sbit = (1 << mib);
    bool exp_x = (_mib_expand_x & sbit);
    unsigned pos_x = mib_x(mib);
    addr_t mib_data = mib_base(mib);

    unsigned l = line - pos_y;
    if (exp_y) {
        /*
         * Vertical expansion.
         */
        l >>= 1;
    }

    addr_t addr = mib_data + l * 3;
    uint8_t byte1 = _mmap->read(addr);
    uint8_t byte2 = _mmap->read(addr + 1);
    uint8_t byte3 = _mmap->read(addr + 2);

    const Rgba &fg_color = _palette[_mib_color[mib]];
    const Rgba &bg_color = Rgba::transparent;

    bool data_pri = (_mib_data_priority & sbit);
    bool mcm = (_mib_multicolor_sel & sbit);

    auto [dcollision, bitmap, visible_bitmap] = mib_bitmap(pos_x, byte1, byte2, byte3, exp_x, mcm, data_pri);

    if (mcm) {
        /*
         * Multi-color mode (4 colours).
         */
        const Rgba4 colors = {
            bg_color,
            _palette[_mib_multicolor[0]],
            fg_color,
            _palette[_mib_multicolor[1]]
        };

        paint_sprite_line_mcm(pos_x, visible_bitmap, colors, exp_x);

    } else {
        /*
         * Hi-Res mode (2 colours).
         */
        paint_sprite_line(pos_x, visible_bitmap, {bg_color, fg_color}, exp_x);
    }

    if (!_mib_data_collision && dcollision) {
        _mib_data_collision = sbit;
    }

    auto cbit = update_collision_mib(mib, pos_x, mcm, bitmap);
    if (!_mib_mib_collision && cbit) {
        _mib_mib_collision = (sbit | cbit);
    }
}

void Mos6569::paint_sprites()
{
    /*
     * MIB-MIB Priority: 0 highest (front), 7 lowest (behind).
     */
    for (uint8_t mib = 7; mib != 255; --mib) {
        paint_sprite(_raster_counter, mib);
    }
}

inline void Mos6569::reset_collision_data()
{
    _collision_data.fill(0);
}

inline void Mos6569::reset_collision_mib()
{
    _mib_bitmaps.fill(0);
}

void Mos6569::update_collision_data(unsigned start, uint8_t bitmap)
{
    auto start_byte   = start >> 3;
    uint8_t start_bit = start - (start_byte << 3);

    if (start_bit == 0) {
        _collision_data[start_byte] = bitmap;

    } else {
        uint8_t byte1 = bitmap >> start_bit;
        uint8_t byte2 = bitmap << (8 - start_bit);

        uint8_t mask  = 0xFF >> start_bit;

        uint8_t prev1 = _collision_data[start_byte]     & ~mask;
        uint8_t prev2 = _collision_data[start_byte + 1] &  mask;

        _collision_data[start_byte]     = byte1 | prev1;
        _collision_data[start_byte + 1] = byte2 | prev2;
    }
}

inline void Mos6569::update_collision_data_mcm(unsigned start, uint8_t bitmap)
{
    /*
     * In multicolor mode 00 and 01 codes are considered background colours (not foreground);
     * the following converts 01 to 00 and 10 to 11 so the proper collision mask is generated.
     */
    update_collision_data(start, utils::convert_01_10(bitmap));
}

std::tuple<bool, uint64_t, uint64_t>
Mos6569::mib_bitmap(unsigned start, uint8_t byte1, uint8_t byte2, uint8_t byte3, bool expand, bool mcm, bool data_pri)
{
    auto start_byte = start >> 3;
    auto start_bit  = start - (start_byte << 3);

    uint64_t bitmap{};
    uint64_t mask{};

    if (expand) {
        uint16_t w1, w2, w3;

        if (mcm) {
            w1 = utils::expand_dibits(byte1);
            w2 = utils::expand_dibits(byte2);
            w3 = utils::expand_dibits(byte3);
        } else {
            w1 = utils::expand_bits(byte1);
            w2 = utils::expand_bits(byte2);
            w3 = utils::expand_bits(byte3);
        }

        bitmap = (static_cast<uint64_t>(w1) << 48) |
                 (static_cast<uint64_t>(w2) << 32) |
                 (static_cast<uint64_t>(w3) << 16);

        mask = 0xFFFFFFFFFFFF0000UL;

    } else {
        bitmap = (static_cast<uint64_t>(byte1) << 56) |
                 (static_cast<uint64_t>(byte2) << 48) |
                 (static_cast<uint64_t>(byte3) << 40);

        mask = 0xFFFFFF0000000000UL;
    }

    uint64_t background     = be64toh(*reinterpret_cast<uint64_t*>(&_collision_data[start_byte])) << start_bit;
    uint64_t collision      = mask & background & bitmap;
    uint64_t visible_bitmap = ((collision && data_pri) ? (~collision & bitmap) : bitmap);

    return {collision != 0, bitmap, visible_bitmap};
}

uint8_t Mos6569::update_collision_mib(uint8_t mib, unsigned start, bool mcm, uint64_t bitmap)
{
    if (mcm) {
        /*
         * Multicolor sprites use 00 as transparent color (no content),
         * the following converts 01 and 10 to 11 so the proper collision data mask is generated.
         */
        gsl::span data{reinterpret_cast<uint8_t*>(&bitmap), sizeof(bitmap)};
        utils::convert_01_10_to_11(data);
    }

    _mib_bitmaps[mib] = bitmap;

    /*
     * Detect a MIB-MIB collision only if there is no other MIB-MIB collision.
     */
    if (!_mib_mib_collision) {
        for (uint8_t mib_c = 0; mib_c < _mib_bitmaps.size(); ++mib_c) {
            uint8_t sbit_c = (1 << mib_c);
            if (mib_c != mib && (_mib_enable & sbit_c)) {
                unsigned start_c = mib_x(mib_c);

                if (!((start_c + MIB_MAX_X_SIZE) < start || (start + MIB_MAX_X_SIZE) < start_c)) {

                    auto cbitmap = _mib_bitmaps[mib_c];
                    bitmap = _mib_bitmaps[mib];

                    int shift = static_cast<int>(start) - static_cast<int>(start_c);
                    if (shift < 0) {
                        bitmap <<= -shift;
                    } else {
                        cbitmap <<= shift;
                    }

                    if (cbitmap & bitmap) {
                        /*
                         * MIB-MIB collision detected.
                         */
                        return sbit_c;
                    }
                }
            }
        }
    }

    /*
     * No MIB-MIB collisions.
     */
    return 0;
}

}
