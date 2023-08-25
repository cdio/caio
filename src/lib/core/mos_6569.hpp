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

#include <algorithm>
#include <array>
#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <tuple>
#include <utility>

#include "aspace.hpp"
#include "clock.hpp"
#include "device.hpp"
#include "pin.hpp"
#include "rgb.hpp"
#include "ui.hpp"
#include "utils.hpp"


namespace caio {

/**
 * MOS6569 (VIC-II PAL) emulator.
 * @see C64 Programmer's Reference Guide.pdf, Appendix N
 * @see mos_6567_vic_ii_preliminary.pdf
 * @see https://www.cebix.net/VIC-Article.txt
 */
class Mos6569 : public Device, public Clockable {
public:
    constexpr static const char* TYPE = "MOS6569";

    /*
     * PAL timing and screen resolution:
     *
     *  |<------------------- 504 ------------------->|
     *      |<--------------- 403 ---------------->|
     *           |<---------- 320 ----------->|
     *  +----------------------------------------------+  -+-
     *  |///////////////// V-BLANK ////////////////////|   |
     *  |///+--------------------------------------+///|   |       -+-
     *  |///|       BORDER                         |///|   |        |
     *  |/ /|    +----------------------------+    |/ /|   |        |       -+-
     *  |/H/|    |                            |    |/H/|   |        |        |
     *  |/ /|    |  DISPLAY AREA              |    |/ /|   |        |        |
     *  |/B/|    |                            |    |/B/|   |        |        |
     *  |/L/|    |                            |    |/L/|   |        |        |
     *  |/A/|    |                            |    |/A/|    > 312    > 284    > 200
     *  |/N/|    |                            |    |/N/|   |        |        |
     *  |/K/|    |                            |    |/K/|   |        |        |
     *  |/ /|    |                            |    |/ /|   |        |        |
     *  |///|    |                            |    |///|   |        |        |
     *  |///|    +----------------------------+    |///|   |        |       -+-
     *  |///|                                      |///|   |        |
     *  |///+--------------------------------------+///|   |       -+-
     *  |//////////////// V-BLANK /////////////////////|   |
     *  +----------------------------------------------+  -+-
     *
     * 8px of a scanline refereshed in 1 clock cycle.
     * Total 63 clock cycles per scanline (including blanks).
     */
    constexpr static unsigned FRAME_WIDTH        = 504;
    constexpr static unsigned FRAME_HEIGHT       = 312;

    constexpr static unsigned VISIBLE_WIDTH      = 403;
    constexpr static unsigned VISIBLE_HEIGHT     = 284;

    constexpr static unsigned VISIBLE_Y_START    = 16;   /* (FRAME_HEIGHT_PAL - FRAME_HEIGHT_VISIBLE_PAL) / 2 */
    constexpr static unsigned VISIBLE_Y_END      = VISIBLE_Y_START + VISIBLE_HEIGHT;

    constexpr static unsigned WIDTH              = VISIBLE_WIDTH;
    constexpr static unsigned HEIGHT             = VISIBLE_HEIGHT;

    constexpr static unsigned DISPLAY_WIDTH      = 320;
    constexpr static unsigned DISPLAY_HEIGHT     = 200;

    constexpr static unsigned DISPLAY_Y_START    = 48;
    constexpr static unsigned DISPLAY_Y_END      = DISPLAY_Y_START + DISPLAY_HEIGHT;

    constexpr static unsigned DISPLAY_X_START    = 42;   /* (VISIBLE_WIDTH - DISPLAY_WIDTH) / 2 */
    constexpr static unsigned DISPLAY_X_END      = DISPLAY_X_START + DISPLAY_WIDTH;

    constexpr static unsigned UBORDER_Y_START    = VISIBLE_Y_START;
    constexpr static unsigned UBORDER_Y_END      = 51;

    constexpr static unsigned BBORDER_Y_START    = 251;
    constexpr static unsigned BBORDER_Y_END      = VISIBLE_Y_END;

