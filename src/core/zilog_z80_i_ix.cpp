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

const Z80::Instruction Z80::ix_instr_set[256] = {
    { "NOP",            Z80::i_NOP,         ArgType::None,  4,  1   },  /* DD 00 */
    { "LD BC, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* DD 01 */
    { "LD (BC), A",     Z80::i_LD_mdd_A,    ArgType::None,  7,  1   },  /* DD 02 */
    { "INC BC",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* DD 03 */
    { "INC B",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* DD 04 */
    { "DEC B",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* DD 05 */
    { "LD B, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* DD 06 */
    { "RLCA",           Z80::i_RLCA,        ArgType::None,  4,  2   },  /* DD 07 */
    { "EX AF, AF'",     Z80::i_EX_AF_sAF,   ArgType::None,  4,  1   },  /* DD 08 */
    { "ADD IX, BC",     Z80::i_ADD_IX_rr,   ArgType::None,  11, 2   },  /* DD 09 */
    { "LD A, (BC)",     Z80::i_LD_A_mdd,    ArgType::None,  7,  1   },  /* DD 0A */
    { "DEC BC",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* DD 0B */
    { "INC C",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* DD 0C */
    { "DEC C",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* DD 0D */
    { "LD C, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* DD 0E */
    { "RRCA",           Z80::i_RRCA,        ArgType::None,  4,  1   },  /* DD 0F */

    { "DJNZ $+",        Z80::i_DJNZ,        ArgType::A8,    13, 2   },  /* DD 10 */
    { "LD DE, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* DD 11 */
    { "LD (DE), A",     Z80::i_LD_mdd_A,    ArgType::None,  7,  1   },  /* DD 12 */
    { "INC DE",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* DD 13 */
    { "INC D",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* DD 14 */
    { "DEC D",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* DD 15 */
    { "LD D, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* DD 16 */
    { "RLA",            Z80::i_RLA,         ArgType::None,  4,  1   },  /* DD 17 */
    { "JR $+",          Z80::i_JR,          ArgType::A8,    12, 1   },  /* DD 18 */
    { "ADD IX, DE",     Z80::i_ADD_IX_rr,   ArgType::None,  11, 2   },  /* DD 19 */
    { "LD A, (DE)",     Z80::i_LD_A_mdd,    ArgType::None,  7,  1   },  /* DD 1A */
    { "DEC DE",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* DD 1B */
    { "INC E",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* DD 1C */
    { "DEC E",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* DD 1D */
    { "LD E, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* DD 1E */
    { "RRA",            Z80::i_RRA,         ArgType::None,  4,  1   },  /* DD 1F */

    { "JR NZ, $+",      Z80::i_JR_NZ,       ArgType::A8,    12, 2   },  /* DD 20 */
    { "LD IX, $^",      Z80::i_LD_IX_nn,    ArgType::A16,   10, 3   },  /* DD 21 */
    { "LD ($^), IX",    Z80::i_LD_mnn_IX,   ArgType::A16,   16, 3   },  /* DD 22 */
    { "INC IX",         Z80::i_INC_IX,      ArgType::None,  6,  1   },  /* DD 23 */
    { "INC IXH",        Z80::i_INC_IXH,     ArgType::None,  4,  1   },  /* DD 24 */
    { "DEC IXH",        Z80::i_DEC_IXH,     ArgType::None,  4,  1   },  /* DD 25 */
    { "LD IXH, $*",     Z80::i_LD_IXH_n,    ArgType::A8,    7,  2   },  /* DD 26 */
    { "DAA",            Z80::i_DAA,         ArgType::None,  4,  1   },  /* DD 27 */
    { "JR Z, $+",       Z80::i_JR_Z,        ArgType::A8,    12, 2   },  /* DD 28 */
    { "ADD IX, IX",     Z80::i_ADD_IX_rr,   ArgType::None,  11, 1   },  /* DD 29 */
    { "LD IX, ($^)",    Z80::i_LD_IX_mnn,   ArgType::A16,   16, 3   },  /* DD 2A */
    { "DEC IX",         Z80::i_DEC_IX,      ArgType::None,  10, 1   },  /* DD 2B */
    { "INC IXL",        Z80::i_INC_IXL,     ArgType::None,  4,  1   },  /* DD 2C */
    { "DEC IXL",        Z80::i_DEC_IXL,     ArgType::None,  4,  1   },  /* DD 2D */
    { "LD IXL, $*",     Z80::i_LD_IXL_n,    ArgType::A8,    7,  2   },  /* DD 2E */
    { "CPL",            Z80::i_CPL,         ArgType::None,  4,  1   },  /* DD 2F */

    { "JR NC, $+",      Z80::i_JR_NC,       ArgType::A8,    12, 2   },  /* DD 30 */
    { "LD SP, $^",      Z80::i_LD_rr_nn,    ArgType::A16,   10, 3   },  /* DD 31 */
    { "LD ($^), A",     Z80::i_LD_mdd_A,    ArgType::A16,   13, 3   },  /* DD 32 */
    { "INC SP",         Z80::i_INC_rr,      ArgType::None,  6,  1   },  /* DD 33 */
    { "INC (IX%)" ,     Z80::i_INC_mIXd,    ArgType::A8,    11, 1   },  /* DD 34 */
    { "DEC (IX%)",      Z80::i_DEC_mIXd,    ArgType::A8,    11, 1   },  /* DD 35 */
    { "LD (IX%), $*",   Z80::i_LD_mIXd_n,   ArgType::A16,   15, 3   },  /* DD 36 */
    { "SCF",            Z80::i_SCF,         ArgType::None,  4,  1   },  /* DD 37 */
    { "JR C, $+",       Z80::i_JR_C,        ArgType::A8,    12, 2   },  /* DD 38 */
    { "ADD IX, SP",     Z80::i_ADD_IX_rr,   ArgType::None,  11, 1   },  /* DD 39 */
    { "LD A, ($^)",     Z80::i_LD_A_mdd,    ArgType::A16,   13, 3   },  /* DD 3A */
    { "DEC SP",         Z80::i_DEC_rr,      ArgType::None,  6,  1   },  /* DD 3B */
    { "INC A",          Z80::i_INC_r,       ArgType::None,  4,  1   },  /* DD 3C */
    { "DEC A",          Z80::i_DEC_r,       ArgType::None,  4,  1   },  /* DD 3D */
    { "LD A, $*",       Z80::i_LD_r_n,      ArgType::A8,    7,  2   },  /* DD 3E */
    { "CCF",            Z80::i_CCF,         ArgType::None,  4,  1   },  /* DD 3F */

    { "LD B, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 40 */
    { "LD B, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 41 */
    { "LD B, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 42 */
    { "LD B, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 43 */
    { "LD B, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 44 */
    { "LD B, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 45 */
    { "LD B, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 46 */
    { "LD B, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 47 */
    { "LD C, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 48 */
    { "LD C, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 49 */
    { "LD C, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 4A */
    { "LD C, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 4B */
    { "LD C, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 4C */
    { "LD C, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 4D */
    { "LD C, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 4E */
    { "LD C, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 4F */

    { "LD D, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 50 */
    { "LD D, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 51 */
    { "LD D, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 52 */
    { "LD D, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 53 */
    { "LD D, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 54 */
    { "LD D, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 55 */
    { "LD D, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 56 */
    { "LD D, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 57 */
    { "LD E, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 58 */
    { "LD E, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 59 */
    { "LD E, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 5A */
    { "LD E, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 5B */
    { "LD E, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 5C */
    { "LD E, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 5D */
    { "LD E, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 5E */
    { "LD E, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 5F */

    { "LD IXH, B",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 60 */
    { "LD IXH, C",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 61 */
    { "LD IXH, D",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 62 */
    { "LD IXH, E",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 63 */
    { "LD IXH, IXH",    Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 64 */
    { "LD IXH, IXL",    Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 65 */
    { "LD H, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 66 */
    { "LD IXH, A",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 67 */
    { "LD IXL, B",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 68 */
    { "LD IXL, C",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 69 */
    { "LD IXL, D",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 6A */
    { "LD IXL, E",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 6B */
    { "LD IXL, IXH",    Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 6C */
    { "LD IXL, IXL",    Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 6D */
    { "LD L, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 6E */
    { "LD IXL, A",      Z80::i_LD_IXHL_r,   ArgType::None,  4,  1   },  /* DD 6F */

    { "LD (IX%), B",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 70 */
    { "LD (IX%), C",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 71 */
    { "LD (IX%), D",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 72 */
    { "LD (IX%), E",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 73 */
    { "LD (IX%), H",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 74 */
    { "LD (IX%), L",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 75 */
    { "HALT",           Z80::i_HALT,        ArgType::None,  4,  1   },  /* DD 76 */
    { "LD (IX%), A",    Z80::i_LD_mIXd_r,   ArgType::A8,    15, 2   },  /* DD 77 */
    { "LD A, B",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 78 */
    { "LD A, C",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 79 */
    { "LD A, D",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 7A */
    { "LD A, E",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 7B */
    { "LD A, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 7C */
    { "LD A, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  4,  1   },  /* DD 7D */
    { "LD A, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    15, 2   },  /* DD 7E */
    { "LD A, A",        Z80::i_LD_r_r,      ArgType::None,  4,  1   },  /* DD 7F */

    { "ADD A, B",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* DD 80 */
    { "ADD A, C",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* DD 81 */
    { "ADD A, D",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* DD 82 */
    { "ADD A, E",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* DD 83 */
    { "ADD A, IXH",     Z80::i_ADD_A_IXHL,  ArgType::None,  4,  1   },  /* DD 84 */
    { "ADD A, IXL",     Z80::i_ADD_A_IXHL,  ArgType::None,  4,  1   },  /* DD 85 */
    { "ADD A, (IX%)",   Z80::i_ADD_A_mIXd,  ArgType::A8,    15, 2   },  /* DD 86 */
    { "ADD A, A",       Z80::i_ADD_A_r,     ArgType::None,  4,  1   },  /* DD 87 */
    { "ADC A, B",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* DD 88 */
    { "ADC A, C",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* DD 89 */
    { "ADC A, D",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* DD 8A */
    { "ADC A, E",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* DD 8B */
    { "ADC A, IXH",     Z80::i_ADC_A_IXHL,  ArgType::None,  4,  1   },  /* DD 8C */
    { "ADC A, IXL",     Z80::i_ADC_A_IXHL,  ArgType::None,  4,  1   },  /* DD 8D */
    { "ADC A, (IX%)",   Z80::i_ADC_A_mIXd,  ArgType::A8,    15, 2   },  /* DD 8E */
    { "ADC A, A",       Z80::i_ADC_A_r,     ArgType::None,  4,  1   },  /* DD 8F */

    { "SUB B",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* DD 90 */
    { "SUB C",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* DD 91 */
    { "SUB D",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* DD 92 */
    { "SUB E",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* DD 93 */
    { "SUB IXH",        Z80::i_SUB_A_IXHL,  ArgType::None,  4,  1   },  /* DD 94 */
    { "SUB IXL",        Z80::i_SUB_A_IXHL,  ArgType::None,  4,  1   },  /* DD 95 */
    { "SUB (IX%)",      Z80::i_SUB_A_mIXd,  ArgType::A8,    15, 2   },  /* DD 96 */
    { "SUB A",          Z80::i_SUB_A_r,     ArgType::None,  4,  1   },  /* DD 97 */
    { "SBC B",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* DD 98 */
    { "SBC C",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* DD 99 */
    { "SBC D",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* DD 9A */
    { "SBC E",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* DD 9B */
    { "SBC IXH",        Z80::i_SBC_A_IXHL,  ArgType::None,  4,  1   },  /* DD 9C */
    { "SBC IXL",        Z80::i_SBC_A_IXHL,  ArgType::None,  4,  1   },  /* DD 9D */
    { "SBC (IX%)",      Z80::i_SBC_A_mIXd,  ArgType::A8,    15, 2   },  /* DD 9E */
    { "SBC A",          Z80::i_SBC_A_r,     ArgType::None,  4,  1   },  /* DD 9F */

    { "AND B",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* DD A0 */
    { "AND C",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* DD A1 */
    { "AND D",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* DD A2 */
    { "AND E",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* DD A3 */
    { "AND IXH",        Z80::i_AND_A_IXHL,  ArgType::None,  4,  1   },  /* DD A4 */
    { "AND IXL",        Z80::i_AND_A_IXHL,  ArgType::None,  4,  1   },  /* DD A5 */
    { "AND (IX%)",      Z80::i_AND_A_mIXd,  ArgType::A8,    15, 2   },  /* DD A6 */
    { "AND A",          Z80::i_AND_A_r,     ArgType::None,  4,  1   },  /* DD A7 */
    { "XOR B",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* DD A8 */
    { "XOR C",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* DD A9 */
    { "XOR D",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* DD AA */
    { "XOR E",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* DD AB */
    { "XOR IXH",        Z80::i_XOR_A_IXHL,  ArgType::None,  4,  1   },  /* DD AC */
    { "XOR IXL",        Z80::i_XOR_A_IXHL,  ArgType::None,  4,  1   },  /* DD AD */
    { "XOR (IX%)",      Z80::i_XOR_A_mIXd,  ArgType::A8,    15, 2   },  /* DD AE */
    { "XOR A",          Z80::i_XOR_A_r,     ArgType::None,  4,  1   },  /* DD AF */

    { "OR B",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* DD B0 */
    { "OR C",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* DD B1 */
    { "OR D",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* DD B2 */
    { "OR E",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* DD B3 */
    { "OR IXH",         Z80::i_OR_A_IXHL,   ArgType::None,  4,  1   },  /* DD B4 */
    { "OR IXL",         Z80::i_OR_A_IXHL,   ArgType::None,  4,  1   },  /* DD B5 */
    { "OR (IX%)",       Z80::i_OR_A_mIXd,   ArgType::A8,    15, 2   },  /* DD B6 */
    { "OR A",           Z80::i_OR_A_r,      ArgType::None,  4,  1   },  /* DD B7 */
    { "CP B",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* DD B8 */
    { "CP C",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* DD B9 */
    { "CP D",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* DD BA */
    { "CP E",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* DD BB */
    { "CP IXH",         Z80::i_CP_A_IXHL,   ArgType::None,  4,  1   },  /* DD BC */
    { "CP IXL",         Z80::i_CP_A_IXHL,   ArgType::None,  4,  1   },  /* DD BD */
    { "CP (IX%)",       Z80::i_CP_A_mIXd,   ArgType::A8,    15, 2   },  /* DD BE */
    { "CP A",           Z80::i_CP_A_r,      ArgType::None,  4,  1   },  /* DD BF */

    { "RET NZ",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD C0 */
    { "POP BC",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* DD C1 */
    { "JP NZ, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD C2 */
    { "JP $^",          Z80::i_JP_nn,       ArgType::A16,   10, 3   },  /* DD C3 */
    { "CALL NZ, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD C4 */
    { "PUSH BC",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* DD C5 */
    { "ADD A, $*",      Z80::i_ADD_A_n,     ArgType::A8,    7,  2   },  /* DD C6 */
    { "RST $00",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD C7 */
    { "RET Z",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD C8 */
    { "RET",            Z80::i_RET,         ArgType::None,  10, 1   },  /* DD C9 */
    { "JP Z, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD CA */
    { "",               {},                 ArgType::GW,    4,  1   },  /* DD CB */
    { "CALL Z, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD CC */
    { "CALL $^",        Z80::i_CALL_nn,     ArgType::A16,   17, 3   },  /* DD CD */
    { "ADC A, $*",      Z80::i_ADC_A_n,     ArgType::A8,    7,  2   },  /* DD CE */
    { "RST $08",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD CF */

    { "RET NC",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD D0 */
    { "POP DE",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* DD D1 */
    { "JP NC, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD D2 */
    { "OUT ($*), A",    Z80::i_OUT_n_A,     ArgType::A8,    11, 2   },  /* DD D3 */
    { "CALL NC, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD D4 */
    { "PUSH DE",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* DD D5 */
    { "SUB $*",         Z80::i_SUB_A_n,     ArgType::A8,    7,  2   },  /* DD D6 */
    { "RST $10",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD D7 */
    { "RET C",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD D8 */
    { "EXX",            Z80::i_EXX,         ArgType::None,  4,  1   },  /* DD D9 */
    { "JP C, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD DA */
    { "IN A, ($*)",     Z80::i_IN_A_n,      ArgType::A8,    11, 2   },  /* DD DB */
    { "CALL C, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD DC */
    { "",               {},                 ArgType::GW,    4,  1   },  /* DD DD */
    { "SBC A, $*",      Z80::i_SBC_A_n,     ArgType::A8,    7,  2   },  /* DD DE */
    { "RST $18",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD DF */

    { "RET PO",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD E0 */
    { "POP IX",         Z80::i_POP_IX,      ArgType::None,  10, 1   },  /* DD E1 */
    { "JP PO, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD E2 */
    { "EX (SP), IX",    Z80::i_EX_mSP_IX,   ArgType::None,  19, 1   },  /* DD E3 */
    { "CALL PO, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD E4 */
    { "PUSH IX",        Z80::i_PUSH_IX,     ArgType::None,  11, 1   },  /* DD E5 */
    { "AND $*",         Z80::i_AND_A_n,     ArgType::A8,    7,  2   },  /* DD E6 */
    { "RST $20",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD E7 */
    { "RET PE",         Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD E8 */
    { "JP (IX)",        Z80::i_JP_IX,       ArgType::None,  4,  1   },  /* DD E9 */
    { "JP PE, $^",      Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD EA */
    { "EX DE, HL",      Z80::i_EX_DE_HL,    ArgType::None,  4,  1   },  /* DD EB */
    { "CALL PE, $^",    Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD EC */
    { "",               {},                 ArgType::GW,    4,  1   },  /* DD ED */
    { "XOR $*",         Z80::i_XOR_A_n,     ArgType::A8,    7,  2   },  /* DD EE */
    { "RST $28",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD EF */

    { "RET P",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD F0 */
    { "POP AF",         Z80::i_POP_rr,      ArgType::None,  10, 1   },  /* DD F1 */
    { "JP P, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD F2 */
    { "DI",             Z80::i_DI,          ArgType::None,  4,  1   },  /* DD F3 */
    { "CALL P, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD F4 */
    { "PUSH AF",        Z80::i_PUSH_rr,     ArgType::None,  11, 1   },  /* DD F5 */
    { "OR $*",          Z80::i_OR_A_n,      ArgType::A8,    7,  2   },  /* DD F6 */
    { "RST $30",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD F7 */
    { "RET M",          Z80::i_RET_cc,      ArgType::None,  11, 1   },  /* DD F8 */
    { "LD SP, IX",      Z80::i_LD_SP_IX,    ArgType::None,  6,  1   },  /* DD F9 */
    { "JP M, $^",       Z80::i_JP_cc_nn,    ArgType::A16,   10, 3   },  /* DD FA */
    { "EI",             Z80::i_EI,          ArgType::None,  4,  1   },  /* DD FB */
    { "CALL M, $^",     Z80::i_CALL_cc_nn,  ArgType::A16,   17, 3   },  /* DD FC */
    { "",               {},                 ArgType::GW,    4,  1   },  /* DD FD */
    { "CP $*",          Z80::i_CP_A_n,      ArgType::A8,    7,  2   },  /* DD FE */
    { "RST $38",        Z80::i_RST_p,       ArgType::None,  11, 1   },  /* DD FF */
};

uint8_t Z80::reg_hilo(uint16_t reg, uint8_t op)
{
    return ((op & 1) == 0 ? (reg >> 8) : (reg & 0xFF));
}

int Z80::add_xx_rr(uint16_t& dst_reg, uint8_t op)
{
    /*
     * ADD IX, rr
     * ADD IY, rr
     * The contents of register pair rr (BC, DE, IX/IX, or SP)
     * are added to the contents of Index Register IX/IY,
     * and the results are stored in IX/IY.
     *
     * S is not affected.
     * Z is not affected.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is not affected.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is reset
     */
    constexpr static uint8_t REG16_MASK = 0x30;
    uint8_t r = op & REG16_MASK;
    uint16_t src_reg = (r == 0x00 ? _regs.BC :
                       (r == 0x10 ? _regs.DE :
                       (r == 0x20 ? dst_reg  : _regs.SP)));

    add16(dst_reg, src_reg);
    _regs.memptr = dst_reg + 1;
    return 0;
}

int Z80::ld_r_xxHL(uint16_t& src_reg, uint8_t op)
{
    /*
     * LD {A,B,C,D,E}, IXH - DD 7C, DD 44, DD 4C, DD 54, DD 5C - Undocumented
     * LD {A,B,C,D,E}, IXL - DD 7D, DD 45, DD 4D, DD 55, DD 5D - Undocumented
     *
     * LD {A,B,C,D,E}, IYH - FD 7C, FD 44, FD 4C, FD 54, FD 5C - Undocumented
     * LD {A,B,C,D,E}, IYL - FD 7D, FD 45, FD 4D, FD 55, FD 5D - Undocumented
     */
    uint8_t value = reg_hilo(src_reg, op);
    uint8_t& dst_reg = reg8_from_opcode(op);
    dst_reg = value;
    return 0;
}

int Z80::ld_xxHL_r(uint16_t& dst_reg, uint8_t op)
{
    /*
     * LD IXH, {A,B,C,D,E,IXH,IXL} - DD 67, DD 60, DD 61, DD 62, DD 63, DD 64, DD 65 - Undocumented
     * LD IXL, {A,B,C,D,E,IXH,IXL} - DD 6F, DD 68, DD 69, DD 6A, DD 6B, DD 6C, DD 6D - Undocumented
     *
     * LD IYH, {A,B,C,D,E,IXH,IXL} - FD 67, FD 60, FD 61, FD 62, FD 63, FD 64, FD 65 - Undocumented
     * LD IYL, {A,B,C,D,E,IXH,IXL} - FD 6F, FD 68, FD 69, FD 6A, FD 6B, FD 6C, FD 6D - Undocumented
     */
    switch (op) {
    case 0x64: /* LD IYH,IYH  LD IXH,IXH */
    case 0x6D: /* LD IYL,IYL  LD IXL,IXL */
        break;
    case 0x65: /* LD IYH,IYL  LD IXH,IXL */
        dst_reg = (dst_reg << 8) | (dst_reg & 0x00FF);
        break;
    case 0x6C: /* LD IYL,IYH  LD IXL,IXH */
        dst_reg = (dst_reg & 0xFF00) | (dst_reg >> 8);
        break;
    default: {
        uint16_t src_reg = reg8_src_from_opcode(op);
        bool lo = (op & 8);
        if (lo) {
            dst_reg = (dst_reg & 0xFF00) | src_reg;
        } else {
            dst_reg = (dst_reg & 0x00FF) | (src_reg << 8);
        }
    }}

    return 0;
}

int Z80::ld_mxxd_n(uint16_t& reg, addr_t arg)
{
    /*
     * LD (IX+d), n
     * LD (IY+d), n
     */
    addr_t addr = reg + static_cast<int8_t>(arg);
    uint8_t value = arg >> 8;
    _regs.memptr = addr;
    write(addr, value);
    return 0;
}

int Z80::ld_mxxd_r(uint16_t& reg, uint8_t op, addr_t arg)
{
    /*
     * LD (IX+d), {ABCDEHL}
     * LD (IY+d), {ABCDEHL}
     */
    uint8_t& src_reg = reg8_src_from_opcode(op);
    addr_t addr = reg + static_cast<int8_t>(arg);
    _regs.memptr = addr;
    write(addr, src_reg);
    return 0;
}

int Z80::ld_r_mxxd(uint16_t& reg, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, (IX+d)
     * LD {ABCDEHL}, (IY+d)
     */
    uint8_t& dst_reg = reg8_from_opcode(op);
    addr_t addr = reg + static_cast<int8_t>(arg);
    _regs.memptr = addr;
    uint8_t value = read(addr);
    dst_reg = value;
    return 0;
}

int Z80::inc_xxH(uint16_t& dst_reg)
{
    /*
     * INC IXH - Undocumented.
     * INC IYH - Undocumented.
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    bool C = test_C();
    uint16_t result = add8(dst_reg >> 8, 1, 0);
    dst_reg = (result << 8) | (dst_reg & 0x00FF);
    flag_C(C);
    return 0;
}

int Z80::inc_xxL(uint16_t& dst_reg)
{
    /*
     * INC IXL - Undocumented.
     * INC IYL - Undocumented.
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    bool C = test_C();
    uint8_t result = add8(dst_reg & 0x00FF, 1, 0);
    dst_reg = (dst_reg & 0xFF00) | result;
    flag_C(C);
    return 0;
}

int Z80::inc_mxxd(uint16_t& reg, addr_t arg)
{
    /*
     * INC (IX+d)
     * INC (IY+d)
     * d = two complement displacement integer.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if (IX+d) was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    addr_t addr = reg + static_cast<int8_t>(arg);
    _regs.memptr = addr;
    uint8_t value = read(addr);
    bool C = test_C();
    value = add8(value, 1, 0);
    flag_C(C);
    write(addr, value);
    _regs.memptr = addr;
    return 0;
}

int Z80::dec_xxH(uint16_t& dst_reg)
{
    /*
     * DEC IXH - Undocumented
     * DEC IYH - Undocumented
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    bool C = test_C();
    uint16_t result = sub8(dst_reg >> 8, 1, 0);
    dst_reg = (result << 8) | (dst_reg & 0x00FF);
    flag_C(C);
    return 0;
}

int Z80::dec_xxL(uint16_t& dst_reg)
{
    /*
     * DEC IXL - Undocumented
     * DEC IYL - Undocumented
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    bool C = test_C();
    uint8_t result = sub8(dst_reg & 0x00FF, 1, 0);
    dst_reg = (dst_reg & 0xFF00) | result;
    flag_C(C);
    return 0;
}

int Z80::dec_mxxd(uint16_t& reg, addr_t arg)
{
    /*
     * DEC (IX+d)
     * DEC (IX+d)
     * d = two complement displacement integer.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if m was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    addr_t addr = reg + static_cast<int8_t>(arg);
    uint8_t value = read(addr);
    bool C = test_C();
    value = sub8(value, 1, 0);
    flag_C(C);
    write(addr, value);
    _regs.memptr = addr;
    return 0;
}

int Z80::i_ADD_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD IXH - 84 - Undocumented
     * ADD IXL - 85 - Undocumented
     *
     * C as defined
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
//FIXME REMAKE
    uint8_t value = self.reg_hilo(self._regs.IX, op);
    return self.add_A(value, 0);
}

int Z80::i_ADD_IX_rr(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD IX, rr
     * The contents of register pair rr (any of register pairs BC, DE, IX, or SP)
     * are added to the contents of Index Register IX, and the results are stored in IX.
     *
     * S is not affected.
     * Z is not affected.
     * H is set if carry from bit 11; otherwise, it is reset.
     * P/V is not affected.
     * N is reset.
     * C is set if carry from bit 15; otherwise, it is rese
     */
    return self.add_xx_rr(self._regs.IX, op);
}

int Z80::i_LD_IX_nn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IX, nn
     */
    self._regs.IX = arg;
    return 0;
}

int Z80::i_LD_mnn_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (nn), IX
     */
    self.write_addr(arg, self._regs.IX);
    return 0;
}

int Z80::i_LD_IX_mnn(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IX, (nn)
     */
    self._regs.IX = self.read_addr(arg);
    return 0;
}

int Z80::i_LD_IXH_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IXH, n - Undocumented
     */
    self._regs.IXh = arg;
    return 0;
}

int Z80::i_LD_IXL_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IXL, n - Undocumented
     */
    self._regs.IXl = arg;
    return 0;
}

int Z80::i_LD_r_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {A,B,C,D,E}, IXH - 7C,44,4C,54,5C - Undocumented
     * LD {A,B,C,D,E}, IXL - 7D,45,4D,55,5D - Undocumented
     */
    return self.ld_r_xxHL(self._regs.IX, op);
}

int Z80::i_LD_IXHL_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IXH, {A,B,C,D,E,IXH,IXL} - 67,60,61,62,63,64,65 - Undocumented
     * LD IXL, {A,B,C,D,E,IXH,IXL} - 6F,68,69,6A,6B,6C,6D - Undocumented
     */
    return self.ld_xxHL_r(self._regs.IX, op);
}

int Z80::i_LD_mIXd_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (IX+d), n
     */
    return self.ld_mxxd_n(self._regs.IX, arg);
}

int Z80::i_LD_mIXd_r(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD (IX+d), {ABCDEHL}
     */
    return self.ld_mxxd_r(self._regs.IX, op, arg);
}

int Z80::i_LD_r_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD {ABCDEHL}, (IX+d)
     */
    return self.ld_r_mxxd(self._regs.IX, op, arg);
}

int Z80::i_INC_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC IX
     */
    ++self._regs.IX;
    return 0;
}

int Z80::i_INC_IXH(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC IXH - Undocumented.
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.inc_xxH(self._regs.IX);
}

int Z80::i_INC_IXL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC IXL - Undocumented.
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.inc_xxL(self._regs.IX);
}

int Z80::i_INC_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * INC (IX+d)
     * d = two complement displacement integer.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if (IX+d) was 7Fh before operation; otherwise, it is reset.
     * N is reset.
     * C is not affected.
     */
    return self.inc_mxxd(self._regs.IX, arg);
}

int Z80::i_DEC_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC IX
     */
    --self._regs.IX;
    return 0;
}

int Z80::i_DEC_IXH(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC IXH - Undocumented
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.dec_xxH(self._regs.IX);
}

int Z80::i_DEC_IXL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC IXL - Undocumented
     *
     * C unaffected
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    return self.dec_xxL(self._regs.IX);
}

int Z80::i_DEC_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * DEC (IX+d)
     * d = two complement displacement integer.
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if borrow from bit 4, otherwise, it is reset.
     * P/V is set if m was 80h before operation; otherwise, it is reset.
     * N is set.
     * C is not affected.
     */
    return self.dec_mxxd(self._regs.IX, arg);
}

int Z80::i_ADC_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC IXH - 8C - Undocumented
     * ADC IXL - 8D - Undocumented
     *
     * C as defined
     * N as defined
     * P/V detects overflow
     * H as defined
     * Z as defined
     * S as defined
     */
    uint8_t value = self.reg_hilo(self._regs.IX, op);
    return self.add_A(value, self.test_C());
}

int Z80::i_ADD_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADD A, (IX+d)
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    int value = self.read(addr);
    int retval = self.add_A(value, 0);
    self._regs.memptr = addr;
    return retval;
}

int Z80::i_ADC_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * ADC A, (IX+d)
     *
     * S is set if result is negative; otherwise, it is reset.
     * Z is set if result is 0; otherwise, it is reset.
     * H is set if carry from bit 3; otherwise, it is reset.
     * P/V is set if overflow; otherwise, it is reset.
     * N is reset.
     * C is set if carry from bit 7; otherwise, it is reset.
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.add_A(value, self.test_C());
}

int Z80::i_SUB_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB IXH - 94 - Undocumented
     * SUB IXL - 95 - Undocumented
     */
    uint8_t value = self.reg_hilo(self._regs.IX, op);
    return self.sub_A(value, 0);
}

int Z80::i_SUB_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SUB (IX+d)
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    int value = self.read(addr);
    self._regs.memptr = addr;
    return self.sub_A(value, 0);
}

int Z80::i_SBC_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC IXH - 9C - Undocumented
     * SBC IXL - 9D - Undocumented
     */
    uint8_t value = self.reg_hilo(self._regs.IX, op);
    return self.sub_A(value, self.test_C());
}

int Z80::i_SBC_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * SBC (IX+d)
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.sub_A(value, self.test_C());
}

int Z80::i_AND_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND IXH - A4 - Undocumented
     * AND IXL - A5 - Undocumented
     */
    return self.and_A(self.reg_hilo(self._regs.IX, op));
}

int Z80::i_AND_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * AND (IX+d)
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.and_A(value);
}

int Z80::i_XOR_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR IXH - AC - Undocumented
     * XOR IXL - AD - Undocumented
     */
    return self.xor_A(self.reg_hilo(self._regs.IX, op));
}

int Z80::i_XOR_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * XOR (IX+d)
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.xor_A(value);
}

int Z80::i_OR_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR IXH - B4 - Undocumented
     * OR IXL - B5 - Undocumented
     */
    return self.or_A(self.reg_hilo(self._regs.IX, op));
}

int Z80::i_OR_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * OR (IX+d)
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.or_A(value);
}

int Z80::i_CP_A_IXHL(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP IXH - BC - Undocumented
     * CP IXL - BD - Undocumented
     */
    return self.cp_A(self.reg_hilo(self._regs.IX, op));
}

int Z80::i_CP_A_mIXd(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * CP (IX+d)
     */
    addr_t addr = self._regs.IX + static_cast<int8_t>(arg);
    uint8_t value = self.read(addr);
    self._regs.memptr = addr;
    return self.cp_A(value);
}

int Z80::i_POP_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * POP IX
     */
    self._regs.IX = self.pop_addr();
    return 0;
}

int Z80::i_EX_mSP_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * EX (SP), IX
     */
    uint16_t value = self.pop_addr();
    self.push_addr(self._regs.IX);
    self._regs.IX = value;
    self._regs.memptr = value;
    return 0;
}

int Z80::i_PUSH_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * PUSH IX
     */
    self.push_addr(self._regs.IX);
    return 0;
}

int Z80::i_JP_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * JP (IX)
     */
    self._regs.PC = self._regs.IX;
    self._regs.memptr = self._regs.IX;
    return 0;
}

int Z80::i_LD_SP_IX(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD SP, IX
     */
    self._regs.SP = self._regs.IX;
    return 0;
}

}
}
