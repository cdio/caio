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

#include <functional>
#include <string_view>

#include "aspace.hpp"
#include "clock.hpp"
#include "device.hpp"
#include "pin.hpp"
#include "rgb.hpp"
#include "ui.hpp"
#include "utils.hpp"

namespace caio {
namespace ricoh {
namespace rp2c02 {

/**
 * Ricoh 2C02 Picture Processing Unit (NTSC).
 *
 * ### Pinout:
 *
 *                +------------+
 *      R/W 01  ->| *          |<-  40 VCC
 *       D0 02  <>|            |->  39 ALE
 *       D1 03  <>|            |<>  38 AD0
 *       D2 04  <>|            |<>  37 AD1
 *       D3 05  <>|            |<>  36 AD2
 *       D4 06  <>|            |<>  35 AD3
 *       D5 07  <>|            |<>  34 AD4
 *       D6 08  <>|            |<>  33 AD5
 *       D7 09  <>|            |<>  32 AD6
 *       A2 10  ->|    2C02    |<>  31 AD7
 *       A1 11  ->|            |->  30 A8
 *       A0 12  ->|            |->  29 A9
 *      /CS 13  ->|            |->  28 A10
 *     EXT0 14  <>|            |->  27 A11
 *     EXT1 15  <>|            |->  26 A12
 *     EXT2 16  <>|            |->  25 A13
 *     EXT3 17  <>|            |->  24 /R
 *      CLK 18  ->|            |->  23 /W
 *     /IRQ 19  <-|            |<-  22 /SYNC
 *      VEE 20  ->|            |->  21 VOUT
 *                +------------+
 *
 * - CLK:
 *   21.48 MHz input clock.
 *
 * - /CS, R/W, D0-D7, A2-A0:
 *   CPU Bus (access to the MMIO registers).
 *
 * - EXT0-EXT3:
 *   I/O ports:
 *   + As input: Pixel color data.
 *   + As output: Co-PPU connection.
 *   In the case of NES, these ports are configured as input, hardwired to 0V.
 *
 * - /IRQ:
 *   Interrupt request output pin.
 *
 * - VOUT:
 *   Composite video output.
 *
 * - /SYNC:
 *   Reset input pin.
 *   In the case of FAMICOM this is pin is set to 1.
 *   In the case of NES this pin is connected to the CPU's reset input
 *   (the video output will be disabled whenever the reset button is held down).
 *
 * - /R, /W, ALE, AD0-AD7, A8-A13:
 *   PPU Bus.
 *
 * ### Screen resolution:
 *
 *     |<------------------------------- 341 ------------------------------->|
 *             |<-- 16 ->|<------------- 256 ------------->|<-- 11 ->|
 *
 *     +---------------------------------------------------------------------+     -+-      -+-      -+-
 *     |       |         |     NOT VISIBLE ON MOST NTSC    |         |       |      |        | 8      |
 *     |                   -     -     -     -     -     -                   |      |       -+-       |
 *     |       |         |                                 |         |       |      |        |        |
 *     |                                                                     |      |        |        |
 *     |       |         |                                 |         |       |      |        |        |
 *     |                                                                     |      |        |        |
 *     |       | LBORDER |                                 | RBORDER |       |      |        |        |
 *     | HORIZ                         VISIBLE                         HORIZ |      |         > 224   |
 *     | BLANK |         |                                 |         | BLANK |       > 240   | (NTSC) |
 *     |                                                                     |      |        |        |
 *     |       |         |                                 |         |       |      |        |        |
 *     |                                                                     |      |        |        |
 *     |       |         |                                 |         |       |      |        |         > 262
 *     |                                                                     |      |        |        |
 *     |       |         |                                 |         |       |      |        |        |
 *     |                   -     -     -     -     -     -                   |      |       -+-       |
 *     |       |         |     NOT VISIBLE ON MOST NTSC    |         |       |      |        | 8      |
 *     | - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |     -+-      -+-       |
 *     |                             POST-RENDER                             |      | 1               |
 *     | - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |     -+-                |
 *     |                                                                     |      |                 |
 *     |                               V-BLANK                               |       > 20             |
 *     |                                                                     |      |                 |
 *     | - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - - |     -+-                |
 *     |                             PRE-RENDER                              |      | 1               |
 *     +---------------------------------------------------------------------+     -+-               -+-
 *
 * Most NTSC TV sets show only 256x224 instead of the 256x240 screen,
 * clipping off the upper 8 and bottom 8 lines.
 *
 * ### Clock frequency and video signals:
 *
 *                   Back    +---+------+------------------------+--------+ Front
 *                   Porch   |   |  LB  |          Display       |   RB   | Porch
 *     --+        +----------+---+------+------------------------+--------+-------+
 *       |        |                                                               |
 *       +--------+                                                               +--
 *         HSync
 *       ^        ^          ^   ^      ^                        ^        ^       ^
 *       |        |          |   |      |                        |        |       |
 *      277      302        326 327     1                       257      268     277
 *
 *       |<- 25 ->|<-- 24 -->|<-- 16 -->|<--------- 256 -------->|<- 11 ->|<- 9 ->|
 *
 *       |<------------------------------ 63.51us ------------------------------->|
 *
 * The NTSC clock is set to 21.477272 MHz (+/- 40 Hz), the PPU takes 4 NTSC cycles
 * to output a pixel on the screen.
 * This implementation of the PPU must be clocked at *NTSC_clock / 4*.
 *
 * The CPU Bus is implemented by the device (caio::Device) part of this class.
 * The PPU Bus is implemented by the address space (caio::ASPace) set during initialisation.
 *
 * ### Memory Map:
 *
 *    Address Range     Size    Description         Provider Device (NES)
 *    -------------------------------------------------------------------
 *    0000-0FFF         1000    Pattern table 0     Cartridge (CHR)
 *    1000-1FFF         1000    Pattern table 1     Cartridge (CHR)
 *    2000-23FF         0400    Nametable 0         Cartridge (VRAM)
 *    2400-27FF         0400    Nametable 1         Cartridge (VRAM)
 *    2800-2BFF         0400    Nametable 2         Cartridge (VRAM)
 *    2C00-2FFF         0400    Nametable 3         Cartridge (VRAM)
 *    3000-3EFF         0F00    Unused
 *    3F00-3F1F         0020    Palette RAM indexes Internal to PPU
 *    3F20-3FFF         00E0    Mirror of 3F00-3F1F Internal to PPU
 *
 * @see https://www.nesdev.org/wiki/PPU
 */
class RP2C02 : public Device, public Clockable {
public:
    constexpr static const char* TYPE                       = "RP2C02";
    constexpr static const unsigned FRAME_WIDTH             = 341;
    constexpr static const unsigned FRAME_HEIGHT            = 262;
    constexpr static const unsigned VISIBLE_WIDTH           = 256;
    constexpr static const unsigned VISIBLE_HEIGHT          = 240;
    constexpr static const unsigned VBLANK_HEIGHT           = 20;
    constexpr static const unsigned WIDTH                   = VISIBLE_WIDTH;
    constexpr static const unsigned HEIGHT                  = VISIBLE_HEIGHT;
    constexpr static const unsigned NTSC_HEIGHT             = VISIBLE_HEIGHT - 16;
    constexpr static const unsigned COLUMNS                 = 32;
    constexpr static const unsigned ROWS                    = 30;

