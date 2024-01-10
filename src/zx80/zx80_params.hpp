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


namespace caio {
namespace sinclair {
namespace zx80 {

constexpr static const unsigned CLOCK_FREQ       = 3250000;

constexpr static const char* ROM_FNAME           = "zx80_rom.bin";
constexpr static const char* ROM_DIGEST          = "9374711CB6A5FD53C1C98D1D20ECB444D01D621A322F3D066EF216D515AC5161";
constexpr static const size_t ROM_SIZE           = 4096;
constexpr static const addr_t ROM4_MAIN_EXEC     = 0x0283;
constexpr static const addr_t ROM4_LOAD_2_4      = 0x020E;
constexpr static const addr_t ROM4_LOAD_4_4      = 0x0226;
constexpr static const addr_t ROM4_LOAD_6_3      = 0x0234;
constexpr static const addr_t ROM4_SAVE_1_4      = 0x01BE;
constexpr static const addr_t ROM4_SAVE_5_6      = 0x01E0;

constexpr static const char* ROM8_FNAME          = "zx81_rom.bin";
constexpr static const char* ROM8_DIGEST         = "14AD84F4243EFCD41587FF46AB932D11087043E8D455A1ED2A227B9657828DFA";
constexpr static const size_t ROM8_SIZE          = 8192;
constexpr static const addr_t ROM8_MAIN_EXEC     = 0x0419;
constexpr static const addr_t ROM8_SLOW_FAST     = 0x0207;
constexpr static const addr_t ROM8_NEXT_LINE     = 0x066C;
constexpr static const addr_t ROM8_NEXT_LINE_10  = ROM8_NEXT_LINE + 10;
constexpr static const addr_t ROM8_IN_BYTE_8     = 0x0354;
constexpr static const addr_t ROM8_GET_BIT_8     = 0x038D;
constexpr static const addr_t ROM8_BREAK_1_4     = 0x0F4A;
constexpr static const addr_t ROM8_SYSVAR_ERR_NR = 0x4000;
constexpr static const addr_t ROM8_SYSVAR_FLAGS  = 0x4001;

constexpr static const uint64_t RAM_INIT_PATTERN = 0x0000000000000000ULL;
constexpr static const size_t INTERNAL_RAM_SIZE  = 1024;
constexpr static const size_t EXTERNAL_RAM_SIZE  = 16384;
constexpr static const addr_t RAMTOP_16K         = 0x8000;
constexpr static const addr_t RAMTOP_1K          = 0x4000;

}
}
}
