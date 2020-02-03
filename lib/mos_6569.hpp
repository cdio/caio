/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "rgb.hpp"
#include "ui.hpp"
#include "utils.hpp"


namespace cemu {

/**
 * MOS6569 (VIC-II) emulator.
 * @note In this implementation one scanline is refreshed at once so it is not
 *       possible to exploit some of the bugs of the real chip using timing tricks.
 * @see C64 Programmer's Reference Guide.pdf, Appendix N
 * @see mos_6567_vic_ii_preliminary.pdf
 * @see https://www.cebix.net/VIC-Article.txt
 */
class Mos6569 : public Device, public Clockable {
public:
    constexpr static const char *TYPE = "MOS6569";

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
    constexpr static unsigned CYCLE_VISIBLE_START = 12;

    constexpr static unsigned UBORDER_Y_START    = VISIBLE_Y_START;
    constexpr static unsigned UBORDER_Y_END      = 51;

    constexpr static unsigned BBORDER_Y_START    = 251;
    constexpr static unsigned BBORDER_Y_END      = VISIBLE_Y_END;

    constexpr static unsigned DISPLAY_Y_START    = 48;
    constexpr static unsigned DISPLAY_Y_END      = DISPLAY_Y_START + DISPLAY_HEIGHT;

    constexpr static unsigned DISPLAY_X_START    = 42;   /* (VISIBLE_WIDTH - DISPLAY_WIDTH) / 2 */
    constexpr static unsigned DISPLAY_X_END      = DISPLAY_X_START + DISPLAY_WIDTH;

    constexpr static unsigned PIXELS_PER_CYCLE   = 8;
    constexpr static unsigned SCANLINE_CYCLES    = FRAME_WIDTH / PIXELS_PER_CYCLE;
    constexpr static unsigned FRAME_CYCLES       = FRAME_HEIGHT * SCANLINE_CYCLES;

    constexpr static unsigned MIB_WIDTH          = 24;
    constexpr static unsigned MIB_HEIGHT         = 21;

    constexpr static unsigned MIB_X_START        = 18;
    constexpr static unsigned MIB_Y_START        = 6;

    constexpr static unsigned MIB_POINTER_OFFSET = 1016;

    constexpr static size_t VIDEO_COLOR_RAM_SIZE = 1024;


    using color2_t = std::array<const Rgba *, 2>;
    using color4_t = std::array<const Rgba *, 4>;


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
        REG_MIB_MULTICOLOR     = 28,    /* 1C */
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

    constexpr static uint8_t REG_CONTROL_1_RSEL        = 0x08;  /* Number of rows: 1 => 25 rows, 0 => 24 rows       */
    constexpr static uint8_t REG_CONTROL_1_YSCROLL     = 0x07;  /* Vertical scroll position                         */

    constexpr static uint8_t REG_CONTROL_2_MCM         = 0x10;  /* Multicolor mode                                  */
    constexpr static uint8_t REG_CONTROL_2_CSEL        = 0x08;  /* Number of columns: 1 => 40 cols, 0 => 38 cols    */
    constexpr static uint8_t REG_CONTROL_2_XSCROLL     = 0x07;  /* Horizontal scroll position                       */

    constexpr static uint8_t REG_INTERRUPT_IRQ         = 0x80;  /* Status of the output !IRQ pin                    */
    constexpr static uint8_t REG_INTERRUPT_ILP         = 0x08;  /* First negative transition of Light Pen x frame   */
    constexpr static uint8_t REG_INTERRUPT_IMMC        = 0x04;  /* MIB-MIB collision interrupt                      */
    constexpr static uint8_t REG_INTERRUPT_IMDC        = 0x02;  /* MIB-DATA collision interrupt                     */
    constexpr static uint8_t REG_INTERRUPT_IRST        = 0x01;  /* Raster count interrupt                           */

    constexpr static uint8_t REG_INTERRUPT_MASK        = REG_INTERRUPT_ILP | REG_INTERRUPT_IMMC |
                                                         REG_INTERRUPT_IMDC | REG_INTERRUPT_IRST;

    constexpr static uint8_t REG_INTERRUPT_ELP         = 0x08;  /* Enable LP x frame transition interrupt           */
    constexpr static uint8_t REG_INTERRUPT_EMMC        = 0x04;  /* Enable MIB-MIB collision interrupt               */
    constexpr static uint8_t REG_INTERRUPT_EMDC        = 0x02;  /* Enable MIB-DATA collision interrupt              */
    constexpr static uint8_t REG_INTERRUPT_ERST        = 0x01;  /* Enable raster count interrupt                    */

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