    constexpr static unsigned LBORDER_X_END      = DISPLAY_X_START;
    constexpr static unsigned RBORDER_X_START    = DISPLAY_X_END;

    constexpr static unsigned PIXELS_PER_CYCLE   = 8;
    constexpr static unsigned SCANLINE_CYCLES    = FRAME_WIDTH / PIXELS_PER_CYCLE;
    constexpr static unsigned FRAME_CYCLES       = FRAME_HEIGHT * SCANLINE_CYCLES;

    constexpr static unsigned MIB_WIDTH          = 24;
    constexpr static unsigned MIB_HEIGHT         = 21;

    constexpr static unsigned MIB_Y_START        = 6;

    constexpr static unsigned MIB_X_COORD_OFFSET = 18;
    constexpr static unsigned MIB_Y_COORD_OFFSET = 1;

    constexpr static unsigned MIB_POINTER_OFFSET = 1016;

    enum Color {
        BLACK                  = 0,
        WHITE                  = 1,
        RED                    = 2,
        CYAN                   = 3,
        PURPLE                 = 4,
        GREEN                  = 5,
        BLUE                   = 6,
        YELLOW                 = 7,
        ORANGE                 = 8,
        BROWN                  = 9,
        LIGHT_RED              = 10,
        DARK_GREY              = 11,
        GREY                   = 12,
        LIGHT_GREEN            = 13,
        LIGHT_BLUE             = 14,
        LIGHT_GREY             = 15,

        MASK                   = 15
    };

    enum Registers {
        REG_MIB_0_X            = 0,     /* 00 */
        REG_MIB_0_Y            = 1,     /* 01 */
        REG_MIB_1_X            = 2,     /* 02 */
        REG_MIB_1_Y            = 3,     /* 03 */
        REG_MIB_2_X            = 4,     /* 04 */
        REG_MIB_2_Y            = 5,     /* 05 */
        REG_MIB_3_X            = 6,     /* 06 */
        REG_MIB_3_Y            = 7,     /* 07 */
        REG_MIB_4_X            = 8,     /* 08 */
        REG_MIB_4_Y            = 9,     /* 09 */
        REG_MIB_5_X            = 10,    /* 0A */
        REG_MIB_5_Y            = 11,    /* 0B */
        REG_MIB_6_X            = 12,    /* 0C */
        REG_MIB_6_Y            = 13,    /* 0D */
        REG_MIB_7_X            = 14,    /* 0E */
        REG_MIB_7_Y            = 15,    /* 0F */
        REG_MIBS_MSB_X         = 16,    /* 10 */

        REG_CONTROL_1          = 17,    /* 11 */
        REG_RASTER_COUNTER     = 18,    /* 12 */
        REG_LIGHT_PEN_X        = 19,    /* 13 */
        REG_LIGHT_PEN_Y        = 20,    /* 14 */
        REG_MIB_ENABLE         = 21,    /* 15 */
        REG_CONTROL_2          = 22,    /* 16 */
        REG_MIB_Y_EXPANSION    = 23,    /* 17 */
        REG_MEMORY_POINTERS    = 24,    /* 18 */
        REG_INTERRUPT          = 25,    /* 19 */
        REG_INTERRUPT_ENABLE   = 26,    /* 1A */
        REG_MIB_DATA_PRI       = 27,    /* 1B */
        REG_MIB_MULTICOLOR_SEL = 28,    /* 1C */
        REG_MIB_X_EXPANSION    = 29,    /* 1D */
        REG_MIB_MIB_COLLISION  = 30,    /* 1E */
        REG_MIB_DATA_COLLISION = 31,    /* 1F */

