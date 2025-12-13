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
#include "name.hpp"
#include "ram.hpp"
#include "rgb.hpp"
#include "types.hpp"
#include "ui.hpp"
#include "zilog_z80.hpp"

namespace caio {
namespace sinclair {
namespace zxspectrum {

/*
 * ZX-Spectrum colour codes.
 */
enum class Colour : uint8_t {
    Black           = 0,
    Blue            = 1,
    Red             = 2,
    Magenta         = 3,
    Green           = 4,
    Cyan            = 5,
    Yellow          = 6,
    White           = 7,
    Bright_Black    = 8,
    Bright_Blue     = 9,
    Bright_Red      = 10,
    Bright_Magenta  = 11,
    Bright_Green    = 12,
    Bright_Cyan     = 13,
    Bright_Yellow   = 14,
    Bright_White    = 15
};

constexpr static const uint8_t COLOUR_MASK = 0x0F;

/**
 * ULA Video interface.
 * The ULA Video interface controls the generation of video content,
 * it also interrupts the CPU at VSync rate.
 *
 * ### Screen resolution:
 *
 *     |<-------------------------- 352 ---------------------------->|
 *
 *     |<- 32 ->|<----------------- 256 ----------------->|<-- 64 -->|
 *         48                                                  48
 *     +-------------------------------------------------------------+     -+-
 *     |                           VSYNC                             |      | 8
 *     +-------------------------------------------------------------+     -+-       -+-
 *     |                   NOT-VISIBLE UPPER BORDER                  |      | 8       |
 *     |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|     -+-         > 56    -+-
 *     |                    VISIBLE UPPER BORDER                     |      | 48      |         |
 *     |        +-----------------------------------------+          |     -+-       -+-        |
 *     |        |                                         |          |      |                   |
 *     |        |              DISPLAY AREA               |          |      |                   |
 *     |        |                                         |          |      |                   |
 *     |        |                                         |          |      |                   |
 *     |        |                                         |          |       > 192               > 286
 *     |        |                                         |          |      |                   |
 *     |        |                                         |          |      |                   |
 *     |        |                                         |          |      |                   |
 *     |        |                                         |          |      |                   |
 *     |        +-----------------------------------------+          |     -+-       -+-        |
 *     |                    VISIBLE BOTTOM BORDER                    |      | 48      |         |
 *     |- - - - - - - - - - - - - - - - - - - - - - - - - - - - - - -|     -+-         > 56    -+-
 *     |                  NOT-VISIBLE BOTTOM BORDER                  |      | 8       |
 *     +-------------------------------------------------------------+     -+-       -+-
 *
 * ### Clock frequency and video signals:
 *
 * In the ZX Spectrum, the ULA device is driven by a 14MHz clock which is internally
 * divided by 2 producing a 7MHz signal called CLK7 (also referred as pixel clock)
 * from this, all the other clock signals are generated including the CPU clock which
 * is CLK7/2.
 *
 * A PAL scanline lasts for 64us, that is 448 CLK7 cycles, subdivided as follows:
 *
 *     --+                               +--------+---------------------------+------------+
 *       |Porch         Blanking         |LBorder            Display             RBorder   |
 *       +--+      +---------------------+--------+---------------------------+------------+
 *          |      |
 *          +------+
 *           HSync
 *       |<----------- 96 -------------->|<- 32 ->|<---------- 256 ---------->|<--- 64 --->|
 *
 * - 96 cycles (12 characters, 13.7us (*)): Not visible horizontal positions:
 *   - Back porch (2.2us)
 *   - H-SYNC pulse (4.6us)
 *   - Blanking (6.9us)
 *
 * - 352 cycles (50.3us): Visible horizontal positions:
 *   - 32 cycles (4 characters):   Left border (4.57us)
 *   - 256 cycles (32 characters): Display area (6.6us)
 *   - 64 cycles (8 characters):   Right border (9.14us)
 *
 * (*) The PAL standard states that during the H-SYNC period the video signal must
 *     be blanked for 12us. The ULA chip instead generates a 13.7us blanking period.
 *
 * To avoid starving the host CPU, instead of painting a single pixel each CLK7 cycle
 * this implementation paints 8 pixels at a time so it has to be ticked at a rate of
 * CLK7/8 which is equal to CLK/4, where CLK is the CPU clock (3.5MHz).
 * As stated before, the real ULA generates the clock for the CPU and controls it,
 * this implementation instead relies on a system clock running at CPU frequency
 * that drives both ULA and the CPU separately (this is the first divergence between
 * the real ULA and the emulated one).
 *
 * Using the approach above the previous clock cycles for a scanline change as
 * follows:
 *
 * - 56 cycles (64us) for a complete scanline
 *
 * - 12 cycles (13.7us): Not visible horizontal positions:
 *   + Back porch (2.2us)
 *   + HSYNC pulse and blanking (4.6us)
 *   + Blanking (6.9us)
 *
 * - 44 cycles (50.3us): Visible horizontal positions:
 *   + 4 cycles:  Left border (4.57us)
 *   + 32 cycles: Display area (6.6us)
 *   + 8 cycles:  Right border (9.14us)
 *
 * With:
 *   CPU clock (system clock): CLK = CLK7 / 2
 *   ULA clock: ULA_CLK = CLK7 / 8 = CLK / 4
 *
 * The V-SYNC pulse lasts for 4 scanlines (256us)
 *
 * The ULA device also generates an interrupt request to the CPU when the V-SYNC
 * period is started, the exact moment coincides with the origin of the display area
 * horizontal position (soon after the left border), so the IRQ is triggered at
 * scanline 0 after 4 clock cycles. ULA does not have interrupt acknowledge
 * capabilities, it just keeps the interrupt pin active for 8 CPU clock cycles
 * (2 ULA clock cycles) and then deactivates it until the next vertical sync.
 *
 * ### Pixel data:
 *
 * The system's base address of pixel data is $8000 ($0000 relative to the 16K RAM).
 * The offset to access the pixel data is as weird as it can be:
 *
 *     A12 A11 A10 A9  A8  A7  A6  A5  A4  A3  A2  A1  A0
 *     L7  L6  L2  L1  L0  L5  L4  L3  C4  C3  C2  C1  C0
 *
 * where:
 *   L: Display line (0..191);
 *   C: Display column (0..31).
 *
 * ### Colour attributes:
 *
 * The display area of 256x192 pixels is divided in blocks of 8x8 pixels (or characters),
 * Each of these characters has a colour attribute (the 64 pixels within a 8x8 block
 * share the same colour attributes) defined as follows:
 *
 *     D7 D6 D5 D4 D3 D2 D1 D0
 *     |  |  |  |  |  |  |  |
 *     |  |  |  |  |  |  |  +-> B \
 *     |  |  |  |  |  |  +----> R  > Foreground colour
 *     |  |  |  |  |  +-------> G /
 *     |  |  |  |  +----------> B \
 *     |  |  |  +-------------> R  > Background colour
 *     |  |  +----------------> G /
 *     |  +-------------------> H: Bright Flag
 *     +----------------------> F: Flash mode (swap foreground and background at a rate of 1.56Hz)
 *
 * @see ULAASpace
 * @see https://en.wikipedia.org/wiki/ZX_Spectrum_graphic_modes
 * @see https://spectrumforeveryone.com/wp-content/uploads/2017/08/ZX-Spectrum-Service-Manual.pdf
 * @see "The ZX Spectrum ULA: How to Design a Microcomputer (ZX Design Retro Computer)", Chris Smith
 */
class ULAVideo : public Clockable, public Name {
public:
    constexpr static const char* TYPE = "ULA-VIDEO";

