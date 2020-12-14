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
#include "mos_6569.hpp"

#include <endian.h>

#include "logger.hpp"


namespace cemu {

/*
 * https://www.colodore.com
 */
RgbaTable Mos6569::builtin_palette{
    0x000000FF,
    0xFFFFFFFF,
    0x813338FF,
    0x75CEC8FF,
    0x8E3C97FF,
    0x56AC4DFF,
    0x2E2C9BFF,
    0xEDF171FF,
    0x8E5029FF,
    0x553800FF,
    0xC46C71FF,
    0x4A4A4AFF,
    0x7B7B7BFF,
    0xA9FF9FFF,
    0x706DEBFF,
    0xB2B2B2FF
};


uint8_t Mos6569::read(addr_t addr) const
{
    uint8_t data;
    try {
        data = _regs.at(addr);
    } catch (const std::out_of_range &) {
        throw InternalError{*this, "read: Invalid address: $" + utils::to_string(addr)};
    }

    switch (addr) {
    case REG_INTERRUPT:
        /*
         * Bits 6, 5 and 4 set to 1 regardless of the written value
         * (see MOS6569 Data Sheet Preliminary, page 14).
         */
        data |= 0x70;
        break;

    case REG_INTERRUPT_ENABLE:
        /*
         * Bits 7, 6, 5 and 4 set to 1 regardless of the written value
         * (see MOS6569 Data Sheet Preliminary, page 14).
         */
        data |= 0xF0;
        break;

    case REG_MIB_MIB_COLLISION:
    case REG_MIB_DATA_COLLISION:
        /*
         * MIB-MIB and MIB-DATA collision registers automatically clear when read.
         */
        const_cast<Mos6569 *>(this)->_regs[addr] = 0;
        break;

    default:;
    }

    return data;
}

void Mos6569::write(addr_t addr, uint8_t data)
{
    switch (addr) {
    case REG_RASTER_COUNTER:
        /*
         * The user writes the stored rasterline and not the register rasterline.
         */
        _stored_rasterline = (_stored_rasterline & 256) | data;
        return;

    case REG_CONTROL_1: {
        /*
         * The user writes the stored rasterline and not the register rasterline.
         */
        unsigned rc8 = ((data & REG_CONTROL_1_RC8) ? 256 : 0);
        _stored_rasterline = (_stored_rasterline & 255) | rc8;

        /*
         * Other bits are written as expected.
         */
        if (_regs[addr] & REG_CONTROL_1_RC8) {
            data |= REG_CONTROL_1_RC8;
        } else {
            data &= ~REG_CONTROL_1_RC8;
        }
        break;
    }

    case REG_CONTROL_2:
        /*
         * Bits 7 and 6 are set to 1 regardless of the written value.
         * Bit 5 is specified as "RES" but this value is never mentioned
         * in the MOS6569 Data Sheet Preliminary, page 14.
         * Other documentation says that bit 5 is not used (see
         * https://dustlayer.com/vic-ii/2013/4/22/when-visibility-matters)
         * and yet other documentation says this is a RESET bit that stops
         * the VIC-II chip when set to one making the screen go black (see
         * http://www.unusedino.de/ec64/technical/project64/mapping_c64.html);
         * this is consistent with the only good explanation provided in the
         * C64 Programmer's Reference Guide, page 448.
         * We just ignore this bit.
         * @see https://www.c64-wiki.com/wiki/Commodore_64_Programmer%27s_Reference_Guide
         */
        data |= 0xC0;
        break;

    case REG_MEMORY_POINTERS:
        /*
         * Bit 0 set to 1 regardless of the written value
         * (see MOS6569 Data Sheet Preliminary, page 14).
         */
        data |= 1;
        break;

    case REG_INTERRUPT:
        /*
         * The REG_INTERRUPT register is a read-only status register.
         * By delivering a write to this register the user acknowledges up
         * to four interrupt sources. Only when all interrupt sources are
         * acknowledged the IRQ pin is de-activated (see interrupt_update()).
         */
        _regs[REG_INTERRUPT] &= ~(data & REG_INTERRUPT_MASK);
        if ((_regs[REG_INTERRUPT] & (_regs[REG_INTERRUPT_ENABLE] | REG_INTERRUPT_IRQ)) == REG_INTERRUPT_IRQ) {
            /*
             * No more acks pending.
             */
            irq_out(false);
        }
        return;

    case REG_INTERRUPT_ENABLE:
        data &= REG_INTERRUPT_MASK;
        break;

    default:;
    }

    try {
        _regs.at(addr) = data;
    } catch (const std::out_of_range &) {
        throw InternalError{*this, "write: Invalid address: $" + utils::to_string(addr)};
    }
}

size_t Mos6569::tick(const Clock &clk)
{
    unsigned line = rasterline();
    bool is_vblank = line < VISIBLE_Y_START && line >= VISIBLE_Y_END;
    bool aec = true;

    if (is_vblank) {
        update_interrupts();
        _cycle = SCANLINE_CYCLES;
        _lp_triggered = false;      /* TODO: Check if this is correct. See trigger_lp()*/

    } else {
        switch (_cycle) {
        case 0:
            if (line == DISPLAY_Y_START) {
                _is_den = is_display_enabled();
            }

            _is_badline = _is_den && line >= DISPLAY_Y_START && line < DISPLAY_Y_END && (line & 7) == display_scroll_y();
            aec = !is_mib_visible(line, 3);
            break;

        case 1:
            aec = !is_mib_visible(line, 3);
            update_interrupts();
            break;

        case 2:
        case 3:
            aec = !is_mib_visible(line, 4);
            break;

        case 4:
        case 5:
            aec = !is_mib_visible(line, 5);
            break;

        case 6:
        case 7:
            aec = !is_mib_visible(line, 6);
            break;

        case 8:
        case 9:
            aec = !is_mib_visible(line, 7);
            break;

        case 10:
        case 11:
        case 12:
        case 13:
            //aec = true;
            break;

        case 14:    // TODO: paint a byte x cycle
        case 15:
        case 16:
        case 17:
        case 18:
        case 19:
        case 20:
        case 21:
        case 22:
        case 23:
        case 24:
        case 25:
        case 26:
        case 27:
        case 28:
        case 29:
        case 30:
        case 31:
        case 32:
        case 33:
        case 34:
        case 35:
        case 36:
        case 37:
        case 38:
        case 39:
        case 40:
        case 41:
        case 42:
        case 43:
        case 44:
        case 45:
        case 46:
        case 47:
        case 48:
        case 49:
            aec = !_is_badline;
            break;

        case 50:
            aec = !_is_badline;
            paint_scanline(line); //FIXME
            break;

        case 51:
        case 52:
        case 53:
        case 54:
            aec = !_is_badline;
            break;

        case 55:
        case 56:
            //aec = true;
            break;

        case 57:
        case 58:
            aec = !is_mib_visible(line, 0);
            break;

        case 59:
        case 60:
            aec = !is_mib_visible(line, 1);
            break;

        case 61:
        case 62:
            aec = !is_mib_visible(line, 2);
            break;
        }

        ++_cycle;
    }

    aec_out(aec);

    if (_cycle == SCANLINE_CYCLES) {
        _cycle = 0;
        ++line;
        if (line == FRAME_HEIGHT) {
            line = 0;
            if (_vsync) {
                /*
                 * Synchronise the caller clock with a screen frame (~50Hz).
                 */
                _vsync(FRAME_CYCLES);
            }
        }

        rasterline(line);
    }

    return (is_vblank ? SCANLINE_CYCLES : 1);
}

void Mos6569::paint_scanline(unsigned line)
{
    if (line >= VISIBLE_Y_START && line < VISIBLE_Y_END) {

        paint(0, WIDTH, {0, 0, 0});

        unsigned start = DISPLAY_Y_START + display_scroll_y();

        if (line >= start && line < start + DISPLAY_HEIGHT) {
            if (!_is_den) {
                /*
                 * A blanked screen is painted in border colour.
                 */
                paint_border(0, WIDTH);

            } else if (!(is_display_extended_color() && is_display_multicolor())) {
                /*
                 * Paint the display area only when the extended color mode
                 * and multi color mode are not set both at the same time.
                 */
                unsigned offset_x = display_scroll_x();
                if (offset_x > 0) {
                    /*
                     * Horizontal scrolled pixels are painted in background colour.
                     */
                    paint(DISPLAY_X_START, offset_x, background_color(0));
                }

                if (is_display_bitmap_mode()) {
                    paint_bitmap_mode(line - start);
                } else {
                    paint_char_mode(line - start);
                }
            }
        }

        /* TODO: implement open-border bug */

        /*
         * Sprites.
         */
        if (line >= MIB_Y_START) {
            paint_sprites(line);
        }

        /*
         * Upper and bottom border columns.
         */
        if (line < UBORDER_Y_END || line >= BBORDER_Y_START) {
            paint_border(DISPLAY_X_START, DISPLAY_WIDTH);
        }

        /*
         * Left and right borders.
         */
        unsigned left_border_end = DISPLAY_X_START;
        unsigned right_border_start = DISPLAY_X_END;
        if (is_display_38_columns()) {
            left_border_end += 8;
            right_border_start -= 8;
        }

        paint_border(0, left_border_end);
        paint_border(right_border_start, 0);

        /*
         * Borders in 24 rows mode cover an already painted background.
         */
        if (is_display_24_rows()) {
            if (line < DISPLAY_Y_START + 7 || line >= DISPLAY_Y_END - 7) {
                paint_border(0, WIDTH);
            }
        }

        /*
         * Render the scanline.
         */
        unsigned sline = line - VISIBLE_Y_START;
        render_line(sline, _scanline);

        reset_collision_data();
        reset_collision_mib();
    }
}

void Mos6569::irq_out(bool active)
{
    if (active) {
        _regs[REG_INTERRUPT] |= REG_INTERRUPT_IRQ;
    } else {
        _regs[REG_INTERRUPT] &= ~REG_INTERRUPT_IRQ;
    }

    if (_trigger_irq) {
        _trigger_irq(active);
    }
}

void Mos6569::trigger_lp()
{
    if (!_lp_triggered) {
        _regs[REG_LIGHT_PEN_X] = static_cast<uint8_t>((_cycle << 3) >> 1);
        _regs[REG_LIGHT_PEN_Y] = _regs[REG_RASTER_COUNTER];

        _regs[REG_INTERRUPT] |= REG_INTERRUPT_ILP;

        _lp_triggered = true;

        if ((_regs[REG_INTERRUPT] & REG_INTERRUPT_IRQ) == 0 && (_regs[REG_INTERRUPT_ENABLE] & REG_INTERRUPT_ILP)) {
            irq_out(true);
        }
    }
}

void Mos6569::update_interrupts()
{
    auto &iflags = _regs[REG_INTERRUPT];

    if (rasterline() == _stored_rasterline) {
        iflags |= REG_INTERRUPT_IRST;
    }

    if (is_mib_data_collision()) {
        iflags |= REG_INTERRUPT_IMDC;
    }

    if (is_mib_mib_collision()) {
        iflags |= REG_INTERRUPT_IMMC;
    }

    uint8_t irq_pin = iflags & REG_INTERRUPT_IRQ;
    if (irq_pin == 0 && (iflags & _regs[REG_INTERRUPT_ENABLE]) != 0) {
        irq_out(true);
    }
}

void Mos6569::paint_byte(unsigned start, uint8_t bitmap, const color2_t &colors)
{
    if (start < _scanline.size()) {
        const auto &bg_color = *colors[0];
        const auto &fg_color = *colors[1];
        uint8_t bit = 128;

        for (auto it = _scanline.begin() + start; bit != 0 && it != _scanline.end(); ++it, bit >>= 1) {
            it->set((bitmap & bit) ? fg_color : bg_color);
        }
    }
}

void Mos6569::paint_mcm_byte(unsigned start, uint8_t bitmap, const color4_t &colors)
{
    if (start + 1 < _scanline.size()) {
        auto it = _scanline.begin() + start;

        for (int shift = 6; shift >= 0; shift -= 2) {
            uint8_t index = (bitmap >> shift) & 3;
            auto *color = colors[index];
            it->set(*color);
            if (++it == _scanline.end()) {
                break;
            }

            it->set(*color);
            if (++it == _scanline.end()) {
                break;
            }
        }
    }
}

void Mos6569::paint_char_mode(unsigned line)
{
    bool mcm = is_display_multicolor();
    bool ecm = is_display_extended_color();

    unsigned row = (line >> 3);
    addr_t ch_addr = video_matrix() + row * CHARMODE_COLUMNS;

    for (unsigned col = 0; col < CHARMODE_COLUMNS; ++col) {
        Color fg_code = video_color_code(col, row);
        int bg = 0;

        addr_t ch = _mmap->read(ch_addr + col);
        if (ecm) {
            bg = ch >> 6;
            ch &= 63;
        }

        uint8_t ch_row_data = _mmap->read(char_base(ch) + (line & 7));

        auto start = DISPLAY_X_START + (col << 3);

        if (mcm && fg_code > 7) {
            /*
             * Multi-color mode.
             */
           const color4_t colors = {
                &background_color(0),
                &background_color(1),
                &background_color(2),
                &_palette[fg_code & 7]
            };

            paint_mcm_byte_scroll(start, ch_row_data, colors);
            update_collision_data_mcm_scroll(start, ch_row_data);

        } else {
            /*
             * Hi-Res or extended-background-color mode.
             */
            color2_t colors = {
                &background_color(bg),
                &_palette[fg_code]
            };

            paint_byte_scroll(DISPLAY_X_START + (col << 3), ch_row_data, colors);
            update_collision_data_scroll(start, ch_row_data);
        }
    }
}

void Mos6569::paint_bitmap_mode(unsigned line)
{
    bool mcm = is_display_multicolor();
    unsigned row = (line >> 3);
    addr_t color_code_addr = video_matrix() + row * CHARMODE_COLUMNS;

    for (unsigned col = 0; col < CHARMODE_COLUMNS; ++col) {
        uint8_t color_code = _mmap->read(color_code_addr + col);
        const Rgba &fg_color = _palette[color_code >> 4];
        const Rgba &bg_color = _palette[color_code & Color::MASK];

        uint8_t byte = _mmap->read(bitmap_base() + row * DISPLAY_WIDTH + (col << 3) + (line & 7));

        auto start = DISPLAY_X_START + (col << 3);

        if (mcm) {
            /*
             * Multi-color bitmap mode.
             */
            const color4_t colors = {
                &background_color(0),
                &fg_color,
                &bg_color,
                &video_color(col, row)
            };

            paint_mcm_byte_scroll(start, byte, colors);
            update_collision_data_mcm_scroll(start, byte);

        } else {
            /*
             * Hi-Res bitmap mode.
             */
            paint_byte_scroll(start, byte, {&bg_color, &fg_color});
            update_collision_data_scroll(start, byte);
        }
    }
}

void Mos6569::paint_sprite_line(unsigned start, uint64_t bitmap, const color2_t &colors, bool expand)
{
    paint_byte(start,      (bitmap >> 56),       colors);
    paint_byte(start + 8,  (bitmap >> 48) & 255, colors);
    paint_byte(start + 16, (bitmap >> 40) & 255, colors);

    if (expand) {
        paint_byte(start + 24, (bitmap >> 32) & 255, colors);
        paint_byte(start + 32, (bitmap >> 24) & 255, colors);
        paint_byte(start + 40, (bitmap >> 16) & 255, colors);
    }
}

void Mos6569::paint_sprite_line_mcm(unsigned start, uint64_t bitmap, const color4_t &colors, bool expand)
{
    paint_mcm_byte(start,      (bitmap >> 56),       colors);
    paint_mcm_byte(start + 8,  (bitmap >> 48) & 255, colors);
    paint_mcm_byte(start + 16, (bitmap >> 40) & 255, colors);

    if (expand) {
        paint_mcm_byte(start + 24, (bitmap >> 32) & 255, colors);
        paint_mcm_byte(start + 32, (bitmap >> 24) & 255, colors);
        paint_mcm_byte(start + 40, (bitmap >> 16) & 255, colors);
    }
}

void Mos6569::paint_sprite(unsigned line, uint8_t mib)
{
    auto [pos_y, max_y, exp_y] = mib_visibility_y(line, mib);
    if (pos_y == static_cast<unsigned>(-1)) {
        return;
    }

    uint8_t sbit = (1 << mib);

    bool exp_x = is_mib_expanded_x(sbit);
    unsigned pos_x = mib_position_x(mib);

    addr_t mib_data = mib_base(mib);

    unsigned l = line - pos_y;
    if (exp_y) {
        /*
         * Vertical expansion.
         */
        l >>= 1;
    }

    addr_t addr = mib_data + l * 3;
    uint8_t byte1 = _mmap->read(addr);
    uint8_t byte2 = _mmap->read(addr + 1);
    uint8_t byte3 = _mmap->read(addr + 2);

    uint8_t color_code = _regs[REG_MIB_0_COLOR + mib] & Color::MASK;
    const auto &fg_color = _palette[color_code];
    const auto &bg_color = Rgba::transparent;

    bool data_pri = is_mib_behind_data(sbit);
    bool mcm = is_mib_multicolor(sbit);

    auto [dcollision, bitmap, visible_bitmap] = mib_bitmap(pos_x, byte1, byte2, byte3, exp_x, mcm, data_pri);

    if (mcm) {
        /*
         * Multi-color mode (4 colours).
         */
        const color4_t colors = {
            &bg_color,
            &_palette[_regs[REG_MIB_MULTICOLOR_0] & Color::MASK],
            &fg_color,
            &_palette[_regs[REG_MIB_MULTICOLOR_1] & Color::MASK]
        };

        paint_sprite_line_mcm(pos_x, visible_bitmap, colors, exp_x);

    } else {
        /*
         * Hi-Res mode (2 colours).
         */
        paint_sprite_line(pos_x, visible_bitmap, {&bg_color, &fg_color}, exp_x);
    }

    if (dcollision && !is_mib_data_collision()) {
        /*
         * Flag only the first DATA-MIB collision.
         */
        mib_data_collision(sbit);
    }

    auto [mcollision, cmib] = update_collision_mib(mib, pos_x, mcm, bitmap);

    if (!is_mib_mib_collision() && mcollision) {
        /*
         * Flag only the first MIB-MIB collision.
         */
        mib_mib_collision(sbit, 1 << cmib);
    }
}

void Mos6569::paint_sprites(unsigned line)
{
    /*
     * MIB-MIB Priority: 0 highest (front), 7 lowest (behind).
     * Implemented by painting from MIB-7 to MIB-0.
     */
    for (uint8_t mib = 7; mib != 255; --mib) {
        paint_sprite(line, mib);
    }
}

void Mos6569::update_collision_data(unsigned start, uint8_t bitmap)
{
    auto start_byte   = start >> 3;
    uint8_t start_bit = start - (start_byte << 3);

    if (start_bit == 0) {
        _collision_data[start_byte] = bitmap;

    } else {
        uint8_t byte1 = bitmap >> start_bit;
        uint8_t byte2 = bitmap << (8 - start_bit);

        uint8_t mask  = 0xFF >> start_bit;

        uint8_t prev1 = _collision_data[start_byte]     & ~mask;
        uint8_t prev2 = _collision_data[start_byte + 1] &  mask;

        _collision_data[start_byte]     = byte1 | prev1;
        _collision_data[start_byte + 1] = byte2 | prev2;
    }
}

void Mos6569::update_collision_data_mcm(unsigned start, uint8_t bitmap)
{
    /*
     * In multicolor mode 00 and 01 codes are considered background colours (not foreground);
     * the following converts 01 into 00 and 10 into 11 so the proper collision mask is generated.
     */
    update_collision_data(start, utils::convert_01_10(bitmap));
}

std::tuple<unsigned, unsigned, bool> Mos6569::mib_visibility_y(unsigned line, uint8_t mib)
{
    uint8_t sbit = (1 << mib);

    if (is_mib_enabled(sbit)) {
        bool exp_y = is_mib_expanded_y(sbit);
        unsigned pos_y = mib_position_y(mib);
        unsigned max_y = pos_y + (MIB_HEIGHT << (exp_y ? 1 : 0));
        if (line >= pos_y && line < max_y) {
            return {pos_y, max_y, exp_y};
        }
    }

    return {static_cast<unsigned>(-1), {}, {}};
}

std::tuple<bool, uint64_t, uint64_t> Mos6569::mib_bitmap(unsigned start, uint8_t byte1, uint8_t byte2, uint8_t byte3,
    bool expand, bool mcm, bool data_pri)
{
    auto start_byte = start >> 3;
    auto start_bit  = start - (start_byte << 3);

    uint64_t bitmap{};
    uint64_t mask{};

    if (expand) {
        uint16_t w1, w2, w3;

        if (mcm) {
            w1 = utils::expand_dibits(byte1);
            w2 = utils::expand_dibits(byte2);
            w3 = utils::expand_dibits(byte3);
        } else {
            w1 = utils::expand_bits(byte1);
            w2 = utils::expand_bits(byte2);
            w3 = utils::expand_bits(byte3);
        }

        bitmap = (static_cast<uint64_t>(w1) << 48) |
                 (static_cast<uint64_t>(w2) << 32) |
                 (static_cast<uint64_t>(w3) << 16);

        mask = 0xFFFFFFFFFFFF0000UL;

    } else {
        bitmap = (static_cast<uint64_t>(byte1) << 56) |
                 (static_cast<uint64_t>(byte2) << 48) |
                 (static_cast<uint64_t>(byte3) << 40);

        mask = 0xFFFFFF0000000000UL;
    }

    uint64_t background     = be64toh(*reinterpret_cast<uint64_t *>(&_collision_data[start_byte])) << start_bit;
    uint64_t collision      = mask & background & bitmap;
    uint64_t visible_bitmap = ((collision && data_pri) ? ~collision & bitmap : bitmap);

    return {collision != 0, bitmap, visible_bitmap};
}

std::pair<bool, uint8_t> Mos6569::update_collision_mib(uint8_t mib, unsigned start, bool mcm, uint64_t bitmap)
{
    if (mcm) {
        /*
         * Multicolor sprites use 00 as transparent color (no content),
         * the following converts 01 and 10 into 11 so the proper collision data mask is generated.
         */
        gsl::span data{reinterpret_cast<uint8_t *>(&bitmap), sizeof(bitmap)};
        utils::convert_01_10_to_11(data);
    }

    _mib_bitmaps[mib] = bitmap;

    /*
     * Detect a MIB-MIB collision only if there are no other MIB-MIB collision.
     */
    if (!is_mib_mib_collision()) {

        for (uint8_t mib_c = 0; mib_c < _mib_bitmaps.size(); ++mib_c) {
            if (mib_c != mib && is_mib_enabled(mib_c)) {
                unsigned start_c = mib_position_x(mib_c);

                if (!((start_c + MIB_MAX_X_SIZE) < start || (start + MIB_MAX_X_SIZE) < start_c)) {

                    auto cbitmap = _mib_bitmaps[mib_c];
                    bitmap = _mib_bitmaps[mib];

                    int shift = static_cast<int>(start) - static_cast<int>(start_c);
                    if (shift < 0) {
                        bitmap <<= -shift;
                    } else {
                        cbitmap <<= shift;
                    }

                    if (cbitmap & bitmap) {
                        /*
                         * MIB-MIB collision detected.
                         */
                        return {true, mib_c};
                    }
                }
            }
        }
    }

    /*
     * No MIB-MIB collisions.
     */
    return {false, 0};
}

}