        REG_BORDER_COLOR       = 32,    /* 20 */
        REG_BACKGROUND_COLOR_0 = 33,    /* 21 */
        REG_BACKGROUND_COLOR_1 = 34,    /* 22 */
        REG_BACKGROUND_COLOR_2 = 35,    /* 23 */
        REG_BACKGROUND_COLOR_3 = 36,    /* 24 */
        REG_MIB_MULTICOLOR_0   = 37,    /* 25 */
        REG_MIB_MULTICOLOR_1   = 38,    /* 26 */
        REG_MIB_0_COLOR        = 39,    /* 27 */
        REG_MIB_1_COLOR        = 40,    /* 28 */
        REG_MIB_2_COLOR        = 41,    /* 29 */
        REG_MIB_3_COLOR        = 42,    /* 2A */
        REG_MIB_4_COLOR        = 43,    /* 2B */
        REG_MIB_5_COLOR        = 44,    /* 2C */
        REG_MIB_6_COLOR        = 45,    /* 2D */
        REG_MIB_7_COLOR        = 46,    /* 2E */

        REGMAX
    };

    constexpr static uint8_t REG_CONTROL_1_RC8         = 0x80;  /* 9th bit of raster line                           */
    constexpr static uint8_t REG_CONTROL_1_ECM         = 0x40;  /* Extended color character mode                    */
    constexpr static uint8_t REG_CONTROL_1_BMM         = 0x20;  /* Bitmap video mode                                */
    constexpr static uint8_t REG_CONTROL_1_DEN         = 0x10;  /* Display Enabled                                  */

    constexpr static uint8_t REG_CONTROL_1_RSEL        = 0x08;  /* Number of rows (0 = 24 rows, 1 = 25 rows)        */
    constexpr static uint8_t REG_CONTROL_1_YSCROLL     = 0x07;  /* Vertical scroll position                         */

    constexpr static uint8_t REG_CONTROL_2_RES         = 0x20;  /* Reset                                            */
    constexpr static uint8_t REG_CONTROL_2_MCM         = 0x10;  /* Multicolor mode                                  */
    constexpr static uint8_t REG_CONTROL_2_CSEL        = 0x08;  /* Number of columns (0 = 38 cols, 1 = 40 cols)     */
    constexpr static uint8_t REG_CONTROL_2_XSCROLL     = 0x07;  /* Horizontal scroll position                       */

    constexpr static uint8_t REG_INTERRUPT_IRQ         = 0x80;  /* Status of the output /IRQ pin                    */
    constexpr static uint8_t REG_INTERRUPT_LP          = 0x08;  /* First negative transition of Light Pen x frame   */
    constexpr static uint8_t REG_INTERRUPT_MMC         = 0x04;  /* MIB-MIB collision interrupt                      */
    constexpr static uint8_t REG_INTERRUPT_MDC         = 0x02;  /* MIB-DATA collision interrupt                     */
    constexpr static uint8_t REG_INTERRUPT_RST         = 0x01;  /* Raster counter interrupt                         */

    constexpr static uint8_t REG_INTERRUPT_MASK        = REG_INTERRUPT_LP  | REG_INTERRUPT_MMC |
                                                         REG_INTERRUPT_MDC | REG_INTERRUPT_RST;

    constexpr static uint8_t REG_MEMORY_POINTERS_VM13  = 0x80;  /* Bit 13 of Video Matrix address                   */
    constexpr static uint8_t REG_MEMORY_POINTERS_VM12  = 0x40;  /* Bit 12 of Video Matrix address                   */
    constexpr static uint8_t REG_MEMORY_POINTERS_VM11  = 0x20;  /* Bit 11 of Video Matrix address                   */
    constexpr static uint8_t REG_MEMORY_POINTERS_VM10  = 0x10;  /* Bit 10 of Video Matrix address                   */
    constexpr static uint8_t REG_MEMORY_POINTERS_CB13  = 0x08;  /* Bit 13 of Character Base address                 */
    constexpr static uint8_t REG_MEMORY_POINTERS_CB12  = 0x04;  /* Bit 12 of Character Base address                 */
    constexpr static uint8_t REG_MEMORY_POINTERS_CB11  = 0x02;  /* Bit 11 of Character Base address                 */

    constexpr static uint8_t REG_MEMORY_POINTERS_CHAR  = REG_MEMORY_POINTERS_CB13 | REG_MEMORY_POINTERS_CB12 |
                                                         REG_MEMORY_POINTERS_CB11;