    constexpr static const unsigned CYCLES                  = FRAME_WIDTH;
    constexpr static const unsigned SCANLINES               = FRAME_HEIGHT;

    constexpr static const unsigned VISIBLE_X_START         = 0;
    constexpr static const unsigned VISIBLE_X_END           = VISIBLE_X_START + VISIBLE_WIDTH;
    constexpr static const unsigned INVALID_X_COORDINATE    = -1;
    constexpr static const unsigned VISIBLE_Y_START         = 0;
    constexpr static const unsigned VISIBLE_Y_END           = VISIBLE_Y_START + VISIBLE_HEIGHT;
    constexpr static const unsigned POST_RENDER_LINE        = VISIBLE_Y_END;
    constexpr static const unsigned VBLANK_START            = POST_RENDER_LINE + 1;
    constexpr static const unsigned VBLANK_END              = VBLANK_START + VBLANK_HEIGHT;
    constexpr static const unsigned PRE_RENDER_LINE         = VBLANK_END;

    constexpr static const addr_t PATTERN_TABLE_0_ADDR      = 0x0000;
    constexpr static const addr_t PATTERN_TABLE_1_ADDR      = 0x1000;
    constexpr static const addr_t NAME_TABLE_ADDR           = 0x2000;
    constexpr static const addr_t ATTR_TABLE_OFFSET         = COLUMNS * ROWS;
    constexpr static const addr_t ATTR_TABLE_ADDR           = NAME_TABLE_ADDR | ATTR_TABLE_OFFSET;
    constexpr static const addr_t PALETTE_ADDR              = 0x3F00;
    constexpr static const addr_t PALETTE_ADDR_MASK         = PALETTE_ADDR;
    constexpr static const addr_t PALETTE_INDEX_0_MASK      = 0x3F0F;
    constexpr static const addr_t PALETTE_SP_OFFSET         = 16;           /* 4 palettes with 4 colors each        */
    constexpr static const uint8_t PALETTE_COLOR_MASK       = 0x3F;         /* Palettes can use 4 of 64 colors      */
    constexpr static const uint8_t PALETTE_GREYSCALE_MASK   = 0x30;
    constexpr static const size_t PALETTES                  = 4;
    constexpr static const size_t PALETTE_SIZE              = 4;
    constexpr static const size_t BACKDROP_CINDEX           = 0;

