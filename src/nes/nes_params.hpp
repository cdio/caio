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
namespace nintendo {
namespace nes {

/* See recommended freq: https://www.nesdev.org/wiki/CPU */
//constexpr static const unsigned NTSC_FREQ        = 21'441'960;      /* vs. 21'477'272 */
constexpr static const unsigned NTSC_FREQ        = 21'476'160;      /* vs. 21'477'272 */
constexpr static const unsigned PPU_FREQ         = NTSC_FREQ / 4;   /* 5'369'040 vs. 5'369'318 */
constexpr static const size_t RAM_SIZE           = 2048;
constexpr static const size_t VRAM_SIZE          = 2048;
constexpr static const uint64_t RAM_INIT_PATTERN = 0x00FF00FF00FF00FF;

}
}
}