    constexpr static uint8_t REG_MEMORY_POINTERS_VIDEO = REG_MEMORY_POINTERS_VM13 | REG_MEMORY_POINTERS_VM12 |
                                                         REG_MEMORY_POINTERS_VM11 | REG_MEMORY_POINTERS_VM10;

    constexpr static unsigned CHARMODE_COLUMNS         = 40;
    constexpr static unsigned CHARMODE_ROWS            = 25;

    constexpr static uint8_t SCROLL_Y_MASK             = 0x07;
    constexpr static uint8_t SCROLL_X_MASK             = 0x07;

    constexpr static const unsigned MIB_MAX_X_SIZE     = 3 * 8 * 2; /* 3 bytes x MIB + expansion */

    /**
     * Initialise this video controller.
     * @param label  Label assigned to this device;
     * @param mmap   Address space for this device (memory area that can
     *               be seen by this video controller through its address lines);
     * @param vcolor Colour RAM (1K).
     * @see render_line(const std::function<void(unsigned, const ui::Scanline&)>&);
     */
    Mos6569(const std::string& label, const sptr_t<ASpace>& mmap, const devptr_t& vcolor);

    virtual ~Mos6569() {
    }

    /**
     * Set the render line callback.
     * The render line callback must send the video output to the user interface.
     * @param rl The render line callback.
     */
    void render_line(const std::function<void(unsigned, const ui::Scanline&)>& rl);

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
    void palette(const RgbaTable& plt);

    /**
     * Set the vsync callback.
     * The vsync callback is called by this controller each time a full screen frame is rendered.
     * @param cb V-Sync callback.
     */
    void vsync(const std::function<void(unsigned)>& cb);

    /**
     * Set the IRQ pin callback.
     * The IRQ pin callback is called when the status of the IRQ output pin of this device is changed.
     * @param irq_out IRQ output pin callback.
     */
    void irq(const OutputPinCb& irq_out);

    /**
     * Set the BA (Bus Available) pin callback.
     * The BA pin callback is called when the value of the BA output pin of this device is changed.
     * @param ba_out BA pin callback.
     */
    void ba(const OutputPinCb& ba_out);

    /**
     * LP edge triggered input.
     */
    void trigger_lp();

    /**
     * @see Device::size()
     */
    size_t size() const override;

    /**
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override;

private:
    /**
     * Clock tick interface (callback).
     * This method must be called by the system clock.
     * @param clk A reference to the caller clock.
     * @return The number of clock cycles that must pass until the clock is allowed to call this method again.
     * @see Clockable
     */
    size_t tick(const Clock& clk) override;

    /**
     * Paint a cycle of the display area in the current scanline.
     * @param x Starting X coordinate (relative to the display area) to paint.
     * @see PIXELS_PER_CYCLE
     */
    void paint_display_cycle(unsigned x);

    /**
     * Paint the borders (if they are enabled).
     */
    void paint_borders();

    /**
     * Render the current scanline.
     * @param line Line number assigned to the curent scanline.
     * This method calls the render_line callback in order to render the specified line.
     * @see render_line(const std::function<void(unsigned, const ui::Scanline &)> &rl)
     */
    void render_line(unsigned line);

    /**
     * Set/Unset an IRQ condition.
     * The IRQ bit in the REG_INTERRUPT register is properly
     * set and the IRQ output callback is called.
     * @param active true if the IRQ pin must be activated; false otherwise.
     */
    void irq_out(bool active);

    /**
     * Set the BA output pin callback.
     * @param active true to activate; false to deactivate.
     */
    void ba_out(bool active);

    /**
     * Get the base address of a character data.
     * @param ch Character code.
     * @return The base address of the specified character data.
     */
    addr_t char_base(uint8_t ch) const;

    /**
     * Get the base address of a MIB data.
     * @param mib MIB (between 0 and 7).
     * @return The base address of the specified MIB data.
     */
    addr_t mib_base(unsigned mib) const;

    /**
     * Get the X coordinate of a MIB.
     * @param mib MIB (bettwen 0 and 7).
     * @return The X coordinate of the specified MIB.
     */
    unsigned mib_x(unsigned mib) const;

