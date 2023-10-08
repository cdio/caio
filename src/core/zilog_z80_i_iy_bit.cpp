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
#include "zilog_z80.hpp"


namespace caio {
namespace zilog {

/*
 * Most of the information in the following tables come from:
 * - Z80 CPU User Manual UM008011-0816.
 * - https://clrhome.org/table/
 */
const std::array<Z80::Instruction, 256> Z80::iy_bit_instr_set{{
    { "RLC (IY%), B",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 00 */ /* Undocumented */
    { "RLC (IY%), C",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 01 */ /* Undocumented */
    { "RLC (IY%), D",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 02 */ /* Undocumented */
    { "RLC (IY%), E",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 03 */ /* Undocumented */
    { "RLC (IY%), H",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 04 */ /* Undocumented */
    { "RLC (IY%), L",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 05 */ /* Undocumented */
    { "RLC (IY%)",      Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 06 */
    { "RLC (IY%), A",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 07 */ /* Undocumented */
    { "RRC (IY%), B",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 08 */ /* Undocumented */
    { "RRC (IY%), C",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 09 */ /* Undocumented */
    { "RRC (IY%), D",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 0A */ /* Undocumented */
    { "RRC (IY%), E",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 0B */ /* Undocumented */
    { "RRC (IY%), H",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 0C */ /* Undocumented */
    { "RRC (IY%), L",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 0D */ /* Undocumented */
    { "RRC (IY%)",      Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 0E */
    { "RRC (IY%), A",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 0F */ /* Undocumented */

    { "RL (IY%), B",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 10 */ /* Undocumented */
    { "RL (IY%), C",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 11 */ /* Undocumented */
    { "RL (IY%), D",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 12 */ /* Undocumented */
    { "RL (IY%), E",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 13 */ /* Undocumented */
    { "RL (IY%), H",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 14 */ /* Undocumented */
    { "RL (IY%), L",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 15 */ /* Undocumented */
    { "RL (IY%)",       Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 16 */
    { "RL (IY%), A",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 17 */ /* Undocumented */
    { "RR (IY%), B",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 18 */ /* Undocumented */
    { "RR (IY%), C",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 19 */ /* Undocumented */
    { "RR (IY%), D",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 1A */ /* Undocumented */
    { "RR (IY%), E",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 1B */ /* Undocumented */
    { "RR (IY%), H",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 1C */ /* Undocumented */
    { "RR (IY%), L",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 1D */ /* Undocumented */
    { "RR (IY%)",       Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 1E */
    { "RR (IY%), A",    Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 1F */ /* Undocumented */

    { "SLA (IY%), B",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 20 */ /* Undocumented */
    { "SLA (IY%), C",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 21 */ /* Undocumented */
    { "SLA (IY%), D",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 22 */ /* Undocumented */
    { "SLA (IY%), E",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 23 */ /* Undocumented */
    { "SLA (IY%), H",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 24 */ /* Undocumented */
    { "SLA (IY%), L",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 25 */ /* Undocumented */
    { "SLA (IY%)",      Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 26 */
    { "SLA (IY%), A",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 27 */ /* Undocumented */
    { "SRA (IY%), B",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 28 */ /* Undocumented */
    { "SRA (IY%), C",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 29 */ /* Undocumented */
    { "SRA (IY%), D",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 2A */ /* Undocumented */
    { "SRA (IY%), E",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 2B */ /* Undocumented */
    { "SRA (IY%), H",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 2C */ /* Undocumented */
    { "SRA (IY%), L",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 2D */ /* Undocumented */
    { "SRA (IY%)",      Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 2E */
    { "SRA (IY%), A",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 2F */ /* Undocumented */

    { "SLL (IY%), B",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 30 */ /* Undocumented */
    { "SLL (IY%), C",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 31 */ /* Undocumented */
    { "SLL (IY%), D",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 32 */ /* Undocumented */
    { "SLL (IY%), E",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 33 */ /* Undocumented */
    { "SLL (IY%), H",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 34 */ /* Undocumented */
    { "SLL (IY%), L",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 35 */ /* Undocumented */
    { "SLL (IY%)",      Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 36 */ /* Undocumented */
    { "SLL (IY%), A",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 37 */ /* Undocumented */
    { "SRL (IY%), B",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 38 */ /* Undocumented */
    { "SRL (IY%), C",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 39 */ /* Undocumented */
    { "SRL (IY%), D",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 3A */ /* Undocumented */
    { "SRL (IY%), E",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 3B */ /* Undocumented */
    { "SRL (IY%), H",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 3C */ /* Undocumented */
    { "SRL (IY%), L",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 3D */ /* Undocumented */
    { "SRL (IY%)",      Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 3E */
    { "SRL (IY%), A",   Z80::i_iy_bit_sr,   ArgType::A8,    23, 4   },  /* FD CB 3F */ /* Undocumented */

    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 40 */ /* Undocumented */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 41 */ /* Undocumented */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 42 */ /* Undocumented */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 43 */ /* Undocumented */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 44 */ /* Undocumented */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 45 */ /* Undocumented */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 46 */
    { "BIT 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 47 */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 48 */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 49 */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 4A */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 4B */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 4C */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 4D */ /* Undocumented */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 4E */
    { "BIT 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 4F */ /* Undocumented */

    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 50 */ /* Undocumented */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 51 */ /* Undocumented */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 52 */ /* Undocumented */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 53 */ /* Undocumented */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 54 */ /* Undocumented */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 55 */ /* Undocumented */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 56 */
    { "BIT 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 57 */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 58 */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 59 */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 5A */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 5B */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 5C */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 5D */ /* Undocumented */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 5E */
    { "BIT 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 5F */ /* Undocumented */

    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 60 */ /* Undocumented */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 61 */ /* Undocumented */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 62 */ /* Undocumented */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 63 */ /* Undocumented */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 64 */ /* Undocumented */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 65 */ /* Undocumented */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 66 */
    { "BIT 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 67 */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 68 */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 69 */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 6A */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 6B */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 6C */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 6D */ /* Undocumented */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 6E */
    { "BIT 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 6F */ /* Undocumented */

    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 70 */ /* Undocumented */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 71 */ /* Undocumented */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 72 */ /* Undocumented */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 73 */ /* Undocumented */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 74 */ /* Undocumented */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 75 */ /* Undocumented */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 76 */
    { "BIT 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 77 */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 78 */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 79 */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 7A */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 7B */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 7C */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 7D */ /* Undocumented */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 7E */
    { "BIT 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    20, 4   },  /* FD CB 7F */ /* Undocumented */

    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 80 */ /* Undocumented */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 81 */ /* Undocumented */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 82 */ /* Undocumented */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 83 */ /* Undocumented */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 84 */ /* Undocumented */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 85 */ /* Undocumented */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 86 */
    { "RES 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 87 */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 88 */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 89 */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 8A */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 8B */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 8C */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 8D */ /* Undocumented */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 8E */
    { "RES 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 8F */ /* Undocumented */

    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 90 */ /* Undocumented */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 91 */ /* Undocumented */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 92 */ /* Undocumented */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 93 */ /* Undocumented */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 94 */ /* Undocumented */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 95 */ /* Undocumented */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 96 */
    { "RES 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 97 */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 98 */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 99 */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 9A */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 9B */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 9C */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 9D */ /* Undocumented */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 9E */
    { "RES 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB 9F */ /* Undocumented */

    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A0 */ /* Undocumented */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A1 */ /* Undocumented */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A2 */ /* Undocumented */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A3 */ /* Undocumented */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A4 */ /* Undocumented */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A5 */ /* Undocumented */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A6 */
    { "RES 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A7 */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A8 */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB A9 */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB AA */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB AB */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB AC */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB AD */ /* Undocumented */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB AE */
    { "RES 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB AF */ /* Undocumented */

    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B0 */ /* Undocumented */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B1 */ /* Undocumented */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B2 */ /* Undocumented */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B3 */ /* Undocumented */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B4 */ /* Undocumented */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B5 */ /* Undocumented */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B6 */
    { "RES 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B7 */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B8 */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB B9 */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB BA */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB BB */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB BC */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB BD */ /* Undocumented */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB BE */
    { "RES 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB BF */ /* Undocumented */

    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C0 */ /* Undocumented */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C1 */ /* Undocumented */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C2 */ /* Undocumented */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C3 */ /* Undocumented */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C4 */ /* Undocumented */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C5 */ /* Undocumented */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C6 */
    { "SET 0, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C7 */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C8 */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB C9 */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB CA */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB CB */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB CC */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB CD */ /* Undocumented */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB CE */
    { "SET 1, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB CF */ /* Undocumented */

    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D0 */ /* Undocumented */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D1 */ /* Undocumented */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D2 */ /* Undocumented */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D3 */ /* Undocumented */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D4 */ /* Undocumented */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D5 */ /* Undocumented */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D6 */
    { "SET 2, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D7 */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D8 */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB D9 */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB DA */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB DB */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB DC */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB DD */ /* Undocumented */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB DE */
    { "SET 3, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB DF */ /* Undocumented */

    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E0 */ /* Undocumented */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E1 */ /* Undocumented */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E2 */ /* Undocumented */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E3 */ /* Undocumented */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E4 */ /* Undocumented */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E5 */ /* Undocumented */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E6 */
    { "SET 4, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E7 */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E8 */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB E9 */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB EA */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB EB */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB EC */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB ED */ /* Undocumented */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB EE */
    { "SET 5, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB EF */ /* Undocumented */

    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F0 */ /* Undocumented */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F1 */ /* Undocumented */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F2 */ /* Undocumented */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F3 */ /* Undocumented */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F4 */ /* Undocumented */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F5 */ /* Undocumented */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F6 */
    { "SET 6, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F7 */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F8 */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB F9 */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB FA */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB FB */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB FC */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB FD */ /* Undocumented */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB FE */
    { "SET 7, (IY%)",   Z80::i_iy_bit,      ArgType::A8,    23, 4   },  /* FD CB FF */ /* Undocumented */
}};

int Z80::i_iy_bit_sr(Z80& self, uint8_t op, addr_t arg)
{
    return self.xx_bit_sr(self._regs.IY, op, arg);
}

int Z80::i_iy_bit(Z80& self, uint8_t op, addr_t arg)
{
    return self.xx_bit(self._regs.IY, op, arg);
}

}
}
