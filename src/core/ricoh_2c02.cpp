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
#include "ricoh_2c02.hpp"

#include "logger.hpp"

namespace caio {
namespace ricoh {
namespace rp2c02 {

/*
 * https://www.nesdev.org/wiki/PPU_palettes#2C02
 */
RgbaTable RP2C02::builtin_palette{
    0x626262FF, 0x012090FF, 0x240BA0FF, 0x470090FF, 0x600062FF, 0x6A0024FF, 0x601100FF, 0x472700FF,
    0x243C00FF, 0x014A00FF, 0x004F00FF, 0x004724FF, 0x003662FF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xABABABFF, 0x1F56E1FF, 0x4D39FFFF, 0x7E23EFFF, 0xA31BB7FF, 0xB42264FF, 0xAC370EFF, 0x8C5500FF,
    0x5E7200FF, 0x2D8800FF, 0x079000FF, 0x008947FF, 0x00739DFF, 0x000000FF, 0x000000FF, 0x000000FF,
    0xFFFFFFFF, 0x67ACFFFF, 0x958DFFFF, 0xC875FFFF, 0xF26AFFFF, 0xFF6FC5FF, 0xFF836AFF, 0xE6A01FFF,
    0xB8BF00FF, 0x85D801FF, 0x5BE335FF, 0x45DE88FF, 0x49CAE3FF, 0x4E4E4EFF, 0x000000FF, 0x000000FF,
    0xFFFFFFFF, 0xBFE0FFFF, 0xD1D3FFFF, 0xE6C9FFFF, 0xF7C3FFFF, 0xFFC4EEFF, 0xFFCBC9FF, 0xF7D7A9FF,
    0xE6E397FF, 0xD1EE97FF, 0xBFF3A9FF, 0xB5F2C9FF, 0xB5EBEEFF, 0xB8B8B8FF, 0x000000FF, 0x000000FF
};

RP2C02::RP2C02(std::string_view label, const sptr_t<ASpace>& mmap, bool ntsc)
    : Device{TYPE, label},
      Clockable{},
      _mmap{mmap},
      _scanline(WIDTH),
      _palette{builtin_palette},
      _visible_y_start{VISIBLE_Y_START + ntsc * 8},
      _visible_y_end{VISIBLE_Y_END - ntsc * 8}
{
}

RP2C02::~RP2C02()
{
}

void RP2C02::render_line(const RendererCb& rl)
{
    _render_line = rl;
}

void RP2C02::palette(const fs::Path& fname)
{
    if (!fname.empty()) {
        _palette.load(fname);
    }
}

void RP2C02::palette(const RgbaTable& plt)
{
    _palette = plt;
}

void RP2C02::irq(const OutputPinCb& irq_out)
{
    _irq_out = irq_out;
}

bool RP2C02::sync_pin(bool active)
{
    _sync_pin = active;
    return _sync_pin;
}

void RP2C02::reset()
{
}

size_t RP2C02::size() const
{
    return MMIO_Registers::REGMAX;
}

uint8_t RP2C02::dev_read(size_t addr, Device::ReadMode mode)
{
    uint8_t data{};

    addr &= REGMASK;

    switch (addr) {
    case PPUSTATUS:
        /*
         * D7 D6 D5 D4 D3 D2 D1 D0
         *  |  |  |  |  |  |  |  |
         *  |  |  |  +--+--+--+--+-> Open bus
         *  |  |  +----------------> Sprite overflow
         *  |  +-------------------> Sprite 0 hit
         *  +----------------------> VBlank period started (cleared after read)
         *
         * D7 set at cycle 1 of scanline 241 (POST_RENDER_LINE + 1).
         * D7 cleared at cycle 1 of PRE_RENDER_LINE (see tick() method).
         */
        data = (_last_mmio_write & 0b0001'1111) |
               (_sp_overflow << 5) |
               (_sp_0_hit << 6) |
               (_vblank_flag << 7);
        _vblank_flag = 0;
        _regs.w = 0;
        irq_out(false);
        return data;

    case OAMADDR:
        return _oam_addr;

    case OAMDATA:
        /*
         * Cycles 1-64 used to clear secondary oam buffer,
         * reads during this period return 255.
         * See https://www.nesdev.org/wiki/PPU_sprite_evaluation
         */
        return ((_cycle > 0 && _cycle <= 64) ? 0xFF : _oam[_oam_addr]);

    case PPUDATA:
        if (is_palette_address(_regs.v)) {
            /*
             * "The referenced 6-bit palette data is returned immediately instead
             * of going to the internal read buffer, and hence no priming read is required.
             * Simultaneously, the PPU also performs a normal read from PPU memory at the
             * specified address, "underneath" the palette data, and the result of this read
             * goes into the read buffer as normal. The old contents of the read buffer are
             * discarded when reading palettes..."
             *
             * "This feature is supported by the 2C02G, 2C02H, and PAL PPUs. The byte returned
             * when reading palettes contains PPU open bus in the top 2 bits, and the value
             * is returned after it is modified by greyscale mode, which clears the bottom
             * 4 bits if enabled."
             *
             * See https://www.nesdev.org/wiki/PPU_registers#PPUDATA
             */
            const auto pos = (_regs.v - PALETTE_ADDR) % 32;
            data = (pos < 16 ? _bg_palette[pos] : _sp_palette[pos - 16]) & _rindex_mask;
            data |= _last_mmio_write & ~PALETTE_COLOR_MASK;
        } else {
            /*
             * Normal VRAM read.
             * Read is delayed through an internal read buffer.
             */
            data = _delayed_data;
        }
        if (mode != ReadMode::Peek) {
            _delayed_data = _mmap->read(_regs.v);
            _regs.v = (_regs.v + _vram_inc) & (A15 - 1);
        }
        return data;

    case PPUCTRL:
    case PPUMASK:
    case PPUSCROLL:
    case PPUADDR:
    default:;
    }

    return _last_mmio_write;
}

void RP2C02::dev_write(size_t addr, uint8_t value)
{
    addr &= REGMASK;

    switch (addr) {
    case PPUCTRL:
        /*
         * D7 D6 D5 D4 D3 D2 D1 D0
         *  |  |  |  |  |  |  |  |
         *  |  |  |  |  |  |  +--+-> Base nametable address (0: 2000, 1: 2400, 2: 2800, 3: 2C00)
         *  |  |  |  |  |  |         (D0: 9th bit of X scroll, D1: 9th bit of Y scroll)
         *  |  |  |  |  |  +-------> VRAM increment (0: 1, 1: 32)
         *  |  |  |  |  +----------> Sprite pattern table address (0: 0000, 1: 1000). Only for 8x8 sprites
         *  |  |  |  +-------------> Background pattern table address (0: 0000, 1: 1000)
         *  |  |  +----------------> Sprite size (0: 8x8 px, 1: 8x16 px)
         *  |  +-------------------> EXT port configuration (0: Input, 1: Output)
         *  +----------------------> Generate IRQ on VBlank (0: Off, 1: On)
         */
        _regs.t      = (_regs.t & ~(A11 | A10)) | ((value & (D1 | D0)) << 10);
        _vram_inc    = ((value & D2) ? 32 : 1);
        _sp_base     = ((value & D3) ? PATTERN_TABLE_1_ADDR : PATTERN_TABLE_0_ADDR);
        _bg_base     = ((value & D4) ? PATTERN_TABLE_1_ADDR : PATTERN_TABLE_0_ADDR);
        _sp_8x16     = value & D5;
        _ext_in      = !(value & D6);
        _irq_enabled = value & D7;
        if (_vblank_flag) {
            /*
             * "The PPU pulls /NMI low if and only if both vblank_flag and NMI_output are true.
             * By toggling NMI_output (PPUCTRL.7) during vertical blank without reading PPUSTATUS,
             * a program can cause /NMI to be pulled low multiple times, causing multiple NMIs to be generated."
             *
             * https://www.nesdev.org/wiki/NMI
             */
            irq_out(_irq_enabled);
        }
        break;

    case PPUMASK:
        /*
         * D7 D6 D5 D4 D3 D2 D1 D0
         *  |  |  |  |  |  |  |  |
         *  |  |  |  |  |  |  |  +-> Greyscale (0: Normal color, 1: Greyscale)
         *  |  |  |  |  |  |  +----> Background on leftmost 8px (0: Hide, 1: Show)
         *  |  |  |  |  |  +-------> Sprites on leftmost 8px (0: Hide, 1: Show)
         *  |  |  |  |  +----------> Background (0: Hide, 1: Show)
         *  |  |  |  +-------------> Sprites (0: Hide, 1: Show)
         *  |  |  +----------------> Emphasize red
         *  |  +-------------------> Emphasize green
         *  +----------------------> Emphasize blue
         *
         * "Bit 0 controls a greyscale mode, which causes the palette to use only
         * the colors from the grey column: $00, $10, $20, $30.
         * This is implemented as a bitwise AND with $30 on any value read from PPU $3F00-$3FFF,
         * both on the display and through PPUDATA. Writes to the palette through PPUDATA
         * are not affected.
         * Also note that black colours like $0F will be replaced by a non-black grey $00."
         *
         * - Emphasis does not affect the black colors in columns $E or $F (black)
         * - In average, emphasized colors are 0.816328 times their absolute voltage
         *
         * See https://www.nesdev.org/wiki/PPU_registers#Color_Control
         * See https://www.nesdev.org/wiki/NTSC_video
         */
        _rindex_mask = ((value & D0) ? PALETTE_GREYSCALE_MASK : PALETTE_COLOR_MASK);
        _bg_lborder  = !(value & D1);
        _sp_lborder  = !(value & D2);
        _bg_enabled  =  (value & D3);
        _sp_enabled  =  (value & D4);
        _tint        = ((value & D5) || (value & D6) || (value & D7));
        _red_tint    = ((value & D5) ? 1.82f : 1.0f);
        _green_tint  = ((value & D6) ? 1.82f : 1.0f);
        _blue_tint   = ((value & D7) ? 1.82f : 1.0f);
        break;

    case OAMADDR:
        _oam_addr = value;
        break;

    case OAMDATA:
        _oam[_oam_addr++] = value;
        break;

    case PPUSCROLL:
        /*
         * Write twice:
         *  1st write (w = 0):
         *      D7 D6 D5 D4 D3 D2 D1 D0
         *       |  |  |  |  |  |  |  |
         *       |  |  |  |  |  +--+--+-> Fine X scroll
         *       +--+--+--+--+----------> Coarse X scroll
         *
         *      A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *       |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *   t:  x   x   x   x   x   x   x   x   x   x  D7  D6  D5  D4  D3
         *   x:                                                 D2  D1  D0
         *   w: 1
         *
         *  2nd write (w = 1):
         *      D7 D6 D5 D4 D3 D2 D1 D0
         *       |  |  |  |  |  |  |  |
         *       |  |  |  |  |  +--+--+-> Fine Y scroll
         *       +--+--+--+--+----------> Coarse Y scroll
         *
         *      A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *       |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *   t:  D2  D1  D0  x   x  D7  D6  D5  D4  D3   x   x   x   x   x
         *   w: 0
         *
         *  After both writes register t contains:
         *      A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *       |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *       |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
         *       |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
         *       |   |   |   +---+-----------------------------------------> Nametable select
         *       +---+---+-------------------------------------------------> Fine Y scroll
         */
        if (_regs.w == 0) {
            _regs.t = (_regs.t & 0b0111'1111'1110'0000) | (value >> 3); /* Coarsee X */
            _regs.x = (value & 0b0000'0111);                            /* Fine X    */
            _regs.w = 1;
        } else {
            _regs.t = (_regs.t & 0b0000'1100'0001'1111) |
                      ((value & 0b1111'1000) << 2) |        /* Coarse Y */
                      ((value & 0b0000'0111) << 12);        /* Fine Y   */
            _regs.w = 0;
        }
        break;

    case PPUADDR:
        /*
         * Write twice:
         *  1st write (w = 0):
         *      D7 D6 D5 D4 D3 D2 D1 D0
         *       |  |  |  |  |  |  |  |
         *       x  x  +--+--+--+--+--+-> Address MSB
         *
         *      A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *       |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *   t:  0  D5  D4  D3  D2  D1  D0   x   x   x   x   x   x   x   x
         *   w: 1
         *
         *  2nd write (w = 1):
         *      D7 D6 D5 D4 D3 D2 D1 D0
         *       |  |  |  |  |  |  |  |
         *       +--+--+--+--+--+--+--+-> Address LSB
         *
         *      A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *       |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *   t:  x   x   x   x   x   x   x  D7  D6  D5  D4  D3  D2  D1  D0
         *   w: 0
         *   v: t
         */
        if (_regs.w == 0) {
            _regs.t = (_regs.t & 0x00FF) | ((value & (0b0011'1111)) << 8);
            _regs.w = 1;
        } else {
            _regs.t = (_regs.t & 0xFF00) | value;
            _regs.v = _regs.t;
            _regs.w = 0;
        }
        break;

    case PPUDATA:
        if (is_palette_address(_regs.v)) {
            /*
             * Palette memory internal to PPU.
             *
             * "... entry 0 of each palette is also unique in that its color
             * value is shared between the background and sprite palettes,
             * so writing to either one updates the same internal storage.
             * This means that the backdrop color can be written through both
             * $3F00 and $3F10. Palette RAM as a whole is also mirrored through
             * the entire $3F00-$3FFF region."
             *
             * https://www.nesdev.org/wiki/PPU_palettes
             */
            value &= PALETTE_COLOR_MASK;
            const auto pos = (_regs.v - PALETTE_ADDR) % 32;
            if (pos == 0 || pos == 16) {
                _bg_palette[0] = _sp_palette[0] = value;
            } else if (pos < 16) {
                _bg_palette[pos] = value;
            } else {
                _sp_palette[pos - 16] = value;
            }
        } else {
            _mmap->write(_regs.v, value);
        }
        _regs.v = (_regs.v + _vram_inc) & (A15 - 1);
        break;

    default:;
    }

    _last_mmio_write = value;
}

void RP2C02::irq_out(bool active)
{
    if (_irq_status != active) {
        _irq_status = active;
        if (_irq_out) {
            _irq_out(active);
        }
    }
}

size_t RP2C02::tick(const Clock& clk)
{
    if (_sync_pin) {
        /*
         * Video output disabled.
         */
        return 1;
    }

    if (_rasterline == POST_RENDER_LINE) {
        /*
         * VBlank period starts at this line.
         */
        _vblank = true;
        _cycle = 0;
        _rasterline = VBLANK_START;
        return CYCLES;
    }

    if (_rasterline == VBLANK_START) {
        if (_cycle == 0) {
            /*
             * Idle cycle.
             */
            _cycle = 1;
            return 1;
        }

        /*
         * Set the VBlank flag.
         */
        _vblank_flag = true;
        if (_irq_enabled) {
            irq_out(true);
        }

        /*
         * Do nothing until the VBlank period is ended.
         */
        _cycle = 0;
        _rasterline = PRE_RENDER_LINE;
        return (VBLANK_END - VBLANK_START) * CYCLES - 1;
    }

    /*
     * From this point all scanlines are active lines (visible or pre-render).
     */
    if (_cycle == 0) {
        /*
         * Idle cycle.
         */
        _cycle = 1;
        return 1;
    }

    if (_cycle == 1 && _rasterline == PRE_RENDER_LINE) {
        /*
         * End the VBlank period.
         */
        _vblank = false;
        if (_vblank_flag) {
            _vblank_flag = false;
            irq_out(false);
        }

        /*
         * Sprite 0 hit and overflow flags cleared.
         */
        _sp_0_hit = false;
        _sp_0_hit_cycle = INVALID_X_COORDINATE;
        _sp_overflow = false;
    }

    if (_bg_enabled) {
        const auto subcycle = _cycle % 8;
        auto& tile = _tiles[_fetch_tile];

        switch (_cycle) {
        case 1 ... 256:
            switch (subcycle) {
            case 2:
                _sp_0_hit_cycle = paint_tile(_cycle - 2, _tiles[_paint_tile], _tiles[(_paint_tile + 1) % TILES]);
                _paint_tile = (_paint_tile + 1) % std::size(_tiles);
                if (_sp_0_hit_cycle != INVALID_X_COORDINATE) {
                    /*
                     * The earliest sprite 0 hit occurs at third cycle.
                     */
                    _sp_0_hit_cycle += 1;
                }
                fetch_tilech(tile);
                break;

            case 4:
                fetch_palette(tile);
                break;

            case 6:
                fetch_bg_pattern(tile, 0);
                break;

            case 0:
                fetch_bg_pattern(tile, 1);
                _fetch_tile = (_fetch_tile + 1) % std::size(_tiles);
                scroll_x_coarse_inc();
                if (_cycle == 256) {
                    scroll_y_inc();
                }
                break;

            default:;
            }
            break;

        case 257:
            /*
             * Horizontal bits copied from t to v:
             *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
             *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
             *    |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
             *    |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
             *    |   |   |   +---+-----------------------------------------> Nametable select
             *    +---+---+-------------------------------------------------> Fine Y scroll
             *    Y   Y   Y   Y   X   Y   Y   Y   Y   Y   X   X   X   X   X
             */
            constexpr static const addr_t MASK = 0b0111'1011'1110'0000;
            _regs.v = (_regs.v & MASK) | (_regs.t & ~MASK);
            break;

        case 280 ... 304:
            if (_rasterline == PRE_RENDER_LINE) {
                /*
                 * Vertical bits copied from t to v:
                 *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
                 *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
                 *    |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
                 *    |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
                 *    |   |   |   +---+-----------------------------------------> Nametable select
                 *    +---+---+-------------------------------------------------> Fine Y scroll
                 *    Y   Y   Y   Y   X   Y   Y   Y   Y   Y   X   X   X   X   X
                 */
                constexpr static const addr_t MASK = 0b0000'0100'0001'1111;
                _regs.v = (_regs.v & MASK) | (_regs.t & ~MASK);
            }
            break;

        case 320:
            _fetch_tile = 0;
            _paint_tile = 0;
            break;

        case 321 ... 340:
            /*
             * First 2 tiles on next scanline.
             */
            tile = _tiles[_fetch_tile];
            switch (subcycle) {
            case 2:
                fetch_tilech(tile);
                break;
            case 4:
                fetch_palette(tile);
                break;
            case 6:
                fetch_bg_pattern(tile, 0);
                break;
            case 0:
                fetch_bg_pattern(tile, 1);
                ++_fetch_tile;
                scroll_x_coarse_inc();
                break;
            default:;
            }
            break;

        default:;
        }
    }

    if (!_sp_0_hit && _cycle == _sp_0_hit_cycle) {
        _sp_0_hit = true;
        _sp_0_hit_cycle = INVALID_X_COORDINATE;
    }

    if (_sp_enabled) {
        /*
         * The OAM secondary was filled
         * during the previous scanline.
         */
        switch (_cycle) {
        case 64:
            /*
             * Cycles 1-64: OAM secondary buffer cleared.
             */
            _oam_sec_count = 0;
            break;

        case 65:
            /*
             * If the sprite address (OAMADDR, $2003) is not zero,
             * the process of starting sprite evaluation triggers
             * an OAM hardware refresh bug that causes the 8 bytes
             * beginning at OAMADDR & $F8 to be copied and replace
             * the first 8 bytes of OAM.
             *
             * See https://www.nesdev.org/wiki/PPU_sprite_evaluation
             */
            if (_oam_addr != 0x00) {
                const uint8_t addr = _oam_addr & 0xF8;
                std::copy_n(&_oam[addr], 8, &_oam[0]);
            }
            break;

        case 256:
            /*
             * Cyles 65-256: Sprite evaluation.
             */
            if (_rasterline < VISIBLE_Y_END - 1) {
                const unsigned nextline = _rasterline + 1;
                for (size_t spindex = 0; spindex < SPRITES; ++spindex) {
                    _sp_overflow = sprite_evaluation(spindex, nextline);
                    if (_sp_overflow) {
                        break;
                    }
                }
            }
            break;

        case 257 ... 320:
            /*
             * Sprite fetches (8 sprites total, 8 cycles per sprite).
             */
            _oam_addr = 0;
            break;

        default:;
        }
    }

    _cycle = (_cycle + 1) % CYCLES;
    if (_cycle == 0) {
        paint_scanline();
        render_line();
        paint_sprites();
        _rasterline = (_rasterline + 1) % SCANLINES;
    }

    return 1;
}

inline void RP2C02::render_line()
{
    if (!_vblank && _rasterline >= _visible_y_start && _rasterline < _visible_y_end && _render_line) {
        _render_line(_rasterline - _visible_y_start, _scanline);
    }

    std::fill(_scanline.begin(), _scanline.end(), backdrop_color());
}

inline void RP2C02::fetch_tilech(TileData& tile)
{
    /*
     * It is expected the v register to contain:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    x   x   x   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
     *                |   |   +---+---+---+---+---------------------> Coarse Y scroll
     *                +---+-----------------------------------------> Nametable select
     */
    const addr_t addr = NAME_TABLE_ADDR | (_regs.v & 0b0000'1111'1111'1111);
    tile.tilech = _mmap->read(addr);
}

inline void RP2C02::fetch_palette(TileData& tile)
{
    /*
     * It is expected the v register to contain:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
     *    |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
     *    |   |   |   +---+-----------------------------------------> Nametable select
     *    +---+---+-------------------------------------------------> Fine Y scroll
     *
     * Attribute address:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    0   |   0   |   |   |   |   |   |   |   |   |   +---+---+-> High 3 bits of coarse X scroll (x / 4)
     *        |       |   |   |   |   |   |   +---+---+-------------> High 3 bits of coarse Y scroll (y / 4)
     *        |       |   |   +---+---+---+-------------------------> 1111: Attribute table offset (960)
     *        |       +---+-----------------------------------------> Nametable select
     *        +-----------------------------------------------------> Nametable base address
     *
     * - The screen of 32x30 tiles is embedded into a 8x8 matrix of bytes:
     *   + Each element of this 8x8 matrix contains a single byte called Attribute.
     *   + Each attribute defines another 4x4 matrix (4x4 tiles)
     *   + The attributes of the last row of the 8x8 matrix defines a matrix of 2x4 tiles instead of 4x4 (30 rows).
     *
     *     D7 D6 D5 D4 D3 D2 D1 D0
     *      |  |  |  |  |  |  |  |
     *      |  |  |  |  |  |  +--+-> Palette index for Top Left
     *      |  |  |  |  +--+-------> Palette index for Top Right
     *      |  |  +--+-------------> Palette index for Bottom Left
     *      +--+-------------------> Palette index for Bottom Right
     *
     *     As a 2x2 matrix:
     *                    Tile Column
     *        +--------+--------+--------+
     *        |        |  0  1  |  2  3  |
     *     T  +--------+--------+--------+
     *     i  |   0    |        |        |
     *     l  |        | D1  D0 | D3  D2 |
     *     e  |   1    |        |        |
     *        +--------+--------+--------+
     *     R  |   2    |        |        |
     *     o  |        | D5  D4 | D7  D6 |
     *     w  |   3    |        |        |
     *        +--------+--------+--------+
     *
     *     Tile Row and Tile Column specify the lowest 2 bits of the actual absolute column and row.
     *
     * See https://www.nesdev.org/wiki/PPU_scrolling#Tile_and_attribute_fetching
     */
    const uint8_t col   = _regs.v & 31;
    const uint8_t row   = (_regs.v >> 5) & 31;
    const uint8_t col4  = col >> 2;
    const uint8_t row4  = row >> 2;
    const addr_t addr   = ATTR_TABLE_ADDR | (_regs.v & (A11 | A10)) | (row4 << 3) | col4;
    const uint8_t attr  = _mmap->read(addr);
    const uint8_t shift = (col & 2) + ((row & 2) << 1);
    const uint8_t index = (attr >> shift) & 3;
    tile.pindex = index;
}

inline void RP2C02::fetch_bg_pattern(TileData& tile, bool plane)
{
    /*
     * It is expected the v register to contain:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
     *    |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
     *    |   |   |   +---+-----------------------------------------> Nametable select
     *    +---+---+-------------------------------------------------> Fine Y scroll
     *
     * Pattern address:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    0   0   |   |   |   |   |   |   |   |   |   |   +---+---+-> Fine Y offset (row within tile)
     *            |   |   |   |   |   |   |   |   |   +-------------> Bit plane (0: LSB, 1: MSB)
     *            |   +---+---+---+---+---+---+---+-----------------> Tile character code (0-255)
     *            +-------------------------------------------------> Pattern table (0: $0000; 1: $1000)
     */
    const addr_t fine_y = (_regs.v >> 12) & 7;
    const addr_t offset = (tile.tilech << 4) | (plane << 3) | fine_y;
    const addr_t addr = _bg_base | offset;
    tile.plane[plane] = _mmap->read(addr);
}

bool RP2C02::sprite_evaluation(uint8_t spindex, unsigned line)
{
    /* XXX TODO implement the sprite overflow bug */
    if (_oam_sec_count < SEC_SPRITES) {
        const Oam sprite = *reinterpret_cast<Oam*>(&_oam[spindex * sizeof(Oam)]);
        const unsigned height = 8 + 8 * _sp_8x16;
        const unsigned y1 = sprite.y + SPRITE_Y_OFFSET;
        const unsigned y2 = y1 + height;
        const bool visible = (line >= y1) && (line < y2);
        if (visible) {
            _oam_sec[_oam_sec_count] = {spindex, sprite};
            ++_oam_sec_count;
        }
    }

    return (_oam_sec_count == SEC_SPRITES);
}

std::pair<uint8_t, uint8_t> RP2C02::sprite_planes(Oam sprite, uint8_t spline)
{
    addr_t addr{};

    if (_sp_8x16) {
        /*
         * SpriteData.tilech:
         *   D7 D6 D5 D4 D3 D2 D1 D0
         *    |  |  |  |  |  |  |  |
         *    |  |  |  |  |  |  |  +-> Pattern table (0: $0000, 1: $1000)
         *    +--+--+--+--+--+--+----> Tile code (0..254)
         *
         * Pattern address for 8x16 sprites:
         *   A15 A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *    0   0   0   |   |   |   |   |   |   |   |   |   |   +---+---+-> Fine Y offset (row within tile & 7)
         *                |   |   |   |   |   |   |   |   |   +-------------> Bit plane (0: LSB, 1: MSB)
         *                |   |   |   |   |   |   |   |   +-----------------> 1 if spline > 7; 0 otherwise
         *                |   +---+---+---+---+---+---+---------------------> Tile character code (0-254)
         *                +-------------------------------------------------> Pattern table (0: $0000; 1: $1000)
         *
         */
        if (sprite.vflip) {
            spline = 15 - (spline & 15);
        }
        const addr_t base = ((sprite.tilech & 1) ? PATTERN_TABLE_1_ADDR : PATTERN_TABLE_0_ADDR);
        const addr_t tilech = (sprite.tilech & ~1) + (spline > 7);
        addr = base | (tilech << 4) | (spline & 7);

    } else {
        /*
         * Pattern address for 8x8 sprites:
         *  A15 A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
         *   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
         *   0   0   0   |   |   |   |   |   |   |   |   |   |   +---+---+-> Fine Y offset (row within tile)
         *               |   |   |   |   |   |   |   |   |   +-------------> Bit plane (0: LSB, 1: MSB)
         *               |   +---+---+---+---+---+---+---+-----------------> Tile character code (0-255)
         *               +-------------------------------------------------> Pattern table (0: $0000; 1: $1000)
         */
        if (sprite.vflip) {
            spline = 7 - spline;
        }
        addr = _sp_base | (sprite.tilech << 4) | spline;
    }

    uint8_t plane0 = _mmap->read(addr);
    uint8_t plane1 = _mmap->read(addr + 8);

    if (sprite.hflip) {
        plane0 = utils::reverse(plane0);
        plane1 = utils::reverse(plane1);
    }

    return {plane0, plane1};
}

unsigned RP2C02::paint_tile(unsigned x, const TileData& left, const TileData& right)
{
    if (x < VISIBLE_X_START || x > VISIBLE_X_END) {
        return INVALID_X_COORDINATE;
    }

    const uint16_t plane0 = (left.plane[0] << 8) | (right.plane[0]);
    const uint16_t plane1 = (left.plane[1] << 8) | (right.plane[1]);
    const uint16_t bitmap = plane0 | plane1;
    unsigned hit_0_x = INVALID_X_COORDINATE;
    size_t bcount = 0;
    uint16_t bit = 0x8000 >> _regs.x;

    if (_bg_lborder && x < 8) {
        x = 8;
        bcount = 8 - x;
        bit >>= bcount;
    }

    auto bgit = std::begin(_bg_scanline) + x;
    auto spit = std::begin(_sp_scanline) + x;

    for (; bcount < 8 && bgit != std::end(_bg_scanline); ++bcount) {
        const auto cindex = (((plane1 & bit) == bit) << 1) | ((plane0 & bit) == bit);
        const auto pindex = ((bit > 0x0080) ? left.pindex : right.pindex) * PALETTE_SIZE + cindex;
        const auto rindex = (cindex == 0 ? _bg_palette[0] : _bg_palette[pindex]);
        const Rgba color = palette_color(rindex);
        bgit->used = bitmap & bit;
        bgit->color = color;

        if (spit->spindex == 0 && spit->used && bgit->used && hit_0_x == INVALID_X_COORDINATE && x != 255) {
            /*
             * Sprite 0 hit does not happen:
             * - If background or sprite rendering is disabled in PPUMASK ($2001)
             * - At x=0 to x=7 if the left-side clipping window is enabled
             *   (if bit 2 or bit 1 of PPUMASK is 0).
             *
             * - At x=255, for an obscure reason related to the pixel pipeline.
             *
             * - At any pixel where the background or sprite pixel is transparent
             *   (2-bit color index from the CHR pattern is %00).
             *
             * - If sprite 0 hit has already occurred this frame.
             *   Bit 6 of PPUSTATUS ($2002) is cleared to 0 at dot 1 of the pre-render line.
             *   This means only the first sprite 0 hit in a frame can be detected.
             *
             * Sprite 0 hit happens regardless of the following:
             * - Sprite priority. Sprite 0 can still hit the background from behind.
             * - The pixel colors. Only the CHR pattern bits are relevant,
             * - The palette.
             * - The PAL PPU blanking on the left and right edges at x=0, x=1, and x=254 (see Overscan).
             *
             * See https://www.nesdev.org/wiki/PPU_OAM#Sprite_0_hits
             */
            hit_0_x = x;
        }

        bit >>= 1;
        ++bgit;
        ++spit;
        ++x;
    }

    return hit_0_x;
}

void RP2C02::paint_sprite(uint8_t spindex)
{
    const OamSec& osec = _oam_sec[spindex];
    const auto sprite = osec.sprite;
    const bool not_visible = (+sprite.x < VISIBLE_X_START) || (+sprite.x >= VISIBLE_X_END);
    if (not_visible) {
        return;
    }

    const auto spline = _rasterline - sprite.y;
    const auto [plane0, plane1] = sprite_planes(sprite, spline);
    const uint8_t bitmap = plane0 | plane1;

    unsigned x = sprite.x;
    uint8_t bit = 128;
    if (_sp_lborder && sprite.x < 8) {
        bit >>= (8 - sprite.x);
        x = 8;
    }

    auto spit = std::begin(_sp_scanline) + x;

    for (; bit && spit != std::end(_sp_scanline); bit >>= 1, ++spit) {
        if (!spit->used && (bitmap & bit)) {
            const auto cindex = (((plane1 & bit) == bit) << 1) | ((plane0 & bit) == bit);
            const auto pindex = sprite.pindex * PALETTE_SIZE + cindex;
            const auto rindex = _sp_palette[pindex];
            const Rgba color = palette_color(rindex);
            spit->spindex = osec.spindex;
            spit->bgpri = sprite.bgpri;
            spit->used = true;
            spit->color = color;
        }
    }
}

void RP2C02::paint_sprites()
{
    for (size_t sp = 0; sp < _oam_sec_count; ++sp) {
        paint_sprite(sp);
    }
}

void RP2C02::paint_scanline()
{
    auto bgit = std::begin(_bg_scanline);
    auto spit = std::begin(_sp_scanline);

    std::for_each(std::begin(_scanline), std::end(_scanline), [&bgit, &spit](Rgba& color) {
        const auto& bg = *bgit++;
        const auto& sp = *spit++;
        color = (sp.used && (!bg.used || (bg.used && !sp.bgpri)) ? sp.color : bg.color);
    });

    std::fill(std::begin(_bg_scanline), std::end(_bg_scanline), TilePixel{});
    std::fill(std::begin(_sp_scanline), std::end(_sp_scanline), SpritePixel{});
}

inline bool RP2C02::is_palette_address(const addr_t addr) const
{
    return ((addr & PALETTE_ADDR_MASK) == PALETTE_ADDR);
}

Rgba RP2C02::palette_color(size_t rindex) const
{
    /*
     * Tint works on colors $00-$0D, $10-$1D, $20-$2D, and $30-$3D.
     */
    const auto color = _palette[rindex & _rindex_mask];
    const bool tinted = (_tint && (rindex & 0x30) < 0x0E);
    return (tinted ?
        Rgba{static_cast<uint8_t>(color.r * _red_tint),
             static_cast<uint8_t>(color.g * _green_tint),
             static_cast<uint8_t>(color.b * _blue_tint)} :
        color);
}

Rgba RP2C02::backdrop_color() const
{
    /*
     * "When both background and sprite rendering are disabled,
     * this is called forced blank. During forced blank, the PPU
     * normally draws the backdrop color. However, if the current
     * VRAM address in v points into palette RAM ($3F00-$3FFF),
     * then the color at that address will be drawn, instead,
     * overriding the backdrop color."
     *
     * https://www.nesdev.org/wiki/PPU_palettes
     */
    const uint8_t cindex = (is_forced_vblank() && is_palette_address(_regs.v) ? _mmap->read(_regs.v) : BACKDROP_CINDEX);
    return _palette[cindex & PALETTE_COLOR_MASK];
}

inline bool RP2C02::is_forced_vblank() const
{
    return (!_bg_enabled && !_sp_enabled);
}

bool RP2C02::is_rendering() const
{
    const bool renabled = _bg_enabled || _sp_enabled;
    const bool visible = (_rasterline == PRE_RENDER_LINE) ||
        (_rasterline >= VISIBLE_Y_START && _rasterline < VISIBLE_Y_END);
    return (renabled && visible);
}

void RP2C02::scroll_x_coarse_inc()
{
    /*
     * It is expected the v register to contain:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
     *    |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
     *    |   |   |   +---+-----------------------------------------> Nametable select
     *    +---+---+-------------------------------------------------> Fine Y scroll
     *
     * See https://www.nesdev.org/wiki/PPU_scrolling#Coarse_X_increment
     */
    constexpr static const addr_t COARSE_SCROLL_MASK = 0b0000'0000'0001'1111;
    constexpr static const addr_t COARSE_X_MASK      = COARSE_SCROLL_MASK;
    constexpr static const addr_t HORIZ_NAMETABLE    = A10;
    if ((_regs.v & COARSE_X_MASK) == COARSE_X_MASK) {
        _regs.v &= ~COARSE_X_MASK;
        _regs.v ^= HORIZ_NAMETABLE;
    } else {
        ++_regs.v;
    }
}

void RP2C02::scroll_y_inc()
{
    /*
     * It is expected the v register to contain:
     *   A14 A13 A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
     *    |   |   |   |   |   |   |   |   |   |   |   |   |   |   |
     *    |   |   |   |   |   |   |   |   |   |   +---+---+---+---+-> Coarse X scroll
     *    |   |   |   |   |   +---+---+---+---+---------------------> Coarse Y scroll
     *    |   |   |   +---+-----------------------------------------> Nametable select
     *    +---+---+-------------------------------------------------> Fine Y scroll
     *
     * See https://www.nesdev.org/wiki/PPU_scrolling#Y_increment
     */
    constexpr static const addr_t COARSE_SCROLL_MASK = 0b0000'0000'0001'1111;
    constexpr static const addr_t COARSE_Y_MASK      = COARSE_SCROLL_MASK << 5;
    constexpr static const addr_t COARSE_Y_INVERT_NT = 0b0000'0000'0001'1101;
    constexpr static const addr_t FINE_Y_MASK        = 0b0111'0000'0000'0000;
    constexpr static const addr_t FINE_Y_ONE         = A12;
    constexpr static const addr_t VERT_NAMETABLE     = A11;

    if ((_regs.v & FINE_Y_MASK) != FINE_Y_MASK) {
        /*
         * Fine Y < 7: Increment fine Y.
         */
        _regs.v += FINE_Y_ONE;

    } else {
        /*
         * Fine Y = 7:
         *  - Set fine Y to zero
         *  - Increment coarse Y:
         *    + if reached 29 set coarse Y to 0 and invert vertical nametable
         *    + if reached 31 set coarse Y to 0 and do not invert nametable
         *    + else increment coarse Y normally
         */
        addr_t coarse_y = (_regs.v & COARSE_Y_MASK) >> 5;

        switch (coarse_y) {
        case COARSE_Y_INVERT_NT:
            /*
             * "Row 29 is the last row of tiles in a nametable.
             *  To wrap to the next nametable when incrementing coarse Y from 29,
             *  the vertical nametable is switched by toggling bit 11,
             *  and coarse Y wraps to row 0."
             */
            _regs.v ^= VERT_NAMETABLE;

            /* FALLTROUGH */

        case COARSE_SCROLL_MASK:
            coarse_y = 0;
            break;

        default:
            ++coarse_y;
        }

        _regs.v = (_regs.v & ~(FINE_Y_MASK | COARSE_Y_MASK)) | (coarse_y << 5);
    }
}

}
}
}
