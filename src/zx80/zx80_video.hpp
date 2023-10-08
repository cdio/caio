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

#include "clock.hpp"
#include "device.hpp"
#include "name.hpp"
#include "rgb.hpp"
#include "ui.hpp"


namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * ZX80 video interface.
 */
class ZX80Video : public Clockable, public Name {
public:
    constexpr static const char* TYPE = "ZX80-Video";

    /*
     * PAL:
     * In a 50Hz TV broadcast signal, there are 25 TV frames per second. In a 60Hz TV broadcast signal,
     * there are 30 TV frames per second. Each frame is composed of two fields. The first field contains
     * the information for all the even lines making up the picture, and is followed by the another field
     * that contains the information for all the odd lines. The fields are displayed so rapidly that the
     * human eye only sees whole TV frames. This mechanism generates an interlaced TV picture.
     *
     * In a broadcast TV picture, each frame consists of 625 scan lines (50Hz picture) / 575 scan lines
     * (60Hz picture). This is divided into odd and even fields of 312.5 scan lines (50Hz picture) / 262.5
     * scan lines (60Hz picture). The half scan line allows an interlaced picture to be easily produced.
     * Of these only 287.5 (50Hz) / 241.5 (60Hz) are visible and hence the equivalent of 25 scan lines
     * do not contain picture information.
     *
     * The first 7.5 scan lines is used for the vertical synchronisation (including equalisation pulses),
     * which instructs the TV set to commence the display of a new TV field.
     * Some of the remaining 17.5 scan lines are used to provide data services such as teletext.
     * For a 50Hz picture, each scan line spans 64µs. Whereas for a 60Hz picture,
     * each scan line spans 63.55µs.
     */

    /*
     * ZX80 timing and screen resolution:
     *  |<------------------- XXX ------------------->|
     *      |<--------------- XXX ---------------->|
     *           |<---------- 256 ----------->|
     *  +----------------------------------------------+  -+-     -+-      -+-
     *  |///////////////// V-BLANK ////////////////////|   |       |        | 6
     *  |///+--------------------------------------+///|   |       |       -+-      -+-
     *  |///|       NOT-VISIBLE UPPER BORDER       |///|   |       |        | 8      |
     *  |///|- - - - - - - - - - - - - - - - - - - |///|   |       |       -+-        > 56
     *  |///|       VISIBLE UPPER BORDER           |///|   |       |        | 48     |
     *  |/ /|    +----------------------------+    |/ /|   |       |       -+-      -+-
     *  |/H/|    |                            |    |/H/|   |       |        |
     *  |/ /|    |  DISPLAY AREA              |    |/ /|   |       |        |
     *  |/B/|    |                            |    |/B/|   |       |        |
     *  |/L/|    |                            |    |/L/|   |       |        |
     *  |/A/|    |                            |    |/A/|    > 312   > 310    > 192
     *  |/N/|    |                            |    |/N/|   |       |        |
     *  |/K/|    |                            |    |/K/|   |       |        |
     *  |/ /|    |                            |    |/ /|   |       |        |
     *  |///|    |                            |    |///|   |       |        |
     *  |///|    +----------------------------+    |///|   |       |       -+-      -+-
     *  |///|       VISIBLE BOTTOM BORDER          |///|   |       |        | 47     |
     *  |///|- - - - - - - - - - - - - - - - - - - |///|   |       |       -+-        > 56
     *  |///|       NOT-VISIBLE BOTTOM BORDER      |///|   |       |        | 9      |
     *  |///+--------------------------------------+///|   |      -+-      -+-      -+-
     *  |//////////////// V-BLANK /////////////////////|   |       | Not used on ZX80
     *  +----------------------------------------------+  -+-     -+-
     */
//XXX move to pal constants
    constexpr static unsigned FRAME_WIDTH        = 504;
    constexpr static unsigned FRAME_HEIGHT       = 312;
    constexpr static unsigned VISIBLE_WIDTH      = 403;
    constexpr static unsigned VISIBLE_HEIGHT     = 192 + 48 + 47;
    constexpr static unsigned WIDTH              = VISIBLE_WIDTH;
    constexpr static unsigned HEIGHT             = VISIBLE_HEIGHT;
    constexpr static unsigned VISIBLE_Y_START    = 6 + 8;
    constexpr static unsigned VISIBLE_Y_END      = VISIBLE_Y_START + VISIBLE_HEIGHT;
    constexpr static const uint8_t CH_LO_MASK    = 0x3F;    /* Bits 6 and 7 not part of character code */
    constexpr static const uint8_t CH_INVERT_BIT = 1 << 7;  /* Bit 7: Invert video flag */


//XXX this here
    constexpr static const size_t PAINT_BYTE_CYCLES = 4;   /* Clock cycles needed to paint 8 pixels */

    using renderer_t = std::function<void(unsigned, const ui::Scanline&)>;

    enum {
        BLACK = 0,
        WHITE = 1
    };

    /**
     * Initialise this video controller.
     * @param label Label assigned to this device;
     * @see render_line(const std::function<void(unsigned, const ui::Scanline &)> &);
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
     * XXX
     */
    void video_data(uint8_t vdata, bool invert);

    /**
     * XXX
     */
    void reset();

    /**
     * XXX
     */
    void hsync() {
        _sync = true;
    }


//    void vsync();

private:
    /**
     * Clock tick.
     * Called by the system clock at specific time intervals, this method paints a scanline.
     * @param clk A reference to the caller clock.
     * @return The number of clock cycles that must pass until the clock is allowed to call this method again.
     * @see Clockable
     */
    size_t tick(const Clock& clk) override;

    /**
     * Render the current scanline.
     * @param line Line number assigned to the curent scanline.
     * This method calls the renderline callback in order to render the specified line.
     * @see render_line(const std::function<void(unsigned, const ui::Scanline&)>& rl)
     */
    void render_line(unsigned line);

    /**
     * Paint a segment in the current scanline.
     * @param start Starting horizontal position;
     * @param width Segment width in pixels (if 0, the entire scanline from the starting position is painted);
     * @param color Colour to use.
     */
    void paint(unsigned start, unsigned width, const Rgba& color);

    /**
     * Paint a byte bitmap (1 bit per pixel) in the current scanline.
     * Set bits (1) are painted with the foreground colour,
     * Cleared bits (0) are painted with the background colour.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap;
     * @param colors Background and foreground colours.
     */
    void paint_byte(unsigned start, uint8_t bitmap, const Rgba2& colors);

    RgbaTable    _palette;              /* Colour palette                               */
    renderer_t   _renderline_cb{};      /* Renderer callback                            */
    ui::Scanline _scanline{};           /* Painted pixel data for the current scanline  */
    uint8_t      _vdata{};              /* 8 pixels video data to paint                 */
    bool         _invert{};             /* Invert painted pixel values                  */
    bool         _sync{};
    unsigned     _rasterline{};         /* Current raster line                          */
    unsigned     _x_coord{};            /* Current horizontal coordinate                */

//    addr_t       _counter{};            /* Lower 3-bit video data address counter           */
//    addr_t       _addr{};               /* Video data address                               */
//    bool         _invert{};             /* Inverted video                                   */
//    bool         _stop{};

    static RgbaTable builtin_palette;   /* Default colour palette                           */
};

}
}
}
