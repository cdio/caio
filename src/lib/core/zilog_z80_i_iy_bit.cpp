/*
 * Copyright (C) 2020-2022 Claudio Castiglia
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
#include "zilog_z80.hpp"

#include "logger.hpp"


namespace caio {

/*
 * Most of the information in the following tables come from:
 * - Z80 CPU User Manual UM008011-0816.
 * - https://clrhome.org/table/
 */
const std::array<ZilogZ80::Instruction, 256> ZilogZ80::iy_bit_instr_set{{
    { "RLC (IY%), B",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 00 */ /* Undocumented */
    { "RLC (IY%), C",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 01 */ /* Undocumented */
    { "RLC (IY%), D",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 02 */ /* Undocumented */
    { "RLC (IY%), E",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 03 */ /* Undocumented */
    { "RLC (IY%), H",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 04 */ /* Undocumented */
    { "RLC (IY%), L",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 05 */ /* Undocumented */
    { "RLC (IY%)",      ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 06 */
    { "RLC (IY%), A",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 07 */ /* Undocumented */
    { "RRC (IY%), B",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 08 */ /* Undocumented */
    { "RRC (IY%), C",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 09 */ /* Undocumented */
    { "RRC (IY%), D",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 0A */ /* Undocumented */
    { "RRC (IY%), E",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 0B */ /* Undocumented */
    { "RRC (IY%), H",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 0C */ /* Undocumented */
    { "RRC (IY%), L",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 0D */ /* Undocumented */
    { "RRC (IY%)",      ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 0E */
    { "RRC (IY%), A",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 0F */ /* Undocumented */

    { "RL (IY%), B",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 10 */ /* Undocumented */
    { "RL (IY%), C",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 11 */ /* Undocumented */
    { "RL (IY%), D",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 12 */ /* Undocumented */
    { "RL (IY%), E",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 13 */ /* Undocumented */
    { "RL (IY%), H",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 14 */ /* Undocumented */
    { "RL (IY%), L",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 15 */ /* Undocumented */
    { "RL (IY%)",       ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 16 */
    { "RL (IY%), A",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 17 */ /* Undocumented */
    { "RR (IY%), B",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 18 */ /* Undocumented */
    { "RR (IY%), C",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 19 */ /* Undocumented */
    { "RR (IY%), D",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 1A */ /* Undocumented */
    { "RR (IY%), E",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 1B */ /* Undocumented */
    { "RR (IY%), H",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 1C */ /* Undocumented */
    { "RR (IY%), L",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 1D */ /* Undocumented */
    { "RR (IY%)",       ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 1E */
    { "RR (IY%), A",    ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 1F */ /* Undocumented */

    { "SLA (IY%), B",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 20 */ /* Undocumented */
    { "SLA (IY%), C",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 21 */ /* Undocumented */
    { "SLA (IY%), D",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 22 */ /* Undocumented */
    { "SLA (IY%), E",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 23 */ /* Undocumented */
    { "SLA (IY%), H",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 24 */ /* Undocumented */
    { "SLA (IY%), L",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 25 */ /* Undocumented */
    { "SLA (IY%)",      ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 26 */
    { "SLA (IY%), A",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 27 */ /* Undocumented */
    { "SRA (IY%), B",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 28 */ /* Undocumented */
    { "SRA (IY%), C",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 29 */ /* Undocumented */
    { "SRA (IY%), D",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 2A */ /* Undocumented */
    { "SRA (IY%), E",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 2B */ /* Undocumented */
    { "SRA (IY%), H",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 2C */ /* Undocumented */
    { "SRA (IY%), L",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 2D */ /* Undocumented */
    { "SRA (IY%)",      ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 2E */
    { "SRA (IY%), A",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 2F */ /* Undocumented */

    { "SLL (IY%), B",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 30 */ /* Undocumented */
    { "SLL (IY%), C",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 31 */ /* Undocumented */
    { "SLL (IY%), D",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 32 */ /* Undocumented */
    { "SLL (IY%), E",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 33 */ /* Undocumented */
    { "SLL (IY%), H",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 34 */ /* Undocumented */
    { "SLL (IY%), L",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 35 */ /* Undocumented */
    { "SLL (IY%)",      ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 36 */ /* Undocumented */
    { "SLL (IY%), A",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 37 */ /* Undocumented */
    { "SRL (IY%), B",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 38 */ /* Undocumented */
    { "SRL (IY%), C",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 39 */ /* Undocumented */
    { "SRL (IY%), D",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 3A */ /* Undocumented */
    { "SRL (IY%), E",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 3B */ /* Undocumented */
    { "SRL (IY%), H",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 3C */ /* Undocumented */
    { "SRL (IY%), L",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 3D */ /* Undocumented */
    { "SRL (IY%)",      ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 3E */
    { "SRL (IY%), A",   ZilogZ80::i_iy_bit_sr,  ArgType::A8,    6,  23, 4   },  /* DF CB 3F */ /* Undocumented */

    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 40 */ /* Undocumented */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 41 */ /* Undocumented */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 42 */ /* Undocumented */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 43 */ /* Undocumented */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 44 */ /* Undocumented */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 45 */ /* Undocumented */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 46 */
    { "BIT 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 47 */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 48 */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 49 */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 4A */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 4B */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 4C */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 4D */ /* Undocumented */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 4E */
    { "BIT 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 4F */ /* Undocumented */

    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 50 */ /* Undocumented */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 51 */ /* Undocumented */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 52 */ /* Undocumented */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 53 */ /* Undocumented */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 54 */ /* Undocumented */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 55 */ /* Undocumented */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 56 */
    { "BIT 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 57 */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 58 */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 59 */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 5A */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 5B */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 5C */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 5D */ /* Undocumented */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 5E */
    { "BIT 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 5F */ /* Undocumented */

    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 60 */ /* Undocumented */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 61 */ /* Undocumented */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 62 */ /* Undocumented */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 63 */ /* Undocumented */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 64 */ /* Undocumented */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 65 */ /* Undocumented */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 66 */
    { "BIT 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 67 */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 68 */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 69 */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 6A */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 6B */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 6C */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 6D */ /* Undocumented */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 6E */
    { "BIT 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 6F */ /* Undocumented */

    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 70 */ /* Undocumented */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 71 */ /* Undocumented */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 72 */ /* Undocumented */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 73 */ /* Undocumented */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 74 */ /* Undocumented */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 75 */ /* Undocumented */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 76 */
    { "BIT 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 77 */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 78 */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 79 */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 7A */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 7B */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 7C */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 7D */ /* Undocumented */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    3,  20, 4   },  /* DF CB 7E */
    { "BIT 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    5,  20, 4   },  /* DF CB 7F */ /* Undocumented */

    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 80 */ /* Undocumented */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 81 */ /* Undocumented */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 82 */ /* Undocumented */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 83 */ /* Undocumented */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 84 */ /* Undocumented */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 85 */ /* Undocumented */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 86 */
    { "RES 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 87 */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 88 */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 89 */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 8A */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 8B */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 8C */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 8D */ /* Undocumented */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 8E */
    { "RES 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 8F */ /* Undocumented */

    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 90 */ /* Undocumented */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 91 */ /* Undocumented */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 92 */ /* Undocumented */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 93 */ /* Undocumented */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 94 */ /* Undocumented */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 95 */ /* Undocumented */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 96 */
    { "RES 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 97 */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 98 */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 99 */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 9A */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 9B */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 9C */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 9D */ /* Undocumented */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 9E */
    { "RES 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB 9F */ /* Undocumented */

    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A0 */ /* Undocumented */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A1 */ /* Undocumented */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A2 */ /* Undocumented */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A3 */ /* Undocumented */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A4 */ /* Undocumented */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A5 */ /* Undocumented */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A6 */
    { "RES 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A7 */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A8 */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB A9 */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB AA */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB AB */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB AC */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB AD */ /* Undocumented */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB AE */
    { "RES 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB AF */ /* Undocumented */

    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B0 */ /* Undocumented */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B1 */ /* Undocumented */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B2 */ /* Undocumented */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B3 */ /* Undocumented */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B4 */ /* Undocumented */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B5 */ /* Undocumented */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B6 */
    { "RES 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B7 */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B8 */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB B9 */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB BA */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB BB */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB BC */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB BD */ /* Undocumented */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB BE */
    { "RES 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB BF */ /* Undocumented */

    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C0 */ /* Undocumented */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C1 */ /* Undocumented */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C2 */ /* Undocumented */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C3 */ /* Undocumented */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C4 */ /* Undocumented */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C5 */ /* Undocumented */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C6 */
    { "SET 0, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C7 */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C8 */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB C9 */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB CA */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB CB */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB CC */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB CD */ /* Undocumented */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB CE */
    { "SET 1, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB CF */ /* Undocumented */

    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D0 */ /* Undocumented */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D1 */ /* Undocumented */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D2 */ /* Undocumented */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D3 */ /* Undocumented */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D4 */ /* Undocumented */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D5 */ /* Undocumented */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D6 */
    { "SET 2, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D7 */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D8 */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB D9 */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB DA */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB DB */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB DC */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB DD */ /* Undocumented */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB DE */
    { "SET 3, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB DF */ /* Undocumented */

    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E0 */ /* Undocumented */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E1 */ /* Undocumented */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E2 */ /* Undocumented */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E3 */ /* Undocumented */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E4 */ /* Undocumented */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E5 */ /* Undocumented */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E6 */
    { "SET 4, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E7 */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E8 */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB E9 */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB EA */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB EB */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB EC */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB ED */ /* Undocumented */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB EE */
    { "SET 5, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB EF */ /* Undocumented */

    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F0 */ /* Undocumented */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F1 */ /* Undocumented */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F2 */ /* Undocumented */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F3 */ /* Undocumented */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F4 */ /* Undocumented */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F5 */ /* Undocumented */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F6 */
    { "SET 6, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F7 */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F8 */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB F9 */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB FA */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB FB */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB FC */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB FD */ /* Undocumented */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB FE */
    { "SET 7, (IY%)",   ZilogZ80::i_iy_bit,     ArgType::A8,    6,  23, 4   },  /* DF CB FF */ /* Undocumented */
}};


int ZilogZ80::i_iy_bit_sr(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    return self.xx_bit_sr(self._regs.IY, op, arg);
}

int ZilogZ80::i_iy_bit(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    return self.xx_bit(self._regs.IY, op, arg);
}

}
