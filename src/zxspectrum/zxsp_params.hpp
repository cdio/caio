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
namespace zxspectrum {

constexpr static const unsigned CLOCK_FREQ       = 3500000;

constexpr static const char* ROM_FNAME           = "zxspectrum48_rom.bin";
constexpr static const char* ROM_DIGEST          = "D55DAA439B673B0E3F5897F99AC37ECB45F974D1862B4DADB85DEC34AF99CB42";
constexpr static const addr_t ROM_SIZE           = 16384;
constexpr static const addr_t ROM_RETN_ADDR      = 0x0072;

constexpr static const addr_t RAM_SIZE           = 49152;
constexpr static const uint64_t RAM_INIT_PATTERN = 0x0000000000000000ULL;
constexpr static const addr_t RAM_BASE_ADDRESS   = 0x4000;

constexpr static const addr_t SA_LD_RET_ADDR     = 0x053F;
constexpr static const addr_t REPORT_Da_ADDR     = 0x0552;
constexpr static const addr_t LD_BYTES_ADDR      = 0x0556;
constexpr static const addr_t LD_LOOK_H_ADDR     = 0x0767;

}
}
}
