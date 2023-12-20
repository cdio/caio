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
 * @see http://martin.hinner.info/vga/pal.html
 */
class ZX80Video : public Name {
public:
    constexpr static const char* TYPE = "ZX80-VID";

    /*
     * ZX80 screen resolution:
     *
     *    |<------------------- 320 ------------------->|
     *         |<-------------- 256 -------------->|
     *    +---------------------------------------------+       -+-       -+-
     *    |          NOT-VISIBLE UPPER BORDER           |        | 9       |
     *    |- - - - - - - - - - - - - - - - - - - - - - -|       -+-         > 56     -+-
     *    |            VISIBLE UPPER BORDER             |        | 47      |          |
     *    |    +-----------------------------------+    |       -+-       -+-         |
     *    |    |                                   |    |        |                    |
     *    |    |         DISPLAY AREA              |    |        |                    |
     *    |    |                                   |    |        |                    |
     *    |    |                                   |    |        |                    |
     *    |    |                                   |    |         > 192                > 286
     *    |    |                                   |    |        |                    |
     *    |    |                                   |    |        |                    |
     *    |    |                                   |    |        |                    |
     *    |    |                                   |    |        |                    |
     *    |    +-----------------------------------+    |       -+-       -+-         |
     *    |           VISIBLE BOTTOM BORDER             |        | 47      |          |
     *    |- - - - - - - - - - - - - - - - - - - - - - -|       -+-         > 56     -+-
     *    |         NOT-VISIBLE BOTTOM BORDER           |        | 9       |
     *    +---------------------------------------------+       -+-       -+-
     *
     * Sync lines not shown.
     */
    constexpr static unsigned VISIBLE_UBORDER        = 47;
    constexpr static unsigned VISIBLE_BBORDER        = 47;
    constexpr static unsigned VISIBLE_DISPLAY        = 192;
    constexpr static unsigned VISIBLE_HEIGHT         = VISIBLE_UBORDER + VISIBLE_DISPLAY + VISIBLE_BBORDER - 9;
    constexpr static unsigned LBORDER_START          = 32;
    constexpr static unsigned DISPLAY_WIDTH          = 256;
    constexpr static unsigned VISIBLE_WIDTH          = DISPLAY_WIDTH + 2 * LBORDER_START;
    constexpr static unsigned WIDTH                  = VISIBLE_WIDTH;
    constexpr static unsigned HEIGHT                 = VISIBLE_HEIGHT;
    constexpr static unsigned SCANLINE_VISIBLE_START = 9;
    constexpr static unsigned SCANLINE_VISIBLE_END   = SCANLINE_VISIBLE_START + VISIBLE_HEIGHT;

    using renderer_t = std::function<void(unsigned, const ui::Scanline&)>;
    using cls_t      = std::function<void(const Rgba&)>;

    enum {
        BLACK = 0,
        WHITE = 1
    };

    /**
     * Initialise this video controller.
     * @param clk   System's clock;
     * @param label Label assigned to this device.
     * @warning If the clk parameter is not valid the process is terminated.
     * @see render_line(const std::function<void(unsigned, const ui::Scanline&)>&);
     */
    ZX80Video(const sptr_t<Clock>& clk, const std::string& label);

    virtual ~ZX80Video() {
    }

    /**
     * Set the render line callback.
     * The render line callback must send the video output to the UI.
     * @param rl The render line callback.
     */
    void render_line(const renderer_t& rl);

    /**
     * Set the clear screen callback.
     * The clear screen callback must call the UI's clear screen method.
     * @param cls The clear screen callback.
     */
    void clear_screen(const cls_t& cls);

    /**
     * Read a colour palette from disk.
     * @param fname Palette file name.
     * @exception IOError
     * @see palette(const RgbaTable&)
     * @see RgbaTable::load(const string&)
     */
    void palette(const std::string& fname);

    /**
     * Set a colour palette.
     * @param plt Colour palette to set.
     * @see RgbaTable
     */
    void palette(const RgbaTable& plt);

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

    /**
     * Paint 8 pixels at the current position in the scanline.
     * @param vdata Bitmap to paint.
     * @see paint_byte(unsigned, uint8_t)
     */
    void bitmap(uint8_t vdata);

private:
    /**
     * Render the current scanline.
     * @see render_line(const renderer_t&)
     */
    void render_line();

    /**
     * Clear the screen.
     * @see clear_screen(const cls_t&)
     */
    void clear_screen();

    /**
     * Paint 8 pixels in the current scanline.
     * Set bits (1) are painted as WHITE, cleared bits (0) are painted as BLACK.
     * @param start  Starting horizontal position within the scanline;
     * @param bitmap Bitmap to pain.
     */
    void paint_byte(unsigned start, uint8_t bitmap);

    /**
     * Retrieve the next line disturbance value after an out-of-sync.
     * @param A Maximum amplitude;
     * @param t Time (at exit it is updated with the new time value).
     * @return The next line disturbance value.
     */
    int disturbance(float A, float& t);

    sptr_t<Clock> _clk;                     /* System clock                 */
    RgbaTable     _palette;                 /* Colour palette               */
    ui::Scanline  _scanline;                /* Current scanline pixel data  */
    renderer_t    _renderline_cb{};         /* Renderer callback            */
    cls_t         _cls_cb{};                /* Clear screen callback        */
    int           _line{};                  /* Current raster line          */
    int           _lineoff{};               /* VSync hack                   */
    unsigned      _column{LBORDER_START};   /* Current horizontal position  */
    size_t        _vsync_count{};           /* Number of vsync pulses       */

    float         _A{};                     /* Out-of-sync amplitude        */
    float         _t{};                     /* Out-of-sync time             */

    static RgbaTable builtin_palette;       /* Default colour palette       */
};

}
}
}