    /*
     * Pixel Coordinates.
     */
    constexpr static const unsigned LBORDER_WIDTH           = 48; //32;
    constexpr static const unsigned RBORDER_WIDTH           = 48; //64;
    constexpr static const unsigned UBORDER_HEIGHT          = 48;
    constexpr static const unsigned BBORDER_HEIGHT          = 48;
    constexpr static const unsigned DISPLAY_WIDTH           = 256;
    constexpr static const unsigned DISPLAY_HEIGHT          = 192;
    constexpr static const unsigned VISIBLE_WIDTH           = LBORDER_WIDTH + DISPLAY_WIDTH + RBORDER_WIDTH;
    constexpr static const unsigned VISIBLE_HEIGHT          = UBORDER_HEIGHT + DISPLAY_HEIGHT + BBORDER_HEIGHT;
    constexpr static const unsigned LBORDER_X_START         = 0;
    constexpr static const unsigned LBORDER_X_END           = LBORDER_X_START + LBORDER_WIDTH;
    constexpr static const unsigned RBORDER_X_START         = LBORDER_X_END + DISPLAY_WIDTH;
    constexpr static const unsigned RBORDER_X_END           = RBORDER_X_START + RBORDER_WIDTH;
    constexpr static const unsigned UBORDER_Y_START         = 0;
    constexpr static const unsigned UBORDER_Y_END           = UBORDER_Y_START + UBORDER_HEIGHT;
    constexpr static const unsigned BBORDER_Y_START         = UBORDER_Y_END + DISPLAY_HEIGHT;
    constexpr static const unsigned BBORDER_Y_END           = BBORDER_Y_START + BBORDER_HEIGHT;
    constexpr static const unsigned WIDTH                   = VISIBLE_WIDTH;
    constexpr static const unsigned HEIGHT                  = VISIBLE_HEIGHT;

