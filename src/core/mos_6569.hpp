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
#include <string_view>
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
namespace mos {

/**
 * MOS-6569 (VIC-II PAL) Video Controller.
 +
 * ### Screen resolution:
 *
 *     |<------------------------------------ 504 ---------------------------------->|
 *
 *                |<------------------------- 403 ------------------------->|
 *
 *     |<-- 88 -->|<- 41 ->|<---------------- 320 ---------------->|<- 42 ->|<- 13 ->|
 *
 *     +-----------------------------------------------------------------------------+     -+-      -+-
 *     |                                    V-BLANK                                  |      | 16     |
 *     +----------+---------------------------------------------------------+--------|     -+-       |       -+-
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |                      UPPER BORDER                       |        |      | 35     |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |        +---------------------------------------+        |        |     -+-       |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     | HORIZ    | LEFT   |             DISPLAY AREA              | RIGHT  | HORIZ  |      |        |        |
 *     |    BLANK | BORDER |                                       | BORDER |  BLANK |      |        |        |
 *     |          |        |                                       |        |        |       > 200    > 312    > 284
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |        +---------------------------------------+        |        |     -+-       |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     |          |                      BOTTOM BORDER                      |        |      | 49     |        |
 *     |          |        |                                       |        |        |      |        |        |
 *     +-----------------------------------------------------------------------------+     -+-       |       -+-
 *     |                                    V-BLANK                                  |      | 12     |
 *     +-----------------------------------------------------------------------------+     -+-      -+-
 *
 * ### Coordinate system:
 *
 * Using visible coordinates as reference:
 * - (Sprite) X coordinates start at horizontal pixel 18;
 * - Left border starts at horizontal pixel 0;
 * - Upper/bottom borders start at horizontal pixel 42;
 * - Display starts at horizontal pixel 42 and vertical pixel 36.
 * - upper border starts at vertical pixel 0;
 *
 * ### Clock frequency and video signals:
 *
 *                           +----------+---------------------------+----------+ Front
 *               Back Porch  | LBorder             Display            RBorder  | Porch
 *     --+     +-------------+----------+---------------------------+----------+---+
 *       |     |                                                                   |
 *       +-----+                                                                   +--
 *        HSync
 *                           |<-- 41 -->|<---------- 320 ---------->|<-- 42 -->|
 *
 *                           |<------------- 403 visible pixels -------------->|
 *
 *       |<---- 11.2us ----->|<------------------- 51.2us -------------------->|<->|
 *                                                                              1.6us
 *       |<-------------------------------- 64us --------------------------------->|
 *
 *
 *       \__________________/\_________________________________________________/\_/
 *                |                                  |                           |
 *         ~11cy * 8 = 88px                  ~50.4cy * 8 = 403px         ~1.6cy * 8 = 13 px
 *
 *       \________________________________________________________________________/
 *                                           |
 *                                  Total 63cy * 8 = 504px
 *
 * - 8 pixels of a scanline refereshed in 1 clock cycle;
 * - There are 63 (504 / 8) clock cycles per scanline (including horizontal blanking period);
 *   + 100 pixels (12.6 cycles, 12.8us) of blanking period.
 *   + 403 visible pixels:
 *     + Left border: 41 pixels
 *     + Display area: 320 pixels
 *     + Right border: 42 pixels
 *
 * ### Tricks & Bugs:
 *
 * Some bugs of the original chip are implemented others are not, and others
 * are partially implemented.
 * - Hyperscreen:    Implemented but colours might not be the expected ones
 * - FLD:            Implemented
 * - FLI:            Partially implemented (sprites not properly rendered)
 * - Sprite stretch: Implemented
 * - Linecrunch:     Not tested
 * - DMA delay:      Not tested
 * - Sprite sync:    Not implemented
 *
 * These bugs are not implemented (or they are but only partially) because
 * the graphics (sprites and borders) are not rendered as the chip does.
 * These bugs are exploited mostly on modern (post 2000) intros and they
 * are not present in the majority of the C64 games around.
 *
 * @see C64 Programmer's Reference Guide, Appendix N
 * @see mos_6567_vic_ii_preliminary.pdf
 * @see https://www.cebix.net/VIC-Article.txt
 */
class Mos6569 : public Device, public Clockable {
public:
    constexpr static const char* TYPE = "MOS6569";

