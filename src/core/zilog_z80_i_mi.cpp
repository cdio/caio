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
const std::array<Z80::Instruction, 256> Z80::mi_instr_set{{
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 00 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 01 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 02 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 03 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 04 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 05 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 06 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 07 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 08 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 09 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 0A */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 0B */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 0C */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 0D */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 0E */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 0F */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 10 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 11 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 12 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 13 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 14 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 15 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 16 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 17 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 18 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 19 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 1A */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 1B */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 1C */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 1D */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 1E */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 1F */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 20 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 21 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 22 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 23 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 24 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 25 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 26 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 27 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 28 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 29 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 2A */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 2B */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 2C */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 2D */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 2E */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 2F */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 30 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 31 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 32 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 33 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 34 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 35 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 36 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 37 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 38 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 39 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 3A */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 3B */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 3C */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 3D */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 3E */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 3F */

    { "IN B, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 40 */
    { "OUT (C), B",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 41 */
    { "SBC HL, BC",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 42 */
    { "LD ($^), BC",    Z80::i_mi_LD_m_rr,  ArgType::A16,   20, 4   },  /* ED 43 */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 44 */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 45 */
    { "IM 0",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 46 */
    { "LD I, A",        Z80::i_mi_LD_x_A,   ArgType::None,  9,  2   },  /* ED 47 */
    { "IN C, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 48 */
    { "OUT (C), C",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 49 */
    { "ADC HL, BC",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 4A */
    { "LD BC, ($^)",    Z80::i_mi_LD_rr_m,  ArgType::A16,   20, 4   },  /* ED 4B */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 4C */ /* Undocumented */
    { "RETI",           Z80::i_mi_RETI,     ArgType::None,  14, 2   },  /* ED 4D */
    { "IM 0",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 4E */ /* Undocumented */
    { "LD R, A",        Z80::i_mi_LD_x_A,   ArgType::None,  9,  2   },  /* ED 4F */

    { "IN D, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 50 */
    { "OUT (C), D",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 51 */
    { "SBC HL, DE",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 52 */
    { "LD ($^), DE",    Z80::i_mi_LD_m_rr,  ArgType::A16,   20, 4   },  /* ED 53 */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 54 */ /* Undocumented */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 55 */ /* Undocumented */
    { "IM 1",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 56 */
    { "LD A, I",        Z80::i_mi_LD_A_x,   ArgType::None,  9,  2   },  /* ED 57 */
    { "IN E, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 58 */
    { "OUT (C), E",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 59 */
    { "ADC HL, DE",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 5A */
    { "LD DE, ($^)",    Z80::i_mi_LD_rr_m,  ArgType::A16,   20, 4   },  /* ED 5B */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 5C */ /* Undocumented */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 5D */ /* Undocumented */
    { "IM 2",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 5E */
    { "LD A, R",        Z80::i_mi_LD_A_x,   ArgType::None,  9,  2   },  /* ED 5F */

    { "IN H, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 60 */
    { "OUT (C), H",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 61 */
    { "SBC HL, HL",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 62 */
    { "LD ($^), HL",    Z80::i_mi_LD_m_rr,  ArgType::A16,   20, 4   },  /* ED 63 */ /* Undocumented */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 64 */ /* Undocumented */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 65 */ /* Undocumented */
    { "IM 0",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 66 */ /* Undocumented */
    { "RRD",            Z80::i_mi_RRD,      ArgType::None,  18, 2   },  /* ED 67 */
    { "IN L, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 68 */
    { "OUT (C), L",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 69 */
    { "ADC HL, HL",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 6A */
    { "LD HL, ($^)",    Z80::i_mi_LD_rr_m,  ArgType::A16,   20, 4   },  /* ED 6B */ /* Undocumented */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 6C */ /* Undocumented */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 6D */ /* Undocumented */
    { "IM 0",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 6E */ /* Undocumented */
    { "RLD",            Z80::i_mi_RLD,      ArgType::None,  18, 2   },  /* ED 6F */

    { "IN (C)",         Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 70 */ /* Undocumented */
    { "OUT (C), 0",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 71 */ /* Undocumented */
    { "SBC HL, SP",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 72 */
    { "LD ($^), SP",    Z80::i_mi_LD_m_rr,  ArgType::A16,   20, 4   },  /* ED 73 */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 74 */ /* Undocumented */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 75 */ /* Undocumented */
    { "IM 1",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 76 */ /* Undocumented */
    { "NOP",            Z80::i_NOP,         ArgType::None,  4,  2   },  /* ED 77 */ /* Undocumented */
    { "IN A, (C)",      Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 78 */
    { "OUT (C), A",     Z80::i_mi_io,       ArgType::None,  12, 2   },  /* ED 79 */
    { "ADC HL, SP",     Z80::i_mi_adcsbc,   ArgType::None,  15, 2   },  /* ED 7A */
    { "LD SP, ($^)",    Z80::i_mi_LD_rr_m,  ArgType::A16,   20, 4   },  /* ED 7B */
    { "NEG",            Z80::i_mi_NEG,      ArgType::None,  8,  2   },  /* ED 7C */ /* Undocumented */
    { "RETN",           Z80::i_mi_RETN,     ArgType::None,  14, 2   },  /* ED 7D */ /* Undocumented */
    { "IM 2",           Z80::i_mi_IM_x,     ArgType::None,  8,  2   },  /* ED 7E */ /* Undocumented */
    { "NOP",            Z80::i_NOP,         ArgType::None,  4,  2   },  /* ED 7F */ /* Undocumented */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 80 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 81 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 82 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 83 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 84 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 85 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 86 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 87 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 88 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 89 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 8A */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 8B */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 8C */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 8D */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 8E */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 8F */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 90 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 91 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 92 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 93 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 94 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 95 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 96 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 97 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 98 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 99 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 9A */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 9B */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 9C */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 9D */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 9E */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED 9F */

    { "LDI",            Z80::i_mi_copy,     ArgType::None,  16, 2   },  /* ED A0 */
    { "CPI",            Z80::i_mi_cmp,      ArgType::None,  16, 2   },  /* ED A1 */
    { "INI",            Z80::i_mi_in,       ArgType::None,  16, 2   },  /* ED A2 */
    { "OUTI",           Z80::i_mi_out,      ArgType::None,  16, 2   },  /* ED A3 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED A4 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED A5 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED A6 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED A7 */
    { "LDD",            Z80::i_mi_copy,     ArgType::None,  16, 2   },  /* ED A8 */
    { "CPD",            Z80::i_mi_cmp,      ArgType::None,  16, 2   },  /* ED A9 */
    { "IND",            Z80::i_mi_in,       ArgType::None,  16, 2   },  /* ED AA */
    { "OUTD",           Z80::i_mi_out,      ArgType::None,  16, 2   },  /* ED AB */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED AC */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED AD */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED AE */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED AF */

    { "LDIR",           Z80::i_mi_copy,     ArgType::None,  21, 2   },  /* ED B0 */
    { "CPIR",           Z80::i_mi_cmp,      ArgType::None,  21, 2   },  /* ED B1 */
    { "INIR",           Z80::i_mi_in,       ArgType::None,  21, 2   },  /* ED B2 */
    { "OTIR",           Z80::i_mi_out,      ArgType::None,  21, 2   },  /* ED B3 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED B4 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED B5 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED B6 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED B7 */
    { "LDDR",           Z80::i_mi_copy,     ArgType::None,  21, 2   },  /* ED B8 */
    { "CPDR",           Z80::i_mi_cmp,      ArgType::None,  21, 2   },  /* ED B9 */
    { "INDR",           Z80::i_mi_in,       ArgType::None,  21, 2   },  /* ED BA */
    { "OTDR",           Z80::i_mi_out,      ArgType::None,  21, 2   },  /* ED BB */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED BC */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED BD */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED BE */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED BF */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C0 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C1 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C2 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C3 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C4 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C5 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C6 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C7 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C8 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED C9 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED CA */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED CB */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED CC */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED CD */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED CE */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED CF */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D0 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D1 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D2 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D3 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D4 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D5 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D6 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D7 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D8 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED D9 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED DA */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED DB */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED DC */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED DD */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED DE */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED DF */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E0 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E1 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E2 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E3 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E4 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E5 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E6 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E7 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E8 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED E9 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED EA */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED EB */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED EC */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED ED */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED EE */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED EF */

    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F0 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F1 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F2 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F3 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F4 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F5 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F6 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F7 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F8 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED F9 */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED FA */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED FB */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED FC */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED FD */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED FE */
    { "",               Z80::i_NOP,         ArgType::None,  8,  2   },  /* ED FF */
}};

int Z80::i_mi_io(Z80& self, uint8_t op, addr_t arg)
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
    uint8_t& reg = self.reg8_from_opcode(op, data);
    uint8_t port = self._regs.C;

    switch (op & OP_MASK) {
    case IN_OP:
        reg = self.io_in(port);
        self.flag_S(reg & 0x80);
        self.flag_Z(reg == 0);
        self.flag_H(0);
        self.flag_V(self.parity(reg));
        self.flag_N(0);
        self.flag_Y(reg & Flags::Y);
        self.flag_X(reg & Flags::X);
        if (op == 0x78 /* IN A, (C) */) {
            self._regs.memptr = self._regs.BC() + 1;
        }
        break;

    case OUT_OP:
        self.io_out(port, reg);
        if (op == 0x79 /* OUT (C), A */) {
            self._regs.memptr = self._regs.BC() + 1;
        }
        break;

    default:
        log.error("Z80: MI: Unrecognised IN/OUT opcode: ED %02X, addr: $04X\n", op, self._regs.PC);
    }

    return 0;
}

void Z80::add16(uint16_t& reg, uint16_t value, bool carry)
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
    uint16_t lo = (reg & 0xFF) + (value & 0xFF) + carry;
    bool c = (lo & 0x100);

    uint16_t hil = ((reg >> 8) & 0x0F) + ((value >> 8) & 0x0F) + c;
    bool hc = (hil & 0x10);

    uint16_t hih = (reg >> 12) + (value >> 12) + hc;
    c = (hih & 0x10);

    reg = (hih << 12) | ((hil & 0x0F) << 8) | (lo & 0xFF);

    flag_H(hc);
    flag_N(0);
    flag_C(c);
    flag_Y(reg & (1 << 13));
    flag_X(reg & (1 << 11));
}

void Z80::adc16(uint16_t& reg, uint16_t value)
{
    /*
     * reg += value + C
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is reset
     */
    bool s1 = reg & 0x8000;
    bool s2 = value & 0x8000;
    add16(reg, value, test_C());
    bool sr = reg & 0x8000;
    flag_S(sr);
    flag_Z(reg == 0);
    flag_V((s1 && s2 && !sr) || (!s1 && !s2 && sr));
}

void Z80::sbc16(uint16_t& reg, uint16_t value)
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
    value = ~value;
    bool s1 = reg & 0x8000;
    bool s2 = value & 0x8000;
    add16(reg, value, !test_C());
    bool sr = reg & 0x8000;
    flag_N(1);
    flag_S(sr);
    flag_Z(reg == 0);
    flag_V((s1 && s2 && !sr) || (!s1 && !s2 && sr));
    flag_C(!test_C());
    flag_H(!test_H());
}

int Z80::i_mi_adcsbc(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC HL, {BC,DE,SP}
     * SBC HL, {BC,DE,SP}
     *
     * 80 40 20 10 08 04 02 01
     * b7 b6 b5 b4 b3 b2 b1 b0
     * 0  1  s  s  1  0  1  0   => ADC HL, ss
     * 0  1  s  s  0  0  1  0   => SBC HL, ss
     */
    constexpr static const uint8_t OP_MASK = 0xCF;
    constexpr static const uint8_t OP_ADC  = 0x4A;
    constexpr static const uint8_t OP_SBC  = 0x42;

    auto [rget, rset] = self.reg16_from_opcode(op);
    uint16_t result = self._regs.HL();
    self._regs.memptr = result + 1;

    switch (op & OP_MASK) {
    case OP_ADC:
        self.adc16(result, rget());
        break;
    case OP_SBC:
        self.sbc16(result, rget());
        break;
    default:
        log.error("Z80: MI: Unrecognised ADC/SBC opcode: ED %02X, addr: %04X\n", op, self._regs.PC);
    }

    self._regs.HL(result);
    return 0;
}

int Z80::i_mi_LD_m_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (nn), BC
     * LD (nn), DE
     * LD (nn), HL
     * LD (nn), SP
     */
    auto [rget, rset] = self.reg16_from_opcode(op);
    uint16_t data = rget();
    self.write_addr(arg, data);
    self._regs.memptr = data + 1;
    return 0;
}

int Z80::i_mi_LD_rr_m(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD BC, (nn)
     * LD DE, (nn)
     * LD HL, (nn)
     * LD SP, (nn)
     */
    auto [rget, rset] = self.reg16_from_opcode(op);
    uint16_t data = self.read_addr(arg);
    rset(data);
    self._regs.memptr = data + 1;
    return 0;
}

int Z80::i_mi_NEG(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * NEG
     * A = -A
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if accumulator was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is set if accumulator was not 00h before operation; otherwise, it is reset.
     */
    uint8_t prev_A = self._regs.A;
    self._regs.A = -self._regs.A;
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_H((prev_A & 0x10) ^ (self._regs.A & 0x10));
    self.flag_V(prev_A == 0x80);
    self.flag_N(1);
    self.flag_C(prev_A != 0x00);
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);
    return 0;
}

int Z80::i_mi_RETN(Z80& self, uint8_t op, addr_t arg)
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

int Z80::i_mi_IM_x(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * Set interrupt mode:
     * IM 0 - 46, 4E, 66, 6E
     * IM 1 - 56, 76
     * IM 2 - 5E, 7E
     */
    switch (op) {
    case 0x46:
    case 0x4E:
    case 0x66:
    case 0x6E:
        self._imode = IMODE_0;
        break;
    case 0x56:
    case 0x76:
        self._imode = IMODE_1;
        break;
    case 0x5E:
    case 0x7E:
        self._imode = IMODE_2;
        break;
    default:
        log.error("Z80: MI: Unrecognised MI X opcode: ED %02X, addr: %04X\n", op, self._regs.PC);
    }

    return 0;
}

int Z80::i_mi_RETI(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * Return from Interrupt.
     * - Restore the contents of the Program Counter (analogous to the RET instruction)
     * - Signal an I/O device that the interrupt routine is completed.
     */
    self._regs.PC = self.pop_addr();
    self._regs.memptr = self._regs.PC;
    //TODO FIXME how to signal an i/o ?
    return 0;
}

int Z80::i_mi_LD_x_A(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD I, A - 47
     * LD R, A - 4F
     */
    uint8_t& reg = (op == 0x47 ? self._regs.I : self._regs.R);
    reg = self._regs.A;
    return 0;
}

int Z80::i_mi_LD_A_x(Z80& self, uint8_t op, addr_t arg)
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
    self.flag_V(self._IFF2);
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);
    return 0;
}

int Z80::i_mi_RRD(Z80& self, uint8_t op, addr_t arg)
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
    addr_t addr = self._regs.HL();
    uint8_t mem = self.read(addr);
    uint8_t tmp = (mem & 0x0F);
    mem = (self._regs.A << 4) | (mem >> 4);
    self._regs.A = (self._regs.A & 0xF0) | tmp;
    self.write(addr, mem);
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_H(0);
    self.flag_V(self.parity(self._regs.A));
    self.flag_N(0);
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);
    self._regs.memptr = addr + 1;
    return 0;
}

int Z80::i_mi_RLD(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * RLD
     *
     * The contents of the low-order four bits (bits 3, 2, 1, and 0)
     * of the memory location (HL) are copied to the high-order four
     * bits (7, 6, 5, and 4) of that same memory location; the previous
     * contents of those high-order four bits are copied to the low-order
     * four bits of the Accumulator (Register A); and the previous contents
     * of the low-order four bits of the Accumulator are copied to the
     * low-order four bits of memory location (HL). The contents of the
     * high-order bits of the Accumulator are unaffected.
     *
     * S is set if the Accumulator is negative after an operation; otherwise, it is reset.
     * Z is set if the Accumulator is 0 after an operation; otherwise, it is reset.
     * H is reset.
     * P/V is set if the parity of the Accumulator is even after an operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     *
     * hi = (*HL) >> 4
     * (*HL) <<= 4
     * (*HL) |= A & 15
     * A = (A & 0xF0) | hi
     */
    addr_t addr = self._regs.HL();
    uint8_t mem = self.read(addr);
    uint8_t hl_hi = mem >> 4;
    mem = (mem << 4) | (self._regs.A & 0x0F);
    self._regs.A = (self._regs.A & 0xF0) | hl_hi;
    self.write(addr, mem);
    self.flag_S(self._regs.A & 0x80);
    self.flag_Z(self._regs.A == 0);
    self.flag_H(0);
    self.flag_V(self.parity(self._regs.A));
    self.flag_N(0);
    self.flag_Y(self._regs.A & Flags::Y);
    self.flag_X(self._regs.A & Flags::X);
    self._regs.memptr = addr + 1;
    return 0;
}

int Z80::i_mi_copy(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LDI  - ED A0 => *DE = *HL; ++DE; ++HL; --BC
     * LDD  - ED A8 => *DE = *HL; --DE; --HL; --BC
     * LDIR - ED B0 => do LDI while (BC);
     * LDDR - ED B8 => do LDD while (BC);
     *
     * S is not affected.
     * Z is not affected.
     * H is reset.
     * P/V is set if BC - 1 != 0; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    uint8_t value = self.read(self._regs.HL());
    self.write(self._regs.DE(), value);

    self._regs.HL(self._regs.HL() + inc);
    self._regs.DE(self._regs.DE() + inc);
    self._regs.BC(self._regs.BC() - 1);

    self.flag_V(self._regs.BC());
    self.flag_H(0);
    self.flag_N(0);

    uint8_t n = self._regs.A + value;
    self.flag_Y(n & 0x02);
    self.flag_X(n & 0x08);

    if (op & REP_OP) {
        if (self._regs.BC() == 0) {
            return 0x00040010; /* M4 T16 */
        }

        self._regs.PC -= 2;
        self._regs.memptr = self._regs.PC;// + 1; XXX
    }

    return 0;
}

int Z80::i_mi_cmp(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CPI  - ED A1 => A - *HL; ++HL; --BC
     * CPD  - ED A9 => A - *HL; --HL; --BC
     * CPIR - ED B1 => do CPI while(BC);
     * CPDR - ED B0 => do CPD while(BC);
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if A is (HL); otherwise, it is reset.
     * H is set if borrow from bit 4; otherwise, it is reset.
     * P/V is set if BC - 1 is not 0; otherwise, it is reset.
     * N is set.
     * C is not affected.
     *
     * Undocumented behaviour (Undocumented Z80 Documented):
     * -----------------------------------------------------
     * Compares a series of bytes in memory to register A. Effectively,
     * it can be said it does CP (HL) at every iteration.
     * The result of that compare sets the HF flag, which is important
     * for the next step. Take the value of register A, subtract the value
     * of the memory address, and finally subtract the value of HF flag,
     * which is set or reset by the hypothetical CP (HL).
     * So, n = A - (HL) - HF.
     *
     * SF, ZF, HF flag Set by the hypothetical CP (HL).
     * YF flag A copy of bit 1 of n.
     * XF flag A copy of bit 3 of n.
     * PF flag Set if BC is not 0.
     * NF flag Always set.
     * CF flag Unchanged.
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    bool C = self.test_C();

    uint8_t value = self.read(self._regs.HL());
    uint8_t result = self.sub8(self._regs.A, value, 0);

    self._regs.HL(self._regs.HL() + inc);
    self._regs.BC(self._regs.BC() - 1);

    self.flag_C(C);
    self.flag_V(self._regs.BC());

    uint8_t n = result - self.test_H();
    self.flag_Y(n & 0x02);
    self.flag_X(n & 0x08);

    self._regs.memptr += inc;

    if (op & REP_OP) {
        if (self._regs.BC() == 0 || result == 0) {
            self._regs.memptr = self._regs.PC;
            return 0x00040010; /* M4 T16 */
        }

        self._regs.PC -= 2;
        self._regs.memptr = self._regs.PC + 1;
    }

    return 0;
}

int Z80::i_mi_in(Z80& self, uint8_t op, addr_t arg)
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
     * C is not affected
     *
     * INIR - ED B2 => do INI while(B);
     * INDR - ED BA => do IND while(B);
     *
     * Z is set, other flags are the same as INI/IND.
     *
     * Undocumented behaviour (Undocumented Z80 Documented):
     * -----------------------------------------------------
     * SF, ZF, YF, XF flags affected by decreasing register B, as in DEC B.
     * NF flag A copy of bit 7 of the value read from or written to an I/O port.
     *
     * k = C + 1 if it’s INI/INIR or C - 1 if it’s IND/INDR.
     *
     * So, ﬁrst of all INI/INIR:
     * HF and CF Both set if ((HL) + ((C + 1) & 255) > 255)
     * PF The parity of (((HL) + ((C + 1) & 255)) & 7) xor B)
     *
     * And last IND/INDR:
     * HF and CF Both set if ((HL) + ((C - 1) & 255) > 255)
     * PF The parity of (((HL) + ((C - 1) & 255)) & 7) xor B)
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    uint8_t data = self.io_in(self._regs.C);
    self.write(self._regs.HL(), data);

    self._regs.HL(self._regs.HL() + inc);
    self._regs.B = self.sub8(self._regs.B, 1, 0);

    self.flag_N(data & 0x80);

    uint16_t k = data + self._regs.C + inc;
    self.flag_H(k > 255);
    self.flag_C(k > 255);
    self.flag_V(self.parity((k & 7) ^ self._regs.B));

    self._regs.memptr = self._regs.BC() + inc;

    if (op & REP_OP) {
        if (self._regs.B == 0) {
            return 0x00040010; /* M4 T16 */
        }

        self._regs.PC -= 2;
    }

    return 0;
}

int Z80::i_mi_out(Z80& self, uint8_t op, addr_t arg)
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
     *
     * Undocumented behaviour (Undocumented Z80 Documented):
     * -----------------------------------------------------
     * SF, ZF, YF, XF flags affected by decreasing register B, as in DEC B.
     * NF flag A copy of bit 7 of the value read from or written to an I/O port.
     *
     * Take state of the L after the increment or decrement of HL;
     * add the value written to the I/O port to; call that k for now. If k > 255,
     * then the CF and HF flags are set. The PF flags is set like the parity of
     * k bitwise and’ed with 7, bitwise xor’ed with B.
     *
     * HF and CF Both set if ((HL) + L > 255)
     * PF The parity of ((((HL) + L) & 7) xor B)
     */
    constexpr static const uint8_t DEC_OP = 0x08;
    constexpr static const uint8_t REP_OP = 0x10;
    int16_t inc = ((op & DEC_OP) == DEC_OP ? -1 : 1);

    uint8_t data = self.read(self._regs.HL());
    self.io_out(self._regs.C, data);

    self._regs.HL(self._regs.HL() + inc);
    self._regs.B = self.sub8(self._regs.B, 1, 0);

    uint16_t k = self._regs.L + data;
    self.flag_H(k > 255);
    self.flag_C(k > 255);
    self.flag_V(self.parity((k & 7) ^ self._regs.B));

    self.flag_N(data & 0x80);

    self._regs.memptr = self._regs.BC() + inc;

    if (op & REP_OP) {
        if (self._regs.B == 0) {
            return 0x00040010; /* M4 T16 */
        }

        self._regs.PC -= 2;
    }

    return 0;
}
}

}