    /**
     * Get the Y coordinate of a MIB.
     * @param mib MIB (bettwen 0 and 7).
     * @return The Y coordinate of the specified MIB.
     */
    unsigned mib_y(unsigned mib) const;

    /**
     * Retrieve a sprite configuration in a specified scanline.
     * @param line Raster line;
     * @param mib  Sprite number (between 0 and 7).
     * @return A tuple containing three elements:
     *   - The sprite Y position;
     *   - The sprite vertical size;
     *   - A flag indicating whether the sprite is vertically expanded or not.
     * If the sprite is not enabled or not present in the specified raster line the returned Y position is -1.
     */
    std::tuple<unsigned, unsigned, bool> mib_visibility_y(unsigned line, uint8_t mib);

    /**
     * Detect the presense of a sprite.
     * @param line Raster line;
     * @param mib  Sprite number (between 0 and 7).
     * @return true if the sprite is enabled and present in the specified line; false otherwise.
     * @see mib_visibility_y(unsigned line, uint8_t mib);
     */
    bool is_mib_visible(unsigned line, uint8_t mib);

    /**
     * Get the colour code from the colour RAM for a specific 40x25 display position.
     * @param x Horizontal coordinate (between 0 and 39);
     * @param y Vertical coordinate (between 0 and 24).
     * @return The colour code.
     * @see _vcolor
     */
    Color video_color_code(unsigned x, unsigned y) const;

    /**
     * Paint a segment in the current scanline.
     * @param start Starting horizontal position;
     * @param width Segment width in pixels (if 0, the entire scanline from the starting position is painted);
     * @param color RGBA colour to use.
     */
    void paint(unsigned start, unsigned width, const Rgba& color);

    /**
     * Paint a byte bitmap (1 bit per pixel) in the current scanline.
     * Bits set to 1 are painted with the foreground colour,
     * bits set to 0 are painted with the background colour.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap;
     * @param colors Background and foreground colours.
     */
    void paint_byte(unsigned start, uint8_t bitmap, const Rgba4& colors);

    /**
     * Paint a multicolor byte bitmap (2 bits per pixel) in the current scanline.
     * 4 double sized pixels are painted (4 pairs of 2 pixels with the same colour).
     * The colour codes of each of the 4 pixels are encoded in two consecutive bits.
     * The specified byte represents a bitmap containing 4 pairs of pixels:
     *   - 00: Two pixels painted using colors[0];
     *   - 01: Two pixels painted using colors[1];
     *   - 10: Two pixels painted using colors[2];
     *   - 11: Two pixels painted using colors[3].
     * The pixel size is doubled but the visual effect is a reduction of the horizontal resolution from 320 to 160.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap containing the 4 pixels to paint;
     * @param colors 4 colours.
     */
    void paint_mcm_byte(unsigned start, uint8_t bitmap, const Rgba4& colors);

    /**
     * Paint a character mode display line.
     * @param line The raster line to paint (relative to the start of the display area);
     * @param x    X Coordinate of the character (relative to the display area).
     */
    void paint_char_mode(unsigned line, unsigned x);

    /**
     * Paint a bitmap mode display line.
     * @param line The raster line to paint (relative to the start of the display area);
     * @param x    X Coordinate of the character (relative to the display area).
     */
    void paint_bitmap_mode(unsigned line, unsigned x);

    /**
     * Paint a sprite bitmap line.
     * @param start  Starting horizontal position;
     * @param bitmap The sprite bitmap (sprite line data);
     * @param colors Sprite background and foreground colours;
     * @param expand true if the bitmap contains an horizontally expanded sprite; false otherwise.
     */
    void paint_sprite_line(unsigned start, uint64_t bitmap, const Rgba4& colors, bool expand);

    /**
     * Paint a multicolor sprite bitmap line.
     * @param start  Starting horizontal position;
     * @param bitmap The sprite bitmap (sprite line data);
     * @param colors 4 colours for the spirte;
     * @param expand true if the bitmap contains an horizontally expanded sprite; false otherwise.
     */
    void paint_sprite_line_mcm(unsigned start, uint64_t bitmap, const Rgba4& colors, bool expand);

