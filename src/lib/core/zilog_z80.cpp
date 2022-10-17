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

#include <sstream>


namespace caio {

/*
 * Most of the information in the following tables come from:
 * - Z80 CPU User Manual UM008011-0816.
 * - https://clrhome.org/table/
 */
const std::array<ZilogZ80::Instruction, 256> ZilogZ80::instr_set{{
    { "NOP",            ZilogZ80::i_NOP,        ArgType::None,  1,  4,  1   },  /* 00 */
    { "LD BC, $^",      ZilogZ80::i_LD_rr_nn,   ArgType::A16,   2,  10, 3   },  /* 01 */
    { "LD (BC), A",     ZilogZ80::i_LD_mdd_A,   ArgType::None,  2,  7,  1   },  /* 02 */
    { "INC BC",         ZilogZ80::i_INC_rr,     ArgType::None,  1,  6,  1   },  /* 03 */
    { "INC B",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 04 */
    { "DEC B",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 05 */
    { "LD B, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 06 */
    { "RLCA",           ZilogZ80::i_RLCA,       ArgType::None,  1,  4,  1   },  /* 07 */
    { "EX AF, AF'",     ZilogZ80::i_EX_AF_sAF,  ArgType::None,  1,  4,  1   },  /* 08 */
    { "ADD HL, BC",     ZilogZ80::i_ADD_HL_rr,  ArgType::None,  3,  11, 1   },  /* 09 */
    { "LD A, (BC)",     ZilogZ80::i_LD_A_mdd,   ArgType::None,  2,  7,  1   },  /* 0A */
    { "DEC BC",         ZilogZ80::i_DEC_rr,     ArgType::None,  1,  6,  1   },  /* 0B */
    { "INC C",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 0C */
    { "DEC C",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 0D */
    { "LD C, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 0E */
    { "RRCA",           ZilogZ80::i_RRCA,       ArgType::None,  1,  4,  1   },  /* 0F */

    { "DJNZ $+",        ZilogZ80::i_DJNZ,       ArgType::A8,    3,  13, 2   },  /* 10 */
    { "LD DE, $^",      ZilogZ80::i_LD_rr_nn,   ArgType::A16,   2,  10, 3   },  /* 11 */
    { "LD (DE), A",     ZilogZ80::i_LD_mdd_A,   ArgType::None,  2,  7,  1   },  /* 12 */
    { "INC DE",         ZilogZ80::i_INC_rr,     ArgType::None,  1,  6,  1   },  /* 13 */
    { "INC D",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 14 */
    { "DEC D",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 15 */
    { "LD D, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 16 */
    { "RLA" ,           ZilogZ80::i_RLA,        ArgType::None,  1,  4,  1   },  /* 17 */
    { "JR $+" ,         ZilogZ80::i_JR,         ArgType::A8,    3,  12, 2   },  /* 18 */
    { "ADD HL, DE",     ZilogZ80::i_ADD_HL_rr,  ArgType::None,  3,  11, 1   },  /* 19 */
    { "LD A, (DE)",     ZilogZ80::i_LD_A_mdd,   ArgType::None,  2,  7,  1   },  /* 1A */
    { "DEC DE",         ZilogZ80::i_DEC_rr,     ArgType::None,  1,  6,  1   },  /* 1B */
    { "INC E",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 1C */
    { "DEC E",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 1D */
    { "LD E, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 1E */
    { "RRA",            ZilogZ80::i_RRA,        ArgType::None,  1,  4,  1   },  /* 1F */

    { "JR NZ, $+",      ZilogZ80::i_JR_NZ,      ArgType::A8,    3,  12, 2   },  /* 20 */
    { "LD HL, $^",      ZilogZ80::i_LD_rr_nn,   ArgType::A16,   2,  10, 3   },  /* 21 */
    { "LD ($^), HL",    ZilogZ80::i_LD_mnn_HL,  ArgType::A16,   5,  16, 3   },  /* 22 */
    { "INC HL",         ZilogZ80::i_INC_rr,     ArgType::None,  1,  6,  1   },  /* 23 */
    { "INC H",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 24 */
    { "DEC H",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 25 */
    { "LD H, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 26 */
    { "DAA",            ZilogZ80::i_DAA,        ArgType::None,  1,  4,  1   },  /* 27 */
    { "JR Z, $+",       ZilogZ80::i_JR_Z,       ArgType::A8,    3,  12, 2   },  /* 28 */
    { "ADD HL, HL",     ZilogZ80::i_ADD_HL_rr,  ArgType::None,  3,  11, 1   },  /* 29 */
    { "LD HL, ($^)",    ZilogZ80::i_LD_HL_mnn,  ArgType::A16,   5,  16, 3   },  /* 2A */
    { "DEC HL",         ZilogZ80::i_DEC_rr,     ArgType::None,  1,  6,  1   },  /* 2B */
    { "INC L",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 2C */
    { "DEC L",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 2D */
    { "LD L, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 2E */
    { "CPL",            ZilogZ80::i_CPL,        ArgType::None,  1,  4,  2   },  /* 2F */

    { "JR NC, $+",      ZilogZ80::i_JR_NC,      ArgType::A8,    3,  12, 2   },  /* 30 */
    { "LD SP, $^",      ZilogZ80::i_LD_rr_nn,   ArgType::A16,   2,  10, 3   },  /* 31 */
    { "LD ($^), A",     ZilogZ80::i_LD_mdd_A,   ArgType::A16,   4,  13, 3   },  /* 32 */
    { "INC SP",         ZilogZ80::i_INC_rr,     ArgType::None,  1,  6,  1   },  /* 33 */
    { "INC (HL)",       ZilogZ80::i_INC_mHL,    ArgType::None,  3,  11, 1   },  /* 34 */
    { "DEC (HL)",       ZilogZ80::i_DEC_mHL,    ArgType::None,  3,  11, 1   },  /* 35 */
    { "LD (HL), $*",    ZilogZ80::i_LD_mHL_n,   ArgType::A8,    3,  10, 2   },  /* 36 */
    { "SCF",            ZilogZ80::i_SCF,        ArgType::None,  4,      1   },  /* 37 */
    { "JR C, $+",       ZilogZ80::i_JR_C,       ArgType::A8,    3,  12, 2   },  /* 38 */
    { "ADD HL, SP",     ZilogZ80::i_ADD_HL_rr,  ArgType::None,  3,  11, 1   },  /* 39 */
    { "LD A, ($^)",     ZilogZ80::i_LD_A_mdd,   ArgType::A16,   4,  13, 3   },  /* 3A */
    { "DEC SP",         ZilogZ80::i_DEC_rr,     ArgType::None,  1,  6,  1   },  /* 3B */
    { "INC A",          ZilogZ80::i_INC_r,      ArgType::None,  1,  4,  1   },  /* 3C */
    { "DEC A",          ZilogZ80::i_DEC_r,      ArgType::None,  1,  4,  1   },  /* 3D */
    { "LD A, $*",       ZilogZ80::i_LD_r_n,     ArgType::A8,    2,  7,  2   },  /* 3E */
    { "CCF",            ZilogZ80::i_CCF,        ArgType::None,  1,  4,  1   },  /* 3F */

    { "LD B, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 40 */
    { "LD B, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 41 */
    { "LD B, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 42 */
    { "LD B, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 43 */
    { "LD B, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 44 */
    { "LD B, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 45 */
    { "LD B, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 46 */
    { "LD B, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 47 */
    { "LD C, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 48 */
    { "LD C, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 49 */
    { "LD C, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 4A */
    { "LD C, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 4B */
    { "LD C, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 4C */
    { "LD C, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 4D */
    { "LD C, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 4E */
    { "LD C, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 4F */

    { "LD D, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 50 */
    { "LD D, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 51 */
    { "LD D, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 52 */
    { "LD D, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 53 */
    { "LD D, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 54 */
    { "LD D, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 55 */
    { "LD D, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 56 */
    { "LD D, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 57 */
    { "LD E, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 58 */
    { "LD E, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 59 */
    { "LD E, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 5A */
    { "LD E, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 5B */
    { "LD E, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 5C */
    { "LD E, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 5D */
    { "LD E, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 5E */
    { "LD E, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 5F */

    { "LD H, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 60 */
    { "LD H, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 61 */
    { "LD H, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 62 */
    { "LD H, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 63 */
    { "LD H, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 64 */
    { "LD H, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 65 */
    { "LD H, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 66 */
    { "LD H, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 67 */
    { "LD L, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 68 */
    { "LD L, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 69 */
    { "LD L, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 6A */
    { "LD L, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 6B */
    { "LD L, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 6C */
    { "LD L, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 6D */
    { "LD L, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 6E */
    { "LD L, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 6F */

    { "LD (HL), B",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 70 */
    { "LD (HL), C",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 71 */
    { "LD (HL), D",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 72 */
    { "LD (HL), E",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 73 */
    { "LD (HL), H",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 74 */
    { "LD (HL), L",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 75 */
    { "HALT",           ZilogZ80::i_HALT,       ArgType::None,  1,  4,  1   },  /* 76 */
    { "LD (HL), A",     ZilogZ80::i_LD_mHL_r,   ArgType::None,  2,  7,  1   },  /* 77 */
    { "LD A, B",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 78 */
    { "LD A, C",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 79 */
    { "LD A, D",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 7A */
    { "LD A, E",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 7B */
    { "LD A, H",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 7C */
    { "LD A, L",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 7D */
    { "LD A, (HL)",     ZilogZ80::i_LD_r_mHL,   ArgType::None,  2,  7,  1   },  /* 7E */
    { "LD A, A",        ZilogZ80::i_LD_r_r,     ArgType::None,  1,  4,  1   },  /* 7F */

    { "ADD A, B",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 80 */
    { "ADD A, C",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 81 */
    { "ADD A, D",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 82 */
    { "ADD A, E",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 83 */
    { "ADD A, H",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 84 */
    { "ADD A, L",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 85 */
    { "ADD A, (HL)",    ZilogZ80::i_ADD_A_mHL,  ArgType::None,  2,  7,  1   },  /* 86 */
    { "ADD A, A",       ZilogZ80::i_ADD_A_r,    ArgType::None,  1,  4,  1   },  /* 87 */
    { "ADC A, B",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 88 */
    { "ADC A, C",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 89 */
    { "ADC A, D",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 8A */
    { "ADC A, E",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 8B */
    { "ADC A, H",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 8C */
    { "ADC A, L",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 8D */
    { "ADC A, (HL)",    ZilogZ80::i_ADC_A_mHL,  ArgType::None,  2,  7,  1   },  /* 8E */
    { "ADC A, A",       ZilogZ80::i_ADC_A_r,    ArgType::None,  1,  4,  1   },  /* 8F */

    { "SUB B",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 90 */
    { "SUB C",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 91 */
    { "SUB D",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 92 */
    { "SUB E",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 93 */
    { "SUB H",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 94 */
    { "SUB L",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 95 */
    { "SUB (HL)",       ZilogZ80::i_SUB_A_mHL,  ArgType::None,  2,  7,  1   },  /* 96 */
    { "SUB A",          ZilogZ80::i_SUB_A_r,    ArgType::None,  1,  4,  1   },  /* 97 */
    { "SBC B",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 98 */
    { "SBC C",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 99 */
    { "SBC D",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 9A */
    { "SBC E",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 9B */
    { "SBC H",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 9C */
    { "SBC L",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 9D */
    { "SBC (HL)",       ZilogZ80::i_SBC_A_mHL,  ArgType::None,  2,  7,  1   },  /* 9E */
    { "SBC A",          ZilogZ80::i_SBC_A_r,    ArgType::None,  1,  4,  1   },  /* 9F */

    { "AND B",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A0 */
    { "AND C",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A1 */
    { "AND D",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A2 */
    { "AND E",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A3 */
    { "AND H",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A4 */
    { "AND L",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A5 */
    { "AND (HL)",       ZilogZ80::i_AND_A_mHL,  ArgType::None,  2,  7,  1   },  /* A6 */
    { "AND A",          ZilogZ80::i_AND_A_r,    ArgType::None,  1,  4,  1   },  /* A7 */
    { "XOR B",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* A8 */
    { "XOR C",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* A9 */
    { "XOR D",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* AA */
    { "XOR E",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* AB */
    { "XOR H",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* AC */
    { "XOR L",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* AD */
    { "XOR (HL)",       ZilogZ80::i_XOR_A_mHL,  ArgType::None,  2,  7,  1   },  /* AE */
    { "XOR A",          ZilogZ80::i_XOR_A_r,    ArgType::None,  1,  4,  1   },  /* AF */

    { "OR B",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B0 */
    { "OR C",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B1 */
    { "OR D",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B2 */
    { "OR E",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B3 */
    { "OR H",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B4 */
    { "OR L",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B5 */
    { "OR (HL)",        ZilogZ80::i_OR_A_mHL,   ArgType::None,  2,  7,  1   },  /* B6 */
    { "OR A",           ZilogZ80::i_OR_A_r,     ArgType::None,  1,  4,  1   },  /* B7 */
    { "CP B",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* B8 */
    { "CP C",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* B9 */
    { "CP D",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* BA */
    { "CP E",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* BB */
    { "CP H",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* BC */
    { "CP L",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* BD */
    { "CP (HL)",        ZilogZ80::i_CP_A_mHL,   ArgType::None,  2,  7,  1   },  /* BE */
    { "CP A",           ZilogZ80::i_CP_A_r,     ArgType::None,  1,  4,  1   },  /* BF */

    { "RET NZ",         ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* C0 */
    { "POP BC",         ZilogZ80::i_POP_rr,     ArgType::None,  3,  10, 1   },  /* C1 */
    { "JP NZ, $^",      ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* C2 */
    { "JP $^",          ZilogZ80::i_JP_nn,      ArgType::A16,   3,  10, 3   },  /* C3 */
    { "CALL NZ, $^",    ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 1   },  /* C4 */
    { "PUSH BC",        ZilogZ80::i_PUSH_rr,    ArgType::None,  3,  11, 1   },  /* C5 */
    { "ADD A, $*",      ZilogZ80::i_ADD_A_n,    ArgType::A8,    2,  7,  2   },  /* C6 */
    { "RST $00",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* C7 */
    { "RET Z",          ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* C8 */
    { "RET",            ZilogZ80::i_RET,        ArgType::None,  3,  10, 1   },  /* C9 */
    { "JP Z, $^",       ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* CA */
    { "",               {},                     ArgType::Bit,   0,  0,  0   },  /* CB */
    { "CALL Z, $^",     ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* CC */
    { "CALL $^",        ZilogZ80::i_CALL_nn,    ArgType::A16,   5,  17, 3   },  /* CD */
    { "ADC A, $*",      ZilogZ80::i_ADC_A_n,    ArgType::A8,    2,  7,  2   },  /* CE */
    { "RST $08",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* CF */

    { "RET NC",         ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* D0 */
    { "POP DE",         ZilogZ80::i_POP_rr,     ArgType::None,  3,  10, 1   },  /* D1 */
    { "JP NC, $^",      ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* D2 */
    { "OUT ($*), A",    ZilogZ80::i_OUT_n_A,    ArgType::A8,    3,  11, 2   },  /* D3 */
    { "CALL NC, $^",    ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* D4 */
    { "PUSH DE",        ZilogZ80::i_PUSH_rr,    ArgType::None,  3,  11, 1   },  /* D5 */
    { "SUB $*",         ZilogZ80::i_SUB_A_n,    ArgType::A8,    2,  7,  2   },  /* D6 */
    { "RST $10",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* D7 */
    { "RET C",          ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* D8 */
    { "EXX",            ZilogZ80::i_EXX,        ArgType::None,  1,  4,  1   },  /* D9 */
    { "JP C, $^",       ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* DA */
    { "IN A, ($*)",     ZilogZ80::i_IN_A_n,     ArgType::A8,    3,  11, 2   },  /* DB */
    { "CALL C, $^",     ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* DC */
    { "",               {},                     ArgType::IX,    0,  0,  1   },  /* DD */
    { "SBC A, $*",      ZilogZ80::i_SBC_A_n,    ArgType::A8,    2,  7,  2   },  /* DE */
    { "RST $18",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* DF */

    { "RET PO",         ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* E0 */
    { "POP HL",         ZilogZ80::i_POP_rr,     ArgType::None,  3,  10, 1   },  /* E1 */
    { "JP PO, $^",      ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* E2 */
    { "EX (SP), HL",    ZilogZ80::i_EX_mSP_HL,  ArgType::None,  5,  19, 1   },  /* E3 */
    { "CALL PO, $^",    ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* E4 */
    { "PUSH HL",        ZilogZ80::i_PUSH_rr,    ArgType::None,  3,  11, 1   },  /* E5 */
    { "AND $*",         ZilogZ80::i_AND_A_n,    ArgType::A8,    2,  7,  2   },  /* E6 */
    { "RST $20",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* E7 */
    { "RET PE",         ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* E8 */
    { "JP (HL)",        ZilogZ80::i_JP_HL,      ArgType::None,  1,  4,  1   },  /* E9 */
    { "JP PE, $^",      ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* EA */
    { "EX DE, HL",      ZilogZ80::i_EX_DE_HL,   ArgType::None,  1,  4,  1   },  /* EB */
    { "CALL PE, $^",    ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* EC */
    { "",               {},                     ArgType::MI,    0,  0,  0   },  /* ED */
    { "XOR $*",         ZilogZ80::i_XOR_A_n,    ArgType::A8,    2,  7,  2   },  /* EE */
    { "RST $28",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* EF */

    { "RET P",          ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* F0 */
    { "POP AF",         ZilogZ80::i_POP_rr,     ArgType::None,  3,  10, 1   },  /* F1 */
    { "JP P, $^",       ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* F2 */
    { "DI",             ZilogZ80::i_DI,         ArgType::None,  1,  4,  1   },  /* F3 */
    { "CALL P, $^",     ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* F4 */
    { "PUSH AF",        ZilogZ80::i_PUSH_rr,    ArgType::None,  3,  11, 1   },  /* F5 */
    { "OR $*",          ZilogZ80::i_OR_A_n,     ArgType::A8,    2,  7,  2   },  /* F6 */
    { "RST $30",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* F7 */
    { "RET M",          ZilogZ80::i_RET_cc,     ArgType::None,  3,  11, 1   },  /* F8 */
    { "LD SP, HL",      ZilogZ80::i_LD_SP_HL,   ArgType::None,  1,  6,  3   },  /* F9 */
    { "JP M, $^",       ZilogZ80::i_JP_cc_nn,   ArgType::A16,   3,  10, 3   },  /* FA */
    { "EI",             ZilogZ80::i_EI,         ArgType::None,  1,  4,  1   },  /* FB */
    { "CALL M, $^",     ZilogZ80::i_CALL_cc_nn, ArgType::A16,   5,  17, 3   },  /* FC */
    { "",               {},                     ArgType::IY,    0,  0,  0   },  /* FD */
    { "CP $*",          ZilogZ80::i_CP_A_n,     ArgType::A8,    2,  7,  2   },  /* FE */
    { "RST $38",        ZilogZ80::i_RST_p,      ArgType::None,  3,  11, 1   },  /* FF */
}};


#if 0
00 NOP            -              RLC  B      rlc (iy+0)->b    MOS_QUIT
01 LD   BC,&0000  -              RLC  C      rlc (iy+0)->c    MOS_CLI
02 LD   (BC),A    -              RLC  D      rlc (iy+0)->d    MOS_BYTE
03 INC  BC        -              RLC  E      rlc (iy+0)->e    MOS_WORD
04 INC  B         -              RLC  H      rlc (iy+0)->h    MOS_WRCH
05 DEC  B         -              RLC  L      rlc (iy+0)->l    MOS_RDCH
06 LD   B,&00     -              RLC  (HL)   RLC  (IY+0)      MOS_FILE
07 RLCA           -              RLC  A      rlc (iy+0)->a    MOS_ARGS
08 EX   AF,AF'    -              RRC  B      rrc (iy+0)->b    MOS_BGET
09 ADD  HL,BC     ADD  IX,BC     RRC  C      rrc (iy+0)->c    MOS_BPUT
0A LD   A,(BC)    -              RRC  D      rrc (iy+0)->d    MOS_GBPB
0B DEC  BC        -              RRC  E      rrc (iy+0)->e    MOS_FIND
0C INC  C         -              RRC  H      rrc (iy+0)->h    MOS_FF0C
0D DEC  C         -              RRC  L      rrc (iy+0)->l    MOS_FF0D
0E LD   C,&00     -              RRC  (HL)   RRC  (IY+0)      MOS_FF0E
0F RRCA           -              RRC  A      rrc (iy+0)->a    MOS_FF0F

10 DJNZ &4546     -              RL   B      rl  (iy+0)->b    -
11 LD   DE,&0000  -              RL   C      rl  (iy+0)->c    -
12 LD   (DE),A    -              RL   D      rl  (iy+0)->d    -
13 INC  DE        -              RL   E      rl  (iy+0)->e    -
14 INC  D         -              RL   H      rl  (iy+0)->h    -
15 DEC  D         -              RL   L      rl  (iy+0)->l    -
16 LD   D,&00     -              RL   (HL)   RL   (IY+0)      -
17 RLA            -              RL   A      rl  (iy+0)->a    -
18 JR   &4546     -              RR   B      rr  (iy+0)->b    -
19 ADD  HL,DE     ADD  IX,DE     RR   C      rr  (iy+0)->c    -
1A LD   A,(DE)    -              RR   D      rr  (iy+0)->d    -
1B DEC  DE        -              RR   E      rr  (iy+0)->e    -
1C INC  E         -              RR   H      rr  (iy+0)->h    -
1D DEC  E         -              RR   L      rr  (iy+0)->l    -
1E LD   E,&00     -              RR   (HL)   RR   (IY+0)      -
1F RRA            -              RR   A      rr  (iy+0)->a    -

20 JR   NZ,&4546  -              SLA  B      sla (iy+0)->b    -
21 LD   HL,&0000  LD   IX,&0000  SLA  C      sla (iy+0)->c    -
22 LD  (&0000),HL LD  (&0000),IX SLA  D      sla (iy+0)->d    -
23 INC  HL        INC  IX        SLA  E      sla (iy+0)->e    -
24 INC  H         INC  IXH       SLA  H      sla (iy+0)->h    -
25 DEC  H         DEC  IXH       SLA  L      sla (iy+0)->l    -
26 LD   H,&00     LD   IXH,&00   SLA  (HL)   SLA  (IY+0)      -
27 DAA            -              SLA  A      sla (iy+0)->a    -
28 JR   Z,&4546   -              SRA  B      sra (iy+0)->b    -
29 ADD  HL,HL     ADD  IX,IX     SRA  C      sra (iy+0)->c    -
2A LD  HL,(&0000) LD  IX,(&0000) SRA  D      sra (iy+0)->d    -
2B DEC  HL        DEC  IX        SRA  E      sra (iy+0)->e    -
2C INC  L         INC  IXL       SRA  H      sra (iy+0)->h    -
2D DEC  L         DEC  IXL       SRA  L      sra (iy+0)->l    -
2E LD   L,&00     LD   IXL,&00   SRA  (HL)   SRA  (IY+0)      -
2F CPL            -              SRA  A      sra (iy+0)->a    -
30 JR   NC,&4546  -              SLS  B      sls (iy+0)->b    -
31 LD   SP,&0000  -              SLS  C      sls (iy+0)->c    -
32 LD   (&0000),A -              SLS  D      sls (iy+0)->d    -
33 INC  SP        -              SLS  E      sls (iy+0)->e    -
34 INC  (HL)      INC  (IX+0)    SLS  H      sls (iy+0)->h    -
35 DEC  (HL)      DEC  (IX+0)    SLS  L      sls (iy+0)->l    -
36 LD   (HL),&00  LD  (IX+0),&00 SLS  (HL)   SLS  (IY+0)      -
37 SCF            -              SLS  A      sls (iy+0)->a    -
38 JR   C,&4546   -              SRL  B      srl (iy+0)->b    -
39 ADD  HL,SP     ADD  IX,SP     SRL  C      srl (iy+0)->c    -
3A LD   A,(&0000) -              SRL  D      srl (iy+0)->d    -
3B DEC  SP        -              SRL  E      srl (iy+0)->e    -
3C INC  A         -              SRL  H      srl (iy+0)->h    -
3D DEC  A         -              SRL  L      srl (iy+0)->l    -
3E LD   A,&00     -              SRL  (HL)   SRL  (IY+0)      -
3F CCF            -              SRL  A      srl (iy+0)->a    -
40 LD   B,B       -              BIT  0,B    bit 0,(iy+0)->b  IN   B,(C)
41 LD   B,C       -              BIT  0,C    bit 0,(iy+0)->c  OUT  (C),B
42 LD   B,D       -              BIT  0,D    bit 0,(iy+0)->d  SBC  HL,BC
43 LD   B,E       -              BIT  0,E    bit 0,(iy+0)->e  LD   (&0000),BC
44 LD   B,H       LD   B,IXH     BIT  0,H    bit 0,(iy+0)->h  NEG
45 LD   B,L       LD   B,IXL     BIT  0,L    bit 0,(iy+0)->l  RETN
46 LD   B,(HL)    LD   B,(IX+0)  BIT  0,(HL) BIT  0,(IY+0)    IM   0
47 LD   B,A       -              BIT  0,A    bit 0,(iy+0)->a  LD   I,A
48 LD   C,B       -              BIT  1,B    bit 1,(iy+0)->b  IN   C,(C)
49 LD   C,C       -              BIT  1,C    bit 1,(iy+0)->c  OUT  (C),C
4A LD   C,D       -              BIT  1,D    bit 1,(iy+0)->d  ADC  HL,BC
4B LD   C,E       -              BIT  1,E    bit 1,(iy+0)->e  LD   BC,(&0000)
4C LD   C,H       LD   C,IXH     BIT  1,H    bit 1,(iy+0)->h  [neg]
4D LD   C,L       LD   C,IXL     BIT  1,L    bit 1,(iy+0)->l  RETI
4E LD   C,(HL)    LD   C,(IX+0)  BIT  1,(HL) BIT  1,(IY+0)    [im0]
4F LD   C,A       -              BIT  1,A    bit 1,(iy+0)->a  LD   R,A
50 LD   D,B       -              BIT  2,B    bit 2,(iy+0)->b  IN   D,(C)
51 LD   D,C       -              BIT  2,C    bit 2,(iy+0)->c  OUT  (C),D
52 LD   D,D       -              BIT  2,D    bit 2,(iy+0)->d  SBC  HL,DE
53 LD   D,E       -              BIT  2,E    bit 2,(iy+0)->e  LD   (&0000),DE
54 LD   D,H       LD   D,IXH     BIT  2,H    bit 2,(iy+0)->h  [neg]
55 LD   D,L       LD   D,IXL     BIT  2,L    bit 2,(iy+0)->l  [retn]
56 LD   D,(HL)    LD   D,(IX+0)  BIT  2,(HL) BIT  2,(IY+0)    IM   1
57 LD   D,A       -              BIT  2,A    bit 2,(iy+0)->a  LD   A,I
58 LD   E,B       -              BIT  3,B    bit 3,(iy+0)->b  IN   E,(C)
59 LD   E,C       -              BIT  3,C    bit 3,(iy+0)->c  OUT  (C),E
5A LD   E,D       -              BIT  3,D    bit 3,(iy+0)->d  ADC  HL,DE
5B LD   E,E       -              BIT  3,E    bit 3,(iy+0)->e  LD   DE,(&0000)
5C LD   E,H       LD   E,IXH     BIT  3,H    bit 3,(iy+0)->h  [neg]
5D LD   E,L       LD   E,IXL     BIT  3,L    bit 3,(iy+0)->l  [reti]
5E LD   E,(HL)    LD   E,(IX+0)  BIT  3,(HL) BIT  3,(IY+0)    IM   2
5F LD   E,A       -              BIT  3,A    bit 3,(iy+0)->a  LD   A,R
60 LD   H,B       LD   IXH,B     BIT  4,B    bit 4,(iy+0)->b  IN   H,(C)
61 LD   H,C       LD   IXH,C     BIT  4,C    bit 4,(iy+0)->c  OUT  (C),H
62 LD   H,D       LD   IXH,D     BIT  4,D    bit 4,(iy+0)->d  SBC  HL,HL
63 LD   H,E       LD   IXH,E     BIT  4,E    bit 4,(iy+0)->e  LD   (&0000),HL
64 LD   H,H       LD   IXH,IXH   BIT  4,H    bit 4,(iy+0)->h  [neg]
65 LD   H,L       LD   IXH,IXL   BIT  4,L    bit 4,(iy+0)->l  [retn]
66 LD   H,(HL)    LD   H,(IX+0)  BIT  4,(HL) BIT  4,(IY+0)    [im0]
67 LD   H,A       LD   IXH,A     BIT  4,A    bit 4,(iy+0)->a  RRD
68 LD   L,B       LD   IXL,B     BIT  5,B    bit 5,(iy+0)->b  IN   L,(C)
69 LD   L,C       LD   IXL,C     BIT  5,C    bit 5,(iy+0)->c  OUT  (C),L
6A LD   L,D       LD   IXL,D     BIT  5,D    bit 5,(iy+0)->d  ADC  HL,HL
6B LD   L,E       LD   IXL,E     BIT  5,E    bit 5,(iy+0)->e  LD   HL,(&0000)
6C LD   L,H       LD   IXL,IXH   BIT  5,H    bit 5,(iy+0)->h  [neg]
6D LD   L,L       LD   IXL,IXL   BIT  5,L    bit 5,(iy+0)->l  [reti]
6E LD   L,(HL)    LD   L,(IX+0)  BIT  5,(HL) BIT  5,(IY+0)    [im0]
6F LD   L,A       LD   IXL,A     BIT  5,A    bit 5,(iy+0)->a  RLD
70 LD   (HL),B    LD   (IX+0),B  BIT  6,B    bit 6,(iy+0)->b  IN   F,(C)
71 LD   (HL),C    LD   (IX+0),C  BIT  6,C    bit 6,(iy+0)->c  OUT  (C),F
72 LD   (HL),D    LD   (IX+0),D  BIT  6,D    bit 6,(iy+0)->d  SBC  HL,SP
73 LD   (HL),E    LD   (IX+0),E  BIT  6,E    bit 6,(iy+0)->e  LD   (&0000),SP
74 LD   (HL),H    LD   (IX+0),H  BIT  6,H    bit 6,(iy+0)->h  [neg]
75 LD   (HL),L    LD   (IX+0),L  BIT  6,L    bit 6,(iy+0)->l  [retn]
76 HALT           -              BIT  6,(HL) BIT  6,(IY+0)    [im1]
77 LD   (HL),A    LD   (IX+0),A  BIT  6,A    bit 6,(iy+0)->a  [ld i,i?]
78 LD   A,B       -              BIT  7,B    bit 7,(iy+0)->b  IN   A,(C)
79 LD   A,C       -              BIT  7,C    bit 7,(iy+0)->c  OUT  (C),A
7A LD   A,D       -              BIT  7,D    bit 7,(iy+0)->d  ADC  HL,SP
7B LD   A,E       -              BIT  7,E    bit 7,(iy+0)->e  LD   SP,(&0000)
7C LD   A,H       LD   A,IXH     BIT  7,H    bit 7,(iy+0)->h  [neg]
7D LD   A,L       LD   A,IXL     BIT  7,L    bit 7,(iy+0)->l  [reti]
7E LD   A,(HL)    LD   A,(IX+0)  BIT  7,(HL) BIT  7,(IY+0)    [im2]
7F LD   A,A       -              BIT  7,A    bit 7,(iy+0)->a  [ld r,r?]
80 ADD  A,B       -              RES  0,B    res 0,(iy+0)->b  -
81 ADD  A,C       -              RES  0,C    res 0,(iy+0)->c  -
82 ADD  A,D       -              RES  0,D    res 0,(iy+0)->d  -
83 ADD  A,E       -              RES  0,E    res 0,(iy+0)->e  -
84 ADD  A,H       ADD  A,IXH     RES  0,H    res 0,(iy+0)->h  -
85 ADD  A,L       ADD  A,IXL     RES  0,L    res 0,(iy+0)->l  -
86 ADD  A,(HL)    ADD  A,(IX+0)  RES  0,(HL) RES  0,(IY+0)    -
87 ADD  A,A       -              RES  0,A    res 0,(iy+0)->a  -
88 ADC  A,B       -              RES  1,B    res 1,(iy+0)->b  -
89 ADC  A,C       -              RES  1,C    res 1,(iy+0)->c  -
8A ADC  A,D       -              RES  1,D    res 1,(iy+0)->d  -
8B ADC  A,E       -              RES  1,E    res 1,(iy+0)->e  -
8C ADC  A,H       ADC  A,IXH     RES  1,H    res 1,(iy+0)->h  -
8D ADC  A,L       ADC  A,IXL     RES  1,L    res 1,(iy+0)->l  -
8E ADC  A,(HL)    ADC  A,(IX+0)  RES  1,(HL) RES  1,(IY+0)    -
8F ADC  A,A       -              RES  1,A    res 1,(iy+0)->a  -
90 SUB  A,B       -              RES  2,B    res 2,(iy+0)->b  -
91 SUB  A,C       -              RES  2,C    res 2,(iy+0)->c  -
92 SUB  A,D       -              RES  2,D    res 2,(iy+0)->d  -
93 SUB  A,E       -              RES  2,E    res 2,(iy+0)->e  -
94 SUB  A,H       SUB  A,IXH     RES  2,H    res 2,(iy+0)->h  -
95 SUB  A,L       SUB  A,IXL     RES  2,L    res 2,(iy+0)->l  -
96 SUB  A,(HL)    SUB  A,(IX+0)  RES  2,(HL) RES  2,(IY+0)    -
97 SUB  A,A       -              RES  2,A    res 2,(iy+0)->a  -
98 SBC  A,B       -              RES  3,B    res 3,(iy+0)->b  -
99 SBC  A,C       -              RES  3,C    res 3,(iy+0)->c  -
9A SBC  A,D       -              RES  3,D    res 3,(iy+0)->d  -
9B SBC  A,E       -              RES  3,E    res 3,(iy+0)->e  -
9C SBC  A,H       SBC  A,IXH     RES  3,H    res 3,(iy+0)->h  -
9D SBC  A,L       SBC  A,IXL     RES  3,L    res 3,(iy+0)->l  -
9E SBC  A,(HL)    SBC  A,(IX+0)  RES  3,(HL) RES  3,(IY+0)    -
9F SBC  A,A       -              RES  3,A    res 3,(iy+0)->a  -
A0 AND  B         -              RES  4,B    res 4,(iy+0)->b  LDI
A1 AND  C         -              RES  4,C    res 4,(iy+0)->c  CPI
A2 AND  D         -              RES  4,D    res 4,(iy+0)->d  INI
A3 AND  E         -              RES  4,E    res 4,(iy+0)->e  OTI
A4 AND  H         AND  IXH       RES  4,H    res 4,(iy+0)->h  -
A5 AND  L         AND  IXL       RES  4,L    res 4,(iy+0)->l  -
A6 AND  (HL)      AND  (IX+0)    RES  4,(HL) RES  4,(IY+0)    -
A7 AND  A         -              RES  4,A    res 4,(iy+0)->a  -
A8 XOR  B         -              RES  5,B    res 5,(iy+0)->b  LDD
A9 XOR  C         -              RES  5,C    res 5,(iy+0)->c  CPD
AA XOR  D         -              RES  5,D    res 5,(iy+0)->d  IND
AB XOR  E         -              RES  5,E    res 5,(iy+0)->e  OTD
AC XOR  H         XOR  IXH       RES  5,H    res 5,(iy+0)->h  -
AD XOR  L         XOR  IXL       RES  5,L    res 5,(iy+0)->l  -
AE XOR  (HL)      XOR  (IX+0)    RES  5,(HL) RES  5,(IY+0)    -
AF XOR  A         -              RES  5,A    res 5,(iy+0)->a  -
B0 OR   B         -              RES  6,B    res 6,(iy+0)->b  LDIR
B1 OR   C         -              RES  6,C    res 6,(iy+0)->c  CPIR
B2 OR   D         -              RES  6,D    res 6,(iy+0)->d  INIR
B3 OR   E         -              RES  6,E    res 6,(iy+0)->e  OTIR
B4 OR   H         OR   IXH       RES  6,H    res 6,(iy+0)->h  -
B5 OR   L         OR   IXL       RES  6,L    res 6,(iy+0)->l  -
B6 OR   (HL)      OR   (IX+0)    RES  6,(HL) RES  6,(IY+0)    -
B7 OR   A         -              RES  6,A    res 6,(iy+0)->a  -
B8 CP   B         -              RES  7,B    res 7,(iy+0)->b  LDDR
B9 CP   C         -              RES  7,C    res 7,(iy+0)->c  CPDR
BA CP   D         -              RES  7,D    res 7,(iy+0)->d  INDR
BB CP   E         -              RES  7,E    res 7,(iy+0)->e  OTDR
BC CP   H         CP   IXH       RES  7,H    res 7,(iy+0)->h  -
BD CP   L         CP   IXL       RES  7,L    res 7,(iy+0)->l  -
BE CP   (HL)      CP   (IX+0)    RES  7,(HL) RES  7,(IY+0)    -
BF CP   A         -              RES  7,A    res 7,(iy+0)->a  -
C0 RET  NZ        -              SET  0,B    set 0,(iy+0)->b  -
C1 POP  BC        -              SET  0,C    set 0,(iy+0)->c  -
C2 JP   NZ,&0000  -              SET  0,D    set 0,(iy+0)->d  -
C3 JP   &0000     -              SET  0,E    set 0,(iy+0)->e  -
C4 CALL NZ,&0000  -              SET  0,H    set 0,(iy+0)->h  -
C5 PUSH BC        -              SET  0,L    set 0,(iy+0)->l  -
C6 ADD  A,&00     -              SET  0,(HL) SET  0,(IY+0)    -
C7 RST  &00       -              SET  0,A    set 0,(iy+0)->a  -
C8 RET  Z         -              SET  1,B    set 1,(iy+0)->b  -
C9 RET            -              SET  1,C    set 1,(iy+0)->c  -
CA JP   Z,&0000   -              SET  1,D    set 1,(iy+0)->d  -
CB **** CB ****   -              SET  1,E    set 1,(iy+0)->e  -
CC CALL Z,&0000   -              SET  1,H    set 1,(iy+0)->h  -
CD CALL &0000     -              SET  1,L    set 1,(iy+0)->l  -
CE ADC  A,&00     -              SET  1,(HL) SET  1,(IY+0)    -
CF RST  &08       -              SET  1,A    set 1,(iy+0)->a  -
D0 RET  NC        -              SET  2,B    set 2,(iy+0)->b  -
D1 POP  DE        -              SET  2,C    set 2,(iy+0)->c  -
D2 JP   NC,&0000  -              SET  2,D    set 2,(iy+0)->d  -
D3 OUT  (&00),A   -              SET  2,E    set 2,(iy+0)->e  -
D4 CALL NC,&0000  -              SET  2,H    set 2,(iy+0)->h  -
D5 PUSH DE        -              SET  2,L    set 2,(iy+0)->l  -
D6 SUB  A,&00     -              SET  2,(HL) SET  2,(IY+0)    -
D7 RST  &10       -              SET  2,A    set 2,(iy+0)->a  -
D8 RET  C         -              SET  3,B    set 3,(iy+0)->b  -
D9 EXX            -              SET  3,C    set 3,(iy+0)->c  -
DA JP   C,&0000   -              SET  3,D    set 3,(iy+0)->d  -
DB IN   A,(&00)   -              SET  3,E    set 3,(iy+0)->e  -
DC CALL C,&0000   -              SET  3,H    set 3,(iy+0)->h  -
DD **** DD ****   -              SET  3,L    set 3,(iy+0)->l  -
DE SBC  A,&00     -              SET  3,(HL) SET  3,(IY+0)    -
DF RST  &18       -              SET  3,A    set 3,(iy+0)->a  -
E0 RET  PO        -              SET  4,B    set 4,(iy+0)->b  -
E1 POP  HL        POP  IX        SET  4,C    set 4,(iy+0)->c  -
E2 JP   PO,&0000  -              SET  4,D    set 4,(iy+0)->d  -
E3 EX   (SP),HL   EX   (SP),IX   SET  4,E    set 4,(iy+0)->e  -
E4 CALL PO,&0000  -              SET  4,H    set 4,(iy+0)->h  -
E5 PUSH HL        PUSH IX        SET  4,L    set 4,(iy+0)->l  -
E6 AND  &00       -              SET  4,(HL) SET  4,(IY+0)    -
E7 RST  &20       -              SET  4,A    set 4,(iy+0)->a  -
E8 RET  PE        -              SET  5,B    set 5,(iy+0)->b  -
E9 JP   (HL)      JP   (IX)      SET  5,C    set 5,(iy+0)->c  -
EA JP   PE,&0000  -              SET  5,D    set 5,(iy+0)->d  -
EB EX   DE,HL     -              SET  5,E    set 5,(iy+0)->e  -
EC CALL PE,&0000  -              SET  5,H    set 5,(iy+0)->h  -
ED **** ED ****   -              SET  5,L    set 5,(iy+0)->l  -
EE XOR  &00       -              SET  5,(HL) SET  5,(IY+0)    -
EF RST  &28       -              SET  5,A    set 5,(iy+0)->a  -
F0 RET  P         -              SET  6,B    set 6,(iy+0)->b  -
F1 POP  AF        -              SET  6,C    set 6,(iy+0)->c  -
F2 JP   P,&0000   -              SET  6,D    set 6,(iy+0)->d  -
F3 DI             -              SET  6,E    set 6,(iy+0)->e  -
F4 CALL P,&0000   -              SET  6,H    set 6,(iy+0)->h  -
F5 PUSH AF        -              SET  6,L    set 6,(iy+0)->l  -
F6 OR   &00       -              SET  6,(HL) SET  6,(IY+0)    -
F7 RST  &30       -              SET  6,A    set 6,(iy+0)->a  -
F8 RET  M         -              SET  7,B    set 7,(iy+0)->b  [z80]
F9 LD   SP,HL     -              SET  7,C    set 7,(iy+0)->c  [z80]
FA JP   M,&0000   -              SET  7,D    set 7,(iy+0)->d  [z80]
FB EI             -              SET  7,E    set 7,(iy+0)->e  ED_LOAD
FC CALL M,&0000   -              SET  7,H    set 7,(iy+0)->h  [z80]
FD **** FD ****   -              SET  7,L    set 7,(iy+0)->l  [z80]
FE CP   &00       -              SET  7,(HL) SET  7,(IY+0)    [z80]
FF RST  &38       -              SET  7,A    set 7,(iy+0)->a  ED_DOS
#endif


std::string ZilogZ80::Registers::to_string(ZilogZ80::Flags fl)
{
    std::ostringstream ss{};

    ss << ((fl & Flags::S) ? "S" : "-")
       << ((fl & Flags::Z) ? "Z" : "-")
       << "-"
       << ((fl & Flags::H) ? "H" : "-")
       << "-"
       << ((fl & Flags::V) ? "V" : "-")
       << ((fl & Flags::N) ? "N" : "-")
       << ((fl & Flags::C) ? "C" : "-");

    return ss.str();
}

std::string ZilogZ80::Registers::to_string() const
{
    std::ostringstream ss{};

    ss << "AF="    << utils::to_string(AF)
       << "  BC="  << utils::to_string(BC)
       << "  DE="  << utils::to_string(DE)
       << "  HL="  << utils::to_string(HL)
       << "  AF'=" << utils::to_string(aAF)
       << "  BC'=" << utils::to_string(aBC)
       << "  DE'=" << utils::to_string(aDE)
       << "  HL'=" << utils::to_string(aHL)
       << std::endl
       << "IX="    << utils::to_string(IX)
       << "  IY="  << utils::to_string(IY)
       << "  SP="  << utils::to_string(SP)
       << "  PC="  << utils::to_string(PC)
       << "  I="   << utils::to_string(I)
       << "  R="   << utils::to_string(R)
       << "  F="   << to_string(static_cast<Flags>(F))
       << "  F'="  << to_string(static_cast<Flags>(aF));

    return ss.str();
}

ZilogZ80::ZilogZ80(const std::string &type, const std::string &label)
    : Name{type, label}
{
}

ZilogZ80::ZilogZ80(const std::shared_ptr<ASpace> &mmap, const std::string &type, const std::string &label)
    : Name{type, (label.empty() ? LABEL : label)}
{
    init(mmap);
}

ZilogZ80::~ZilogZ80()
{
}

void ZilogZ80::init_monitor(std::istream &is, std::ostream &os)
{
    static std::map<std::string, std::function<int(const ZilogZ80 &)>> regvals{
        { "ra",    [](const ZilogZ80 &cpu) { return cpu._regs.A;    }},
        { "rf",    [](const ZilogZ80 &cpu) { return cpu._regs.F;    }},
        { "raf",   [](const ZilogZ80 &cpu) { return cpu._regs.AF;   }},
        { "rb",    [](const ZilogZ80 &cpu) { return cpu._regs.B;    }},
        { "rc",    [](const ZilogZ80 &cpu) { return cpu._regs.C;    }},
        { "rbc",   [](const ZilogZ80 &cpu) { return cpu._regs.BC;   }},
        { "rd",    [](const ZilogZ80 &cpu) { return cpu._regs.D;    }},
        { "re",    [](const ZilogZ80 &cpu) { return cpu._regs.E;    }},
        { "rde",   [](const ZilogZ80 &cpu) { return cpu._regs.DE;   }},
        { "rh",    [](const ZilogZ80 &cpu) { return cpu._regs.H;    }},
        { "rl",    [](const ZilogZ80 &cpu) { return cpu._regs.L;    }},
        { "rhl",   [](const ZilogZ80 &cpu) { return cpu._regs.HL;   }},
        { "ra'",   [](const ZilogZ80 &cpu) { return cpu._regs.aA;   }},
        { "rf'",   [](const ZilogZ80 &cpu) { return cpu._regs.aF;   }},
        { "raf'",  [](const ZilogZ80 &cpu) { return cpu._regs.aAF;  }},
        { "rb'",   [](const ZilogZ80 &cpu) { return cpu._regs.aB;   }},
        { "rc'",   [](const ZilogZ80 &cpu) { return cpu._regs.aC;   }},
        { "rbc'",  [](const ZilogZ80 &cpu) { return cpu._regs.aBC;  }},
        { "rd'",   [](const ZilogZ80 &cpu) { return cpu._regs.aD;   }},
        { "re'",   [](const ZilogZ80 &cpu) { return cpu._regs.aE;   }},
        { "rde'",  [](const ZilogZ80 &cpu) { return cpu._regs.aDE;  }},
        { "rh'",   [](const ZilogZ80 &cpu) { return cpu._regs.aH;   }},
        { "rl'",   [](const ZilogZ80 &cpu) { return cpu._regs.aL;   }},
        { "rhl'",  [](const ZilogZ80 &cpu) { return cpu._regs.aHL;  }},
        { "ri",    [](const ZilogZ80 &cpu) { return cpu._regs.I;    }},
        { "rr",    [](const ZilogZ80 &cpu) { return cpu._regs.R;    }},
        { "rx",    [](const ZilogZ80 &cpu) { return cpu._regs.IX;   }},
        { "ry",    [](const ZilogZ80 &cpu) { return cpu._regs.IY;   }},
        { "rsp",   [](const ZilogZ80 &cpu) { return cpu._regs.SP;   }},
        { "rpc",   [](const ZilogZ80 &cpu) { return cpu._regs.PC;   }},
        { "rf.s",  [](const ZilogZ80 &cpu) { return cpu.test_S();   }},
        { "rf.z",  [](const ZilogZ80 &cpu) { return cpu.test_Z();   }},
        { "rf.h",  [](const ZilogZ80 &cpu) { return cpu.test_H();   }},
        { "rf.v",  [](const ZilogZ80 &cpu) { return cpu.test_V();   }},
        { "rf.n",  [](const ZilogZ80 &cpu) { return cpu.test_N();   }},
        { "rf.c",  [](const ZilogZ80 &cpu) { return cpu.test_C();   }},
        { "rf'.s", [](const ZilogZ80 &cpu) { return cpu.test_aS();  }},
        { "rf'.z", [](const ZilogZ80 &cpu) { return cpu.test_aZ();  }},
        { "rf'.h", [](const ZilogZ80 &cpu) { return cpu.test_aH();  }},
        { "rf'.v", [](const ZilogZ80 &cpu) { return cpu.test_aV();  }},
        { "rf'.n", [](const ZilogZ80 &cpu) { return cpu.test_aN();  }},
        { "rf'.c", [](const ZilogZ80 &cpu) { return cpu.test_aC();  }}
    };

    if (!_mmap) {
        throw InvalidArgument{*this, "System mappings not defined"};
    }

    auto regs = [this](std::ostream &os) {
        os << this->_regs.to_string();
    };

    auto pc = [this]() -> addr_t & {
        return this->_regs.PC;
    };

    auto read = [this](addr_t addr) {
        return this->read(addr);
    };

    auto write = [this](addr_t addr, uint8_t data) {
        this->write(addr, data);
    };

    auto disass = [this](std::ostream &os, addr_t start, addr_t count, bool show_pc) {
        this->disass(os, start, count, show_pc);
    };

    auto mmap = [this]() {
        return this->_mmap;
    };

    auto ebreak = [this]() {
        this->ebreak();
    };

    auto load = [/* this */](const std::string &fname, addr_t &start) {
#if 0 /* TODO */
        PrgFile prog{fname};
        addr_t addr = prog.address();

        if (start != 0) {
            addr = start;
            prog.address(start);
        }

        for (auto c : prog) {
            this->write(addr++, c);
        }

        return static_cast<addr_t>(prog.size());
#endif
        return 0;
    };

    auto save = [/*this*/](const std::string &fname, addr_t start, addr_t end) {
#if 0 /* TODO */
        PrgFile prog{};
        for (auto addr = start; addr <= end; ++addr) {
            uint8_t c = this->read(addr);
            prog.push_back(c);
        }
        prog.save(fname, start);
#endif
    };

    auto loglevel = [this](const std::string &lv) {
        if (!empty(lv)) {
            this->loglevel(lv);
        }
        return this->loglevel();
    };

    auto regvalue = [this](const std::string &rname) -> uint16_t {
        auto it = regvals.find(rname);
        if (it == regvals.end()) {
            throw InvalidArgument{};
        }

        return it->second(*this);
    };

    MonitoredCpu monitor_funcs{
        .regs     = regs,
        .pc       = pc,
        .read     = read,
        .write    = write,
        .disass   = disass,
        .mmap     = mmap,
        .ebreak   = ebreak,
        .load     = load,
        .save     = save,
        .loglevel = loglevel,
        .regvalue = regvalue
    };

    _monitor = std::make_unique<Monitor>(is, os, std::move(monitor_funcs));
    _monitor->add_breakpoint(vRESET);
}

void ZilogZ80::init(const std::shared_ptr<ASpace> &mmap)
{
    if (mmap) {
        _mmap = mmap;
    }

    reset();
}

void ZilogZ80::loglevel(const std::string &lvs)
{
    _log.loglevel(lvs);
}

Logger::Level ZilogZ80::loglevel() const
{
    return _log.loglevel();
}

void ZilogZ80::reset()
{
    _IFF1 = false;
    _IFF2 = false;
    _regs = {
        .I  = 0,
        .PC = vRESET,
    };

//TODO
//    flag(0);
}

bool ZilogZ80::int_pin(bool active)
{
    _int_pin = active;
    return _int_pin;
}

bool ZilogZ80::nmi_pin(bool active)
{
    _nmi_pin = active;
    return _nmi_pin;
}

bool ZilogZ80::wait_pin(bool active)
{
    _wait_pin = active;
    return _wait_pin;
}

bool ZilogZ80::reset_pin(bool active)
{
    _reset_pin = active;
    return _reset_pin;
}

void ZilogZ80::ebreak()
{
    _break = true;
}

void ZilogZ80::bpadd(addr_t addr, const breakpoint_cb_t &cb, void *arg)
{
    _breakpoints[addr] = {cb, arg};
}

void ZilogZ80::bpdel(addr_t addr)
{
    _breakpoints.erase(addr);
}

const ZilogZ80::Registers &ZilogZ80::regs() const
{
    return _regs;
}

void ZilogZ80::disass(std::ostream &os, addr_t start, size_t count, bool show_pc)
{
    for (addr_t addr = start; count; --count) {
        const std::string &line = disass(addr, show_pc);
        os << line << std::endl;
    }
}

std::string ZilogZ80::disass(addr_t &addr, bool show_pc)
{
    /*
     * Output format:
     *  8000: 00 00      LD  A(HL)
     *  8002: 00 00 00   AND A
     *  8005: 60         RET
     *
     *  0         1         2         3
     *  012345678901234567890123456789012345
     */
    constexpr static const size_t HEX_SIZE = 18;
    constexpr static const size_t MNE_SIZE = 18;

    /*
     * Get the opcode.
     */
    uint8_t opcode = read(addr);
    auto &ins = instr_set[opcode];

    if (show_pc && addr != _regs.PC) {
        show_pc = false;
    }

    /*
     * Print address and opcode.
     */
    std::ostringstream hex{};
    hex << utils::to_string(addr) << ": " << utils::to_string(opcode);
    ++addr;

    /*
     * Get the instruction arguments based on the format string.
     */
    std::string format{ins.format};
    auto pos = format.find_first_of("*^+");
    if (pos != std::string::npos) {
        /*
         * '^', '%', '*', '+' is present in the format string: Find the operands.
         */
        char v = format[pos];
        if ((v == '+' && ins.size < 2) ||
            (v == '*' && ins.size < 2) ||
            (v == '^' && ins.size < 3) ||
            (v == '%' && ins.size < 3)) {
            std::ostringstream err{};
            err << "Invalid instruction encoding: "
                << "opcode " << utils::to_string(opcode)
                << ", size " << ins.size
                << ", fmt " << std::quoted(ins.format);
            throw InternalError{*this, err.str()};
        }

        /*
         * First byte of the operand.
         */
        std::ostringstream ops{};

        addr_t operand{};
        uint8_t ophi{}, oplo{};

        oplo = read(addr++);

        hex << " " << utils::to_string(oplo);

        switch (v) {
        case '*':
            /* Operand is an 8 bits value, must be disassembled as $00 or #$00 */
            ops << utils::to_string(oplo);
            break;

        case '^':
            /* Operand is a 16 bit value, must be disassembled as $0000 */
            ophi = read(addr++);
            operand = (static_cast<addr_t>(ophi) << 8) | oplo;
            hex << " " << utils::to_string(ophi);
            ops << utils::to_string(operand);
            break;

        case '%': //if A8 sames as $* but adds sign and in decimal
#if 0
    for A16 see DD 36 => IX instructions LD (IX+d), nn

        if A16 and ^ => A16 hex
        if A16 and % => A16 >> 4, if prev $ base is 16 else base is 10
        if A16 and * => A16 & 255, if prev $ base is 16 else base is 10

        if A8 and * => 8 bits, if prev $ base is 16 else base is 10
#endif
            //TODO
            break;

        case '+':
            /* Operand is a relative address: The disassembled string must show the absolute address */
            operand = static_cast<int8_t>(oplo) + addr;
            ops << utils::to_string(operand);
            break;
        }

        format = format.replace(pos, 1, ops.str());
    }

    /*
     * Build the string for the disassembled instruction.
     */
    std::ostringstream line{};
    line << std::setw(HEX_SIZE) << std::left << hex.str()
         << std::setw(MNE_SIZE) << std::left << format;

    if (show_pc) {
        line << "<";
    }

    return line.str();
}

size_t ZilogZ80::single_step()
{
    bool is_nmi{};

    if (_is_halted) {
        /*
         * HALT State (output, active Low). HALT indicates that the CPU has executed a
         * HALT instruction and is waiting for either a nonmaskable or a maskable interrupt
         * (with the mask enabled) before operation can resume. During HALT, the CPU executes
         * NOPs to maintain memory refreshes.
         */
        /* TODO */
        return 1;
    }

    //TODO If interrupt during execution of LD A,I (ED 57, misc instruction) parity flag is 0


    if (_nmi_pin) {
        is_nmi = true;
        _IFF2 = _IFF1;
        push_addr(_regs.PC);
        /* TODO */

    } else if (_IFF1 && _int_pin) {
        /* TODO */
    }

    /*
     * INT. Interrupt Request (input, active Low). An Interrupt Request is generated by I/O
     * devices. The CPU honors a request at the end of the current instruction if the internal
     * software-controlled interrupt enable flip-flop (IFF) is enabled. INT is normally
     * wired-OR and requires an external pull-up for these applications.
     */

    /*
     * NMI. Nonmaskable Interrupt (input, negative edge-triggered). NMI contains a higher
     * priority than INT. NMI is always recognized at the end of the current instruction,
     * independent of the status of the interrupt enable flip-flop, and automatically
     * forces the CPU to restart at location 0066h.
     */

    /*
     * RESET. Reset (input, active Low). RESET initializes the CPU as follows: it resets the
     * interrupt enable flip-flop, clears the Program Counter and registers I and R, and
     * sets the interrupt status to Mode 0. During reset time, the address and data bus
     * enter a high-impedance state, and all control output signals enter an inactive state.
     * RESET must be active for a minimum of three full clock cycles before a reset operation
     * is complete.
     */

    /*
     * WAIT. WAIT (input, active Low). WAIT communicates to the CPU that the addressed
     * memory or I/O devices are not ready for a data transfer. The CPU continues to enter
     * a WAIT state as long as this signal is active. Extended WAIT periods can prevent
     * the CPU from properly refreshing dynamic memory.
     */

    if (is_nmi) {
    }

#if 0
    addr_t addr{};
    bool is_nmi{};

    if (!_rdy_pin) {
        /*
         * CPU freezed on read operations.
         * In the real CPU several clock cycles are needed to execute one single instruction,
         * in this emulator, instructions are executed in one single_step() call, the number of
         * cycles is returned back to the caller clock which will call this method again after
         * those cycles are passed.
         * This means that we are not able to discern between read and write cycles, so we assume
         * the next operation will always be a read operation.
         *
         * For the reasons above, this pin acts as the AEC pin.
         */
        return 1;
    }

    if (_nmi_pin) {
        _nmi_pin.reset();   /* Reset the pin to simulate an edge triggered interrupt */
        addr = read_addr(vNMI);
        is_nmi = true;
    } else if (is_irq_enabled() && _irq_pin) {
        addr = read_addr(vIRQ);
    }

    if (addr) {
        push_addr(_regs.PC);
        push_P();
        _regs.PC = addr;
        flag(Flags::I);
        _log.debug("Detected %s interrupt. Extra cycles=7\n", (is_nmi ? "NMI" : "IRQ"));
        return 7;
    }

    std::string line{};
    if (_log.is_debug()) {
        addr_t addr = _regs.PC;
        line = disass(addr);
    }

    uint8_t opcode = read(_regs.PC++);
    addr_t arg{};
    auto &ins = instr_set[opcode];

    if (ins.size >= 1) {
        /*
         * Get the instruction operand.
         */
        switch (ins.mode) {
        case ArgType::None:
            break;

        case MODE_IMM:
        case MODE_ZP:
        case MODE_ZP_X:
        case MODE_ZP_Y:
        case MODE_IND_X:
        case MODE_IND_Y:
        case MODE_REL:
            arg = read(_regs.PC);
            break;

        case MODE_ABS:
        case MODE_ABS_X:
        case MODE_ABS_Y:
        case MODE_IND:
            arg = read_addr(_regs.PC);
            break;
        }

        /*
         * Get the indexing mode.
         */
        switch (ins.mode) {
        case MODE_NONE:
        case MODE_IMM:
        case MODE_ZP:
        case MODE_REL:
        case MODE_ABS:
            break;

        case MODE_ZP_X:
        case MODE_ABS_X:
            arg += _regs.X;                     /* XXX: Zero page index bug */
            break;

        case MODE_ZP_Y:
        case MODE_ABS_Y:
            arg += _regs.Y;                     /* XXX: Zero page index bug */
            break;

        case MODE_IND_X:
            arg = read_addr(arg + _regs.X);     /* XXX: Zero page index bug */
            break;

        case MODE_IND_Y:
            arg = read_addr(arg) + _regs.Y;     /* XXX: Zero page index bug */
            break;

        case MODE_IND:
            arg = read_addr(arg);
            break;
        }

        _regs.PC += (ins.size - 1);
    }

    ins.fn(*this, arg);

    if (_log.is_debug()) {
        std::ostringstream msg{};
        msg << std::setw(35) << std::left << line << _regs.to_string() << "  cycles=" << ins.cycles << std::endl;
        _log.debug(msg.str());
    }

    return ins.cycles;
#endif
    //TODO
    return 1;
}

size_t ZilogZ80::tick(const Clock &clk)
{
    if (_break && !_monitor) {
        /*
         * Break hot-key but monitor not active.
         */
        log.debug("System halt requested from breakpoint\n");
        return Clockable::HALT;
    }

    if (_monitor && (_break || _monitor->is_breakpoint(_regs.PC))) {
        /*
         * Break hot-key and monitor active or monitor breakpoint.
         */
        _break = false;
        if (!_monitor->run()) {
            log.debug("System halt requested from monitor\n");
            return Clockable::HALT;
        }
    }

    auto bp = _breakpoints.find(_regs.PC);
    if (bp != _breakpoints.end()) {
        /*
         * System breakpoint (from some part of the emulator).
         */
        auto &fn = bp->second.first;
        auto *arg = bp->second.second;
        fn(*this, arg);
    }

    size_t cycles = single_step();
    return (cycles == 0 ? Clockable::HALT : cycles);
}

addr_t ZilogZ80::read_addr(size_t addr) const
{
    uint8_t lo = read(addr);
    uint8_t hi = read(addr + 1);

    return (static_cast<addr_t>(hi) << 8 | lo);
}

void ZilogZ80::write_addr(addr_t addr, addr_t data)
{
    uint8_t lo = static_cast<uint8_t>(data & 0xFF);
    uint8_t hi = static_cast<uint8_t>(data >> 8);

    write(addr, lo);
    write(addr + 1, hi);
}

uint8_t ZilogZ80::read(addr_t addr) const
{
    return _mmap->read(addr);
}

void ZilogZ80::write(addr_t addr, uint8_t data)
{
    _mmap->write(addr, data);
}

}
