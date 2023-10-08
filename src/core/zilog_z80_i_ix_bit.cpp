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
const std::array<Z80::Instruction, 256> Z80::ix_bit_instr_set{{
    { "RLC (IX%), B",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 00 */ /* Undocumented */
    { "RLC (IX%), C",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 01 */ /* Undocumented */
    { "RLC (IX%), D",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 02 */ /* Undocumented */
    { "RLC (IX%), E",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 03 */ /* Undocumented */
    { "RLC (IX%), H",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 04 */ /* Undocumented */
    { "RLC (IX%), L",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 05 */ /* Undocumented */
    { "RLC (IX%)",      Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 06 */
    { "RLC (IX%), A",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 07 */ /* Undocumented */
    { "RRC (IX%), B",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 08 */ /* Undocumented */
    { "RRC (IX%), C",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 09 */ /* Undocumented */
    { "RRC (IX%), D",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 0A */ /* Undocumented */
    { "RRC (IX%), E",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 0B */ /* Undocumented */
    { "RRC (IX%), H",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 0C */ /* Undocumented */
    { "RRC (IX%), L",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 0D */ /* Undocumented */
    { "RRC (IX%)",      Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 0E */
    { "RRC (IX%), A",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 0F */ /* Undocumented */

    { "RL (IX%), B",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 10 */ /* Undocumented */
    { "RL (IX%), C",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 11 */ /* Undocumented */
    { "RL (IX%), D",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 12 */ /* Undocumented */
    { "RL (IX%), E",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 13 */ /* Undocumented */
    { "RL (IX%), H",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 14 */ /* Undocumented */
    { "RL (IX%), L",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 15 */ /* Undocumented */
    { "RL (IX%)",       Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 16 */
    { "RL (IX%), A",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 17 */ /* Undocumented */
    { "RR (IX%), B",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 18 */ /* Undocumented */
    { "RR (IX%), C",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 19 */ /* Undocumented */
    { "RR (IX%), D",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 1A */ /* Undocumented */
    { "RR (IX%), E",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 1B */ /* Undocumented */
    { "RR (IX%), H",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 1C */ /* Undocumented */
    { "RR (IX%), L",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 1D */ /* Undocumented */
    { "RR (IX%)",       Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 1E */
    { "RR (IX%), A",    Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 1F */ /* Undocumented */

    { "SLA (IX%), B",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 20 */ /* Undocumented */
    { "SLA (IX%), C",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 21 */ /* Undocumented */
    { "SLA (IX%), D",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 22 */ /* Undocumented */
    { "SLA (IX%), E",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 23 */ /* Undocumented */
    { "SLA (IX%), H",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 24 */ /* Undocumented */
    { "SLA (IX%), L",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 25 */ /* Undocumented */
    { "SLA (IX%)",      Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 26 */
    { "SLA (IX%), A",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 27 */ /* Undocumented */
    { "SRA (IX%), B",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 28 */ /* Undocumented */
    { "SRA (IX%), C",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 29 */ /* Undocumented */
    { "SRA (IX%), D",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 2A */ /* Undocumented */
    { "SRA (IX%), E",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 2B */ /* Undocumented */
    { "SRA (IX%), H",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 2C */ /* Undocumented */
    { "SRA (IX%), L",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 2D */ /* Undocumented */
    { "SRA (IX%)",      Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 2E */
    { "SRA (IX%), A",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 2F */ /* Undocumented */

    { "SLL (IX%), B",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 30 */ /* Undocumented */
    { "SLL (IX%), C",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 31 */ /* Undocumented */
    { "SLL (IX%), D",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 32 */ /* Undocumented */
    { "SLL (IX%), E",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 33 */ /* Undocumented */
    { "SLL (IX%), H",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 34 */ /* Undocumented */
    { "SLL (IX%), L",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 35 */ /* Undocumented */
    { "SLL (IX%)",      Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 36 */ /* Undocumented */
    { "SLL (IX%), A",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 37 */ /* Undocumented */
    { "SRL (IX%), B",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 38 */ /* Undocumented */
    { "SRL (IX%), C",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 39 */ /* Undocumented */
    { "SRL (IX%), D",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 3A */ /* Undocumented */
    { "SRL (IX%), E",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 3B */ /* Undocumented */
    { "SRL (IX%), H",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 3C */ /* Undocumented */
    { "SRL (IX%), L",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 3D */ /* Undocumented */
    { "SRL (IX%)",      Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 3E */
    { "SRL (IX%), A",   Z80::i_ix_bit_sr,   ArgType::A8,    23, 4   },  /* DD CB 3F */ /* Undocumented */

    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 40 */ /* Undocumented */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 41 */ /* Undocumented */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 42 */ /* Undocumented */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 43 */ /* Undocumented */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 44 */ /* Undocumented */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 45 */ /* Undocumented */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 46 */
    { "BIT 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 47 */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 48 */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 49 */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 4A */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 4B */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 4C */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 4D */ /* Undocumented */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 4E */
    { "BIT 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 4F */ /* Undocumented */

    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 50 */ /* Undocumented */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 51 */ /* Undocumented */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 52 */ /* Undocumented */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 53 */ /* Undocumented */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 54 */ /* Undocumented */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 55 */ /* Undocumented */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 56 */
    { "BIT 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 57 */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 58 */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 59 */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 5A */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 5B */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 5C */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 5D */ /* Undocumented */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 5E */
    { "BIT 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 5F */ /* Undocumented */

    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 60 */ /* Undocumented */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 61 */ /* Undocumented */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 62 */ /* Undocumented */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 63 */ /* Undocumented */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 64 */ /* Undocumented */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 65 */ /* Undocumented */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 66 */
    { "BIT 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 67 */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 68 */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 69 */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 6A */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 6B */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 6C */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 6D */ /* Undocumented */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 6E */
    { "BIT 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 6F */ /* Undocumented */

    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 70 */ /* Undocumented */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 71 */ /* Undocumented */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 72 */ /* Undocumented */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 73 */ /* Undocumented */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 74 */ /* Undocumented */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 75 */ /* Undocumented */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 76 */
    { "BIT 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 77 */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 78 */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 79 */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 7A */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 7B */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 7C */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 7D */ /* Undocumented */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 7E */
    { "BIT 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    20, 4   },  /* DD CB 7F */ /* Undocumented */

    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 80 */ /* Undocumented */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 81 */ /* Undocumented */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 82 */ /* Undocumented */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 83 */ /* Undocumented */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 84 */ /* Undocumented */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 85 */ /* Undocumented */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 86 */
    { "RES 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 87 */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 88 */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 89 */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 8A */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 8B */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 8C */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 8D */ /* Undocumented */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 8E */
    { "RES 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 8F */ /* Undocumented */

    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 90 */ /* Undocumented */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 91 */ /* Undocumented */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 92 */ /* Undocumented */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 93 */ /* Undocumented */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 94 */ /* Undocumented */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 95 */ /* Undocumented */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 96 */
    { "RES 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 97 */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 98 */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 99 */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 9A */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 9B */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 9C */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 9D */ /* Undocumented */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 9E */
    { "RES 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB 9F */ /* Undocumented */

    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A0 */ /* Undocumented */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A1 */ /* Undocumented */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A2 */ /* Undocumented */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A3 */ /* Undocumented */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A4 */ /* Undocumented */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A5 */ /* Undocumented */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A6 */
    { "RES 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A7 */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A8 */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB A9 */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB AA */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB AB */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB AC */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB AD */ /* Undocumented */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB AE */
    { "RES 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB AF */ /* Undocumented */

    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B0 */ /* Undocumented */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B1 */ /* Undocumented */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B2 */ /* Undocumented */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B3 */ /* Undocumented */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B4 */ /* Undocumented */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B5 */ /* Undocumented */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B6 */
    { "RES 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B7 */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B8 */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB B9 */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB BA */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB BB */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB BC */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB BD */ /* Undocumented */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB BE */
    { "RES 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB BF */ /* Undocumented */

    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C0 */ /* Undocumented */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C1 */ /* Undocumented */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C2 */ /* Undocumented */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C3 */ /* Undocumented */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C4 */ /* Undocumented */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C5 */ /* Undocumented */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C6 */
    { "SET 0, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C7 */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C8 */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB C9 */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB CA */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB CB */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB CC */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB CD */ /* Undocumented */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB CE */
    { "SET 1, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB CF */ /* Undocumented */

    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D0 */ /* Undocumented */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D1 */ /* Undocumented */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D2 */ /* Undocumented */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D3 */ /* Undocumented */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D4 */ /* Undocumented */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D5 */ /* Undocumented */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D6 */
    { "SET 2, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D7 */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D8 */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB D9 */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB DA */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB DB */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB DC */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB DD */ /* Undocumented */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB DE */
    { "SET 3, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB DF */ /* Undocumented */

    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E0 */ /* Undocumented */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E1 */ /* Undocumented */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E2 */ /* Undocumented */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E3 */ /* Undocumented */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E4 */ /* Undocumented */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E5 */ /* Undocumented */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E6 */
    { "SET 4, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E7 */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E8 */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB E9 */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB EA */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB EB */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB EC */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB ED */ /* Undocumented */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB EE */
    { "SET 5, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB EF */ /* Undocumented */

    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F0 */ /* Undocumented */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F1 */ /* Undocumented */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F2 */ /* Undocumented */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F3 */ /* Undocumented */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F4 */ /* Undocumented */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F5 */ /* Undocumented */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F6 */
    { "SET 6, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F7 */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F8 */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB F9 */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB FA */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB FB */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB FC */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB FD */ /* Undocumented */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB FE */
    { "SET 7, (IX%)",   Z80::i_ix_bit,      ArgType::A8,    23, 4   },  /* DD CB FF */ /* Undocumented */
}};

int Z80::xx_bit_sr(uint16_t& reg, uint8_t op, addr_t arg)
{
    /*
     * RLC (IX/IY+d), r
     * RRC (IX/IY+d), r
     * RL  (IX/IY+d), r
     * RR  (IX/IY+d), r
     * SLA (IX/IY+d), r
     * SRA (IX/IY+d), r
     * SLL (IX/IY+d), r
     * SRL (IX/IY+d), r
     *
     * RLC (IX/IY+d)
     * RRC (IX/IY+d)
     * RL  (IX/IY+d)
     * RR  (IX/IY+d)
     * SLA (IX/IY+d)
     * SRA (IX/IY+d)
     * SLL (IX/IY+d)
     * SRL (IX/IY+d)
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * ------+------- ---+----
     *       |           +-----> Source register:
     *       |                   000 = 0x00 => B    => Undocumented
     *       |                   001 = 0x01 => C    => Undocumented
     *       |                   010 = 0x02 => D    => Undocumented
     *       |                   011 = 0x03 => E    => Undocumented
     *       |                   100 = 0x04 => H    => Undocumented
     *       |                   101 = 0x05 => L    => Undocumented
     *       |                   110 = 0x06 => No source register
     *       |                   111 = 0x07 => A    => Undocumented
     *       |
     *       +-----------------> OP:
     *                           00000 = 0x00 => RLC
     *                           00001 = 0x08 => RRC
     *                           00010 = 0x10 => RL
     *                           00011 = 0x18 => RR
     *                           00100 = 0x20 => SLA
     *                           00101 = 0x28 => SRA
     *                           00110 = 0x30 => SLL  => Undocumented operation
     *                           00111 = 0x38 => SRL
     */
    constexpr static uint8_t OP_MASK = 0xF8;
    constexpr static uint8_t OP_RLC  = 0x00;
    constexpr static uint8_t OP_RRC  = 0x08;
    constexpr static uint8_t OP_RL   = 0x10;
    constexpr static uint8_t OP_RR   = 0x18;
    constexpr static uint8_t OP_SLA  = 0x20;
    constexpr static uint8_t OP_SRA  = 0x28;
    constexpr static uint8_t OP_SLL  = 0x30;
    constexpr static uint8_t OP_SRL  = 0x38;

    addr_t addr = reg + arg;
    uint8_t data = read(addr);

    switch (op & OP_MASK) {
    case OP_RLC:
        rlc(data);
        break;

    case OP_RRC:
        rrc(data);
        break;

    case OP_RL:
        rl(data);
        break;

    case OP_RR:
        rr(data);
        break;

    case OP_SLA:
        sla(data);
        break;

    case OP_SRA:
        sra(data);
        break;

    case OP_SLL:
        sll(data);
        break;

    case OP_SRL:
        srl(data);
        break;

    default:
        log.error("Z80: IX/IY_BIT_sr: Unrecognised opcode: DD/FD CB %02X, addr: $04X\n", op, _regs.PC);
    }

    write(addr, data);

    if ((op & 0x07) != 0x06) {
        /*
         * Undocumented: Put result in the specified source register.
         */
        uint8_t dummy{};
        uint8_t& rr = reg8_src_from_opcode(op, dummy);
        rr = data;
    }

    return 0;
}

int Z80::xx_bit(uint16_t& reg, uint8_t op, addr_t arg)
{
    /*
     * BIT b, (IX/IY+d)
     * RES b, (IX/IY+d)
     * SET b, (IX/IY+d)
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * --+-- ---+---- ----+---
     *   |      |         +----> 110 = 0x06 => OP b, (IX/IY+d)
     *   |      |                Other combinations produce the same effect but are undocumented.
     *   |      |
     *   |      +--------------> Bit no: b = 0..7
     *   |
     *   +---------------------> OP: 01 = 0x40 => BIT
     *                               10 = 0x80 => RES
     *                               11 = 0xC0 => SET
     */
    constexpr static uint8_t OP_MASK = 0xC0;
    constexpr static uint8_t OP_BIT  = 0x40;
    constexpr static uint8_t OP_RES  = 0x80;
    constexpr static uint8_t OP_SET  = 0xC0;

    uint8_t bit = bit_from_opcode(op);

    addr_t addr = reg + arg;
    uint8_t data = read(addr);
    uint8_t result{};

    switch (op & OP_MASK) {
    case OP_BIT:
        result = data & bit;
        flag_S(result == 0x80);
        flag_Z(result == 0);
        flag_H(1);
        flag_V(result == 0);
        flag_N(0);
        flag_Y(addr & (Flags::Y << 8));
        flag_X(addr & (Flags::X << 8));
        return 0;
        /* NOTREACHED */

    case OP_RES:
        data &= ~bit;
        break;

    case OP_SET:
        data |= bit;
        break;

    default:
        log.error("Z80: IX/IY_BIT: Unrecognised opcode: DD/FD CB %02X, addr: $04X\n", op, _regs.PC);
    }

    write(addr, data);

    if ((op & 0x07) != 0x06) {
        /*
         * Undocumented: Put result in the specified source register.
         */
        uint8_t dummy{};
        uint8_t& rr = reg8_src_from_opcode(op, dummy);
        rr = data;
    }

    return 0;
}

int Z80::i_ix_bit_sr(Z80& self, uint8_t op, addr_t arg)
{
    return self.xx_bit_sr(self._regs.IX, op, arg);
}

int Z80::i_ix_bit(Z80& self, uint8_t op, addr_t arg)
{
    return self.xx_bit(self._regs.IX, op, arg);
}

}
}