    constexpr static const unsigned FRAME_WIDTH              = 504;
    constexpr static const unsigned FRAME_HEIGHT             = 312;
    constexpr static const unsigned VISIBLE_WIDTH            = 403;
    constexpr static const unsigned VISIBLE_HEIGHT           = 284;
    constexpr static const unsigned DISPLAY_WIDTH            = 320;
    constexpr static const unsigned DISPLAY_HEIGHT           = 200;
    constexpr static const unsigned WIDTH                    = VISIBLE_WIDTH;
    constexpr static const unsigned HEIGHT                   = VISIBLE_HEIGHT;
    constexpr static const unsigned COLUMNS                  = 40;
    constexpr static const unsigned ROWS                     = 25;

    constexpr static const unsigned VISIBLE_Y_START          = 16;
    constexpr static const unsigned VISIBLE_Y_END            = VISIBLE_Y_START + VISIBLE_HEIGHT;
    constexpr static const unsigned DISPLAY_Y_START          = 48;
    constexpr static const unsigned DISPLAY_Y_END            = DISPLAY_Y_START + DISPLAY_HEIGHT;
    constexpr static const unsigned DISPLAY_X_START          = 42;
    constexpr static const unsigned DISPLAY_X_END            = DISPLAY_X_START + DISPLAY_WIDTH;
    constexpr static const unsigned UBORDER_Y_END            = 51;
    constexpr static const unsigned BBORDER_Y_START          = 251;
    constexpr static const unsigned LBORDER_X_END            = DISPLAY_X_START;
    constexpr static const unsigned RBORDER_X_START          = DISPLAY_X_END;

    constexpr static const unsigned PIXELS_PER_CYCLE         = 8;
    constexpr static const unsigned SCANLINE_CYCLES          = FRAME_WIDTH / PIXELS_PER_CYCLE;

    constexpr static const uint8_t REG_CONTROL_1_RC8         = 0x80;     /* 9th bit of raster line           */
    constexpr static const uint8_t REG_CONTROL_1_ECM         = 0x40;     /* Extended colour character mode   */
    constexpr static const uint8_t REG_CONTROL_1_BMM         = 0x20;     /* Bitmap video mode                */
    constexpr static const uint8_t REG_CONTROL_1_DEN         = 0x10;     /* Display Enabled                  */
    constexpr static const uint8_t REG_CONTROL_1_RSEL        = 0x08;     /* 0: 24 rows, 1: 25 rows           */
    constexpr static const uint8_t REG_CONTROL_1_YSCROLL     = 0x07;     /* Vertical scroll position         */

    constexpr static const uint8_t REG_CONTROL_2_RES         = 0x20;     /* Reset                            */
    constexpr static const uint8_t REG_CONTROL_2_MCM         = 0x10;     /* Multicolour mode                 */
    constexpr static const uint8_t REG_CONTROL_2_CSEL        = 0x08;     /* 0: 38 columns, 1: 40 columns     */
    constexpr static const uint8_t REG_CONTROL_2_XSCROLL     = 0x07;     /* Horizontal scroll position       */

    constexpr static const uint8_t REG_INTERRUPT_IRQ         = 0x80;     /* Status of the /IRQ output pin    */
    constexpr static const uint8_t REG_INTERRUPT_LP          = 0x08;     /* Light Pen interrupt              */
    constexpr static const uint8_t REG_INTERRUPT_MMC         = 0x04;     /* MOB-MOB collision interrupt      */
    constexpr static const uint8_t REG_INTERRUPT_MDC         = 0x02;     /* MOB-DATA collision interrupt     */
    constexpr static const uint8_t REG_INTERRUPT_RST         = 0x01;     /* Raster counter interrupt         */

    constexpr static const uint8_t REG_INTERRUPT_MASK        = REG_INTERRUPT_LP  | REG_INTERRUPT_MMC |
                                                               REG_INTERRUPT_MDC | REG_INTERRUPT_RST;