    /**
     * Paint a sprite line.
     * The sprite line is painted only if it is enabled and visible in the specified raster line.
     * @param line Raster line to paint;
     * @param mib  Sprite number (between 0 and 7).
     * @see mib_bitmap()
     * @see paint_sprite_line()
     * @see paint_sprite_line_mcm()
     */
    void paint_sprite(unsigned line, uint8_t mib);

    /**
     * Paint all the sprite lines.
     * Only the lines of the visible sprites are painted in the current scan line,
     * In case of sprite superpositions the fixed sprite-sprite priority is followed.
     * @see paint_sprite(unsigned, uint8_t)
     */
    void paint_sprites();

    /**
     * Reset the collision background data array.
     */
    void reset_collision_data();

    /**
     * Reset the collision sprite bitmap data array.
     */
    void reset_collision_mib();

    /**
     * Update the collision data for this scanline.
     * The collision data is upated each time a character data or bitmap data is painted in the scanline.
     * @param start  Starting horizontal coordinate;
     * @param bitmap Byte bitmap painted.
     */
    void update_collision_data(unsigned start, uint8_t bitmap);

    /**
     * Update the collision data for this scanline in multicolor mode.
     * The collision data is upated each time a character data or bitmap data is painted in the scanline.
     * @param start  Starting horizontal coordinate;
     * @param bitmap Byte bitmap encoded with 4 color codes.
     */
    void update_collision_data_mcm(unsigned start, uint8_t bitmap);

    /**
     * Generate the shapes of a sprite line and detect collisions with the background image.
     * Two sprite bitmap lines are generated:
     *   - The complete sprite bitmap line;
     *   - The visible sprite bitmap line (not obscured by the background image).
     * @param start    Starting horizontal position of the sprite;
     * @param byte1    Sprite line first data byte;
     * @param byte2    Sprite line second data byte;
     * @param byte3    Sprite line third data byte;
     * @param expand   true if the sprite must be expanded horizontally; false otherwise.
     * @param mcm      true if it is a multi-color sprite; false otherwise.
     * @param data_pri true if a background image has the visibility priority over the sprite; false otherwise.
     * @return A tuple containing:
     *  - The sprite-background (MIB-DATA) collision flag;
     *  - The sprite bitmap line;
     *  - The visible sprite bitmap line.
     */
    std::tuple<bool, uint64_t, uint64_t>
    mib_bitmap(unsigned start, uint8_t byte1, uint8_t byte2, uint8_t byte3, bool expand, bool mcm, bool data_pri);

    /**
     * Update a sprite collision mask and detect collisions with other sprites.
     * @param mib    The sprite number (between 0 and 7);
     * @param start  Sprite horizontal position;
     * @param mcm    true if the sprite is multicolor; false otherwise;
     * @param bitmap The complete bitmap line of the sprite.
     * @return The collided sprite bit if there are no previous collisions and a new collision is detected; otherwise 0.
     */
    uint8_t update_collision_mib(uint8_t mib, unsigned start, bool mcm, uint64_t bitmap);

    std::function<void(unsigned, const ui::Scanline&)> _render_line{};  /* Line renderer callback                   */
    std::function<void(size_t)> _vsync{};                       /* Vertical sync callback                           */

    OutputPinCb             _irq_out{};                         /* IRQ output pin callback                          */
    OutputPinCb             _ba_out{};                          /* BA output pin callback                           */

    std::shared_ptr<ASpace> _mmap{};                            /* Address space mappings                           */
    devptr_t                _vcolor{};                          /* Video colour RAM                                 */
    RgbaTable               _palette{};                         /* Colour palette                                   */
    ui::Scanline            _scanline{};                        /* Pixel data for the current visible raster line   */