    constexpr static const size_t SPRITES                   = 64;           /* Maximum 64 sprites                   */
    constexpr static const size_t SEC_SPRITES               = 8;            /* Max 8 visible sprites on a scanline  */
    constexpr static const unsigned SPRITE_Y_OFFSET         = 1;            /* Sprite Y coordinate offsetted by 1   */

    constexpr static const size_t TILES                     = 3;            /* Tile data buffer size                */

    using RendererCb = std::function<bool(unsigned, const ui::Scanline&)>;

    /*
     * Memory mapped registers (CPU Bus).
     */
    enum MMIO_Registers {
        PPUCTRL     = 0,
        PPUMASK     = 1,
        PPUSTATUS   = 2,
        OAMADDR     = 3,
        OAMDATA     = 4,
        PPUSCROLL   = 5,
        PPUADDR     = 6,
        PPUDATA     = 7,

        REGMASK     = 7,
        REGMAX
    };

    /**
     * Internal registers.
     */
    struct Registers {
        addr_t      v;              /* VRAM address/Scroll position (15 bits)   */
        addr_t      t;              /* Temporary VRAM address (15 bits)         */
        uint8_t     x;              /* Fine X scroll (3 bits)                   */
        bool        w;              /* Write toggle (1 bit)                     */
    };

    /**
     * Tile data.
     */
    struct TileData {
        uint8_t     tilech{};       /* Tile code (0-255)                        */
        uint8_t     pindex{};       /* Palette number (0-3)                     */
        uint8_t     plane[2]{};     /* Tile pixel data                          */
    };

    /**
     * OAM (sprite) data as stored in PPU memory.
     */
    struct Oam {
        uint8_t y;
        uint8_t tilech;
        uint8_t pindex:2;
        uint8_t unused:3;
        uint8_t bgpri:1;
        uint8_t hflip:1;
        uint8_t vflip:1;
        uint8_t x;
    } __attribute__((packed));

