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
#include "fs.hpp"
#include "name.hpp"
#include "rgb.hpp"
#include "ui.hpp"

namespace caio {
namespace sinclair {
namespace zx80 {

/**
 * ZX80 video interface.
 *
 * ### Screen resolution:
 *
 *     |<-------------------------- 352 -------------------------->|
 *
 *     |<- 48 ->|<----------------- 256 ----------------->|<- 48 ->|
 *
 *     +-----------------------------------------------------------+     -+-       -+-
 *     |                   NOT-VISIBLE UPPER BORDER                |      | 9       |
 *     |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|     -+-         > 56    -+-
 *     |                    VISIBLE UPPER BORDER                   |      | 47      |         |
 *     |        +-----------------------------------------+        |     -+-       -+-        |
 *     |        |                                         |        |      |                   |
 *     |        |              DISPLAY AREA               |        |      |                   |
 *     |        |                                         |        |      |                   |
 *     |        |                                         |        |      |                   |
 *     |        |                                         |        |       > 192               > 286
 *     |        |                                         |        |      |                   |
 *     |        |                                         |        |      |                   |
 *     |        |                                         |        |      |                   |
 *     |        |                                         |        |      |                   |
 *     |        +-----------------------------------------+        |     -+-       -+-        |
 *     |                    VISIBLE BOTTOM BORDER                  |      | 47      |         |
 *     |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|     -+-         > 56    -+-
 *     |                  NOT-VISIBLE BOTTOM BORDER                |      | 9       |
 *     +-----------------------------------------------------------+     -+-       -+-
 */
class ZX80Video : public Name {
public:
    constexpr static const char* TYPE = "ZX80-VID";

    constexpr static unsigned LBORDER_WIDTH          = 48;
    constexpr static unsigned RBORDER_WIDTH          = 48;
    constexpr static unsigned UBORDER_HEIGHT         = 47;
    constexpr static unsigned BBORDER_HEIGHT         = 47 - 9;
    constexpr static unsigned DISPLAY_WIDTH          = 256;
    constexpr static unsigned DISPLAY_HEIGHT         = 192;

    constexpr static unsigned VISIBLE_WIDTH          = LBORDER_WIDTH + DISPLAY_WIDTH + RBORDER_WIDTH;
    constexpr static unsigned VISIBLE_HEIGHT         = UBORDER_HEIGHT + DISPLAY_HEIGHT + BBORDER_HEIGHT;

    constexpr static unsigned SCANLINE_VISIBLE_START = 9;
    constexpr static unsigned SCANLINE_VISIBLE_END   = SCANLINE_VISIBLE_START + VISIBLE_HEIGHT;

    constexpr static unsigned LBORDER_START          = 0;
    constexpr static unsigned LBORDER_END            = LBORDER_START + LBORDER_WIDTH;
    constexpr static unsigned RBORDER_START          = LBORDER_END + DISPLAY_WIDTH;
    constexpr static unsigned RBORDER_END            = RBORDER_START + RBORDER_WIDTH;
    constexpr static unsigned UBORDER_START          = 0;
    constexpr static unsigned UBORDER_END            = UBORDER_START + UBORDER_HEIGHT;
    constexpr static unsigned BBORDER_START          = UBORDER_END + DISPLAY_HEIGHT;
    constexpr static unsigned BBORDER_END            = SCANLINE_VISIBLE_END;
    constexpr static unsigned WIDTH                  = VISIBLE_WIDTH;
    constexpr static unsigned HEIGHT                 = VISIBLE_HEIGHT;

    using RendererCb = std::function<void(unsigned, const ui::Scanline&)>;
    using ClsCb      = std::function<void(const Rgba&)>;

    enum {
        BLACK = 0,
        WHITE = 1
    };

    /**
     * Initialise this video controller.
     * @param clk   System's clock;
     * @param rvideo True to reverse video, false otherwise;
     * @param label Label assigned to this device.
     * @see render_line(const renderer_t&)
     */
    ZX80Video(std::string_view label, const sptr_t<Clock>& clk, bool rvideo);

    virtual ~ZX80Video() {
    }

    /**
     * Set the render line callback.
     * The render line callback must send the video output to the UI.
     * @param rl The render line callback.
     */
    void render_line(const RendererCb& rl);

    /**
     * Set the clear screen callback.
     * The clear screen callback must call the UI's clear screen method.
     * @param cls The clear screen callback.
     */
    void clear_screen(const ClsCb& cls);

    /**
     * Read a colour palette from disk.
     * @param fname Palette file name.
     * @exception IOError
     * @see palette(const RgbaTable&)
     * @see RgbaTable::load(const fs::Path&)
     */
    void palette(const fs::Path& fname);

    /**
     * Set a colour palette.
     * @param plt Colour palette to set.
     * @see RgbaTable
     */
    void palette(const RgbaTable& plt);

    /**
     * Clear the screen.
     * @see clear_screen(const cls_t&)
     */
    void clear_screen();

    /**
     * Paint 8 pixels in the current scanline.
     * Set bits (1) are painted using the ink colour and
     * cleared bits (0) are painted using the paper colour.
     * @param start  Starting horizontal position within the scanline;
     * @param bitmap Bitmap to pain.
     * @see ink(uint8_t)
     * @see paper(uint8_t)
     */
    void paint_byte(unsigned start, uint8_t bitmap);

    /**
     * Paint 8 pixels at the current position in the scanline.
     * @param vdata Bitmap to paint.
     * @see paint_byte(unsigned, uint8_t)
     */
    void bitmap(uint8_t vdata);

    /**
     * Horizontal Synchronisation.
     * Render the current scanline and prepare to start the next one.
     */
    void hsync();

    /**
     * Initiate/terminate the vertical synchronisation.
     * @param on true to initiate the vertical sync; false to terminate it.
     */
    void vsync(bool on);

private:
    /**
     * Render the current scanline.
     * @see render_line(const renderer_t&)
     */
    void render_line();

    /**
     * Get the background colour.
     * @return The background colour.
     * @see _rvideo
     */
    const Rgba& bg_colour() const;

    /**
     * Get the foreground colour.
     * @return The foreground colour.
     * @see _rvideo
     */
    const Rgba& fg_colour() const;

    /**
     * Retrieve the next line disturbance value after an out-of-sync.
     * @param A Maximum amplitude;
     * @param t Time (at exit it is updated with the new time value).
     * @return The next line disturbance value.
     */
    int disturbance(float A, float& t);

    sptr_t<Clock> _clk;                     /* System clock                     */
    bool          _rvideo;                  /* Reverse video flag               */
    RgbaTable     _palette;                 /* Colour palette                   */
    ui::Scanline  _scanline;                /* Current scanline pixel data      */

    RendererCb    _renderline_cb{};         /* Renderer callback                */
    ClsCb         _cls_cb{};                /* Clear screen callback            */
    int           _line{};                  /* Current raster line              */
    unsigned      _column{LBORDER_START};   /* Current horizontal position      */
    size_t        _vsync_count{};           /* Number of vsync pulses           */
    int           _lineoff{};               /* Out-of-sync scanline variation   */
    float         _A{};                     /* Out-of-sync amplitude            */
    float         _t{};                     /* Out-of-sync time                 */

    static RgbaTable builtin_palette;       /* Default colour palette           */
};

}
}
}