    constexpr static const uint8_t REG_MEMORY_POINTERS_VM13  = 0x80;     /* Bit 13 of Video Matrix address   */
    constexpr static const uint8_t REG_MEMORY_POINTERS_VM12  = 0x40;     /* Bit 12 of Video Matrix address   */
    constexpr static const uint8_t REG_MEMORY_POINTERS_VM11  = 0x20;     /* Bit 11 of Video Matrix address   */
    constexpr static const uint8_t REG_MEMORY_POINTERS_VM10  = 0x10;     /* Bit 10 of Video Matrix address   */
    constexpr static const uint8_t REG_MEMORY_POINTERS_CB13  = 0x08;     /* Bit 13 of Character Base address */
    constexpr static const uint8_t REG_MEMORY_POINTERS_CB12  = 0x04;     /* Bit 12 of Character Base address */
    constexpr static const uint8_t REG_MEMORY_POINTERS_CB11  = 0x02;     /* Bit 11 of Character Base address */

    constexpr static const uint8_t REG_MEMORY_POINTERS_CHAR  = REG_MEMORY_POINTERS_CB13 |
                                                               REG_MEMORY_POINTERS_CB12 |
                                                               REG_MEMORY_POINTERS_CB11;

    constexpr static const uint8_t REG_MEMORY_POINTERS_VIDEO = REG_MEMORY_POINTERS_VM13 |
                                                               REG_MEMORY_POINTERS_VM12 |
                                                               REG_MEMORY_POINTERS_VM11 |
                                                               REG_MEMORY_POINTERS_VM10;

    constexpr static const uint8_t SCROLL_Y_MASK             = 0x07;
    constexpr static const uint8_t SCROLL_X_MASK             = 0x07;

    constexpr static const uint8_t MOBS                      = 8;
    constexpr static const uint16_t MOB_X_COORD_OFFSET       = 18;
    constexpr static const uint16_t MOB_Y_COORD_OFFSET       = 1;
    constexpr static const unsigned MOB_WIDTH                = 3;       /* MOB width in bytes   */
    constexpr static const unsigned MOB_HEIGHT               = 21;      /* MOB height in lines  */
    constexpr static const unsigned MOB_SIZE                 = MOB_HEIGHT * MOB_WIDTH;
    constexpr static const unsigned MOB_MAX_X_SIZE           = MOB_WIDTH * MOBS * 2 /* expansion */;

    using Renderer = std::function<void(unsigned, const ui::Scanline&)>;

    /**
     * Colour codes.
     */
    enum Color {
        BLACK       = 0,
        WHITE       = 1,
        RED         = 2,
        CYAN        = 3,
        PURPLE      = 4,
        GREEN       = 5,
        BLUE        = 6,
        YELLOW      = 7,
        ORANGE      = 8,
        BROWN       = 9,
        LIGHT_RED   = 10,
        DARK_GREY   = 11,
        GREY        = 12,
        LIGHT_GREEN = 13,
        LIGHT_BLUE  = 14,
        LIGHT_GREY  = 15,

        MASK        = 15
    };

    /**
     * Video modes.
     * Encoded as: ecm.bmm.mcm
     * - ecm: Text extended colour mode;
     * - bmm: Bitmap mode;
     * - mcm: Multicolor mode.
     * @see video_mode(bool, bool, bool)
     */
    enum class VideoMode {
        TextStandard            = 0b000,
        TextMulticolor          = 0b001,
        TextExtendedColor       = 0b100,
        BitmapStandard          = 0b010,
        BitmapMulticolor        = 0b011,
        TextInvalid             = 0b101,
        BitmapInvalid           = 0b110,
        BitmapMulticolorInvalid = 0b111
    };

    /**
     * Registers.
     */
    enum Registers {
        REG_MOB_0_X             = 0,    /* 00 */
        REG_MOB_0_Y             = 1,    /* 01 */
        REG_MOB_1_X             = 2,    /* 02 */
        REG_MOB_1_Y             = 3,    /* 03 */
        REG_MOB_2_X             = 4,    /* 04 */
        REG_MOB_2_Y             = 5,    /* 05 */
        REG_MOB_3_X             = 6,    /* 06 */
        REG_MOB_3_Y             = 7,    /* 07 */
        REG_MOB_4_X             = 8,    /* 08 */
        REG_MOB_4_Y             = 9,    /* 09 */
        REG_MOB_5_X             = 10,   /* 0A */
        REG_MOB_5_Y             = 11,   /* 0B */
        REG_MOB_6_X             = 12,   /* 0C */
        REG_MOB_6_Y             = 13,   /* 0D */
        REG_MOB_7_X             = 14,   /* 0E */
        REG_MOB_7_Y             = 15,   /* 0F */
        REG_MOBS_MSB_X          = 16,   /* 10 */