    constexpr static unsigned COORDINATE_X_END         = 343; //XXX FIXME REMOVE

    constexpr static const unsigned MIB_MAX_X_SIZE     = 3 * 8 * 2; /* 3 bytes x MIB + expansion */


    /**
     * Initialise this video controller.
     * This instance is not usable until a video driver is specified (see ui()).
     * @param label  Label assigned to this device;
     * @param mmap   Address space for to this device;
     * @param vcolor Colour RAM (1K).
     */
    Mos6569(const std::string &label, std::shared_ptr<ASpace> mmap, devptr_t vcolor)
        : Device{TYPE, label},
          Clockable{},
          _mmap{mmap},
          _vcolor{vcolor},
          _palette{builtin_palette},
          _scanline(WIDTH) {
    }

    virtual ~Mos6569() {
    }

    /**
     * Set the user interface.
     * @param ui The user interface to set.
     */
    void ui(std::shared_ptr<UI> ui) {
        _ui = ui;
    }

    /**
     * Set a colour palette from disk.
     * @param fname Palette file name.
     * @exception IOError see RgbaTable::load().
     * @see palette(const RgbaTable &)
     */
    void palette(const std::string &fname) {
        if (!fname.empty()) {
            _palette.load(fname);
        }
    }

    /**
     * Set a colour palette from memory.
     * @param plt Colour palette to set.
     * @see RgbaTable
     */
    void palette(const RgbaTable &plt) {
        _palette = plt;
    }

    /**
     * Set the vsync callback.
     * The vsync callback is called by this controller each time a full screen frame is rendered.
     * @param cb V-Sync callback.
     */
    void vsync(std::function<void(unsigned)> cb) {
        _vsync = cb;
    }

    /**
     * Set the IRQ pin callback.
     * The IRQ pin callback is called when the status of the IRQ output pin of this device is changed.
     * @param trigger_irq IRQ pin callback.
     */
    void irq(std::function<void(bool)> trigger_irq) {
        _trigger_irq = trigger_irq;
    }

    /**
     * Set the AEC pin callback.
     * The AEC pin callback is called when the value of the AEC output pin of this device is changed.
     * @param set_aec BA pin callback.
     */
    void aec(std::function<void(bool)> set_aec) {
        _set_aec = set_aec;
    }

    /**
     * @see Device::size()
     */
    size_t size() const override {
        return _regs.size();
    }

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream &dump(std::ostream &os, addr_t base = 0) const override {
        return utils::dump(os, _regs, base);
    }

    /**
     * LP edge triggered input.
     */
    void trigger_lp();

private:
    /**
     * Clock tick interface (callback).
     * This method must be called by the system clock.
     * @param clk A reference to the caller clock.
     * @return The number of clock cycles that must pass until the clock is allowed to call this method again.
     * @see Clockable
     */
    size_t tick(const Clock &clk) override;

    /**
     * Set the status of the IRQ output pin.
     * The IRQ bit in the REG_INTERRUPT register is properly
     * set and the IRQ trigger callback is called.
     * @param active true if the IRQ ouput pin must be activated; false otherwise.
     * @see _trigger_irq
     */
    void irq_out(bool active);

    /**
     * Set the AEC pin.
     * @param active true to activate; false to deactivate.
     */
    void aec_out(bool active) {
        if (_set_aec) {
            _set_aec(active);
        }
    }

    /**
     * Update the interrupt flags and activate the IRQ output pin if necessary.
     */
    void update_interrupts();

    /**
     * @return The current raster line.
     */
    unsigned rasterline() const {
        return _regs[REG_RASTER_COUNTER] + ((_regs[REG_CONTROL_1] & REG_CONTROL_1_RC8) ? 256 : 0);
    }

    /**
     * Set the current raster line.
     * @param line Line to set.
     */
    void rasterline(unsigned line) {
        _regs[REG_RASTER_COUNTER] = line & 255;
        if (line > 255) {
            _regs[REG_CONTROL_1] |= REG_CONTROL_1_RC8;
        } else {
            _regs[REG_CONTROL_1] &= ~REG_CONTROL_1_RC8;
        }
    }