    uint8_t                 _mib_enable{};                      /* MIB enable (visible) flags                       */
    std::array<uint16_t, 8> _mib_coord_x{};                     /* MIB X coordinates                                */
    std::array<uint8_t, 8>  _mib_coord_y{};                     /* MIB Y coordinates                                */
    uint8_t                 _mib_expand_x{};                    /* MIB X expansion flags                            */
    uint8_t                 _mib_expand_y{};                    /* MIB Y expansion flags                            */
    uint8_t                 _mib_data_priority{};               /* MIB-DATA Priority flags                          */
    uint8_t                 _mib_multicolor_sel{};              /* MIB multicolor selection flags                   */
    uint8_t                 _mib_mib_collision{};               /* MIB-MIB collision flags                          */
    uint8_t                 _mib_data_collision{};              /* MIB-DATA collision flags                         */
    std::array<Color, 8>    _mib_color{};                       /* MIB colours                                      */
    std::array<Color, 2>    _mib_multicolor{};                  /* MIB multicolour flags                            */

    uint16_t                _raster_counter{};                  /* Current raster line                              */
    uint16_t                _stored_raster{};                   /* Interrupt raster line                            */

    bool                    _den{};                             /* Display Enabled flag                             */
    bool                    _mcm_mode{};                        /* Multicolor mode flag                             */
    bool                    _ecm_mode{};                        /* Extended color mode flag                         */
    bool                    _bmm_mode{};                        /* Bitmap mode flag                                 */
    bool                    _25_rows{};                         /* 25 rows flag                                     */
    bool                    _40_columns{};                      /* 40 columns flag                                  */

    uint8_t                 _scroll_x{};                        /* Scroll X position                                */
    uint8_t                 _scroll_y{};                        /* Scroll Y position                                */

    uint8_t                 _light_pen_x{};                     /* Latched light pen X position                     */
    uint8_t                 _light_pen_y{};                     /* Latched light pen Y position                     */
    bool                    _light_pen_latched{};               /* Light pen latched flag                           */

    addr_t                  _char_base{};                       /* Base address of character memory                 */
    addr_t                  _video_matrix{};                    /* Base address of video RAM                        */
    addr_t                  _bitmap_base{};                     /* Base address of Bitmap mode video RAM            */

    Color                   _border_color{};                    /* Border colour                                    */
    std::array<Color, 4>    _background_color{};                /* Background colours                               */

    uint8_t                 _irq_status{};                      /* IRQ status register                              */
    uint8_t                 _irq_enable{};                      /* IRQ enable mask                                  */

    bool                    _bl_den{};                          /* _den flag checked for a badline                  */
    bool                    _bad_line{};                        /* Current raster line is a bad line                */

    bool                    _main_border{};                     /* Main border enabled flag                         */
    bool                    _vertical_border{};                 /* Vertical borders enabled flag                    */

    bool                    _idle_mode{};                       /* Idle vs Display mode                             */
    unsigned                _video_counter{};                   /* Higher part of the current display area line     */
    uint8_t                 _row_counter{};                     /* Lower 3 bits of the current display area line    */
    unsigned                _uborder_end{UBORDER_Y_END};        /* Last line of upper border + 1                    */
    unsigned                _bborder_start{BBORDER_Y_START};    /* First line of bottom border                      */
    unsigned                _lborder_end{LBORDER_X_END};        /* Last line of left border + 1                     */
    unsigned                _rborder_start{RBORDER_X_START};    /* First line of right border                       */

    unsigned                _cycle{};                           /* Current horizontal cycle                         */
    bool                    _vblank{};                          /* Vertical blanking flag                           */

    /*
     * Bitmap scanline used to detect collisions between the background (data) and the sprites.
     */
    std::array<uint8_t, utils::align(static_cast<uint64_t>(utils::ceil(WIDTH / 8.0f) + 8))> _collision_data{};

    /*
     * Bitmap masks used to detect collisions between sprites.
     * The elements of the array contain the bitmap mask for each sprite in the current scanline.
     */
    std::array<uint64_t, 8> _mib_bitmaps{};

    static RgbaTable builtin_palette;                           /* Default colour palette                           */
};

}