        REG_CONTROL_1           = 17,   /* 11 */
        REG_RASTER_COUNTER      = 18,   /* 12 */
        REG_LIGHT_PEN_X         = 19,   /* 13 */
        REG_LIGHT_PEN_Y         = 20,   /* 14 */
        REG_MOB_ENABLE          = 21,   /* 15 */
        REG_CONTROL_2           = 22,   /* 16 */
        REG_MOB_Y_EXPANSION     = 23,   /* 17 */
        REG_MEMORY_POINTERS     = 24,   /* 18 */
        REG_INTERRUPT           = 25,   /* 19 */
        REG_INTERRUPT_ENABLE    = 26,   /* 1A */
        REG_MOB_DATA_PRI        = 27,   /* 1B */
        REG_MOB_MULTICOLOR_SEL  = 28,   /* 1C */
        REG_MOB_X_EXPANSION     = 29,   /* 1D */
        REG_MOB_MOB_COLLISION   = 30,   /* 1E */
        REG_MOB_DATA_COLLISION  = 31,   /* 1F */

        REG_BORDER_COLOR        = 32,   /* 20 */
        REG_BACKGROUND_COLOR_0  = 33,   /* 21 */
        REG_BACKGROUND_COLOR_1  = 34,   /* 22 */
        REG_BACKGROUND_COLOR_2  = 35,   /* 23 */
        REG_BACKGROUND_COLOR_3  = 36,   /* 24 */
        REG_MOB_MULTICOLOR_0    = 37,   /* 25 */
        REG_MOB_MULTICOLOR_1    = 38,   /* 26 */
        REG_MOB_0_COLOR         = 39,   /* 27 */
        REG_MOB_1_COLOR         = 40,   /* 28 */
        REG_MOB_2_COLOR         = 41,   /* 29 */
        REG_MOB_3_COLOR         = 42,   /* 2A */
        REG_MOB_4_COLOR         = 43,   /* 2B */
        REG_MOB_5_COLOR         = 44,   /* 2C */
        REG_MOB_6_COLOR         = 45,   /* 2D */
        REG_MOB_7_COLOR         = 46,   /* 2E */

        REGMAX
    };

    /**
     * Colour and character data.
     * This strucutre contains data read during c-access.
     * @see c_access()
     */
    struct CData {
        uint8_t ch;
        bool mcm;
        std::function<Rgba4()> colors;
    };

    /**
     * Sprite line data.
     * This strucutre contains data read during p-access and s-access.
     * @see ps_access(uint8_t)
     */
    struct SData {
        bool    dma{};          /* Sprite scheduled for processing  */
        bool    visible;        /* Sprite is enabled and visible    */
        bool    exp_y;          /* Vertical expansion flag          */
        addr_t  mc;             /* 6 bits data counter register     */
        uint8_t byte1;          /* Sprite line data 1               */
        uint8_t byte2;          /* Sprite line data 2               */
        uint8_t byte3;          /* Sprite line data 3               */
    };

    /**
     * Initialise this MOS-6569 video controller.
     * @param label  Label;
     * @param mmap   Address space;
     * @param vcolor Colour RAM (1K).
     * @see render_line(const Renderer&)
     * @see ASpace
     */
    Mos6569(std::string_view label, const sptr_t<ASpace>& mmap, const devptr_t& vcolor);

    ~Mos6569();

    /**
     * Set the render line callback.
     * The render line callback sends the video output to the user interface.
     * @param rl The render line callback.
     */
    void render_line(const Renderer& rl);

    /**
     * Set a colour palette from disk.
     * @param fname Palette file name.
     * @exception IOError See RgbaTable::load().
     * @see palette(const RgbaTable&)
     */
    void palette(std::string_view fname);

    /**
     * Set a colour palette from memory.
     * @param plt Colour palette to set.
     * @see RgbaTable
     */
    void palette(const RgbaTable& plt);

    /**
     * Set the IRQ pin callback.
     * This callback is called when the state of the IRQ output pin is changed.
     * @param irq_out IRQ output pin callback.
     */
    void irq(const OutputPinCb& irq_out);

    /**
     * Set the AEC pin callback.
     * This callback is called when the state of the AEC output pin is changed.
     * @param aec_out AEC output pin callback.
     */
    void aec(const OutputPinCb& aec_out);

    /**
     * Trigger the LP input.
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
     * @see Device::dev_read()
     */
    uint8_t dev_read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write()
     */
    void dev_write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override;

