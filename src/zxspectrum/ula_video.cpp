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
#include "ula_video.hpp"

#include "logger.hpp"
#include "signal.hpp"
#include "types.hpp"
#include "utils.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

RgbaTable ULAVideo::builtin_palette{
    0x101010FF,
    0x0100CEFF,
    0xCF0100FF,
    0xCF01CEFF,
    0x00CF15FF,
    0x01CFCFFF,
    0xCFCF15FF,
    0xCFCFCFFF,
    0x101010FF,
    0x0200FDFF,
    0xFF0201FF,
    0xFF02FDFF,
    0x00FF1CFF,
    0x02FFFFFF,
    0xFFFF1DFF,
    0xFFFFFFFF
};

ULAVideo::ULAVideo(const sptr_t<Z80>& cpu, const sptr_t<RAM>& ram, const std::string& label)
    : Clockable{},
      Name{TYPE, label},
      _cpu{cpu},
      _ram{ram},
      _palette{builtin_palette},
      _scanline(WIDTH)
{
    CAIO_ASSERT(_cpu && _ram && _ram->size() > VRAM_MIN_SIZE);
}

ULAVideo::~ULAVideo()
{
}

void ULAVideo::palette(const std::string& fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }
}

void ULAVideo::palette(const RgbaTable& plt)
{
    _palette = plt;
}

void ULAVideo::render_line(const renderer_t& rl)
{
    _renderline_cb = rl;
}

void ULAVideo::border_colour(uint8_t code)
{
    _border_colour = to_rgba(static_cast<Colour>(code & COLOUR_MASK));
}

size_t ULAVideo::tick(const Clock& clk)
{
    if (_line == IRQ_SCANLINE) {
        if (!_intreq && _cycle == IRQ_CYCLE_START) {
            _intreq = true;
            _cpu->int_pin(true);
        } else if (_intreq && _cycle >= IRQ_CYCLE_END) {
            _intreq = false;
            _cpu->int_pin(false);
        }
    }

    paint_display();

    ++_cycle;
    if (_cycle == SCANLINE_CYCLES) {
        /*
         * HSync.
         */
        render_line();
        _cycle = 0;

        ++_line;
        if (_line == SCANLINES) {
            /*
             * VSync.
             */
            _line = 0;
        }
    }

    _flash_counter = (_flash_counter + 1) % COLOUR_FLASH_TICKS;
    _flash_swap ^= (_flash_counter == 0);

    return ULA_TICK_CYCLES;
}

inline void ULAVideo::render_line()
{
    ssize_t line = _line - SCANLINE_VISIBLE_START;
    if (line >= 0 && line < VISIBLE_HEIGHT && _renderline_cb) {
        _renderline_cb(line, _scanline);
    }

    std::fill(_scanline.begin(), _scanline.end(), _border_colour);
}

inline const Rgba& ULAVideo::to_rgba(Colour code) const
{
    return _palette[static_cast<size_t>(code) % _palette.size()];
}

void ULAVideo::paint_byte(unsigned start, uint8_t bitmap, const Rgba& fg, const Rgba& bg)
{
    if (start < _scanline.size()) {
        uint8_t bit = 128;
        for (auto it = _scanline.begin() + start; bit != 0 && it != _scanline.end(); ++it, bit >>= 1) {
            *it = ((bitmap & bit) ? fg : bg);
        }
    }
}

void ULAVideo::paint_display()
{
    ssize_t col = _cycle - DISPLAY_CYCLE_START;
    ssize_t line = _line - DISPLAY_SCANLINE_START;

    if (col >= 0 && col < DISPLAY_COLUMNS && line >= 0 && line < DISPLAY_HEIGHT) {
        ssize_t row = line >> 3;

        addr_t bitmap_addr = DISPLAY_BASE_ADDR |
            ((line & (D7 | D6)) << 5) |
            ((line & 7) << 8) |
            ((line & (D3 | D4 | D5)) << 2) |
            col;

        uint8_t bitmap = _ram->read(bitmap_addr);

        addr_t colour_addr = COLOUR_ATTR_BASE_ADDR + row * DISPLAY_COLUMNS + col;
        uint8_t cattr = _ram->read(colour_addr);

        bool flash = (cattr & D7);
        uint8_t bright = (cattr & D6) >> 3;
        Colour fgcode = static_cast<Colour>((cattr & 0x07) | bright);
        Colour bgcode = static_cast<Colour>(((cattr >> 3) & 0x07) | bright);

        if (flash && _flash_swap) {
            std::swap(fgcode, bgcode);
        }

        const auto& fg = to_rgba(fgcode);
        const auto& bg = to_rgba(bgcode);

        paint_byte(LBORDER_WIDTH + (col << 3), bitmap, fg, bg);
    }
}

}
}
}