    constexpr static const unsigned LBORDER_COLUMNS         = LBORDER_WIDTH >> 3;
    constexpr static const unsigned RBORDER_COLUMNS         = RBORDER_WIDTH >> 3;
    constexpr static const unsigned DISPLAY_COLUMNS         = DISPLAY_WIDTH >> 3;
    constexpr static const unsigned VISIBLE_COLUMNS         = RBORDER_X_END >> 3;

    constexpr static const unsigned DISPLAY_ROWS            = DISPLAY_HEIGHT >> 3;

    /*
     * Horizontal timing.
     */
    constexpr static const unsigned HSYNC_CYCLES            = 12;
    constexpr static const unsigned LBORDER_CYCLES          = 4;
    constexpr static const unsigned DISPLAY_CYCLES          = 32;
    constexpr static const unsigned RBORDER_CYCLES          = 8;
    constexpr static const unsigned HSYNC_CYCLE_START       = 0;
    constexpr static const unsigned HSYNC_CYCLE_END         = HSYNC_CYCLE_START + HSYNC_CYCLES;
    constexpr static const unsigned LBORDER_CYCLE_START     = HSYNC_CYCLE_END;
    constexpr static const unsigned LBORDER_CYCLE_END       = LBORDER_CYCLE_START + LBORDER_CYCLES;
    constexpr static const unsigned DISPLAY_CYCLE_START     = LBORDER_CYCLE_END;
    constexpr static const unsigned DISPLAY_CYCLE_END       = DISPLAY_CYCLE_START + DISPLAY_CYCLES;
    constexpr static const unsigned RBORDER_CYCLE_START     = DISPLAY_CYCLE_END;
    constexpr static const unsigned RBORDER_CYCLE_END       = RBORDER_CYCLE_START + RBORDER_CYCLES;
    constexpr static const unsigned SCANLINE_CYCLES         = RBORDER_CYCLE_END;

    /*
     * Vertical timing.
     */
    constexpr static const unsigned SCANLINE_VSYNC_START    = 0;
    constexpr static const unsigned SCANLINE_VSYNC_END      = 9;
    constexpr static const unsigned SCANLINE_VISIBLE_START  = SCANLINE_VSYNC_END + 8;
    constexpr static const unsigned UBORDER_SCANLINE_START  = SCANLINE_VISIBLE_START;
    constexpr static const unsigned UBORDER_SCANLINE_END    = UBORDER_SCANLINE_START + UBORDER_HEIGHT;
    constexpr static const unsigned DISPLAY_SCANLINE_START  = UBORDER_SCANLINE_END;
    constexpr static const unsigned DISPLAY_SCANLINE_END    = DISPLAY_SCANLINE_START + DISPLAY_HEIGHT;
    constexpr static const unsigned BBORDER_SCANLINE_START  = DISPLAY_SCANLINE_END;
    constexpr static const unsigned BBORDER_SCANLINE_END    = BBORDER_SCANLINE_START + BBORDER_HEIGHT;
    constexpr static const unsigned SCANLINE_VISIBLE_END    = BBORDER_SCANLINE_END;
    constexpr static const unsigned SCANLINES               = SCANLINE_VISIBLE_END + 8;

    /*
     * IRQ timing.
     */
    constexpr static const unsigned IRQ_SCANLINE            = 0;
    constexpr static const unsigned IRQ_CYCLES              = 2;
    constexpr static const unsigned IRQ_CYCLE_START         = DISPLAY_CYCLE_START;
    constexpr static const unsigned IRQ_CYCLE_END           = IRQ_CYCLE_START + IRQ_CYCLES;

