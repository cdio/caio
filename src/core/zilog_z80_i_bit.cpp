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

const Z80::Instruction Z80::bit_instr_set[256] = {
    { "RLC B",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 00 */
    { "RLC C",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 01 */
    { "RLC D",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 02 */
    { "RLC E",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 03 */
    { "RLC H",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 04 */
    { "RLC L",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 05 */
    { "RLC (HL)",       Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 06 */
    { "RLC A",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 07 */
    { "RRC B",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 08 */
    { "RRC C",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 09 */
    { "RRC D",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 0A */
    { "RRC E",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 0B */
    { "RRC H",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 0C */
    { "RRC L",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 0D */
    { "RRC (HL)",       Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 0E */
    { "RRC A",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 0F */

    { "RL B",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 10 */
    { "RL C",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 11 */
    { "RL D",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 12 */
    { "RL E",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 13 */
    { "RL H",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 14 */
    { "RL L",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 15 */
    { "RL (HL)",        Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 16 */
    { "RL A",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 17 */
    { "RR B",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 18 */
    { "RR C",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 19 */
    { "RR D",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 1A */
    { "RR E",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 1B */
    { "RR H",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 1C */
    { "RR L",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 1D */
    { "RR (HL)",        Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 1E */
    { "RR A",           Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 1F */

    { "SLA B",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 20 */
    { "SLA C",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 21 */
    { "SLA D",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 22 */
    { "SLA E",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 23 */
    { "SLA H",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 24 */
    { "SLA L",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 25 */
    { "SLA (HL)",       Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 26 */
    { "SLA A",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 27 */
    { "SRA B",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 28 */
    { "SRA C",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 29 */
    { "SRA D",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 2A */
    { "SRA E",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 2B */
    { "SRA H",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 2C */
    { "SRA L",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 2D */
    { "SRA (HL)",       Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 2E */
    { "SRA A",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 2F */

    { "SLL B",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 30 */
    { "SLL C",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 31 */
    { "SLL D",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 32 */
    { "SLL E",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 33 */
    { "SLL H",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 34 */
    { "SLL L",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 35 */
    { "SLL (HL)",       Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 36 */
    { "SLL A",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 37 */
    { "SRL B",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 38 */
    { "SRL C",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 39 */
    { "SRL D",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 3A */
    { "SRL E",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 3B */
    { "SRL H",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 3C */
    { "SRL L",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 3D */
    { "SRL (HL)",       Z80::i_bit_sr,  ArgType::None,  15, 1   },  /* CB 3E */
    { "SRL A",          Z80::i_bit_sr,  ArgType::None,  8,  1   },  /* CB 3F */

    { "BIT 0, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 40 */
    { "BIT 0, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 41 */
    { "BIT 0, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 42 */
    { "BIT 0, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 43 */
    { "BIT 0, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 44 */
    { "BIT 0, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 45 */
    { "BIT 0, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 46 */
    { "BIT 0, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 47 */
    { "BIT 1, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 48 */
    { "BIT 1, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 49 */
    { "BIT 1, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 4A */
    { "BIT 1, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 4B */
    { "BIT 1, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 4C */
    { "BIT 1, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 4D */
    { "BIT 1, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 4E */
    { "BIT 1, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 4F */

    { "BIT 2, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 50 */
    { "BIT 2, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 51 */
    { "BIT 2, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 52 */
    { "BIT 2, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 53 */
    { "BIT 2, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 54 */
    { "BIT 2, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 55 */
    { "BIT 2, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 56 */
    { "BIT 2, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 57 */
    { "BIT 3, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 58 */
    { "BIT 3, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 59 */
    { "BIT 3, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 5A */
    { "BIT 3, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 5B */
    { "BIT 3, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 5C */
    { "BIT 3, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 5D */
    { "BIT 3, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 5E */
    { "BIT 3, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 5F */

    { "BIT 4, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 60 */
    { "BIT 4, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 61 */
    { "BIT 4, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 62 */
    { "BIT 4, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 63 */
    { "BIT 4, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 64 */
    { "BIT 4, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 65 */
    { "BIT 4, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 66 */
    { "BIT 4, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 67 */
    { "BIT 5, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 68 */
    { "BIT 5, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 69 */
    { "BIT 5, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 6A */
    { "BIT 5, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 6B */
    { "BIT 5, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 6C */
    { "BIT 5, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 6D */
    { "BIT 5, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 6E */
    { "BIT 5, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 6F */

    { "BIT 6, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 70 */
    { "BIT 6, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 71 */
    { "BIT 6, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 72 */
    { "BIT 6, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 73 */
    { "BIT 6, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 74 */
    { "BIT 6, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 75 */
    { "BIT 6, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 76 */
    { "BIT 6, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 77 */
    { "BIT 7, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 78 */
    { "BIT 7, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 79 */
    { "BIT 7, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 7A */
    { "BIT 7, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 7B */
    { "BIT 7, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 7C */
    { "BIT 7, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 7D */
    { "BIT 7, (HL)",    Z80::i_bit_b,   ArgType::None,  12, 1   },  /* CB 7E */
    { "BIT 7, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 7F */

    { "RES 0, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 80 */
    { "RES 0, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 81 */
    { "RES 0, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 82 */
    { "RES 0, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 83 */
    { "RES 0, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 84 */
    { "RES 0, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 85 */
    { "RES 0, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB 86 */
    { "RES 0, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 87 */
    { "RES 1, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 88 */
    { "RES 1, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 89 */
    { "RES 1, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 8A */
    { "RES 1, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 8B */
    { "RES 1, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 8C */
    { "RES 1, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 8D */
    { "RES 1, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB 8E */
    { "RES 1, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 8F */

    { "RES 2, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 90 */
    { "RES 2, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 91 */
    { "RES 2, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 92 */
    { "RES 2, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 93 */
    { "RES 2, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 94 */
    { "RES 2, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 95 */
    { "RES 2, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB 96 */
    { "RES 2, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 97 */
    { "RES 3, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 98 */
    { "RES 3, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 99 */
    { "RES 3, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 9A */
    { "RES 3, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 9B */
    { "RES 3, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 9C */
    { "RES 3, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 9D */
    { "RES 3, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB 9E */
    { "RES 3, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB 9F */

    { "RES 4, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A0 */
    { "RES 4, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A1 */
    { "RES 4, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A2 */
    { "RES 4, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A3 */
    { "RES 4, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A4 */
    { "RES 4, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A5 */
    { "RES 4, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB A6 */
    { "RES 4, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A7 */
    { "RES 5, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A8 */
    { "RES 5, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB A9 */
    { "RES 5, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB AA */
    { "RES 5, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB AB */
    { "RES 5, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB AC */
    { "RES 5, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB AD */
    { "RES 5, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB AE */
    { "RES 5, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB AF */

    { "RES 6, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B0 */
    { "RES 6, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B1 */
    { "RES 6, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B2 */
    { "RES 6, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B3 */
    { "RES 6, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B4 */
    { "RES 6, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B5 */
    { "RES 6, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB B6 */
    { "RES 6, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B7 */
    { "RES 7, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B8 */
    { "RES 7, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB B9 */
    { "RES 7, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB BA */
    { "RES 7, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB BB */
    { "RES 7, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB BC */
    { "RES 7, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB BD */
    { "RES 7, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB BE */
    { "RES 7, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB BF */

    { "SET 0, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C0 */
    { "SET 0, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C1 */
    { "SET 0, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C2 */
    { "SET 0, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C3 */
    { "SET 0, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C4 */
    { "SET 0, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C5 */
    { "SET 0, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB C6 */
    { "SET 0, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C7 */
    { "SET 1, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C8 */
    { "SET 1, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB C9 */
    { "SET 1, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB CA */
    { "SET 1, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB CB */
    { "SET 1, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB CC */
    { "SET 1, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB CD */
    { "SET 1, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB CE */
    { "SET 1, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB CF */

    { "SET 2, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D0 */
    { "SET 2, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D1 */
    { "SET 2, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D2 */
    { "SET 2, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D3 */
    { "SET 2, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D4 */
    { "SET 2, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D5 */
    { "SET 2, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB D6 */
    { "SET 2, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D7 */
    { "SET 3, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D8 */
    { "SET 3, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB D9 */
    { "SET 3, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB DA */
    { "SET 3, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB DB */
    { "SET 3, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB DC */
    { "SET 3, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB DD */
    { "SET 3, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB DE */
    { "SET 3, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB DF */

    { "SET 4, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E0 */
    { "SET 4, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E1 */
    { "SET 4, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E2 */
    { "SET 4, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E3 */
    { "SET 4, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E4 */
    { "SET 4, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E5 */
    { "SET 4, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB E6 */
    { "SET 4, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E7 */
    { "SET 5, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E8 */
    { "SET 5, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB E9 */
    { "SET 5, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB EA */
    { "SET 5, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB EB */
    { "SET 5, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB EC */
    { "SET 5, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB ED */
    { "SET 5, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB EE */
    { "SET 5, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB EF */

    { "SET 6, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F0 */
    { "SET 6, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F1 */
    { "SET 6, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F2 */
    { "SET 6, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F3 */
    { "SET 6, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F4 */
    { "SET 6, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F5 */
    { "SET 6, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB F6 */
    { "SET 6, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F7 */
    { "SET 7, B",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F8 */
    { "SET 7, C",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB F9 */
    { "SET 7, D",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB FA */
    { "SET 7, E",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB FB */
    { "SET 7, H",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB FC */
    { "SET 7, L",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB FD */
    { "SET 7, (HL)",    Z80::i_bit_b,   ArgType::None,  15, 1   },  /* CB FE */
    { "SET 7, A",       Z80::i_bit_b,   ArgType::None,  8,  1   },  /* CB FF */
};

void Z80::rlc(uint8_t& reg)
{
    /*
     * RLC r
     * The contents of register r are rotated left 1 bit position.
     * The contents of bit 7 are copied to the Carry flag and also to bit 0.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 7 of source register.
     */
    bool b7 = (reg & 0x80);
    reg = (reg << 1) | b7;
    flag_C(b7);
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::rrc(uint8_t& reg)
{
    /*
     * RRC
     * The contents of the m operand are rotated right 1 bit position.
     * The contents of bit 0 are copied to the Carry flag and also to bit 7
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 0 of source register.
     */
    uint8_t b0 = ((reg & 0x01) ? 0x80 : 0x00);
    reg =(reg >> 1) | b0;
    flag_C(b0);
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::rl(uint8_t& reg)
{
    /*
     * RL
     * The contents of the m operand are rotated left 1 bit position.
     * The contents of bit 7 are copied to the Carry flag,
     * and the previous contents of the Carry flag are copied to bit 0.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 7 of source register.
     */
    uint8_t cy = test_C();
    flag_C(reg & 0x80);
    reg = (reg << 1) | cy;
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::rr(uint8_t& reg)
{
    /*
     * RR
     * The contents of operand m are rotated right 1 bit position through the Carry flag.
     * The contents of bit 0 are copied to the Carry flag and
     * the previous contents of the Carry flag are copied to bit 7
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 0 of source register.
     */
    bool b0 = (reg & 0x01);
    reg = (reg >> 1) | (test_C() ? 0x80 : 0x00);
    flag_C(b0);
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::sla(uint8_t& reg)
{
    /*
     * SLA
     * An arithmetic shift left 1 bit position is performed on the contents of operand m.
     * The contents of bit 7 are copied to the Carry flag.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity is even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 7.
     */
    flag_C(reg & 0x80);
    reg <<= 1;
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::sra(uint8_t& reg)
{
    /*
     * SRA
     * An arithmetic shift right 1 bit position is performed on the contents of operand m.
     * The contents of bit 0 are copied to the Carry flag and
     * the previous contents of bit 7 remain unchanged.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity is even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 0 of source register.
     */
    flag_C(reg & 0x01);
    uint8_t b7 = reg & 0x80;
    reg = (reg >> 1) | b7;
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::sll(uint8_t& reg)
{
    /*
     * SLL - Undocumented instruction.
     * SL1
     *
     * The contents of r are shifted left one bit position.
     * The contents of bit 7 are put into the carry flag and a one is put into bit 0.
     *
     * C as defined
     * N reset
     * P/V detects parity
     * H reset
     * Z as defined
     * S as defined
     */
    flag_C(reg & 0x80);
    reg = (reg << 1) | 1;
    flag_S(reg & 0x80);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

void Z80::srl(uint8_t& reg)
{
    /*
     * SRL
     * The contents of operand m are shifted right 1 bit position.
     * The contents of bit 0 are copied to the Carry flag, and bit 7 is reset.
     *
     * S is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity is even; otherwise, it is reset.
     * N is reset.
     * C is data from bit 0 of source register.
     */
    flag_C(reg & 0x01);
    reg >>= 1;
    flag_S(0);
    flag_Z(reg == 0);
    flag_H(0);
    flag_V(parity(reg));
    flag_N(0);
    flag_Y(reg & Flags::Y);
    flag_X(reg & Flags::X);
}

uint8_t Z80::bit_from_opcode(uint8_t op)
{
    /*
     * XXbbbXXX
     */
    constexpr static uint8_t BIT_SHIFT = 3;
    constexpr static uint8_t BIT_MASK = 0x07;
    uint8_t bit = (op >> BIT_SHIFT) & BIT_MASK;
    return (1 << bit);
}

int Z80::i_bit_sr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * RLC {ABCDEHL}    - CB {07 00 01 02 03 04 05}
     * RRC {ABCDEHL}    - CB {0F 08 09 0A 0B 0C 0D}
     * RL  {ABCDEHL}    - CB {17 10 11 12 13 14 15}
     * RR  {ABCDEHL}    - CB {1F 18 19 1A 1B 1C 1D}
     * SLA {ABCDEHL}    - CB {27 20 21 22 23 24 25}
     * SRA {ABCDEHL}    - CB {2F 28 29 2A 2B 2C 2D}
     * SLL {ABCDEHL}    - CB {37 30 31 32 33 34 35} - Undocumented
     * SRL {ABCDEHL}    - CB {3F 38 39 3A 3B 3C 3D}
     *
     * RLC (HL)         - CB 06
     * RRC (HL)         - CB 0E
     * RL  (HL)         - CB 16
     * RR  (HL)         - CB 1E
     * SLA (HL)         - CB 26
     * SRA (HL)         - CB 2E
     * SLL (HL)         - CB 36 - Undocumented
     * SRL (HL)         - CB 3E
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * ------+------- ---+----
     *       |           +-----> Source register:
     *       |                   000 = 0x00 => B
     *       |                   001 = 0x01 => C
     *       |                   010 = 0x02 => D
     *       |                   011 = 0x03 => E
     *       |                   100 = 0x04 => H
     *       |                   101 = 0x05 => L
     *       |                   110 = 0x06 => *(HL)
     *       |                   111 = 0x07 => A
     *       |
     *       +-----------------> OP:
     *                           00000 = 0x00 => RLC
     *                           00001 = 0x08 => RRC
     *                           00010 = 0x10 => RL
     *                           00011 = 0x18 => RR
     *                           00100 = 0x20 => SLA
     *                           00101 = 0x28 => SRA
     *                           00110 = 0x30 => SLL    - Undocumented
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

    uint8_t data{};
    uint8_t& reg = self.reg8_src_from_opcode(op, data);
    bool noreg = (&reg == &data);
    if (noreg) {
        /*
         * OP (HL)
         */
        data = self.read(self._regs.HL);
    }

    switch (op & OP_MASK) {
    case OP_RLC:
        self.rlc(reg);
        break;

    case OP_RRC:
        self.rrc(reg);
        break;

    case OP_RL:
        self.rl(reg);
        break;

    case OP_RR:
        self.rr(reg);
        break;

    case OP_SLA:
        self.sla(reg);
        break;

    case OP_SRA:
        self.sra(reg);
        break;

    case OP_SLL:
        self.sll(reg);
        break;

    case OP_SRL:
        self.srl(reg);
        break;

    default:
        arg = self._iaddr;
        log.error("Z80: i_bit_sr: Invalid opcode: CB {:02X}, addr: ${:04X}\n{}\n", op, arg, self.disass(arg));
        return 0;
    }

    if (noreg) {
        /*
         * OP (HL)
         */
        self.write(self._regs.HL, data);
    }

    return 0;
}

int Z80::i_bit_b(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * BIT b, {ABCDEHL} - CB 01bbbrrr
     * RES b, {ABCDEHL} - CB 10bbbrrr
     * SET b, {ABCDEHL} - CB 11bbbrrr
     *
     * BIT b, (HL)      - CB 01bbb110
     * RES b, (HL)      - CB 10bbb110
     * SET b, (HL)      - CB 11bbb110
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * --+-- ---+---- ----+---
     *   |      |         +----> Register:
     *   |      |                000 = 0x00 => B
     *   |      |                001 = 0x01 => C
     *   |      |                010 = 0x02 => D
     *   |      |                011 = 0x03 => E
     *   |      |                100 = 0x04 => H
     *   |      |                101 = 0x05 => L
     *   |      |                110 = 0x06 => *(HL)
     *   |      |                111 = 0x07 => A
     *   |      |
     *   |      +--------------> Bit no: b = 0..7
     *   |
     *   +---------------------> OP: 01 = 0x40 => BIT
     *                               10 = 0x80 => RES
     *                               11 = 0xC0 => SET
     */
    constexpr static uint8_t OP_MASK  = 0xC0;
    constexpr static uint8_t OP_BIT   = 0x40;
    constexpr static uint8_t OP_RES   = 0x80;
    constexpr static uint8_t OP_SET   = 0xC0;
    constexpr static uint8_t REG_MASK = 0x07;
    constexpr static uint8_t NOREG    = 0x06;

    uint8_t data{};
    uint8_t& reg = self.reg8_src_from_opcode(op, data);

    bool noreg = ((op & REG_MASK) == NOREG);
    if (noreg) {
        /*
         * OP b, (HL)
         */
        data = self.read(self._regs.HL);
    }

    uint8_t bit = self.bit_from_opcode(op);
    uint8_t result{};

    switch (op & OP_MASK) {
    case OP_BIT:
        result = reg & bit;
        self.flag_S(result == 0x80);
        self.flag_Z(result == 0);
        self.flag_H(1);
        self.flag_V(result == 0);
        self.flag_N(0);
        if (noreg) {
            /*
             * BIT b, (HL)
             */
            self.flag_Y(self._regs.memptr & (Flags::Y << 8));
            self.flag_X(self._regs.memptr & (Flags::X << 8));
        } else {
            /*
             * BIT b, r
             */
            self.flag_Y(result & Flags::Y);
            self.flag_X(result & Flags::X);
        }
        return 0;
        /* NOTREACHED */

    case OP_RES:
        reg &= ~bit;
        break;

    case OP_SET:
        reg |= bit;
        break;

    default:
        arg = self._iaddr;
        log.error("Z80: i_bit_b: Invalid opcode: CB {:02X}, addr: ${:04X}\n{}\n", op, arg, self.disass(arg));
        return 0;
    }

    if (noreg) {
        /*
         * RES b, (HL)
         * SET b, (HL)
         */
        self.write(self._regs.HL, data);
    }

    return 0;
}

}
}