    /**
     * Get the base address of a character data.
     * @param ch Character code.
     * @return The base address of the specified character data.
     */
    addr_t char_base(uint8_t ch = 0) const {
        addr_t addr = static_cast<addr_t>(_regs[REG_MEMORY_POINTERS] & REG_MEMORY_POINTERS_CHAR) << 10;
        return addr + (static_cast<addr_t>(ch) << 3);
    }

    /**
     * Get the base address of the video matrix.
     * The video matrix is an area of 1000 bytes of memory. How
     * this data is interpreted depends on the current video mode
     * - Hi-Res character mode (default):
     *   The video matrix contains the character codes displayed at each
     *   position on the 40x25 screen, 16 colours.
     *
     * - Extended-colour character mode:
     *   The video matrix contains 5 bits of the character codes displayed at
     *   each position on the 40x25 screen plus two bits for the colour codes.
     *
     * - Multi-color character mode:
     *   The video matrix contains the character codes displayed at each
     *   position on the 40x25 screen, 8 colours.
     *
     * - Bitmap mode:
     *   The video matrix contains the foreground and background colours
     *   for each of the 8x8 squares on the screen.
     *
     * - Multi-color bitmap mode:
     *   The video matrix contains two of the four available colours for
     *   each of the 8x8 squares on the screen.
     *
     * @return The base address of the video matrix.
     */
    addr_t video_matrix() const {
        addr_t addr = static_cast<addr_t>(_regs[REG_MEMORY_POINTERS] & REG_MEMORY_POINTERS_VIDEO) << 6;
        return addr;
    }

    /**
     * Get the base address of the video pixel data in bitmap mode.
     * The video pixel data is an area of 8000 bytes of memory containing
     * the information for each single pixel in the 320x200 (or 160x200) screen.
     * How this data is interpreted depends on the current video mode.
     * @return The base address of the video pixel data in bitmap mode.
     */
    addr_t bitmap_base() const {
        addr_t addr = ((_regs[REG_MEMORY_POINTERS] & REG_MEMORY_POINTERS_CB13) ? (1 << 13) : 0);
        return addr;
    }

    /**
     * Get the base address of a MIB data.
     * @param mib MIB (between 0 and 7).
     * @return The base address of the specified MIB data.
     */
    addr_t mib_base(unsigned mib) const {
        addr_t addr = static_cast<addr_t>(_mmap->read(video_matrix() + MIB_POINTER_OFFSET + mib)) << 6;
        return addr;
    }

    /**
     * Detect the current video mode.
     * @return true if bitmap mode is active; false if character mode is active.
     */
    bool is_display_bitmap_mode() const {
        return (_regs[REG_CONTROL_1] & REG_CONTROL_1_BMM);
    }

    /**
     * @return true if extended colour mode is enabled; false otherwise.
     */
    bool is_display_extended_color() const {
        return (_regs[REG_CONTROL_1] & REG_CONTROL_1_ECM);
    }

    /**
     * @return true if multi colour mode is enabled; false otherwise.
     */
    bool is_display_multicolor() const {
        return (_regs[REG_CONTROL_2] & REG_CONTROL_2_MCM);
    }

    /**
     * Detect whether the display is enabled or not.
     * When the display is not enabled (blanked)
     * it is filled with the border color.
     * @return true if the display in enabled; false otherwise.
     */
    bool is_display_enabled() const {
        return (_regs[REG_CONTROL_1] & REG_CONTROL_1_DEN);
    }

    /**
     * Detect whether the display has 24 or 25 rows.
     * The rendering is always the same but the border covers
     * part of the background in 24 rows mode.
     * @return true on 24 rows mode; false on 25 rows mode.
     */
    bool is_display_24_rows() const {
        return (!(_regs[REG_CONTROL_1] & REG_CONTROL_1_RSEL));
    }

    /**
     * Detect whether the number of columns is 38 or 40.
     * The rendering is always the same but the border covers
     * part of the background in 38 columns mode.
     * @return true on 38 columns mode; false on 40 columns mode.
     */
    bool is_display_38_columns() const {
        return (!(_regs[REG_CONTROL_2] & REG_CONTROL_2_CSEL));
    }

    /**
     * @return The horizontal scroll position.
     */
    uint8_t display_scroll_x() const {
        return (_regs[REG_CONTROL_2] & REG_CONTROL_2_XSCROLL);
    }