    /*
     * ULA clock timing and misc.
     */
    constexpr static const size_t ULA_TICK_CYCLES           = 4;    /* CLK/4 */
    constexpr static const float ULA_FREQ                   = 7'000'000 / 8.0f;
    constexpr static const float FLASH_FREQ                 = 1.56f;
    constexpr static const size_t COLOUR_FLASH_TICKS        = ULA_FREQ / FLASH_FREQ;

    /*
     * VRAM addressing.
     */
    constexpr static const addr_t DISPLAY_PIXELS            = DISPLAY_WIDTH * DISPLAY_HEIGHT;
    constexpr static const addr_t DISPLAY_CHARACTERS        = DISPLAY_PIXELS >> 3;
    constexpr static const addr_t DISPLAY_COLOURS           = DISPLAY_CHARACTERS >> 3;
    constexpr static const addr_t VRAM_MIN_SIZE             = DISPLAY_CHARACTERS + DISPLAY_COLOURS;
    constexpr static const addr_t DISPLAY_BASE_ADDR         = 0x0000;   /* Relative to the 16K RAM */
    constexpr static const addr_t COLOUR_ATTR_BASE_ADDR     = DISPLAY_BASE_ADDR + DISPLAY_CHARACTERS;

    using Renderer = std::function<void(unsigned, const ui::Scanline&)>;

    /**
     * Initialise this video controller.
     * @param cpu   CPU;
     * @param ram   RAM containing the video memory;
     * @param label Label assigned to this device.
     * @see render_line(const Renderer&)
     */
    ULAVideo(const sptr_t<Z80>& cpu, const sptr_t<RAM>& ram, std::string_view label);

    virtual ~ULAVideo() = default;

    /**
     * Set the render line callback.
     * The render line callback must send the video output to the UI.
     * @param rl The render line callback.
     */
    void render_line(const Renderer& rl);

    /**
     * Read a colour palette from disk.
     * @param fname Palette file name.
     * @exception IOError
     * @see palette(const RgbaTable&)
     * @see RgbaTable::load(std::string_view)
     */
    void palette(std::string_view fname);

    /**
     * Set a colour palette.
     * @param plt Colour palette to set.
     * @see RgbaTable
     */
    void palette(const RgbaTable& plt);

    /**
     * Set the border colour.
     * @param code Colour code.
     * @see Colour
     */
    void border_colour(uint8_t code);

private:
    /**
     * Video clock tick.
     * Paint 8 pixels in the current scanline, when the entire scanline is painted
     * render it and start a HSync or a VSync period depending on the position of
     * the raster line.
     * The frequency of the flash colour attribute is also handled by this method.
     * @param clk Caller clock.
     * @return The number of clock cycles the must pass before calling this method again.
     * @see Clockable::tick(const Clock&)
     */
    size_t tick(const Clock& clk) override;

    /**
     * Render the current scanline.
     * @see render_line(const Renderer&)
     */
    void render_line();

    /**
     * Get an RGBA colour from its Colour code counterpart.
     * @param code Colour code.
     * @return The RGBa colour.
     * @see Colour
     * @see _palette
     */
    Rgba to_rgba(Colour code) const;

    /**
     * Paint 8 pixels in the current scanline.
     * @param start  Horizontal position in the scanline to start painting.
     * @param bitmap Bitmap to paint;
     * @param fg     Foreground colour;
     * @param bg     Background colour.
     */
    void paint_byte(unsigned start, uint8_t bitmap, Rgba fg, Rgba bg);

    /**
     * Paint 8 pixels of the display area in the current coordinates in the scanline.
     */
    void paint_display();

    sptr_t<Z80>   _cpu;
    sptr_t<RAM>   _ram;
    RgbaTable     _palette;
    ui::Scanline  _scanline;

    Renderer      _renderline_cb{};         /* Renderer callback                        */
    int           _line{};                  /* Current scanline                         */
    unsigned      _cycle{};                 /* Current horizontal character position    */
    unsigned      _flash_counter{};         /* Counter for the flash attribute          */
    bool          _flash_swap{};            /* Swap background and foreground colours   */
    bool          _intreq{};                /* Interrupt request flag                   */
    Rgba          _border_colour{};         /* Border colour                            */

    static RgbaTable builtin_palette;       /* Default colour palette                   */

    friend Serializer& operator&(Serializer&, ULAVideo&);
};

}
}
}