    /**
     * Get the video mode from video flags.
     * @param ecm Extended text colour mode flag;
     * @param bmm Bitmap mode flag;
     * @param mcm Multicolor mode flag.
     * @return The video mode.
     * @see VideoMode
     */
    constexpr VideoMode video_mode(bool ecm, bool bmm, bool mcm)
    {
        return static_cast<VideoMode>((ecm << 2) | (bmm << 1) | mcm);
    }

private:
    /**
     * Clock tick.
     * @param clk A reference to the caller clock.
     * @return The number of clock cycles that must pass before this method is called again.
     * @see Clockable
     */
    size_t tick(const Clock& clk) override;

    /**
     * If the current scanline is a badline set the badline flag.
     * @see _bad_line
     */
    void set_badline();

    /**
     * C-Access (colour/character code access).
     * Retrieve colour and character code (text mode) during a badline.
     * @return The state for the AEC output pin.
     */
    bool c_access();

    /**
     * G-Access (graphics data access).
     * Called 40 times (one for each column in the display line) it retrieves
     * and renders the data corresponding to the displayed character or bitmap.
     */
    void g_access();

    /**
     * Schedule a sprite for processing.
     * If the specified sprite is enabled and visible in the current scanline
     * it is scheduled for processing (during the proper ps_access()).
     * @param mob Sprite number (0-7).
     * @return True if the sprite is scheduled for processing; false otherwise.
     * @see mob_dma_off(SData&)
     */
    bool mob_dma_on(uint8_t mob);

    /**
     * Unschedule an already processed sprite.
     * @param mob Sprite data.
     * @see mob_dma_on(uint8_t)
     */
    void mob_dma_off(SData& md);

    /**
     * Sprite pointer and graphics data access.
     * @param mob Sprite number (0-7).
     * @return The state for the AEC output pin.
     * @see SData
     */
    bool ps_access(uint8_t mob);

    /**
     * Paint the display area in the current scanline.
     * @param bitmap Bitmap to paint;
     * @param colors Colour table;
     * @param mcm    Multicolor flag (true: 4 colours, 2 bits per pixel; false: 2 colours, 1 bit per pixel).
     * @see paint_byte(unsigned, uint8_t, const Rgba4&)
     * @see paint_mcm_byte(unsigned, uint8_t, const Rgba4&)
     */
    void paint_display(uint8_t bitmap, const Rgba4& colors, bool mcm);

    /**
     * Paint the sprites visible in the current raster line.
     * In case of superpositions a fixed sprite-sprite priority is followed.
     * @see paint_mob(uint8_t)
     */
    void paint_mobs();

    /**
     * Paint a sprite line only if it is enabled and visible.
     * @param mob Sprite number.
     * @see paint_mob(unsigned, uint64_t, const Rgba4&, bool)
     */
    void paint_mob(uint8_t mob);

    /**
     * Paint a sprite line.
     * @param start  Starting horizontal position;
     * @param bitmap Sprite bitmap;
     * @param colors Colour table;
     * @param expand True if the bitmap contains an horizontally expanded sprite; false otherwise;
     */
    void paint_mob(unsigned start, uint64_t bitmap, const Rgba4& colors, bool expand);

    /**
     * Paint a multicolor sprite line.
     * @param start  Starting horizontal position;
     * @param bitmap Sprite bitmap;
     * @param colors Colour table;
     * @param expand True if the bitmap contains an horizontally expanded sprite; false otherwise;
     */
    void paint_mob_mcm(unsigned start, uint64_t bitmap, const Rgba4& colors, bool exp_x);

    /**
     * Paint 8 pixels in the current scanline.
     * @param start  Starting horizontal position;
     * @param bitmap Bitmap (1 bit per pixel);
     * @param colors Colour table (only the first two colours are used).
     */
    void paint_byte(unsigned start, uint8_t bitmap, const Rgba4& colors);

    /**
     * Paint 4 pixels in the current scanline.
     * @param start  Starting horizontal position;
     * @param bitmap Bitmap (2 bits per pixel);
     * @param colors Colour table.
     */
    void paint_mcm_byte(unsigned start, uint8_t bitmap, const Rgba4& colors);

    /**
     * Paint borders.
     * If enabled, vertical and side borders are painted in the current scanline.
     */
    void paint_borders();

