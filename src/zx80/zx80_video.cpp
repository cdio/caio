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
    : Clockable{},
      Name{TYPE, label},
      _palette{builtin_palette},
      _scanline(WIDTH)
{
}

void ZX80Video::palette(const std::string& fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }

    std::fill(_scanline.begin(), _scanline.end(), _palette[WHITE]);
}

inline void ZX80Video::render_line(unsigned line)
{
    if (line >= VISIBLE_Y_START && line < VISIBLE_Y_END && _renderline_cb) {
        _renderline_cb(line - VISIBLE_Y_START, _scanline);
    }
}

void ZX80Video::paint(unsigned start, unsigned width, const Rgba& color)
{
    if (start < _scanline.size()) {
        if (width == 0 || start + width > _scanline.size()) {
            width = _scanline.size() - start;
        }

        std::fill_n(_scanline.begin() + start, width, color);
    }
}

void ZX80Video::paint_byte(unsigned start, uint8_t bitmap, const Rgba2& colors)
{
    if (start < _scanline.size()) {
        const auto& bg_color = colors[WHITE];
        const auto& fg_color = colors[BLACK];
        uint8_t bit = 128;

        for (auto it = _scanline.begin() + start; bit != 0 && it != _scanline.end(); ++it, bit >>= 1) {
            it->set((bitmap & bit) ? fg_color : bg_color);
        }
    }
}

void ZX80Video::video_data(uint8_t vdata, bool invert)
{
    _vdata = vdata;
    _invert = invert;
//    log.debug("new data: %02x, invert: %d\n", _vdata, _invert);
}

void ZX80Video::reset()
{
    _rasterline = 0;
//    _counter = 0;
    _x_coord = 0;
//    _addr = 0;
    paint(0, _scanline.size(), _palette[WHITE]);
}

#if 0

void ZX80Video::hsync()
{
//XXX    log.error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>H-SYNC raster=%d, visible=%d <<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n", _rasterline, _rasterline - VISIBLE_Y_START);
    render_line(_rasterline);

    _x_coord = 0;
    ++_rasterline;
    _counter = (_counter + 1) % 8; //XXX
    _stop = false;

    if (_rasterline == FRAME_HEIGHT)
;//        reset();
}

void ZX80Video::vsync()
{
    log.error(">>>>>>>>>>>>>>>>>>>>>>>>>>>>>>V-SYNC<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<<\n");
    hsync();
    _rasterline = 0;
    _counter = 0;
}
#endif

#if 0
void ZX80Video::addr_hi(uint8_t hi)
{
    _addr = (static_cast<addr_t>(hi) << 8) | (_addr & 0x00FF);
}

void ZX80Video::addr_lo(uint8_t lo)
{
    _addr = (_addr & 0xFE00) | (static_cast<addr_t>(lo & CH_LO_MASK) << 3);
    //XXX _addr = (_addr & 0xFF00) | (static_cast<addr_t>(lo & CH_LO_MASK) << 3);
    _invert = lo & CH_INVERT_BIT;
}

void ZX80Video::stopline()
{
    _stop = true;
    hsync();
}

void ZX80Video::endline()
{
    const auto& color = (_invert ? _palette[BLACK] : _palette[WHITE]);
    paint(_x_coord, 0, color);
    hsync();
}
#endif

size_t ZX80Video::tick(const Clock& clk)
{
    //FIXME:    horizontal and vertical sync must be handled externally by zx80-aspace

#if 0
    if (_sync) {
        if (_rasterline >= VISIBLE_Y_START && _rasterline < VISIBLE_Y_END) {
            paint(_x_coord, 0, _palette[WHITE]);
        }

        size_t cycles = (PAINT_BYTE_CYCLES >> 2) * (_scanline.size() - _x_coord); // + hblank cycles  FIXME

        render_line(_rasterline);

        _x_coord = 0;

        ++_rasterline;

        _sync = false; //pull-up counter? to use as vsync?
        return cycles;
    }

    if (_x_coord < _scanline.size()) {
        const auto& colors = (_invert ? Rgba2{_palette[BLACK], _palette[WHITE]} :
                                        Rgba2{_palette[WHITE], _palette[BLACK]});

        paint_byte(_x_coord, _vdata, colors);

        ++_x_coord;

        if (_x_coord == _scanline.size()) {
            render_line(_rasterline);
        }
    }

    return PAINT_BYTE_CYCLES;
#else
    if (_rasterline < VISIBLE_Y_START || _rasterline >= VISIBLE_Y_END) {
        _rasterline = (_rasterline + 1) % FRAME_HEIGHT;
        return 20;
    }

    const auto& colors = (_invert ? Rgba2{_palette[BLACK], _palette[WHITE]} :
                                    Rgba2{_palette[WHITE], _palette[BLACK]});

    paint_byte(_x_coord, _vdata, colors);

    ++_x_coord;

    if (_x_coord == _scanline.size()) {
        render_line(_rasterline);
        _x_coord = 0;
        ++_rasterline;
        paint(0, 0, _palette[WHITE]);
        return 4;// + PAINT_BYTE_CYCLES; //4  hsync time
    }

    return PAINT_BYTE_CYCLES;
#endif
}


#if 0
        the idea:
    
            * this class implements a video pal controller
            * depending on the cycle, the tick method paints 8 pixels for each tick call,
              the x coordinate in incremented but never reset until a sync signal is received.
            * The sync signal is external and it *should* last for a specific (pal defined) time
                    

            
#endif
}
}
}
