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
namespace commodore {
namespace c64 {

constexpr static const unsigned CLOCK_FREQ        = 985248;

constexpr static const char* KERNAL_FNAME         = "c64_kernal.901227-03.bin";
constexpr static const char* KERNAL_DIGEST        = "83C60D47047D7BEAB8E5B7BF6F67F80DAA088B7A6A27DE0D7E016F6484042721";

constexpr static const char* BASIC_FNAME          = "c64_basic.901226-01.bin";
constexpr static const char* BASIC_DIGEST         = "89878CEA0A268734696DE11C4BAE593EAAA506465D2029D619C0E0CBCCDFA62D";

constexpr static const char* CHARGEN_FNAME        = "c64_characters.901225-01.bin";
constexpr static const char* CHARGEN_DIGEST       = "FD0D53B8480E86163AC98998976C72CC58D5DD8EB824ED7B829774E74213B420";

constexpr static const size_t KERNAL_SIZE         = 8192;
constexpr static const size_t BASIC_SIZE          = 8192;
constexpr static const size_t CHARGEN_SIZE        = 4096;
constexpr static const size_t VRAM_SIZE           = 1024;
constexpr static const size_t RAM_SIZE            = 65536;

constexpr static const addr_t BASIC_READY_ADDR    = 0xA474;     /* Basic waiting for user commands              */
constexpr static const addr_t BASIC_PRG_START     = 0x0801;     /* Start address of basic programs              */

/* See https://www.c64-wiki.com/wiki/Zeropage */
constexpr static const addr_t BASIC_TXTTAB        = 0x002B;     /* Pointer to start of basic program            */
constexpr static const addr_t BASIC_VARTAB        = 0x002D;     /* Pointer to end of basic program +1           */
constexpr static const addr_t BASIC_ARYTAB        = 0x002F;     /* Pointer to start of basic array variables    */
constexpr static const addr_t BASIC_STREND        = 0x0031;     /* Pointer to end of basic array variables      */
constexpr static const addr_t BASIC_KEYB_BUFF     = 0x0277;     /* Keyboard buffer used by basic (10 bytes)     */
constexpr static const addr_t BASIC_KEYB_BUFF_POS = 0x00C6;     /* Number of elements in the keyboard buffer    */

/*
 * RAM initialisation patterns.
 * See the comments here: https://csdb.dk/forums/?roomid=11&topicid=116800&showallposts=1
 */
constexpr static const uint64_t RAM_INIT_PATTERN1 = 0x00FF00FF00FF00FFULL;
constexpr static const uint64_t RAM_INIT_PATTERN2 = 0xFFFFFFFF00000000ULL;
constexpr static const uint64_t RAM_INIT_PATTERN3 = 0x0000FFFFFFFF0000ULL;

}
}
}
