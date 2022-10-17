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
const std::array<ZilogZ80::Instruction, 256> ZilogZ80::mi_instr_set{{
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 00 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 01 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 02 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 03 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 04 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 05 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 06 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 07 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 08 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 09 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 0A */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 0B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 0C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 0D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 0E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 0F */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 10 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 11 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 12 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 13 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 14 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 15 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 16 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 17 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 18 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 19 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 1A */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 1B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 1C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 1D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 1E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 1F */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 20 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 21 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 22 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 23 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 24 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 25 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 26 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 27 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 28 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 29 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 2A */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 2B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 2C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 2D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 2E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 2F */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 30 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 31 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 32 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 33 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 34 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 35 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 36 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 37 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 38 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 39 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 3A */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 3B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 3C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 3D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 3E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 3F */

    { "IN B, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 40 */
    { "OUT (C), B",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 41 */
    { "SBC HL, BC",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 42 */
    { "LD ($^), BC",    ZilogZ80::i_mi_LD_m_rr, ArgType::A16,   6,  20, 4   },  /* ED 43 */
    { "NEG",            ZilogZ80::i_mi_NEG,     ArgType::None,  2,  8,  2   },  /* ED 44 */
    { "RETN",           ZilogZ80::i_mi_RETN,    ArgType::None,  4,  14, 2   },  /* ED 45 */
    { "MI 0",           ZilogZ80::i_mi_IM_x,    ArgType::None,  2,  8,  2   },  /* ED 46 */
    { "LD I, A",        ZilogZ80::i_mi_LD_x_A,  ArgType::None,  2,  9,  2   },  /* ED 47 */
    { "IN C, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 48 */
    { "OUT (C), C",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 49 */
    { "ADC HL, BC",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 4A */
    { "LD BC, ($^)",    ZilogZ80::i_mi_LD_rr_m, ArgType::A16,   6,  20, 4   },  /* ED 4B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 4C */
    { "RETI",           ZilogZ80::i_mi_RETI,    ArgType::None,  4,  14, 2   },  /* ED 4D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 4E */
    { "LD R, A",        ZilogZ80::i_mi_LD_x_A,  ArgType::None,  2,  9,  2   },  /* ED 4F */

    { "IN D, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 50 */
    { "OUT (C), D",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 51 */
    { "SBC HL, DE",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 52 */
    { "LD ($^), DE",    ZilogZ80::i_mi_LD_m_rr, ArgType::A16,   6,  20, 4   },  /* ED 53 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 54 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 55 */
    { "MI 1",           ZilogZ80::i_mi_IM_x,    ArgType::None,  2,  8,  2   },  /* ED 56 */
    { "LD A, I",        ZilogZ80::i_mi_LD_A_x,  ArgType::None,  2,  9,  2   },  /* ED 57 */
    { "IN E, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 58 */
    { "OUT (C), E",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 59 */
    { "ADC HL, DE",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 5A */
    { "LD DE, ($^)",    ZilogZ80::i_mi_LD_rr_m, ArgType::A16,   6,  20, 4   },  /* ED 5B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 5C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 5D */
    { "IM 2",           ZilogZ80::i_mi_IM_x,    ArgType::None,  2,  8,  2   },  /* ED 5E */
    { "LD A, R",        ZilogZ80::i_mi_LD_A_x,  ArgType::None,  2,  9,  2   },  /* ED 5F */

    { "IN H, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 60 */
    { "OUT (C), H",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 61 */
    { "SBC HL, HL",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 62 */
    { "LD ($^), HL",    ZilogZ80::i_mi_LD_m_rr, ArgType::A16,   6,  20, 4   },  /* ED 63 */ /* Undocumented */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 64 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 65 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 66 */
    { "RRD",            ZilogZ80::i_mi_RRD,     ArgType::None,  5,  18, 2   },  /* ED 67 */
    { "IN L, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 68 */
    { "OUT (C), L",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 69 */
    { "ADC HL, HL",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 6A */
    { "LD HL, ($^)",    ZilogZ80::i_mi_LD_rr_m, ArgType::A16,   6,  20, 4   },  /* ED 6B */ /* Undocumented */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 6C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 6D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 6E */
    { "RLD",            ZilogZ80::i_mi_RLD,     ArgType::None,  5,  18, 2   },  /* ED 6F */

    { "IN (C)",         ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 70 */ /* Undocumented */
    { "OUT (C), 0",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 71 */ /* Undocumented */
    { "SBC HL, SP",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 72 */
    { "LD ($^), SP",    ZilogZ80::i_mi_LD_m_rr, ArgType::A16,   6,  20, 4   },  /* ED 73 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 74 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 75 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 76 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 77 */
    { "IN A, (C)",      ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 78 */
    { "OUT (C), A",     ZilogZ80::i_mi_io,      ArgType::None,  3,  12, 2   },  /* ED 79 */
    { "ADC HL, SP",     ZilogZ80::i_mi_adcsbc,  ArgType::None,  4,  15, 2   },  /* ED 7A */
    { "LD SP, ($^)",    ZilogZ80::i_mi_LD_rr_m, ArgType::A16,   6,  20, 4   },  /* ED 7B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 7C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 7D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 7E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 7F */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 80 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 81 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 82 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 83 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 84 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 85 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 86 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 87 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 88 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 89 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 8A */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 8B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 8C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 8D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 8E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 8F */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 90 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 91 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 92 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 93 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 94 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 95 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 96 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 97 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 98 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 99 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 9A */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 9B */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 9C */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 9D */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 9E */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED 9F */

    { "LDI",            ZilogZ80::i_mi_copy,    ArgType::None,  4,  16, 2   },  /* ED A0 */
    { "CPI",            ZilogZ80::i_mi_cmp,     ArgType::None,  4,  16, 2   },  /* ED A1 */
    { "INI",            ZilogZ80::i_mi_in,      ArgType::None,  4,  16, 2   },  /* ED A2 */
    { "OUTI",           ZilogZ80::i_mi_out,     ArgType::None,  4,  16, 2   },  /* ED A3 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED A4 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED A5 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED A6 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED A7 */
    { "LDD",            ZilogZ80::i_mi_copy,    ArgType::None,  4,  16, 2   },  /* ED A8 */
    { "CPD",            ZilogZ80::i_mi_cmp,     ArgType::None,  4,  16, 2   },  /* ED A9 */
    { "IND",            ZilogZ80::i_mi_in,      ArgType::None,  4,  16, 2   },  /* ED AA */
    { "OUTD",           ZilogZ80::i_mi_out,     ArgType::None,  4,  16, 2   },  /* ED AB */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED AC */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED AD */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED AE */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED AF */

    { "LDIR",           ZilogZ80::i_mi_copy,    ArgType::None,  5,  21, 2   },  /* ED B0 */
    { "CPIR",           ZilogZ80::i_mi_cmp,     ArgType::None,  5,  21, 2   },  /* ED B1 */
    { "INIR",           ZilogZ80::i_mi_in,      ArgType::None,  5,  21, 2   },  /* ED B2 */
    { "OTIR",           ZilogZ80::i_mi_out,     ArgType::None,  5,  21, 2   },  /* ED B3 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED B4 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED B5 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED B6 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED B7 */
    { "LDDR",           ZilogZ80::i_mi_copy,    ArgType::None,  5,  21, 2   },  /* ED B8 */
    { "CPDR",           ZilogZ80::i_mi_cmp,     ArgType::None,  5,  21, 2   },  /* ED B9 */
    { "INDR",           ZilogZ80::i_mi_in,      ArgType::None,  5,  21, 2   },  /* ED BA */
    { "OTDR",           ZilogZ80::i_mi_out,     ArgType::None,  5,  21, 2   },  /* ED BB */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED BC */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED BD */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED BE */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED BF */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C0 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C1 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C2 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C3 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C4 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C5 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C6 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C7 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C8 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED C9 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED CA */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED CB */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED CC */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED CD */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED CE */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED CF */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D0 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D1 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D2 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D3 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D4 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D5 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D6 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D7 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D8 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED D9 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED DA */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED DB */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED DC */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED DD */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED DE */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED DF */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E0 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E1 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E2 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E3 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E4 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E5 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E6 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E7 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E8 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED E9 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED EA */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED EB */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED EC */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED ED */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED EE */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED EF */

    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F0 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F1 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F2 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F3 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F4 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F5 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F6 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F7 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F8 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED F9 */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED FA */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED FB */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED FC */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED FD */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED FE */
    { "",               {},                     ArgType::Inv,   0,  0,  2   },  /* ED FF */
}};


int ZilogZ80::i_mi_io(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * IN {ABCDEHL}, (C)
     * IN (C)
     * S is set if input data is negative; otherwise, it is reset.
     * Z is set if input data is 0; otherwise, it is reset.
     * H is reset.
     * P/V is set if parity is even; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     *
     * OUT (C), {ABCDEHL}
     * OUT (C), 0
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * 0  1  r  r  r  0  0  0  => IN  (C), r
     * 0  1  r  r  r  0  0  1  => OUT (C), r
     *
     * rrr = 0x06 => (IN only affect flags, OUT writes 0) - Undocumented
     */
    constexpr static const uint8_t OP_MASK = 0xC7;
    constexpr static const uint8_t IN_OP   = 0x40;
    constexpr static const uint8_t OUT_OP  = 0x41;

    uint8_t data{};
    uint8_t &reg = self.reg8_from_opcode(op, data);
    uint8_t port = self._regs.C;

    switch (op & OP_MASK) {
    case IN_OP:
        reg = self.in(port);
        self.flag_S(reg & 0x80);
        self.flag_Z(reg == 0);
        self.flag_H(0);
        self.flag_V(self.parity(reg));
        self.flag_N(0);
        break;

    case OUT_OP:
        self.out(port, reg);
        break;

    default:
        log.error("Z80: MI: Unrecognised IN/OUT opcode: ED %02X, addr: $04X\n", op, self._regs.PC);
    }

    return 0;
}

void ZilogZ80::add16(uint16_t &reg, uint16_t value)
{
    /*
     * reg += value;
     *
     * S is not affected.
     * Z is not affected.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is not affected.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is reset.
     */
    unsigned result = static_cast<unsigned>(reg) + static_cast<unsigned>(value);
    flag_H((reg & (1 << 11)) ^ (result & (1 << 11)));
    flag_N(0);
    flag_C(result & 0x10000);
    reg = static_cast<uint16_t>(result & 0xFFFF);
}

void ZilogZ80::adc16(uint16_t &reg, uint16_t value)
{
    /*
     * reg += value + C
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * * N is reset.
     * C is set if carry from bit 15; otherwise, it is reset
     */
    int result = static_cast<int>(reg) + static_cast<int>(value) + test_C();
    flag_S(result < 0);
    flag_Z(result == 0);
    flag_H((reg & (1 << 11)) ^ (result & (1 << 11)));
    flag_V(test_C() ^ (result & 0x10000));
    flag_N(0);
    flag_C(result & 0x10000);
    reg = static_cast<uint16_t>(result & 0xFFFF);
}

void ZilogZ80::sbc16(uint16_t &reg, uint16_t value)
{
    /*
     * reg = reg - value - B
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 12; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is set.
     * C is set if borrow; otherwise, it is reset.
     */
    int result = static_cast<int>(reg) - static_cast<int>(value) - (!test_C());
    bool s1 = (reg & 0x8000);
    bool s2 = (value & 0x8000);
    bool sr = (result < 0);
    flag_S(sr);
    flag_Z(result == 0);
    flag_H((reg & (1 << 12)) ^ (result & (1 << 12)));
    flag_V((s1 && !s2 && !sr) || (!s1 && s2 && sr));
    flag_N(1);
    flag_C(!(result & 0x10000));
    reg = static_cast<uint16_t>(result & 0xFFFF);
}

int ZilogZ80::i_mi_adcsbc(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * ADC HL, {BC,DE,SP}
     * SBC HL, {BC,DE,SP}
     *
     * b7 b6 b5 b4 b3 b2 b1 b0
     * 0  1  s  s  1  0  1  0   => ADC HL, ss
     * 0  1  s  s  0  0  1  0   => SBC HL, ss
     */
    constexpr static const uint8_t OP_MASK = 0xC7;
    constexpr static const uint8_t OP_ADC  = 0xCA;
    constexpr static const uint8_t OP_SBC  = 0xC2;

    uint16_t &reg16 = self.reg16_from_opcode(op);

    switch (op & OP_MASK) {
    case OP_ADC:
        self.adc16(self._regs.HL, reg16);
        break;

    case OP_SBC:
        self.sbc16(self._regs.HL, reg16);
        break;

    default:
        log.error("Z80: MI: Unrecognised ADC/SBC opcode: ED %02X, addr: %04X\n", op, self._regs.PC);
    }

    return 0;
}

int ZilogZ80::i_mi_LD_m_rr(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * LD (nn), BC
     * LD (nn), DE
     * LD (nn), HL
     * LD (nn), SP
     */
    uint16_t &reg16 = self.reg16_from_opcode(op);
    self.write_addr(arg, reg16);
    return 0;
}

int ZilogZ80::i_mi_LD_rr_m(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * LD BC, (nn)
     * LD DE, (nn)
     * LD HL, (nn)
     * LD SP, (nn)
     */
    uint16_t &reg16 = self.reg16_from_opcode(op);
    reg16 = self.read_addr(arg);
    return 0;
}

int ZilogZ80::i_mi_NEG(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * NEG
     * A = -A
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if Accumulator was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is set if Accumulator was not 00h before operation; otherwise, it is reset
     */
    int result = -static_cast<int>(self._regs.A);
    self.flag_S(result < 0);
    self.flag_Z(result == 0);
    self.flag_H((self._regs.A & (1 << 4)) ^ (result & (1 << 4)));
    self.flag_V(self._regs.A == 0x80);
    self.flag_N(1);
    self.flag_C(self._regs.A != 0x00);
    self._regs.A = static_cast<uint8_t>(result & 0xFF);
    return 0;
}

int ZilogZ80::i_mi_RETN(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * RETN
     * This instruction is used at the end of a nonmaskable interrupts service routine to restore
     * the contents of the Program Counter (analogous to the RET instruction). The state of IFF2
     * is copied back to IFF1 so that maskable interrupts are enabled immediately following the
     * RETN if they were enabled before the nonmaskable interrupt.
     */
    self._IFF1 = self._IFF2;
    self._regs.PC = self.pop_addr();
    return 0;
}

int ZilogZ80::i_mi_IM_x(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * Set interrupt mode:
     * IM 0 - 46
     * IM 1 - 56
     * IM 2 - 5E
     */
    switch (op) {
    case 0x46:
        self._IM = IMODE_0;
        break;
    case 0x56:
        self._IM = IMODE_1;
        break;
    case 0x5E:
        self._IM = IMODE_2;
        break;
    default:
        log.error("Z80: MI: Unrecognised MI X opcode: ED %02X, addr: %04X\n", op, self._regs.PC);
    }

    return 0;
}

int ZilogZ80::i_mi_RETI(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * Return from Interrupt.
     * - Restore the contents of the Program Counter (analogous to the RET instruction)
     * - Signal an I/O device that the interrupt routine is completed.
     */
    self._regs.PC = self.pop_addr();
    //TODO FIXME how to signal an i/o ?
    return 0;
}

int ZilogZ80::i_mi_LD_x_A(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * LD I, A - 47
     * LD R, A - 4F
     */
    uint8_t &reg = (op == 0x47 ? self._regs.I : self._regs.R);
    reg = self._regs.A;
    return 0;
}

int ZilogZ80::i_mi_LD_A_x(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * LD A, I - 57
     * LD A, R - 5F
     * S is set if the I/R Register is negative; otherwise, it is reset.
     * Z is set if the I/R Register is 0; otherwise, it is reset.
     * H is reset.
     * P/V contains contents of IFF2.
     * N is reset.
     * C is not affected.
     * If an interrupt occurs during execution of this instruction, the Parity flag contains a 0.
     */
    self._regs.A = (op == 0x57 ? self._regs.I : self._regs.R);
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_H(0);
    self.flag_N(0);

    //TODO FIXME: if interrupt arrived this flag must be cleared
    self.flag_V(self._IFF2);

    return 0;
}

int ZilogZ80::i_mi_RRD(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * RRD
     *
     * The contents of the low-order four bits (bits 3, 2, 1, and 0)
     * of memory location (HL) are copied to the low-order four bits
     * of the Accumulator (Register A). The previous contents of the
     * low-order four bits of the Accumulator are copied to the
     * high-order four bits (7, 6, 5, and 4) of location (HL);
     * and the previous contents of the high-order four bits of (HL) are
     * copied to the low-order four bits of (HL). The contents of the
     * high-order bits of the Accumulator are unaffected.
     *
     * S is set if the Accumulator is negative after an operation; otherwise, it is reset.
     * Z is set if the Accumulator is 0 after an operation; otherwise, it is reset.
     * H is reset
     * P/V is set if the parity of the Accumulator is even after an operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     *
     * *HL.3210 = *HL.7654
     * *HL.7654 = A.3210
     * A.3210   = prev *HL.3210
     */
    uint8_t mem = self.read(self._regs.HL);
    uint8_t tmp = (mem & 0x07);
    mem = (self._regs.A << 4) | (mem >> 4);
    self._regs.A = (self._regs.A & 0xF0) | tmp;
    self.write(self._regs.HL, mem);
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_H(0);
    self.flag_V(self.parity(self._regs.A));
    self.flag_N(0);
    return 0;
}

int ZilogZ80::i_mi_RLD(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * RRD
     *
     * The contents of the low-order four bits (bits 3, 2, 1, and 0)
     * of the memory location (HL) are copied to the high-order four bits
     * (7, 6, 5, and 4) of that same memory location; the previous contents
     * of those high-order four bits are copied to the low-order four bits of the
     * Accumulator (Register A); and the previous contents of the low-order
     * four bits of the Accumulator are copied to the low-order four bits of
     * memory location (HL).
     * The contents of the high-order bits of the Accumulator are unaffected.
     *
     * S is set if the Accumulator is negative after an operation; otherwise, it is reset.
     * Z is set if the Accumulator is 0 after an operation; otherwise, it is reset.
     * H is reset.
     * P/V is set if the parity of the Accumulator is even after an operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     *
     * *HL.3210 = *HL.7654
     * *HL.7654 = A.3210
     * A.3210   = prev *HL.3210
     */
    uint8_t mem = self.read(self._regs.HL);
    uint8_t tmp = (mem >> 4);
    mem = (mem << 4) | (self._regs.A & 0x07);
    self._regs.A = (self._regs.A & 0xF0) | tmp;
    self.write(self._regs.HL, mem);
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_H(0);
    self.flag_V(self.parity(self._regs.A));
    self.flag_N(0);
    return 0;
}

int ZilogZ80::i_mi_copy(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * LDI  - ED A0 => *DE = *HL; ++DE; ++HL; --BC
     * LDD  - ED A8 => *DE = *HL; --DE; --HL; --BC
     * LDIR - ED B0 => while (BC) LDI;
     * LDDR - ED B8 => while (BC) LDD;
     *
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is set if BC - 1 != 0; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    constexpr static const uint8_t INC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & INC_OP) == INC_OP ? 1 : -1);

    self.flag_H(0);
    self.flag_N(0);
    self.flag_V(self._regs.BC != 0);

    uint8_t data = self.read(self._regs.HL);
    self.write(self._regs.DE, data);
    self._regs.HL += inc;
    self._regs.DE += inc;
    --self._regs.BC;

    if (op & REP_OP) {
        self._regs.PC -= 2;
        if (self._regs.BC == 0xFFFF) {
            return 0x00040010; /* M4 T16 */
        }
    }

    return 0;
}

int ZilogZ80::i_mi_cmp(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * CPI  - ED A1 => A - *HL; ++HL; --BC
     * CPD  - ED A9 => A - *HL; --HL; --BC
     * CPIR - ED B1 => do LDI while(BC);
     * CPDR - ED B0 => do LDD while(BC);
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if A is (HL); otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if BC - 1 is not 0; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    int value = self.read(self._regs.HL);
    int result = static_cast<int>(self._regs.A) - value;
    self.flag_S(result < 0);
    self.flag_Z(result == 0);
    self.flag_H(!((self._regs.A & 0x10) ^ (result & 0x10)));
    self.flag_V(self._regs.BC - 1 != 0);
    self.flag_N(1);

    self._regs.HL += inc;
    --self._regs.BC;

    if (op & REP_OP) {
        if (self._regs.BC != 0 && result == 0) {
            self._regs.PC -= 2;
        }

        if (self._regs.BC == 0 && result == 0) {
            return 0x00040010; /* M4 T16 */
        }
    }

    return 0;
}

int ZilogZ80::i_mi_in(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * INI  - ED A2 => *HL = in(C); ++HL; --B
     * IND  - ED AA => *HL = in(C); --HL; --B
     *
     * The contents of Register C are placed on the bottom half (A0 through A7) of the address
     * bus to select the I/O device at one of 256 possible ports. Register B can be used as a
     * byte counter, and its contents are placed on the top half (A8 through A15) of the address
     * bus at this time.
     * Then one byte from the selected port is placed on the data bus and written to the
     * CPU. The contents of the HL register pair are then placed on the address bus and the input
     * byte is written to the corresponding location of memory. Finally, the byte counter is decre-
     * mented and register pair HL is incremented (INI) or decremented (IND).
     *
     * S is unknown.
     * Z is set if B - 1 = 0; otherwise it is reset.
     * H is unknown.
     * P/V is unknown.
     * N is set.
     * C is not affected.
     *
     * INIR - ED B2 => do INI while(B);
     * INDR - ED BA => do IND while(B);
     *
     * Z is set, other flags are the same as INI/IND.
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    self.flag_Z(self._regs.B == 0x01);
    self.flag_N(1);

    uint8_t data = self.in(self._regs.C);
    self.write(self._regs.HL, data);
    self._regs.HL += inc;
    --self._regs.B;

    if (op & REP_OP) {
        if (self._regs.B == 0) {
            return 0x00040010; /* M4 T16 */
        }

        self._regs.PC -= 2;
    }

    return 0;
}

int ZilogZ80::i_mi_out(ZilogZ80 &self, uint8_t op, addr_t arg)
{
    /*
     * OUTI - ED A3 => out(C, *HL); --B, ++HL;
     * OUTD - ED AB => out(C, *HL); --B, --HL;
     *
     * The contents of the HL register pair are placed on the address bus to select
     * a location in memory. The byte contained in this memory location is temporarily
     * stored in the CPU. Then, after the byte counter (B) is decremented, the contents
     * of Register C are placed on the bottom half (A0 through A7) of the address bus
     * to select the I/O device at one of 256 possible ports.
     * Register B can be used as a byte counter, and its decremented value is placed on
     * the top half (A8 through A15) of the address bus. The byte to be output is placed
     * on the data bus and written to a selected peripheral device. Finally, the register
     * pair HL is incremented/decremented.
     *
     * S is unknown.
     * Z is set if B - 1 = 0; otherwise, it is reset.
     * H is unknown.
     * P/V is unknown.
     * N is set.
     * C is not affected.
     *
     * OTIR - ED B3 => do INI while(B);
     * OTDR - ED BB => do IND while(B);
     *
     * Z is set, other flags are the same as INI/IND.
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    self.flag_Z(self._regs.B == 0x01);
    self.flag_N(1);

    uint8_t data = self.read(self._regs.HL);
    self.out(self._regs.C, data);
    self._regs.HL += inc;
    --self._regs.B;

    if (op & REP_OP) {
        if (self._regs.B == 0) {
            return 0x00040010; /* M4 T16 */
        }

        self._regs.PC -= 2;
    }

    return 0;
}

}