    /**
     * Paint a segment in the current scanline.
     * @param start Starting horizontal position;
     * @param width Width (pixels); use 0 to paint until the end of the scanlne;
     * @param color Colour.
     */
    void paint(unsigned start, unsigned width, Rgba color);

    /**
     * Render the current scanline.
     * This method calls the renderer callback.
     * @see render_line(const Renderer&)
     */
    void render_line();

    /**
     * Set/clear the IRQ output pin.
     * @param active The new state for the IRQ output pin.
     */
    void irq_out(bool active);

    /**
     * Set an interrupt flag and trigger in IRQ if a condition is true.
     * If a given condition is true an interrupt flag is set and, if this type of
     * interrupt is enabled and the IRQ output pin is not active, trigger an IRQ.
     * @param iflag Interrupt flag (REG_INTERRUPT_LP, REG_INTERRUPT_MMC, REG_INTERRUPT_MDC);
     * @param cond  Condition.
     * @see irq_out(bool)
     */
    void trigger_irq_if(uint8_t iflag, bool cond);

    /**
     * Set/clear the AEC output pin.
     * @param active The new state for the AEC output pin.
     */
    void aec_out(bool active);

    /**
     * Get the X coordinate of a MOB.
     * @param mob MOB (0-7).
     * @return The X coordinate of the specified MOB.
     */
    uint16_t mob_coord_x(uint8_t mob) const;

    /**
     * Get the Y coordinate of a MOB relative to the raster line.
     * @param mob MOB (0-7).
     * @return The Y coordinate of the specified MOB relative to the raster line.
     */
    uint16_t mob_coord_y(uint8_t mob) const;

    /**
     * Generate the bitmap of a sprite and detect collisions with the background image.
     * @param start  Starting horizontal position of the sprite;
     * @param byte1  Sprite line first data byte;
     * @param byte2  Sprite line second data byte;
     * @param byte3  Sprite line third data byte;
     * @param exp_x  Horizontal expansion flag;
     * @param mcm    Multicolor flag;
     * @param bg_pri Backgroud priority flag (background above sprite).
     * @return A tuple containing:
     *  - Sprite-Background collision flag;
     *  - Bitmap;
     *  - Visible bitmap.
     */
    std::tuple<bool, uint64_t, uint64_t> mob_bitmap(unsigned start, uint8_t byte1, uint8_t byte2, uint8_t byte3,
        bool exp_x, bool mcm, bool bg_pri);

    /**
     * Reset the sprite-background collision array.
     */
    void reset_collision_data();

    /**
     * Reset the sprite-sprite collision array.
     */
    void reset_collision_mob();

    /**
     * Update the collision data for this scanline.
     * @param start  Starting horizontal position;
     * @param bitmap Bitmap.
     */
    void update_collision_data(unsigned start, uint8_t bitmap);

    /**
     * Update the collision data for this scanline in multicolor mode.
     * @param start  Starting horizontal position;
     * @param bitmap Multicolor bitmap.
     */
    void update_collision_data_mcm(unsigned start, uint8_t bitmap);

    /**
     * Update the sprite-sprite collision array and detect sprite-sprite collisions.
     * @param mob    The sprite number (0-7);
     * @param start  Sprite horizontal position;
     * @param mcm    Multicolor flag;
     * @param bitmap Sprite bitmap.
     * @return The collided sprite bit if there are no other previous collision; 0 otherwise.
     */
    uint8_t update_collision_mob(uint8_t mob, unsigned start, bool mcm, uint64_t bitmap);

