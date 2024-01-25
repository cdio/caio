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

const Z80::Instruction Z80::iy_instr_set[256] = {
    { "NOP",            Z80::i_NOP,         ArgType::None,  4,  1   },  /* FD 00 */
    { "LD BC, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* FD 01 */
    { "LD (BC), A",     Z80::i_LD_mdd_A,    ArgType::None,  7,  1   },  /* FD 02 */
    { "INC BC",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* FD 03 */
    { "INC B",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* FD 04 */
    { "DEC B",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* FD 05 */
    { "LD B, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* FD 06 */
    { "RLCA",           Z80::i_RLCA,        ArgType::None,  4,  2   },  /* FD 07 */
    { "EX AF, AF'",     Z80::i_EX_AF_sAF,   ArgType::None,  4,  1   },  /* FD 08 */
    { "ADD IY, BC",     Z80::i_ADD_IY_rr,   ArgType::None,  11, 2   },  /* FD 09 */
    { "LD A, (BC)",     Z80::i_LD_A_mdd,    ArgType::None,  7,  1   },  /* FD 0A */
    { "DEC BC",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* FD 0B */
    { "INC C",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* FD 0C */
    { "DEC C",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* FD 0D */
    { "LD C, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* FD 0E */
    { "RRCA",           Z80::i_RRCA,        ArgType::None,  4,  1   },  /* FD 0F */

    { "DJNZ $+",        Z80::i_DJNZ,        ArgType::A8,    13, 2   },  /* FD 10 */
    { "LD DE, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* FD 11 */
    { "LD (DE), A",     Z80::i_LD_mdd_A,    ArgType::None,  7,  1   },  /* FD 12 */
    { "INC DE",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* FD 13 */
    { "INC D",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* FD 14 */
    { "DEC D",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* FD 15 */
    { "LD D, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* FD 16 */
    { "RLA",            Z80::i_RLA,         ArgType::None,  4,  1   },  /* FD 17 */
    { "JR $+",          Z80::i_JR,          ArgType::A8,    12, 1   },  /* FD 18 */
    { "ADD IY, DE",     Z80::i_ADD_IY_rr,   ArgType::None,  11, 2   },  /* FD 19 */
    { "LD A, (DE)",     Z80::i_LD_A_mdd,    ArgType::None,  7,  1   },  /* FD 1A */
    { "DEC DE",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* FD 1B */
    { "INC E",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* FD 1C */
    { "DEC E",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* FD 1D */
    { "LD E, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* FD 1E */
    { "RRA",            Z80::i_RRA,         ArgType::None,  4,  1   },  /* FD 1F */

    { "JR NZ, $+",      Z80::i_JR_NZ,       ArgType::A8,    12, 2   },  /* FD 20 */
    { "LD IY, $^",      Z80::i_LD_IY_nn,    ArgType::A16,   14, 4   },  /* FD 21 */
    { "LD ($^), IY",    Z80::i_LD_mnn_IY,   ArgType::A16,   20, 4   },  /* FD 22 */
    { "INC IY",         Z80::i_INC_IY,      ArgType::None,  6,  1   },  /* FD 23 */
    { "INC IYH",        Z80::i_INC_IYH,     ArgType::None,  4,  1   },  /* FD 24 */
    { "DEC IYH",        Z80::i_DEC_IYH,     ArgType::None,  4,  1   },  /* FD 25 */
    { "LD IYH, $*",     Z80::i_LD_IYH_n,    ArgType::A8,    7,  2   },  /* FD 26 */
    { "DAA",            Z80::i_DAA,         ArgType::None,  4,  1   },  /* FD 27 */
    { "JR Z, $+",       Z80::i_JR_Z,        ArgType::A8,    12, 2   },  /* FD 28 */
    { "ADD IY, IY",     Z80::i_ADD_IY_rr,   ArgType::None,  11, 1   },  /* FD 29 */
    { "LD IY, ($^)",    Z80::i_LD_IY_mnn,   ArgType::A16,   16, 3   },  /* FD 2A */
    { "DEC IY",         Z80::i_DEC_IY,      ArgType::None,  10, 1   },  /* FD 2B */
    { "INC IYL",        Z80::i_INC_IYL,     ArgType::None,  4,  1   },  /* FD 2C */
    { "DEC IYL",        Z80::i_DEC_IYL,     ArgType::None,  4,  1   },  /* FD 2D */
    { "LD IYL, $*",     Z80::i_LD_IYL_n,    ArgType::A8,    7,  2   },  /* FD 2E */
    { "CPL",            Z80::i_CPL,         ArgType::None,  4,  1   },  /* FD 2F */

    { "JR NC, $+",      Z80::i_JR_NC,       ArgType::A8,    12, 2   },  /* FD 30 */
    { "LD SP, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* FD 31 */
    { "LD ($^), A",     Z80::i_LD_mdd_A,    ArgType::A16,   13, 3   },  /* FD 32 */
    { "INC SP",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* FD 33 */
    { "INC (IY%)",      Z80::i_INC_mIYd,    ArgType::A8,    11, 1   },  /* FD 34 */
    { "DEC (IY%)",      Z80::i_DEC_mIYd,    ArgType::A8,    11, 1   },  /* FD 35 */
    { "LD (IY%), $*",   Z80::i_LD_mIYd_n,   ArgType::A16,   15, 3   },  /* FD 36 */
    { "SCF",            Z80::i_SCF,         ArgType::None,  4,  1   },  /* FD 37 */
    { "JR C, $+",       Z80::i_JR_C,        ArgType::A8,    12, 2   },  /* FD 38 */
    { "ADD IY, SP",     Z80::i_ADD_IY_rr,   ArgType::None,  11, 1   },  /* FD 39 */
    { "LD A, ($^)",     Z80::i_LD_A_mdd,    ArgType::A16,   13, 3   },  /* FD 3A */
    { "DEC SP",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* FD 3B */
    { "INC A",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* FD 3C */
    { "DEC A",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* FD 3D */
    { "LD A, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* FD 3E */
    { "CCF",            Z80::i_CCF,         ArgType::None,  4,  1   },  /* FD 3F */

    { "LD B, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 40 */
    { "LD B, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 41 */
    { "LD B, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 42 */
    { "LD B, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 43 */
    { "LD B, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 44 */
    { "LD B, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 45 */
    { "LD B, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 46 */
    { "LD B, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 47 */
    { "LD C, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 48 */
    { "LD C, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 49 */
    { "LD C, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 4A */
    { "LD C, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 4B */
    { "LD C, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 4C */
    { "LD C, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 4D */
    { "LD C, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 4E */
    { "LD C, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 4F */

    { "LD D, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 50 */
    { "LD D, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 51 */
    { "LD D, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 52 */
    { "LD D, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 53 */
    { "LD D, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 54 */
    { "LD D, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 55 */
    { "LD D, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 56 */
    { "LD D, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 57 */
    { "LD E, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 58 */
    { "LD E, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 59 */
    { "LD E, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 5A */
    { "LD E, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 5B */
    { "LD E, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 5C */
    { "LD E, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 5D */
    { "LD E, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 5E */
    { "LD E, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 5F */

    { "LD IYH, B",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 60 */
    { "LD IYH, C",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 61 */
    { "LD IYH, D",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 62 */
    { "LD IYH, E",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 63 */
    { "LD IYH, IYH",    Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 64 */
    { "LD IYH, IYL",    Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 65 */
    { "LD H, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 66 */
    { "LD IYH, A",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 67 */
    { "LD IYL, B",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 68 */
    { "LD IYL, C",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 69 */
    { "LD IYL, D",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 6A */
    { "LD IYL, E",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 6B */
    { "LD IYL, IYH",    Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 6C */
    { "LD IYL, IYL",    Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 6D */
    { "LD L, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 6E */
    { "LD IYL, A",      Z80::i_LD_IYHL_r,   ArgType::None,  4,  1   },  /* FD 6F */

    { "LD (IY%), B",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 70 */
    { "LD (IY%), C",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 71 */
    { "LD (IY%), D",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 72 */
    { "LD (IY%), E",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 73 */
    { "LD (IY%), H",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 74 */
    { "LD (IY%), L",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 75 */
    { "HALT",           Z80::i_HALT,        ArgType::None,  4,  1   },  /* DD 76 */
    { "LD (IY%), A",    Z80::i_LD_mIYd_r,   ArgType::A8,    15, 2   },  /* FD 77 */
    { "LD A, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 78 */
    { "LD A, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 79 */
    { "LD A, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 7A */
    { "LD A, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 7B */
    { "LD A, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 7C */
    { "LD A, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  4,  1   },  /* FD 7D */
    { "LD A, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    15, 2   },  /* FD 7E */
    { "LD A, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* FD 7F */

    { "ADD A, B",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* FD 80 */
    { "ADD A, C",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* FD 81 */
    { "ADD A, D",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* FD 82 */
    { "ADD A, E",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* FD 83 */
    { "ADD A, IYH",     Z80::i_ADD_A_IYHL,  ArgType::None,  4,  1   },  /* FD 84 */
    { "ADD A, IYL",     Z80::i_ADD_A_IYHL,  ArgType::None,  4,  1   },  /* FD 85 */
    { "ADD A, (IY%)",   Z80::i_ADD_A_mIYd,  ArgType::A8,    15, 2   },  /* FD 86 */
    { "ADD A, A",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* FD 87 */
    { "ADC A, B",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* FD 88 */
    { "ADC A, C",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* FD 89 */
    { "ADC A, D",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* FD 8A */
    { "ADC A, E",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* FD 8B */
    { "ADC A, IYH",     Z80::i_ADC_A_IYHL,  ArgType::None,  4,  1   },  /* FD 8C */
    { "ADC A, IYL",     Z80::i_ADC_A_IYHL,  ArgType::None,  4,  1   },  /* FD 8D */
    { "ADC A, (IY%)",   Z80::i_ADC_A_mIYd,  ArgType::A8,    15, 2   },  /* FD 8E */
    { "ADC A, A",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* FD 8F */

    { "SUB B",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* FD 90 */
    { "SUB C",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* FD 91 */
    { "SUB D",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* FD 92 */
    { "SUB E",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* FD 93 */
    { "SUB IYH",        Z80::i_SUB_A_IYHL,  ArgType::None,  4,  1   },  /* FD 94 */
    { "SUB IYL",        Z80::i_SUB_A_IYHL,  ArgType::None,  4,  1   },  /* FD 95 */
    { "SUB (IY%)",      Z80::i_SUB_A_mIYd,  ArgType::A8,    15, 2   },  /* FD 96 */
    { "SUB A",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* FD 97 */
    { "SBC B",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* FD 98 */
    { "SBC C",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* FD 99 */
    { "SBC D",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* FD 9A */
    { "SBC E",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* FD 9B */
    { "SBC IYH",        Z80::i_SBC_A_IYHL,  ArgType::None,  4,  1   },  /* FD 9C */
    { "SBC IYL",        Z80::i_SBC_A_IYHL,  ArgType::None,  4,  1   },  /* FD 9D */
    { "SBC (IY%)",      Z80::i_SBC_A_mIYd,  ArgType::A8,    15, 2   },  /* FD 9E */
    { "SBC A",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* FD 9F */

    { "AND B",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* FD A0 */
    { "AND C",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* FD A1 */
    { "AND D",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* FD A2 */
    { "AND E",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* FD A3 */
    { "AND IYH",        Z80::i_AND_A_IYHL,  ArgType::None,  4,  1   },  /* FD A4 */
    { "AND IYL",        Z80::i_AND_A_IYHL,  ArgType::None,  4,  1   },  /* FD A5 */
    { "AND (IY%)",      Z80::i_AND_A_mIYd,  ArgType::A8,    15, 2   },  /* FD A6 */
    { "AND A",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* FD A7 */
    { "XOR B",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* FD A8 */
    { "XOR C",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* FD A9 */
    { "XOR D",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* FD AA */
    { "XOR E",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* FD AB */
    { "XOR IYH",        Z80::i_XOR_A_IYHL,  ArgType::None,  4,  1   },  /* FD AC */
    { "XOR IYL",        Z80::i_XOR_A_IYHL,  ArgType::None,  4,  1   },  /* FD AD */
    { "XOR (IY%)",      Z80::i_XOR_A_mIYd,  ArgType::A8,    15, 3   },  /* FD AE */
    { "XOR A",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* FD AF */

    { "OR B",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* FD B0 */
    { "OR C",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* FD B1 */
    { "OR D",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* FD B2 */
    { "OR E",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* FD B3 */
    { "OR IYH",         Z80::i_OR_A_IYHL,   ArgType::None,  4,  1   },  /* FD B4 */
    { "OR IYL",         Z80::i_OR_A_IYHL,   ArgType::None,  4,  1   },  /* FD B5 */
    { "OR (IY%)",       Z80::i_OR_A_mIYd,   ArgType::A8,    15, 2   },  /* FD B6 */
    { "OR A",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* FD B7 */
    { "CP B",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* FD B8 */
    { "CP C",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* FD B9 */
    { "CP D",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* FD BA */
    { "CP E",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* FD BB */
    { "CP IYH",         Z80::i_CP_A_IYHL,   ArgType::None,  4,  1   },  /* FD BC */
    { "CP IYL",         Z80::i_CP_A_IYHL,   ArgType::None,  4,  1   },  /* FD BD */
    { "CP (IY%)",       Z80::i_CP_A_mIYd,   ArgType::A8,    15, 2   },  /* FD BE */
    { "CP A",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* FD BF */

    { "RET NZ",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD C0 */
    { "POP BC",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* FD C1 */
    { "JP NZ, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD C2 */
    { "JP $^",          Z80::i_JP_nn,       ArgType::A16,   10, 3   },  /* FD C3 */
    { "CALL NZ, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD C4 */
    { "PUSH BC",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* FD C5 */
    { "ADD A, $*",      Z80::i_ADD_A_n,     ArgType::A8,    7,  2   },  /* FD C6 */
    { "RST $00",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD C7 */
    { "RET Z",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD C8 */
    { "RET",            Z80::i_RET,         ArgType::None,  10, 1   },  /* FD C9 */
    { "JP Z, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD CA */
    { "",               {},                 ArgType::GW,    4,  1   },  /* FD CB */
    { "CALL Z, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD CC */
    { "CALL $^",        Z80::i_CALL_nn,     ArgType::A16,   17, 3   },  /* FD CD */
    { "ADC A, $*",      Z80::i_ADC_A_n,     ArgType::A8,    7,  2   },  /* FD CE */
    { "RST $08",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD CF */

    { "RET NC",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD D0 */
    { "POP DE",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* FD D1 */
    { "JP NC, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD D2 */
    { "OUT ($*), A",    Z80::i_OUT_n_A,     ArgType::A8,    11, 2   },  /* FD D3 */
    { "CALL NC, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD D4 */
    { "PUSH DE",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* FD D5 */
    { "SUB $*",         Z80::i_SUB_A_n,     ArgType::A8,    7,  2   },  /* FD D6 */
    { "RST $10",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD D7 */
    { "RET C",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD D8 */
    { "EXX",            Z80::i_EXX,         ArgType::None,  4,  1   },  /* FD D9 */
    { "JP C, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD DA */
    { "IN A, ($*)",     Z80::i_IN_A_n,      ArgType::A8,    11, 2   },  /* FD DB */
    { "CALL C, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD DC */
    { "",               {},                 ArgType::GW,    4,  1   },  /* FD DD */
    { "SBC A, $*",      Z80::i_SBC_A_n,     ArgType::A8,    7,  2   },  /* FD DE */
    { "RST $18",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD DF */

    { "RET PO",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD E0 */
    { "POP IY",         Z80::i_POP_IY,      ArgType::None,  10, 1   },  /* FD E1 */
    { "JP PO, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD E2 */
    { "EX (SP), IY",    Z80::i_EX_mSP_IY,   ArgType::None,  19, 1   },  /* FD E3 */
    { "CALL PO, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD E4 */
    { "PUSH IY",        Z80::i_PUSH_IY,     ArgType::None,  11, 1   },  /* FD E5 */
    { "AND $*",         Z80::i_AND_A_n,     ArgType::A8,    7,  2   },  /* FD E6 */
    { "RST $20",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD E7 */
    { "RET PE",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD E8 */
    { "JP (IY)",        Z80::i_JP_IY,       ArgType::None,  4,  1   },  /* FD E9 */
    { "JP PE, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD EA */
    { "EX DE, HL",      Z80::i_EX_DE_HL,    ArgType::None,  4,  1   },  /* FD EB */
    { "CALL PE, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD EC */
    { "",               {},                 ArgType::GW,    4,  1   },  /* FD ED */
    { "XOR $*",         Z80::i_XOR_A_n,     ArgType::A8,    7,  2   },  /* FD EE */
    { "RST $28",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD EF */

    { "RET P",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD F0 */
    { "POP AF",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* FD F1 */
    { "JP P, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD F2 */
    { "DI",             Z80::i_DI,          ArgType::None,  4,  1   },  /* FD F3 */
    { "CALL P, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD F4 */
    { "PUSH AF",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* FD F5 */
    { "OR $*",          Z80::i_OR_A_n,      ArgType::A8,    7,  2   },  /* FD F6 */
    { "RST $30",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD F7 */
    { "RET M",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* FD F8 */
    { "LD SP, IY",      Z80::i_LD_SP_IY,    ArgType::None,  6,  1   },  /* FD F9 */
    { "JP M, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* FD FA */
    { "EI",             Z80::i_EI,          ArgType::None,  4,  1   },  /* FD FB */
    { "CALL M, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* FD FC */
    { "",               {},                 ArgType::GW,    4,  1   },  /* FD FD */
    { "CP $*",          Z80::i_CP_A_n,      ArgType::A8,    7,  2   },  /* FD FE */
    { "RST $38",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* FD FF */
};

int Z80::i_ADD_IY_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD IY, rr
     * The contents of register pair rr (any of register pairs BC, DE, IY, or SP)
     * are added to the contents of Index Register IY, and the results are stored in IY.
     *
     * S is not affected.
     * Z is not affected.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is not affected.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is rese
     */
    return self.add_xx_rr(self._regs.IY, op);
}

int Z80::i_LD_IY_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IY, nn
     */
    self._regs.IY = arg;
    return 0;
}

int Z80::i_LD_mnn_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (nn), IY
     */
    self.write_addr(arg, self._regs.IY);
    return 0;
}

int Z80::i_LD_IY_mnn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IY, (nn)
     */
    self._regs.IY = self.read_addr(arg);
    return 0;
}

int Z80::i_LD_IYH_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IYH, n - Undocumented
     */
    self._regs.IY = (arg << 8) | (self._regs.IY & 0x00FF);
    return 0;
}

int Z80::i_LD_IYL_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IYL, n - Undocumented
     */
    self._regs.IY = (self._regs.IY & 0xFF00) | (arg & 0x00FF);
    return 0;
}

int Z80::i_LD_r_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {A,B,C,D,E,IYH,IYL}, IYH - 7C,44,4C,54,5C - Undocumented
     * LD {A,B,C,D,E,IYH,IYL}, IYL - 7D,45,4D,55,5D - Undocumented
     */
    return self.ld_r_xxHL(self._regs.IY, op);
}

int Z80::i_LD_IYHL_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IYH, {A,B,C,D,E,IYH,IYL} - 67,60,61,62,63,64,65 - Undocumented
     * LD IYL, {A,B,C,D,E,IYH,IYL} - 6F,68,69,6A,6B,6C,6D - Undocumented
     */
    return self.ld_xxHL_r(self._regs.IY, op);
}

int Z80::i_LD_mIYd_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (IY+d), n
     */
    return self.ld_mxxd_n(self._regs.IY, arg);
}

int Z80::i_LD_mIYd_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (IY+d), {ABCDEHL}
     */
    return self.ld_mxxd_r(self._regs.IY, op, arg);
}

int Z80::i_LD_r_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, (IY+d)
     */
    return self.ld_r_mxxd(self._regs.IY, op, arg);
}

int Z80::i_INC_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC IY
     */
    ++self._regs.IY;
    return 0;
}

int Z80::i_INC_IYH(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC IYH - Undocumented.
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.inc_xxH(self._regs.IY);
}

int Z80::i_INC_IYL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC IYL - Undocumented.
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.inc_xxL(self._regs.IY);
}

int Z80::i_INC_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC (IY+d)
     * d = two complement displacement integer.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if (IY+d) was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    return self.inc_mxxd(self._regs.IY, arg);
}

int Z80::i_DEC_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC IY
     */
    --self._regs.IY;
    return 0;
}

int Z80::i_DEC_IYH(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC IYH - Undocumented
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.dec_xxH(self._regs.IY);
}

int Z80::i_DEC_IYL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC IYL - Undocumented
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.dec_xxL(self._regs.IY);
}

int Z80::i_DEC_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC (IY+d)
     * d = two complement displacement integer.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if m was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    return self.dec_mxxd(self._regs.IY, arg);
}

int Z80::i_ADD_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD IYH - 84 - Undocumented
     * ADD IYL - 85 - Undocumented
     *
     * C as defined
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    uint8_t value = self.reg_hilo(self._regs.IY, op);
    return self.add_A(value, 0);
}

int Z80::i_ADC_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC IYH - 8C - Undocumented
     * ADC IYL - 8D - Undocumented
     *
     * C as defined
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    uint8_t value = self.reg_hilo(self._regs.IY, op);
    return self.add_A(value, self.test_C());
}

int Z80::i_ADD_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, (IY+d)
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    int retval = self.add_A(value, 0);
    self._regs.memptr = addr;
    return retval;
}

int Z80::i_ADC_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, (IY+d)
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.add_A(value, self.test_C());
}

int Z80::i_SUB_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB IYH - 94 - Undocumented
     * SUB IYL - 95 - Undocumented
     */
    uint8_t value = self.reg_hilo(self._regs.IY, op);
    return self.sub_A(value, 0);
}

int Z80::i_SUB_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB (IY+d)
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    return self.sub_A(value, 0);
}

int Z80::i_SBC_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC IYH - 9C - Undocumented
     * SBC IYL - 9D - Undocumented
     */
    uint8_t value = self.reg_hilo(self._regs.IY, op);
    return self.sub_A(value, self.test_C());
}

int Z80::i_SBC_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC (IY+d)
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    return self.sub_A(value, self.test_C());
}

int Z80::i_AND_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND IYH - A4 - Undocumented
     * AND IYL - A5 - Undocumented
     */
    return self.and_A(self.reg_hilo(self._regs.IY, op));
}

int Z80::i_AND_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND (IY+d)
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    return self.and_A(value);
}

int Z80::i_XOR_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR IYH - AC - Undocumented
     * XOR IYL - AD - Undocumented
     */
    return self.xor_A(self.reg_hilo(self._regs.IY, op));
}

int Z80::i_XOR_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR (IY+d)
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    return self.xor_A(value);
}

int Z80::i_OR_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR IYH - B4 - Undocumented
     * OR IYL - B5 - Undocumented
     */
    return self.or_A(self.reg_hilo(self._regs.IY, op));
}

int Z80::i_OR_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR (IY+d)
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    return self.or_A(value);
}

int Z80::i_CP_A_IYHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP IYH - BC - Undocumented
     * CP IYL - BD - Undocumented
     */
    return self.cp_A(self.reg_hilo(self._regs.IY, op));
}

int Z80::i_CP_A_mIYd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP (IY+d)
     */
    addr_t addr = self._regs.IY + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    return self.cp_A(value);
}

int Z80::i_POP_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * POP IY
     */
    self._regs.IY = self.pop_addr();
    return 0;
}

int Z80::i_EX_mSP_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EX (SP), IY
     */
    uint16_t value = self.pop_addr();
    self.push_addr(self._regs.IY);
    self._regs.IY = value;
    self._regs.memptr = value;
    return 0;
}

int Z80::i_PUSH_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * PUSH IY
     */
    self.push_addr(self._regs.IY);
    return 0;
}

int Z80::i_JP_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JP (IY)
     */
    self._regs.PC = self._regs.IY;
    return 0;
}

int Z80::i_LD_SP_IY(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD SP, IY
     */
    self._regs.SP = self._regs.IY;
    return 0;
}

}
}