    /**
     * @return The vertical scroll position.
     */
    uint8_t display_scroll_y() const {
        return (_regs[REG_CONTROL_1] & REG_CONTROL_1_YSCROLL);
    }

    /**
     * Detect whether a sprite is enabled or disabled.
     * @param sbit Sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @return true if the specified sprite is enabled; false otherwise.
     */
    bool is_mib_enabled(uint8_t sbit) const {
        return (_regs[REG_MIB_ENABLE] & sbit);
    }

    /**
     * Detect the priority of a sprite against a background image.
     * @param sbit Sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @return true if the specified sprite is behind the background;
     * false if the sprite is covers the background.
     */
    bool is_mib_behind_data(uint8_t sbit) const {
        return (_regs[REG_MIB_DATA_PRI] & sbit);
    }

    /**
     * Detect whether a sprite is multicolor.
     * @param sbit Sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @return true if the specified sprite is a multi-color one; false otherwise.
     */
    bool is_mib_multicolor(uint8_t sbit) const {
        return (_regs[REG_MIB_MULTICOLOR] & sbit);
    }

    /**
     * Detect whether a sprite is expanded in the horizontal direction.
     * @param sbit Sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @return true if the specified sprite is expanded; false otherwise.
     */
    bool is_mib_expanded_x(uint8_t sbit) const {
        return (_regs[REG_MIB_X_EXPANSION] & sbit);
    }

    /**
     * Detect whether a sprite is expanded in the vertical direction.
     * @param sbit Sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @return true if the specified sprite is expanded; false otherwise.
     */
    bool is_mib_expanded_y(uint8_t sbit) const {
        return (_regs[REG_MIB_Y_EXPANSION] & sbit);
    }

    /**
     * Get the vertical position of a sprite.
     * @param mib Sprite number (between 0 and 7).
     * @return The vertical position of the specified sprite.
     */
    unsigned mib_position_y(uint8_t mib) const {
        return (_regs[REG_MIB_0_Y + (mib << 1)]);
    }

    /**
     * Get the horizontal position of a sprite.
     * @param mib Sprite number (between 0 and 7).
     * @return The horizontal position of the specified sprite.
     */
    unsigned mib_position_x(uint8_t mib) const {
        return (MIB_X_START + (_regs[REG_MIB_0_X + (mib << 1)] +
            ((_regs[REG_MIBS_MSB_X] & (1 << mib)) != 0 ? 0x100 : 0)));
    }

    /**
     * Detect whether a sprite is visible in a scanline.
     * @param line Raster line;
     * @param mib  Sprite number (between 0 and 7).
     * @return true if the sprite is enabled and visible in the specified line; false otherwise.
     * @see mib_visibility_y()
     */
    bool is_mib_visible(unsigned line, uint8_t mib) {
        return (std::get<0>(mib_visibility_y(line, mib)) != static_cast<unsigned>(-1));
    }

    /**
     * Retrieve a sprite configuration in a specified scanline.
     * @param line Raster line;
     * @param mib  Sprite number (between 0 and 7).
     * @return A tuple containing three elements:
     *   - The sprite Y position;
     *   - The sprite vertical size;
     *   - A flag indicating whether the sprite is vertically expanded.
     * If the sprite is not enabled or not visible in the specified raster line the returned Y position is -1.
     */
    std::tuple<unsigned, unsigned, bool> mib_visibility_y(unsigned line, uint8_t mib);

    /**
     * @return true if at least one MIB-DATA collision was already detected and not yet acknowledged by the user.
     */
    bool is_mib_data_collision() const {
        return (_regs[REG_MIB_DATA_COLLISION] != 0);
    }

    /**
     * @return true if at least one MIB-MIB collision was already detected and not yet acknowledged by the user.
     */
    bool is_mib_mib_collision() const {
        return (_regs[REG_MIB_MIB_COLLISION] != 0);
    }

    /**
     * Set a MIB-DATA collision flag.
     * @param sbit Sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @see is_mib_data_collision()
     */
    void mib_data_collision(uint8_t sbit) {
        _regs[REG_MIB_DATA_COLLISION] |= sbit;
    }