    Renderer                   _render_line{};                   /* Line renderer callback                          */
    OutputPinCb                _irq_out{};                       /* IRQ output pin callback                         */
    OutputPinCb                _aec_out{};                       /* AEC output pin callback                         */
    bool                       _aec_pin{};                       /* AEC output pin state                            */
    sptr_t<ASpace>             _mmap{};                          /* Address space mappings                          */
    devptr_t                   _vcolor{};                        /* Video colour RAM                                */
    RgbaTable                  _palette{};                       /* Color palette                                   */
    ui::Scanline               _scanline{};                      /* Pixel data for the current visible raster line  */
    uint8_t                    _mob_enable{};                    /* MOB enable flags                                */
    std::array<uint16_t, MOBS> _mob_coord_x{};                   /* MOB X coordinates                               */
    std::array<uint8_t, MOBS>  _mob_coord_y{};                   /* MOB Y coordinates                               */
    uint8_t                    _mob_expand_x{};                  /* MOB X expansion flags                           */
    uint8_t                    _mob_expand_y{};                  /* MOB Y expansion flags                           */
    uint8_t                    _mob_data_priority{};             /* MOB-DATA Priority flags                         */
    uint8_t                    _mob_mcm_sel{};                   /* MOB multicolor selection flags                  */
    uint8_t                    _mob_mob_collision{};             /* MOB-MOB collision flags                         */
    uint8_t                    _mob_data_collision{};            /* MOB-DATA collision flags                        */
    std::array<Color, MOBS>    _mob_color{};                     /* MOB colours                                     */
    std::array<Color, 2>       _mob_mcm{};                       /* MOB multicolor flags                            */
    uint16_t                   _raster_counter{};                /* Current raster line                             */
    uint16_t                   _stored_raster{};                 /* Interrupt raster line                           */
    bool                       _den{};                           /* Display Enabled flag                            */
    bool                       _mcm_mode{};                      /* Multicolor mode flag                            */
    bool                       _ecm_mode{};                      /* Extended colour mode flag                       */
    bool                       _bmm_mode{};                      /* Bitmap mode flag                                */
    bool                       _25_rows{};                       /* 25 rows flag                                    */
    bool                       _40_columns{};                    /* 40 columns flag                                 */
    uint8_t                    _scroll_x{};                      /* Scroll X position                               */
    uint8_t                    _scroll_y{};                      /* Scroll Y position                               */
    uint8_t                    _lp_x{};                          /* Latched light pen X position                    */
    uint8_t                    _lp_y{};                          /* Latched light pen Y position                    */
    bool                       _lp_latched{};                    /* Light pen latched flag                          */
    addr_t                     _char_base{};                     /* Base address of character memory                */
    addr_t                     _video_matrix{};                  /* Base address of video RAM                       */
    addr_t                     _bitmap_base{};                   /* Base address of Bitmap mode video RAM           */
    Color                      _border_color{};                  /* Border colour                                   */
    std::array<Color, 4>       _background_color{};              /* Background colours                              */
    uint8_t                    _irq_status{};                    /* IRQ status register                             */
    uint8_t                    _irq_enable{};                    /* IRQ enable mask                                 */
    bool                       _bl_den{};                        /* _den flag checked for a badline                 */
    bool                       _bad_line{};                      /* Current raster line is a bad line               */
    bool                       _idle_mode{};                     /* Idle vs Display mode                            */
    VideoMode                  _mode{};                          /* Current video mode                              */
    addr_t                     _vc_base{};                       /* 10 bits video counter base register             */
    addr_t                     _vc{};                            /* 10 bits video counter register                  */
    addr_t                     _rc{};                            /* 3 bits row counter register                     */
    size_t                     _col{};                           /* 6 bits counter to keep track the current column */
    std::array<CData, COLUMNS> _cdata{};                         /* c_access display data                           */
    std::array<SData, MOBS>    _mdata{};                         /* p_access and s_access mob data                  */
    bool                       _main_border{};                   /* Main border enabled flag                        */
    bool                       _vertical_border{};               /* Vertical borders enabled flag                   */
    unsigned                   _uborder_end{UBORDER_Y_END};      /* Last line of upper border + 1                   */
    unsigned                   _bborder_start{BBORDER_Y_START};  /* First line of bottom border                     */
    unsigned                   _lborder_end{LBORDER_X_END};      /* Last line of left border + 1                    */
    unsigned                   _rborder_start{RBORDER_X_START};  /* First line of right border                      */
    unsigned                   _cycle{};                         /* Current horizontal cycle                        */
    bool                       _vblank{};                        /* Vertical blanking flag                          */

    /*
     * Sprite-Background collision array.
     * This array contains the mask of the background image and it is
     * used to detect collisions between the background image and sprites.
     */
    std::array<uint8_t, caio::align(static_cast<uint64_t>(caio::ceil(WIDTH / 8.0) + 8))> _collision_data{};

    /*
     * Sprite-Sprite collision array.
     * These arrays contain the masks of each sprite in the current scanline
     * and they are used to detect collisions between sprites.
     */
    std::array<uint64_t, MOBS> _mob_bitmaps{};

    static const CData idle_cdata;
    static RgbaTable builtin_palette;
};

}

using Mos6569 = mos::Mos6569;

}
