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

const Z80::Instruction Z80::ix_bit_instr_set[256] = {
    { "RLC (IX%), B",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 00 */
    { "RLC (IX%), C",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 01 */
    { "RLC (IX%), D",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 02 */
    { "RLC (IX%), E",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 03 */
    { "RLC (IX%), H",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 04 */
    { "RLC (IX%), L",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 05 */
    { "RLC (IX%)",          Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 06 */
    { "RLC (IX%), A",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 07 */
    { "RRC (IX%), B",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 08 */
    { "RRC (IX%), C",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 09 */
    { "RRC (IX%), D",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 0A */
    { "RRC (IX%), E",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 0B */
    { "RRC (IX%), H",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 0C */
    { "RRC (IX%), L",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 0D */
    { "RRC (IX%)",          Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 0E */
    { "RRC (IX%), A",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 0F */

    { "RL (IX%), B",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 10 */
    { "RL (IX%), C",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 11 */
    { "RL (IX%), D",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 12 */
    { "RL (IX%), E",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 13 */
    { "RL (IX%), H",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 14 */
    { "RL (IX%), L",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 15 */
    { "RL (IX%)",           Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 16 */
    { "RL (IX%), A",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 17 */
    { "RR (IX%), B",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 18 */
    { "RR (IX%), C",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 19 */
    { "RR (IX%), D",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 1A */
    { "RR (IX%), E",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 1B */
    { "RR (IX%), H",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 1C */
    { "RR (IX%), L",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 1D */
    { "RR (IX%)",           Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 1E */
    { "RR (IX%), A",        Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 1F */

    { "SLA (IX%), B",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 20 */
    { "SLA (IX%), C",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 21 */
    { "SLA (IX%), D",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 22 */
    { "SLA (IX%), E",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 23 */
    { "SLA (IX%), H",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 24 */
    { "SLA (IX%), L",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 25 */
    { "SLA (IX%)",          Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 26 */
    { "SLA (IX%), A",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 27 */
    { "SRA (IX%), B",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 28 */
    { "SRA (IX%), C",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 29 */
    { "SRA (IX%), D",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 2A */
    { "SRA (IX%), E",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 2B */
    { "SRA (IX%), H",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 2C */
    { "SRA (IX%), L",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 2D */
    { "SRA (IX%)",          Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 2E */
    { "SRA (IX%), A",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 2F */

    { "SLL (IX%), B",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 30 */
    { "SLL (IX%), C",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 31 */
    { "SLL (IX%), D",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 32 */
    { "SLL (IX%), E",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 33 */
    { "SLL (IX%), H",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 34 */
    { "SLL (IX%), L",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 35 */
    { "SLL (IX%)",          Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 36 */
    { "SLL (IX%), A",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 37 */
    { "SRL (IX%), B",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 38 */
    { "SRL (IX%), C",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 39 */
    { "SRL (IX%), D",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 3A */
    { "SRL (IX%), E",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 3B */
    { "SRL (IX%), H",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 3C */
    { "SRL (IX%), L",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 3D */
    { "SRL (IX%)",          Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 3E */
    { "SRL (IX%), A",       Z80::i_ix_bit_sr,   ArgType::A8_Inv,   15, 2   },  /* DD CB DD 3F */

    { "BIT 0, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 40 */
    { "BIT 0, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 41 */
    { "BIT 0, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 42 */
    { "BIT 0, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 43 */
    { "BIT 0, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 44 */
    { "BIT 0, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 45 */
    { "BIT 0, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 46 */
    { "BIT 0, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 47 */
    { "BIT 1, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 48 */
    { "BIT 1, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 49 */
    { "BIT 1, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 4A */
    { "BIT 1, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 4B */
    { "BIT 1, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 4C */
    { "BIT 1, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 4D */
    { "BIT 1, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 4E */
    { "BIT 1, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 4F */

    { "BIT 2, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 50 */
    { "BIT 2, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 51 */
    { "BIT 2, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 52 */
    { "BIT 2, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 53 */
    { "BIT 2, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 54 */
    { "BIT 2, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 55 */
    { "BIT 2, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 56 */
    { "BIT 2, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 57 */
    { "BIT 3, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 58 */
    { "BIT 3, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 59 */
    { "BIT 3, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 5A */
    { "BIT 3, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 5B */
    { "BIT 3, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 5C */
    { "BIT 3, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 5D */
    { "BIT 3, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 5E */
    { "BIT 3, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 5F */

    { "BIT 4, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 60 */
    { "BIT 4, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 61 */
    { "BIT 4, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 62 */
    { "BIT 4, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 63 */
    { "BIT 4, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 64 */
    { "BIT 4, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 65 */
    { "BIT 4, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 66 */
    { "BIT 4, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 67 */
    { "BIT 5, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 68 */
    { "BIT 5, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 69 */
    { "BIT 5, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 6A */
    { "BIT 5, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 6B */
    { "BIT 5, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 6C */
    { "BIT 5, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 6D */
    { "BIT 5, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 6E */
    { "BIT 5, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 6F */

    { "BIT 6, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 70 */
    { "BIT 6, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 71 */
    { "BIT 6, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 72 */
    { "BIT 6, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 73 */
    { "BIT 6, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 74 */
    { "BIT 6, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 75 */
    { "BIT 6, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 76 */
    { "BIT 6, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 77 */
    { "BIT 7, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 78 */
    { "BIT 7, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 79 */
    { "BIT 7, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 7A */
    { "BIT 7, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 7B */
    { "BIT 7, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 7C */
    { "BIT 7, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 7D */
    { "BIT 7, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 7E */
    { "BIT 7, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   12, 2   },  /* DD CB DD 7F */

    { "RES 0, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 80 */
    { "RES 0, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 81 */
    { "RES 0, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 82 */
    { "RES 0, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 83 */
    { "RES 0, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 84 */
    { "RES 0, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 85 */
    { "RES 0, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 86 */
    { "RES 0, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 87 */
    { "RES 1, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 88 */
    { "RES 1, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 89 */
    { "RES 1, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 8A */
    { "RES 1, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 8B */
    { "RES 1, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 8C */
    { "RES 1, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 8D */
    { "RES 1, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 8E */
    { "RES 1, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 8F */

    { "RES 2, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 90 */
    { "RES 2, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 91 */
    { "RES 2, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 92 */
    { "RES 2, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 93 */
    { "RES 2, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 94 */
    { "RES 2, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 95 */
    { "RES 2, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 96 */
    { "RES 2, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 97 */
    { "RES 3, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 98 */
    { "RES 3, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 99 */
    { "RES 3, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 9A */
    { "RES 3, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 9B */
    { "RES 3, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 9C */
    { "RES 3, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 9D */
    { "RES 3, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 9E */
    { "RES 3, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD 9F */

    { "RES 4, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A0 */
    { "RES 4, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A1 */
    { "RES 4, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A2 */
    { "RES 4, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A3 */
    { "RES 4, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A4 */
    { "RES 4, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A5 */
    { "RES 4, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A6 */
    { "RES 4, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A7 */
    { "RES 5, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A8 */
    { "RES 5, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD A9 */
    { "RES 5, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD AA */
    { "RES 5, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD AB */
    { "RES 5, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD AC */
    { "RES 5, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD AD */
    { "RES 5, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD AE */
    { "RES 5, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD AF */

    { "RES 6, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B0 */
    { "RES 6, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B1 */
    { "RES 6, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B2 */
    { "RES 6, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B3 */
    { "RES 6, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B4 */
    { "RES 6, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B5 */
    { "RES 6, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B6 */
    { "RES 6, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B7 */
    { "RES 7, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B8 */
    { "RES 7, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD B9 */
    { "RES 7, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD BA */
    { "RES 7, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD BB */
    { "RES 7, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD BC */
    { "RES 7, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD BD */
    { "RES 7, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD BE */
    { "RES 7, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD BF */

    { "SET 0, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C0 */
    { "SET 0, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C1 */
    { "SET 0, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C2 */
    { "SET 0, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C3 */
    { "SET 0, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C4 */
    { "SET 0, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C5 */
    { "SET 0, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C6 */
    { "SET 0, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C7 */
    { "SET 1, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C8 */
    { "SET 1, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD C9 */
    { "SET 1, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD CA */
    { "SET 1, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD CB */
    { "SET 1, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD CC */
    { "SET 1, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD CD */
    { "SET 1, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD CE */
    { "SET 1, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD CF */

    { "SET 2, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D0 */
    { "SET 2, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D1 */
    { "SET 2, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D2 */
    { "SET 2, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D3 */
    { "SET 2, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D4 */
    { "SET 2, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D5 */
    { "SET 2, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D6 */
    { "SET 2, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D7 */
    { "SET 3, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D8 */
    { "SET 3, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD D9 */
    { "SET 3, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD DA */
    { "SET 3, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD DB */
    { "SET 3, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD DC */
    { "SET 3, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD DD */
    { "SET 3, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD DE */
    { "SET 3, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD DF */

    { "SET 4, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E0 */
    { "SET 4, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E1 */
    { "SET 4, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E2 */
    { "SET 4, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E3 */
    { "SET 4, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E4 */
    { "SET 4, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E5 */
    { "SET 4, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E6 */
    { "SET 4, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E7 */
    { "SET 5, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E8 */
    { "SET 5, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD E9 */
    { "SET 5, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD EA */
    { "SET 5, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD EB */
    { "SET 5, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD EC */
    { "SET 5, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD ED */
    { "SET 5, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD EE */
    { "SET 5, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD EF */

    { "SET 6, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F0 */
    { "SET 6, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F1 */
    { "SET 6, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F2 */
    { "SET 6, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F3 */
    { "SET 6, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F4 */
    { "SET 6, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F5 */
    { "SET 6, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F6 */
    { "SET 6, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F7 */
    { "SET 7, (IX%), B",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F8 */
    { "SET 7, (IX%), C",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD F9 */
    { "SET 7, (IX%), D",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD FA */
    { "SET 7, (IX%), E",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD FB */
    { "SET 7, (IX%), H",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD FC */
    { "SET 7, (IX%), L",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD FD */
    { "SET 7, (IX%)",       Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD FE */
    { "SET 7, (IX%), A",    Z80::i_ix_bit,      ArgType::A8_Inv,   15, 2   },  /* DD CB DD FF */
};

int Z80::xx_bit_sr(uint16_t& reg, uint8_t op, addr_t arg)
{
    /*
     * RLC (IX/IY+d), r     - DD/FD CB dddddddd ooooorrr
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
     *       |                   000 = 0x00 => B    Undocumented
     *       |                   001 = 0x01 => C    Undocumented
     *       |                   010 = 0x02 => D    Undocumented
     *       |                   011 = 0x03 => E    Undocumented
     *       |                   100 = 0x04 => H    Undocumented
     *       |                   101 = 0x05 => L    Undocumented
     *       |                   110 = 0x06 => No source register
     *       |                   111 = 0x07 => A    Undocumented
     *       |
     *       +-----------------> OP:
     *                           00000 = 0x00 => RLC
     *                           00001 = 0x08 => RRC
     *                           00010 = 0x10 => RL
     *                           00011 = 0x18 => RR
     *                           00100 = 0x20 => SLA
     *                           00101 = 0x28 => SRA
     *                           00110 = 0x30 => SLL    Undocumented
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
        addr = _iaddr;
        log.error("Z80: BIT_sr: A8_Invalid opcode: %s CB %02X, addr: $04X\n",
            (_iprefix == Prefix::IX ? "DD " : (_iprefix == Prefix::IY ? "FD " : "")),
            op, addr, disass(addr).c_str(), disass(addr).c_str(), disass(addr).c_str());
        return 0;
    }

    write(addr, data);
    uint8_t& rr = reg8_src_from_opcode(op);
    rr = data;
    return 0;
}

int Z80::xx_bit(uint16_t& reg, uint8_t op, addr_t arg)
{
    /*
     * BIT b, (IX/IY+d), [{A,B,C,D,E,H,L}]  - DD/FD CB dddddddd 01bbbrrr
     * RES b, (IX/IY+d), [{A,B,C,D,E,H,L}]  - DD/FD CB dddddddd 10bbbrrr
     * SET b, (IX/IY+d), [{A,B,C,D,E,H,L}]  - DD/FD CB dddddddd 11bbbrrr
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * --+-- ---+---- ----+---
     *   |      |         |
     *   |      |         +----> Destination register:
     *   |      |                000 = 0x00 => B    Undocumented
     *   |      |                001 = 0x01 => C    Undocumented
     *   |      |                010 = 0x02 => D    Undocumented
     *   |      |                011 = 0x03 => E    Undocumented
     *   |      |                100 = 0x04 => H    Undocumented
     *   |      |                101 = 0x05 => L    Undocumented
     *   |      |                110 = 0x06 => No register
     *   |      |                111 = 0x07 => A    Undocumented
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
        flag_Y(addr & (static_cast<addr_t>(Flags::Y) << 8));
        flag_X(addr & (static_cast<addr_t>(Flags::X) << 8));
        return 0;
        /* NOTREACHED */

    case OP_RES:
        data &= ~bit;
        break;

    case OP_SET:
        data |= bit;
        break;

    default:
        addr = _iaddr;
        log.error("Z80: xx_bit: A8_Invalid opcode: %sCB %02X, addr: $%04X\n%s\n",
            (_iprefix == Prefix::IX ? "DD " : (_iprefix == Prefix::IY ? "FD " : "")),
            op, addr, disass(addr).c_str());
        return 0;
    }

    write(addr, data);
    uint8_t& rr = reg8_src_from_opcode(op);
    rr = data;
    return 0;
}

int Z80::i_ix_bit_sr(Z80& self, uint8_t op, addr_t arg)
{
    auto& reg = (self._iprefix == Prefix::IX ? self._regs.IX : self._regs.IY);
    return self.xx_bit_sr(reg, op, arg);
}

int Z80::i_ix_bit(Z80& self, uint8_t op, addr_t arg)
{
    auto& reg = (self._iprefix == Prefix::IX ? self._regs.IX : self._regs.IY);
    return self.xx_bit(reg, op, arg);
}

}
}
