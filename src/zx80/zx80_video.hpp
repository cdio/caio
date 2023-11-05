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
#pragma once

#include "device.hpp"
#include "name.hpp"
#include "rgb.hpp"
#include "ui.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * ZX80 video device.
 */
class ZX80Video : public Name {
public:
    constexpr static const char* TYPE = "ZX80-VID";

    /*
     * ZX80 timing and screen resolution:
     *      |<--------------- 288 ---------------->|
     *           |<---------- 256 ----------->|
     *  +----------------------------------------------+  -+-     -+-      -+-
     *  |///////////////// V-BLANK ////////////////////|   |       |        | 6
     *  |///+--------------------------------------+///|   |       |       -+-      -+-
     *  |///|       NOT-VISIBLE UPPER BORDER       |///|   |       |        | 8      |
     *  |///|- - - - - - - - - - - - - - - - - - - |///|   |       |       -+-        > 56   -+-
     *  |///|       VISIBLE UPPER BORDER           |///|   |       |        | 48     |        |
     *  |/ /|    +----------------------------+    |/ /|   |       |       -+-      -+-       |
     *  |/H/|    |                            |    |/H/|   |       |        |                 |
     *  |/ /|    |  DISPLAY AREA              |    |/ /|   |       |        |                 |
     *  |/B/|    |                            |    |/B/|   |       |        |                 |
     *  |/L/|    |                            |    |/L/|   |       |        |                 |
     *  |/A/|    |                            |    |/A/|    > 312   > 310    > 192             > 287
     *  |/N/|    |                            |    |/N/|   |       |        |                 |
     *  |/K/|    |                            |    |/K/|   |       |        |                 |
     *  |/ /|    |                            |    |/ /|   |       |        |                 |
     *  |///|    |                            |    |///|   |       |        |                 |
     *  |///|    +----------------------------+    |///|   |       |       -+-      -+-       |
     *  |///|       VISIBLE BOTTOM BORDER          |///|   |       |        | 47     |        |
     *  |///|- - - - - - - - - - - - - - - - - - - |///|   |       |       -+-        > 56   -+-
     *  |///|       NOT-VISIBLE BOTTOM BORDER      |///|   |       |        | 9      |
     *  +---+--------------------------------------+---+  -+-     -+-      -+-      -+-
     */
    constexpr static unsigned FRAME_WIDTH        = 504;
    constexpr static unsigned FRAME_HEIGHT       = 312;
    constexpr static unsigned LBORDER_START      = 16;
    constexpr static unsigned VISIBLE_WIDTH      = 288 + 2 * LBORDER_START;
    constexpr static unsigned VISIBLE_HEIGHT     = 287;     /* PAL visible scanlines */
    constexpr static unsigned WIDTH              = VISIBLE_WIDTH;
    constexpr static unsigned HEIGHT             = VISIBLE_HEIGHT;
    constexpr static unsigned VISIBLE_Y_START    = 6 + 8;
    constexpr static unsigned VISIBLE_Y_END      = VISIBLE_Y_START + VISIBLE_HEIGHT;

    using renderer_t = std::function<void(unsigned, const ui::Scanline&)>;

    enum {
        BLACK = 0,
        WHITE = 1
    };

    /**
     * Initialise this video controller.
     * @param label Label assigned to this device;
     * @see render_line(const std::function<void(unsigned, const ui::Scanline&)>&);
     */
    ZX80Video(const std::string& label);

    virtual ~ZX80Video() {
    }

    /**
     * Set the render line callback.
     * The render line callback must send the video output to the user interface.
     * @param rl The render line callback.
     */
    void render_line(const renderer_t& rl) {
        _renderline_cb = rl;
    }

    /**
     * Set a colour palette from disk.
     * @param fname Palette file name.
     * @exception IOError see RgbaTable::load().
     * @see palette(const RgbaTable &)
     */
    void palette(const std::string& fname);

    /**
     * Set a colour palette from memory.
     * @param plt Colour palette to set.
     * @see RgbaTable
     */
    void palette(const RgbaTable& plt) {
        _palette = plt;
    }

    /**
     * Horizontal Synchronisation.
     * Render the current scanline and prepare to start with the next.
     */
    void hsync();

    /**
     * Vertical Synchronisation.
     * Perform an horizontal synchronisation and reset the line counter.
     */
    void vsync();

    /**
     * Paint 8 pixels at the current position in the scanline.
     * @param vdata Bitmap to paint.
     */
    void bitmap(uint8_t vdata);

private:
    /**
     * Render the current scanline.
     * @param line Line number assigned to the curent scanline.
     * This method calls the renderline callback in order to render the specified line.
     * @see render_line(const std::function<void(unsigned, const ui::Scanline&)>& rl)
     */
    void render_line();

    /**
     * Paint a byte bitmap (1 bit per pixel) in the current scanline.
     * Set bits (1) are painted as WHITE colour,
     * Cleared bits (0) are painted as BLACK colour.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap;
     */
    void paint_byte(unsigned start, uint8_t bitmap);

    RgbaTable    _palette;                  /* Colour palette                    */
    ui::Scanline _scanline;                 /* Current scanline pixel data       */
    renderer_t   _renderline_cb{};          /* Renderer callback                 */
    unsigned     _line{};                   /* Current raster line               */
    unsigned     _column{LBORDER_START};    /* Current horizontal coordinate     */

    static RgbaTable builtin_palette;       /* Default colour palette            */
};

}
}
}