    /**
     * Seconadry OAM (sprite) data.
     */
    struct OamSec {
        uint8_t spindex{};          /* Sprite index (0-63)                  */
        Oam     sprite{};           /* Sprite data                          */

        OamSec(uint8_t spindex = 0, Oam sprite = {})
            : spindex{spindex},
              sprite{sprite}
        {
        }
    };

    /*
     * Tile pixel data.
     */
    struct TilePixel {
        bool used{};                /* In use (0: transparent, 1: opaque)   */
        Rgba color{};               /* Pixel color                          */
    };

    /*
     * Sprite pixel data.
     */
    struct SpritePixel : TilePixel {
        uint8_t spindex{};          /* Sprite index (0-63)                  */
        bool    bgpri{};            /* Background priority over sprites     */
    };

    /**
     * Initialise this PPU.
     * @param label PPU label;
     * @param mmap  Memory mappings (PPU bus);
     * @param ntsc  Enable NTSC mode (224 lines instead of 240).
     */
    RP2C02(std::string_view label, const sptr_t<ASpace>& mmap, bool ntsc);

    virtual ~RP2C02();

    /**
     * Set the renderline callback.
     * The renderline callback sends the video output to the user interface.
     * @param rl The render line callback.
     */
    void render_line(const RendererCb& rl);

    /**
     * Set a color palette from disk.
     * @param fname Palette file name.
     * @exception IOError
     * @see palette(const RgbaTable&)
     * @see RgbaTable::load()
     */
    void palette(const fs::Path& fname);

    /**
     * Set a color palette from memory.
     * @param plt Color palette.
     * @see RgbaTable
     */
    void palette(const RgbaTable& plt);

    /**
     * Set the /IRQ output pin callback.
     * This callback is called when the state of the /IRQ output pin is changed.
     * @param irq Callback.
     */
    void irq(const OutputPinCb& irq);

    /**
     * Set the status of the /SYNC input pin.
     * @param active Status to set (true to activate, false to deactive).
     * @return The status of the /SYNC pin after this call.
     */
    bool sync_pin(bool active);

