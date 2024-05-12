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

const Z80::Instruction Z80::ed_instr_set[256] = {
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 00 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 01 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 02 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 03 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 04 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 05 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 06 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 07 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 08 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 09 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 0A */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 0B */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 0C */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 0D */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 0E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 0F */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 10 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 11 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 12 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 13 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 14 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 15 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 16 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 17 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 18 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 19 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 1A */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 1B */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 1C */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 1D */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 1E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 1F */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 20 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 21 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 22 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 23 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 24 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 25 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 26 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 27 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 28 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 29 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 2A */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 2B */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 2C */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 2D */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 2E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 2F */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 30 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 31 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 32 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 33 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 34 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 35 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 36 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 37 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 38 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 39 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 3A */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 3B */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 3C */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 3D */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 3E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 3F */

    { "IN B, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 40 */
    { "OUT (C), B",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 41 */
    { "SBC HL, BC",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 42 */
    { "LD ($^), BC",    Z80::i_mi_LD_m_rr,  Z80::ArgType::A16,   16, 3   },  /* ED 43 */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 44 */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  10, 1   },  /* ED 45 */
    { "IM 0",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 46 */
    { "LD I, A",        Z80::i_mi_LD_x_A,   Z80::ArgType::None,  5,  1   },  /* ED 47 */
    { "IN C, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 48 */
    { "OUT (C), C",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 49 */
    { "ADC HL, BC",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 4A */
    { "LD BC, ($^)",    Z80::i_mi_LD_rr_m,  Z80::ArgType::A16,   16, 3   },  /* ED 4B */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 4C */
    { "RETI",           Z80::i_mi_RETI,     Z80::ArgType::None,  10, 1   },  /* ED 4D */
    { "IM 0",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 4E */
    { "LD R, A",        Z80::i_mi_LD_x_A,   Z80::ArgType::None,  5,  1   },  /* ED 4F */

    { "IN D, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 50 */
    { "OUT (C), D",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 51 */
    { "SBC HL, DE",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 52 */
    { "LD ($^), DE",    Z80::i_mi_LD_m_rr,  Z80::ArgType::A16,   16, 3   },  /* ED 53 */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 54 */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  14, 1   },  /* ED 55 */
    { "IM 1",           Z80::i_mi_IM_x,     Z80::ArgType::None,  8,  1   },  /* ED 56 */
    { "LD A, I",        Z80::i_mi_LD_A_x,   Z80::ArgType::None,  9,  1   },  /* ED 57 */
    { "IN E, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 58 */
    { "OUT (C), E",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 59 */
    { "ADC HL, DE",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 5A */
    { "LD DE, ($^)",    Z80::i_mi_LD_rr_m,  Z80::ArgType::A16,   16, 3   },  /* ED 5B */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 5C */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  10, 1   },  /* ED 5D */
    { "IM 2",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 5E */
    { "LD A, R",        Z80::i_mi_LD_A_x,   Z80::ArgType::None,  5,  1   },  /* ED 5F */

    { "IN H, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 60 */
    { "OUT (C), H",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 61 */
    { "SBC HL, HL",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 62 */
    { "LD ($^), HL",    Z80::i_mi_LD_m_rr,  Z80::ArgType::A16,   16, 3   },  /* ED 63 */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 64 */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  10, 1   },  /* ED 65 */
    { "IM 0",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 66 */
    { "RRD",            Z80::i_mi_RRD,      Z80::ArgType::None,  14, 1   },  /* ED 67 */
    { "IN L, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 68 */
    { "OUT (C), L",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 69 */
    { "ADC HL, HL",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 6A */
    { "LD HL, ($^)",    Z80::i_mi_LD_rr_m,  Z80::ArgType::A16,   16, 3   },  /* ED 6B */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 6C */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  10, 1   },  /* ED 6D */
    { "IM 0",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 6E */
    { "RLD",            Z80::i_mi_RLD,      Z80::ArgType::None,  14, 1   },  /* ED 6F */

    { "IN (C)",         Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 70 */
    { "OUT (C), 0",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 71 */
    { "SBC HL, SP",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 72 */
    { "LD ($^), SP",    Z80::i_mi_LD_m_rr,  Z80::ArgType::A16,   16, 3   },  /* ED 73 */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 74 */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  10, 1   },  /* ED 75 */
    { "IM 1",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 76 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 77 */
    { "IN A, (C)",      Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 78 */
    { "OUT (C), A",     Z80::i_mi_io,       Z80::ArgType::None,  8,  1   },  /* ED 79 */
    { "ADC HL, SP",     Z80::i_mi_adcsbc,   Z80::ArgType::None,  11, 1   },  /* ED 7A */
    { "LD SP, ($^)",    Z80::i_mi_LD_rr_m,  Z80::ArgType::A16,   16, 3   },  /* ED 7B */
    { "NEG",            Z80::i_mi_NEG,      Z80::ArgType::None,  4,  1   },  /* ED 7C */
    { "RETN",           Z80::i_mi_RETN,     Z80::ArgType::None,  10, 1   },  /* ED 7D */
    { "IM 2",           Z80::i_mi_IM_x,     Z80::ArgType::None,  4,  1   },  /* ED 7E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 7F */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 80 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 81 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 82 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 83 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 84 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 85 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 86 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 87 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 88 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 89 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 8A */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 8B */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 8C */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 8D */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 8E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 8F */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 90 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 91 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 92 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 93 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 94 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 95 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 96 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 97 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 98 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 99 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 9A */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 9B */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 9C */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 9D */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 9E */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED 9F */

    { "LDI",            Z80::i_mi_copy,     Z80::ArgType::None,  12, 1   },  /* ED A0 */
    { "CPI",            Z80::i_mi_cmp,      Z80::ArgType::None,  12, 1   },  /* ED A1 */
    { "INI",            Z80::i_mi_in,       Z80::ArgType::None,  12, 1   },  /* ED A2 */
    { "OUTI",           Z80::i_mi_out,      Z80::ArgType::None,  12, 1   },  /* ED A3 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED A4 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED A5 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED A6 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED A7 */
    { "LDD",            Z80::i_mi_copy,     Z80::ArgType::None,  12, 1   },  /* ED A8 */
    { "CPD",            Z80::i_mi_cmp,      Z80::ArgType::None,  12, 1   },  /* ED A9 */
    { "IND",            Z80::i_mi_in,       Z80::ArgType::None,  12, 1   },  /* ED AA */
    { "OUTD",           Z80::i_mi_out,      Z80::ArgType::None,  12, 1   },  /* ED AB */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED AC */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED AD */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED AE */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED AF */

    { "LDIR",           Z80::i_mi_copy,     Z80::ArgType::None,  17, 1   },  /* ED B0 */
    { "CPIR",           Z80::i_mi_cmp,      Z80::ArgType::None,  17, 1   },  /* ED B1 */
    { "INIR",           Z80::i_mi_in,       Z80::ArgType::None,  17, 1   },  /* ED B2 */
    { "OTIR",           Z80::i_mi_out,      Z80::ArgType::None,  17, 1   },  /* ED B3 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED B4 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED B5 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED B6 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED B7 */
    { "LDDR",           Z80::i_mi_copy,     Z80::ArgType::None,  17, 1   },  /* ED B8 */
    { "CPDR",           Z80::i_mi_cmp,      Z80::ArgType::None,  17, 1   },  /* ED B9 */
    { "INDR",           Z80::i_mi_in,       Z80::ArgType::None,  17, 1   },  /* ED BA */
    { "OTDR",           Z80::i_mi_out,      Z80::ArgType::None,  17, 1   },  /* ED BB */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED BC */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED BD */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED BE */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED BF */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C0 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C1 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C2 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C3 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C4 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C5 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C6 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C7 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C8 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED C9 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED CA */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED CB */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED CC */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED CD */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED CE */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED CF */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D0 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D1 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D2 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D3 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D4 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D5 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D6 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D7 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D8 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED D9 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED DA */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED DB */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED DC */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED DD */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED DE */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED DF */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E0 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E1 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E2 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E3 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E4 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E5 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E6 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E7 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E8 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED E9 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED EA */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED EB */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED EC */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED ED */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED EE */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED EF */

    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F0 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F1 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F2 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F3 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F4 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F5 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F6 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F7 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F8 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED F9 */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED FA */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED FB */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED FC */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED FD */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED FE */
    { "NOP",            Z80::i_NOP,         Z80::ArgType::None,  4,  1   },  /* ED FF */
};

int Z80::i_mi_io(Z80& self, uint8_t op, addr_t)
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
     * 0  1  r  r  r  0  0  0  => IN r, (C)
     * 0  1  r  r  r  0  0  1  => OUT (C), r
     *
     * rrr = 0x06 => IN only affect flags, OUT writes 0 - Undocumented
     */
    constexpr static const uint8_t OP_MASK = 0xC7;
    constexpr static const uint8_t IN_OP   = 0x40;
    constexpr static const uint8_t OUT_OP  = 0x41;

    uint8_t data{};
    uint8_t& reg = self.reg8_from_opcode(op, data);
    uint16_t prev = static_cast<uint16_t>(reg) << 8;
    addr_t port = self._regs.BC;

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
        break;

    case OUT_OP:
        self.io_out(port, reg);
        break;

    default:
        prev = self._iaddr;
        log.error("Z80: MI: Unrecognised IN/OUT opcode: ED %02X, addr: $%04X\n%s\n",
            op, self._iaddr, self.disass(prev).c_str());
        return 0;
    }

    self._regs.memptr = port + 1;
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
    flag_Y(reg & (static_cast<uint16_t>(Flags::Y) << 8));
    flag_X(reg & (static_cast<uint16_t>(Flags::X) << 8));
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

    auto src_reg = self.reg16_from_opcode(op);
    uint16_t result = self._regs.HL;
    self._regs.memptr = result + 1;

    switch (op & OP_MASK) {
    case OP_ADC:
        self.adc16(result, src_reg);
        break;
    case OP_SBC:
        self.sbc16(result, src_reg);
        break;
    default:
        log.error("Z80: MI: Unrecognised ADC/SBC opcode: ED %02X, addr: %04X\n", op, self._regs.PC);
    }

    self._regs.HL = result;
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
    auto src_reg = self.reg16_from_opcode(op);
    self.write_addr(arg, src_reg);
    self._regs.memptr = src_reg + 1;
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
    auto& dst_reg = self.reg16_from_opcode(op);
    uint16_t data = self.read_addr(arg);
    dst_reg = data;
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
        self._imode = IMode::M0;
        break;
    case 0x56:
    case 0x76:
        self._imode = IMode::M1;
        break;
    case 0x5E:
    case 0x7E:
        self._imode = IMode::M2;
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
    //TODO FIXME how to signal interrupt completion?
    return 0;
}

int Z80::i_mi_LD_x_A(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD I, A - ED 47
     * LD R, A - ED 4F
     */
    uint8_t& reg = (op == 0x47 ? self._regs.I : self._regs.R);
    reg = self._regs.A;
    return 0;
}

int Z80::i_mi_LD_A_x(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD A, I - ED 57
     * LD A, R - ED 5F
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
    addr_t addr = self._regs.HL;
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
    addr_t addr = self._regs.HL;
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

    uint8_t value = self.read(self._regs.HL);
    self.write(self._regs.DE, value);

    self._regs.HL += inc;
    self._regs.DE += inc;
    --self._regs.BC;

    self.flag_V(self._regs.BC);
    self.flag_H(0);
    self.flag_N(0);

    uint8_t n = self._regs.A + value;
    self.flag_Y(n & 0x02);
    self.flag_X(n & Flags::X);

    if (op & REP_OP) {
        if (self._regs.BC == 0) {
            return 0x00040010;  /* M4 T16 */
        }

        self._regs.PC -= 2;
        self._regs.memptr = self._regs.PC + 1;
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

    uint8_t value = self.read(self._regs.HL);
    uint8_t result = self.sub8(self._regs.A, value, 0);

    self._regs.HL += inc;
    --self._regs.BC;

    self.flag_C(C);
    self.flag_V(self._regs.BC);

    uint8_t n = result - self.test_H();
    self.flag_Y(n & 0x02);
    self.flag_X(n & Flags::X);

    self._regs.memptr += inc;

    if (op & REP_OP) {
        if (self._regs.BC == 0 || result == 0) {
            return 0x00040010;  /* M4 T16 */
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

    addr_t port = self._regs.BC;
    uint8_t data = self.io_in(port);
    self.write(self._regs.HL, data);

    self._regs.HL += inc;
    self._regs.B = self.sub8(self._regs.B, 1, 0);

    self.flag_N(data & 0x80);

    uint16_t k = data + self._regs.C + inc;
    self.flag_H(k > 255);
    self.flag_C(k > 255);
    self.flag_V(self.parity((k & 7) ^ self._regs.B));

    self._regs.memptr = port + inc;

    if (op & REP_OP) {
        if (self._regs.B == 0) {
            return 0x00040010;  /* M4 T16 */
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

    addr_t port = self._regs.BC;
    uint8_t data = self.read(self._regs.HL);
    self.io_out(port, data);

    self._regs.HL += inc;
    self._regs.B = self.sub8(self._regs.B, 1, 0);

    uint16_t k = self._regs.L + data;
    self.flag_H(k > 255);
    self.flag_C(k > 255);
    self.flag_V(self.parity((k & 7) ^ self._regs.B));

    self.flag_N(data & 0x80);

    self._regs.memptr = port + inc;

    if (op & REP_OP) {
        if (self._regs.B == 0) {
            return 0x00040010;  /* M4 T16 */
        }

        self._regs.PC -= 2;
    }

    return 0;
}
}

}
