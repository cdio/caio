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
#include "zx80_video.hpp"

#include <gsl/assert>

#include "clock.hpp"
#include "logger.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

RgbaTable ZX80Video::builtin_palette{
    0x202020FF,
    0xCFCFCFFF
};

ZX80Video::ZX80Video(const sptr_t<Clock>& clk, const std::string& label)
    : Name{TYPE, label},
      _clk{clk},
      _palette{builtin_palette},
      _scanline(WIDTH, _palette[WHITE])
{
    using namespace gsl;
    Expects(clk);
}

void ZX80Video::palette(const std::string& fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }

    std::fill(_scanline.begin(), _scanline.end(), _palette[WHITE]);
}

void ZX80Video::palette(const RgbaTable& plt)
{
    _palette = plt;
}

void ZX80Video::render_line(const renderer_t& rl)
{
    _renderline_cb = rl;
}

inline void ZX80Video::render_line()
{
    auto line = _line - _lineoff - SCANLINE_VISIBLE_START;
    if (line >= 0 && line < VISIBLE_HEIGHT && _renderline_cb) {
        _renderline_cb(line, _scanline);
    }
}

void ZX80Video::clear_screen(const cls_t& cls)
{
    _cls_cb = cls;
}

inline void ZX80Video::clear_screen()
{
    if (_cls_cb) {
        _cls_cb(_palette[BLACK]);
    }
}

void ZX80Video::paint_byte(unsigned start, uint8_t bitmap)
{
    if (start < _scanline.size()) {
        const auto& fg_color = _palette[BLACK];
        const auto& bg_color = _palette[WHITE];
        uint8_t bit = 128;
        for (auto it = _scanline.begin() + start; bit != 0 && it != _scanline.end(); ++it, bit >>= 1) {
            it->set((bitmap & bit) ? fg_color : bg_color);
        }
    }
}

void ZX80Video::bitmap(uint8_t vdata)
{
    paint_byte(_column, vdata);
    _column += 8;
}

void ZX80Video::hsync()
{
    render_line();
    std::fill(_scanline.begin(), _scanline.end(), _palette[WHITE]);
    _column = LBORDER_START;
    ++_line;
}

void ZX80Video::vsync(bool on)
{
    if (on) {
        /*
         * Simulate the out-of-sync signal.
         */
        ++_vsync_count;
        if (_vsync_count > 9) {
            clear_screen();
            _lineoff = 10;
        } else {
            _lineoff = 0;
        }
    } else {
        /*
         * VSYNC terminated.
         */
        _vsync_count = 0;
        _line = 0;
        _column = LBORDER_START;
    }
}

}
}
}