    /**
     * Set a MIB-MIB collision flag.
     * @param sbit1 Frist collided sprite bit (1 << n, where n is the sprite number between 0 and 7);
     * @param sbit2 Second collided sprite bit (1 << n, where n is the sprite number between 0 and 7).
     * @see is_mib_mib_collision()
     */
    void mib_mib_collision(uint8_t sbit1, uint8_t sbit2) {
        _regs[REG_MIB_MIB_COLLISION] |= (sbit1 | sbit2);
    }

    /**
     * Get a colour from a register.
     * @param reg Register containing the colour codes.
     * @return A constant reference to the RGBA colour assigned to the colour codes.
     * @see Color
     * @see Rgba
     */
    const Rgba &reg_color(int reg) const {
        return _palette[_regs[reg] & Color::MASK];
    }

    /**
     * @return The border colour.
     */
    const Rgba &border_color() const {
        return reg_color(REG_BORDER_COLOR);
    }

    /**
     * Get the background colour.
     * @param bg Number of background (between 0 and 3).
     * @return The specified background colour.
     */
    const Rgba &background_color(int bg) const {
        return reg_color((bg & 3) + REG_BACKGROUND_COLOR_0);
    }

    /**
     * Get the colour code from the colour RAM for a specific 40x25 display position.
     * @param x Horizontal coordinate (between 0 and 39);
     * @param y Vertical coordinate (between 0 and 24).
     * @return The colour code.
     * @see _vcolor
     */
    Color video_color_code(unsigned x, unsigned y) const {
        addr_t addr = static_cast<addr_t>(x + y * CHARMODE_COLUMNS);
        return static_cast<Color>(_vcolor->read(addr) & Color::MASK);
    }

    /**
     * Get the RGBA colour from the colour RAM for a specific 40x25 display position.
     * @param x Horizontal coordinate (between 0 and 39);
     * @param y Vertical coordinate (between 0 and 24).
     * @return The RGBA colour.
     */
    const Rgba &video_color(unsigned x, unsigned y) const {
        return _palette[video_color_code(x, y)];
    }

    /**
     * Paint a segment in the current scanline.
     * @param start Starting horizontal position;
     * @param width Segment width in pixels (if, 0 the entire scanline from the starting position is painted);
     * @param color RGBA colour to use.
     */
    void paint(unsigned start, unsigned width, const Rgba &color) {
        if (start < _scanline.size()) {
            if (width == 0 || start + width > _scanline.size()) {
                width = _scanline.size() - start;
            }

            std::fill_n(_scanline.begin() + start, width, color);
        }
    }

    /**
     * Paint a byte bitmap (1 bit per pixel) in the current scanline.
     * Bits set to 1 are painted with the foreground colour,
     * bits set to 0 are painted with the background colour.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap;
     * @param colors Background and foreground colours;
     */
    void paint_byte(unsigned start, uint8_t bitmap, const color2_t &colors);

    /**
     * Paint a byte bitmap (1 bit per pixel) in the current scanline considering the horizontal scroll.
     * Bits set to 1 are painted with the foreground colour,
     * bits set to 0 are painted with the background colour.
     * The coordinates to be painted are adjusted with the current display horizontal scroll position.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap;
     * @param colors Background and foreground colours;
     * @see paint_byte()
     * @see display_scroll_x()
     */
    void paint_byte_scroll(unsigned start, uint8_t bitmap, const color2_t &colors) {
        paint_byte(start + display_scroll_x(), bitmap, colors);
    }

    /**
     * Paint a byte bitmap (2 bits per pixel) in the current scanline.
     * 4 double sized pixels are painted (4 pairs of 2 pixels with the same colour).
     * The colour codes of each of the 4 pixels are encoded in two consecutive bits.
     * The specified byte represents a bitmap containing 4 pairs of pixels:
     *   - 00: Two pixels painted using color[0];
     *   - 01: Two pixels painted using color[1];
     *   - 10: Two pixels painted using color[2];
     *   - 11: Two pixels painted using color[3].
     * The pixel size is doubled but the visual effect is a reduction of the horizontal resolution from 320 to 160.
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap containing the 4 pixels to paint;
     * @param colors Table with pointers to the 4 colours;
     */
    void paint_mcm_byte(unsigned start, uint8_t bitmap, const color4_t &colors);

