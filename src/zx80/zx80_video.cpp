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

#include "logger.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

RgbaTable ZX80Video::builtin_palette{
    0x202020FF,
    0xA0A0A0FF
};

ZX80Video::ZX80Video(const std::string& label)
    : Name{TYPE, label},
      _palette{builtin_palette},
      _scanline(WIDTH, _palette[WHITE])
{
}

void ZX80Video::palette(const std::string& fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }

    std::fill(_scanline.begin(), _scanline.end(), _palette[WHITE]);
}

inline void ZX80Video::render_line()
{
    if (_line >= VISIBLE_Y_START && _line < VISIBLE_Y_END && _renderline_cb) {
        _renderline_cb(_line - VISIBLE_Y_START, _scanline);
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
    if (_line < FRAME_HEIGHT) {
        ++_line;
    }
}

void ZX80Video::vsync()
{
    hsync();
    _line = 0;
    _column = LBORDER_START;
}

}
}
}