    /**
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @see Device::size()
     */
    size_t size() const override;

private:
    /**
     * @see Device::dev_read()
     */
    uint8_t dev_read(size_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::dev_write()
     */
    void dev_write(size_t addr, uint8_t data) override;

    /**
     * Set/clear the IRQ output pin.
     * @param active The new state for the IRQ output pin.
     */
    void irq_out(bool active);

    /**
     * Clock tick.
     * @param clk A reference to the caller clock.
     * @return The number of clock cycles that must pass before this method is called again.
     * @see Clockable
     */
    size_t tick(const Clock& clk) override;

    /**
     * Render the current scanline.
     * This method calls the renderer callback.
     * After rendering the scanline is cleared using the backdrop color.
     * @see render_line(const RendererCb&)
     * @see backdrop_color()
     */
    void render_line();

    /**
     * Fetch a tile code from the name table.
     * Read the tile code from VRAM, the address is build
     * based on the contents of the 'v' register.
     * @param tile Destination tile data.
     * @see Registers
     * @see TileData
     */
    void fetch_tilech(TileData& tile);

    /**
     * Fetch the palette index from the attribute table.
     * Read an entry from the attribute table stored in VRAM
     * (soon after the name table), the address is built based
     * on the contents of the 'v' register.
     * The palette index is deduced from the attribute and the
     * contents of the 'v' register.
     * @param tile Destination tile data.
     * @see TileData
     * @see Registers
     */
    void fetch_palette(TileData& tile);

    /**
     * Fetch a tile pattern (tile bitmap data).
     * The address of the tile pattern is built based
     * on the contentes of the 'v' register.
     * @param tilech Tile code;
     * @param plane  Bitmap plane (0 or 1).
     * @see TileData
     * @see Registers
     */
    void fetch_bg_pattern(TileData& tile, bool plane);

    /**
     * Evaluate sprites and fill the secondary OAM buffer.
     * Evaluate a sprite and if it is visible in the specified
     * scanline copy it into the secondary OAM buffer.
     * @param spindex Sprite index within the OAM data buffer (0-63);
     * @param line    Scanline.
     * @return True if the secondary OAM buffer is full; false otherwise.
     */
    bool sprite_evaluation(uint8_t spindex, unsigned line);

    /**
     * Fetch a sprite pattern data (bitmap).
     * Feth the sprite pattern and, if necessary, flip it as specified
     * in the sprite data structure.
     * @param sprite Sprite data;
     * @param spline Sprite pattern line to fetch (0-7 for 8x8 sprites and 0-15 for 8x16 sprites).
     * @return A pair containing bit planes 0 and 1.
     * @see Oam
     */
    std::pair<uint8_t, uint8_t> sprite_planes(Oam sprite, uint8_t spline);

    /**
     * Paint a tile line.
     * The tile is painted in a temporary "background scanline" (_bg_scanline).
     * Sprite 0 hits are detected so this method expects the "sprite scanline"
     * (_sp_scanline) to be properly set within the range [start, start + 7].
     * The paint_scanline() method must be called before the actual rendering.
     * @param x      Starting horizontal coordinates within the current scanline;
     * @param left   Left tile;
     * @param right  Right tile.
     * @return The X coordinate of a sprite 0 hit, otherwise INVALID_X_COORDINATE.
     * @see paint_scanline()
     * @see _bg_scanline
     * @see TileData
     */
    unsigned paint_tile(unsigned x, const TileData& left, const TileData& right);

    /**
     * Paint a sprite line.
     * The specified sprite data is taken from the secondary oam buffer and
     * painted in a temporary "sprite scanline" (_sp_scanline).
     * The paint_scanline() method must be called before the actual rendering.
     * @param spindex Index of the sprite to paint (0-7).
     * @see sprite_planes(Oam, uint8_t)
     * @see paint_scanline()
     * @see _sp_scanline
     * @see _oam_sec
     */
    void paint_sprite(uint8_t spindex);

    /**
     * Paint all the sprites present in the secondary oam buffer.
     * @see paint_sprite(uint8_t)
     * @see _oam_sec
     */
    void paint_sprites();

    /**
     * Create the scanline ready to be rendered.
     * The scanline is created by merging the pixel data from
     * both the temporary "background" and "sprite" scanlines.
     * Both temporary scanlines are cleraed before returning back to the caller.
     * @see _bg_scanline
     * @see _sp_scanline
     */
    void paint_scanline();

    /**
     * Detect whether an address belongs to the palette area.
     * @param addr Address.
     * @return True if the address falls within a color palette; false otherwise.
     */
    bool is_palette_address(const addr_t addr) const;

    /**
     * Get a RGBA color from the palette.
     * Get the color from the RGBA palette and apply tint and greyscale modulators.
     * @param rindex RGBA color index.
     * @return The RGBA color.
     * @see _palette
     */
    Rgba palette_color(size_t rindex) const;

    /**
     * Get the backdrop (default) RGBA color.
     * @return The backdrop RGBA color.
     * @see _palette
     */
    Rgba backdrop_color() const;

    /**
     * Detect a forced VBlank condition.
     * Forced VBlank occurs when both sprites and background rendering are disabled.
     * @return True if forced VBlank is enabled; false otherwise.
     */
    bool is_forced_vblank() const;

    /**
     * Get the rendering status.
     * Rendering is enabled if the current scanline is a visible line
     * and rendering of background, sprites or both is enabled.
     * @return True if rendering is enabled; false otherwise.
     */
    bool is_rendering() const;

    /**
     * Increment the X coarse coordinate.
     * The 'v' register must contain a valid nametable address.
     */
    void scroll_x_coarse_inc();

    /**
     * Increment the Y coordinate.
     * The 'v' register must contain a valid nametable address.
     */
    void scroll_y_inc();

    sptr_t<ASpace>  _mmap;                                  /* PPU bus memory mappings                  */
    ui::Scanline    _scanline;                              /* Current scanline pixel data              */
    RgbaTable       _palette;                               /* Color palette                            */
    unsigned        _visible_y_start;                       /* Starting Y visible coordinate            */
    unsigned        _visible_y_end;                         /* Ending Y visible coordinate              */

    RendererCb      _render_line{};                         /* Scanline renderer callback               */
    Registers       _regs{};                                /* Internal registers                       */
    OutputPinCb     _irq_out{};                             /* /IRQ output pin                          */
    bool            _irq_status{};                          /* /IRQ pin status                          */
    InputPin        _sync_pin{};                            /* /SYNC input pin                          */
    uint8_t         _bg_palette[PALETTES * PALETTE_SIZE];   /* Background image palettes (x4)           */
    uint8_t         _sp_palette[PALETTES * PALETTE_SIZE];   /* Sprite palettes (x4)                     */

    addr_t          _vram_inc{};                            /* VRAM address increment                   */
    addr_t          _sp_base{};                             /* Sprite pattern table (8x8 sprites)       */
    addr_t          _bg_base{};                             /* Background pattern table                 */
    bool            _sp_8x16{};                             /* Sprite size (0: 8x8, 1: 8x16)            */
    bool            _irq_enabled{};                         /* Generate IRQ when VBlank starts          */
    bool            _ext_in{};                              /* EXT ports configured as input            */

    uint8_t         _rindex_mask{PALETTE_COLOR_MASK};       /* Color index mask (greyscale vs. color)   */
    bool            _bg_lborder{};                          /* Hide background on leftmost 8 pixels     */
    bool            _sp_lborder{};                          /* Hide sprites on leftmost 8 pixels        */
    bool            _bg_enabled{};                          /* Render background                        */
    bool            _sp_enabled{};                          /* Render sprites                           */
    bool            _tint{};                                /* Emphasize enabled                        */
    float           _red_tint{};                            /* Emphasize red component                  */
    float           _green_tint{};                          /* Emphasize green component                */
    float           _blue_tint{};                           /* Emphasize blue component                 */

    bool            _sp_0_hit{};                            /* Sprite 0 hit                             */
    bool            _sp_overflow{};                         /* Sprite overflow                          */
    unsigned        _sp_0_hit_cycle{INVALID_X_COORDINATE};  /* Sprite 0 hit cycle                       */
    bool            _vblank{};                              /* VBlank period                            */
    bool            _vblank_flag{};                         /* VBlank status flags (PPUSTATUS)          */

    uint8_t         _oam_addr{};                            /* OAM address                              */
    uint8_t         _oam[SPRITES * sizeof(Oam)]{};          /* OAM data                                 */
    OamSec          _oam_sec[SEC_SPRITES]{};                /* Secondary OAM data                       */
    size_t          _oam_sec_count{};                       /* Visible sprites in scanline (maximum 8)  */
    SpritePixel     _sp_scanline[WIDTH]{};                  /* Scanline made of sprites                 */
    TilePixel       _bg_scanline[WIDTH]{};                  /* Scanline made of tiles                   */

    uint8_t         _delayed_data{};                        /* Delayed read PPUDATA value               */
    uint8_t         _last_mmio_write{};                     /* Last MMIO register write                 */

    unsigned        _rasterline{};                          /* Current scanline                         */
    unsigned        _cycle{};                               /* Current cycle                            */

    TileData        _tiles[TILES];                          /* Tile data buffer                         */
    size_t          _fetch_tile{TILES - 1};                 /* Tile index being fetched                 */
    size_t          _paint_tile{0};                         /* Tile index being painted                 */

    static RgbaTable builtin_palette;
};

}
}

using RP2C02 = ricoh::rp2c02::RP2C02;

}