    /**
     * Paint a byte bitmap (2 bits per pixel) in the current scanline.
     * The coordinates to be painted are adjusted with the current display horizontal scroll position.
     * 4 double sized pixels are painted (4 pairs of 2 pixels with the same colour).
     * The colour codes each of the 4 pixels are encoded in two consecutive bits.
     * The visual effect is a reduction of the horizontal resolution from 320 to 160 (pixel size doubled).
     * @param start  Starting horizontal position;
     * @param bitmap Byte bitmap containing the 4 pixels to paint;
     * @param colors Table with pointers to the 4 colours;
     * @see paint_mcm_byte()
     * @see display_scroll_x()
     */
    void paint_mcm_byte_scroll(unsigned start, uint8_t bitmap, const color4_t &colors) {
        paint_mcm_byte(start + display_scroll_x(), bitmap, colors);
    }

    /**
     * Paint a segment in the current scanline using the border colour.
     * @param start Starting horizontal position;
     * @param width Segment width (if 0, the entire scanline from the starting position is painted).
     */
    void paint_border(unsigned start = 0, unsigned width = 0) {
        paint(start, width, border_color());
    }

    /**
     * Paint a character mode display line.
     * @param line The raster line to paint (relative to the start of the display area).
     */
    void paint_char_mode(unsigned line);

    /**
     * Paint a bitmap mode display line.
     * @param line The raster line to paint (relative to the start of the display area).
     */
    void paint_bitmap_mode(unsigned line);

    /**
     * Paint a sprite bitmap line.
     * @param start  Starting horizontal position;
     * @param bitmap The shaped sprite bitmap;
     * @param colors Sprite background and foreground colours;
     * @param expand true if the bitmap contains an horizontally expanded sprite; false otherwise.
     */
    void paint_sprite_line(unsigned start, uint64_t bitmap, const color2_t &colors, bool expand);

    /**
     * Paint a multicolor sprite bitmap line.
     * @param start  Starting horizontal position;
     * @param bitmap The shaped sprite bitmap;
     * @param colors Sprite colours.
     * @param expand true if the bitmap contains an horizontally expanded sprite; false otherwise.
     */
    void paint_sprite_line_mcm(unsigned start, uint64_t bitmap, const color4_t &colors, bool expand);

    /**
     * Paint a sprite line.
     * The sprite line is painted only if it is enabled and visible in the specified raster line.
     * @param line Raster line to paint.
     * @param mib  Sprite number (between 0 and 7).
     * @see is_mib_enabled()
     * @see mib_bitmap()
     * @see paint_sprite_line()
     * @see paint_sprite_line_mcm()
     */
    void paint_sprite(unsigned line, uint8_t mib);

    /**
     * Paint all the sprite lines.
     * Only the lines of the visible sprites are painted in the specified raster line,
     * in case of sprite superpositions the fixed sprite-sprite priority is followed.
     * @param line The raster line to paint.
     * @see paint_sprite()
     */
    void paint_sprites(unsigned line);

    /**
     * Paint a complete raster line.
     * Paint borders, display area and sprites.
     * @param line The raster line to paint.
     */
    void paint_scanline(unsigned line);

    /**
     * Reset the collision background data array.
     */
    void reset_collision_data() {
        _collision_data.fill(0);
    }

    /**
     * Reset the collision sprite bitmap data array.
     */
    void reset_collision_mib() {
        _mib_bitmaps.fill(0);
    }

    /**
     * Update the collision data for this scanline.
     * The collision data is upated each time a character data or bitmap data is painted in the scanline.
     * @param start  Starting horizontal coordinate;
     * @param bitmap Byte bitmap painted.
     */
    void update_collision_data(unsigned start, uint8_t bitmap);

    /**
     * Update the collision data for this scanline.
     * The coordinates to be updated are adjusted with the current display horizontal scroll position.
     * The collision data is upated each time a character data or bitmap data is painted in the scanline.
     * @param start  Starting horizontal coordinate;
     * @param bitmap Byte bitmap painted.
     * @see update_collision_data()
     */
    void update_collision_data_scroll(unsigned start, uint8_t bitmap) {
        update_collision_data(start + display_scroll_x(), bitmap);
    }

    /**
     * Update the collision data for this scanline in multicolor mode.
     * The collision data is upated each time a character data or bitmap data is painted in the scanline.
     * @param start  Starting horizontal coordinate;
     * @param bitmap Byte bitmap encoded with 4 color codes.
     * @see update_collision_data()
     */
    void update_collision_data_mcm(unsigned start, uint8_t bitmap);

    /**
     * Update the collision data for this scanline in multicolor mode.
     * The coordinates to be updated are adjusted with the current display horizontal scroll position.
     * The collision data is upated each time a character data or bitmap data is painted in the scanline.
     * @param start  Starting horizontal coordinate;
     * @param bitmap Byte bitmap encoded with 4 color codes.
     * @see update_collision_data_mcm()
     */
    void update_collision_data_mcm_scroll(unsigned start, uint8_t bitmap) {
        update_collision_data_mcm(start + display_scroll_x(), bitmap);
    }

    /**
     * Generate the shapes of a sprite line and detect collisions with the background image.
     * Generate two shapes of a sprite: The complete shape and the visible part (not obscured by the background image).
     * @param start    Starting horizontal position of the sprite;
     * @param byte1    Sprite line first data byte;
     * @param byte2    Sprite line second data byte;
     * @param byte3    Sprite line third data byte;
     * @param expand   true if the sprite must be expanded horizontally; false otherwise.
     * @param mcm      true if it is a multi-color sprite; false otherwise.
     * @param data_pri true if a background image has the visibility priority over the sprite; false otherwise.
     * @return A tuple containing: The sprite-background (MIB-DATA) collision flag,
     * the sprite data and the visible sprite data.
     */
    std::tuple<bool, uint64_t, uint64_t>
    mib_bitmap(unsigned start, uint8_t byte1, uint8_t byte2, uint8_t byte3, bool expand, bool mcm, bool data_pri);

    /**
     * Update a sprite collision mask and detect eventual collisions with other sprites.
     * @param mib    The sprite number (between 0 and 7).
     * @param start  Sprite horizontal position;
     * @param mcm    true if the sprite is multicolor; false otherwise;
     * @param bitmap The complete bitmap of the sprite.
     * @return A pair containing: A MIB-MIB collision flag (if there are no other MIB-MIB collisions detected)
     * and the number of the other collided sprite (this value is valid only if the collision flag is active).
     */
    std::pair<bool, uint8_t> update_collision_mib(uint8_t mib, unsigned start, bool mcm, uint64_t bitmap);

    /**
     * Registers.
     */
    std::array<uint8_t, REGMAX> _regs{};

    /**
     * Address space mappings.
     * These mappings define the devices that can be seen
     * by this video controller through its address lines.
     */
    std::shared_ptr<ASpace> _mmap{};

    /**
     * Video colour RAM.
     */
    devptr_t _vcolor{};

    /**
     * V-Sync callback.
     */
    std::function<void(size_t)> _vsync{};

    /**
     * User interface (video driver).
     */
    std::shared_ptr<UI> _ui{};

    /**
     * Colour palette.
     * Translation table from colour codes RGBA colours.
     */
    RgbaTable _palette{};

    /**
     * IRQ ouput pin trigger.
    */
    std::function<void(bool)> _trigger_irq{};

    /**
     * AEC (Address Enable Control) output pin trigger.
     */
    std::function<void(bool)> _set_aec{};

    /**
     * Scanline pixel data.
     * Refreshed each SCANLINE_CYCLES clock cycles.
     */
    ui::Scanline _scanline{};

    /**
     * Stored raster line.
     * Raster line written by the user; it is used to generate an interrupt when
     * the current raster line register (REG_RASTER_COUNTER) matches this value.
     * @see rasterline()
     */
    unsigned _stored_rasterline{};

    /**
     * Whether the DEN bit is enabled.
     * This value is updated on cycle 0 of raster line $30.
     */
    bool _is_den{};

    /**
     * Whether the LP was triggered on this frame.
     */
    bool _lp_triggered{};

    /**
     * Whether the current raster line is a bad line.
     */
    bool _is_badline{};

    /**
     * The current rasterline cycle.
     */
    unsigned _cycle{};

    /**
     * Bitmap scanline used to detect collisions between the background (data) and the sprites.
     */
    std::array<uint8_t, utils::align(static_cast<uint64_t>(utils::ceil(WIDTH / 8.0f) + 8))> _collision_data{};

    /**
     * Bitmap masks used to detect collisions between sprites.
     * The elements of the array contain the bitmap mask for each sprite in the current scanline.
     */
    std::array<uint64_t, 8> _mib_bitmaps{};

    /**
     * Builtin (default) colour palette.
     */
    static RgbaTable builtin_palette;
};

}
