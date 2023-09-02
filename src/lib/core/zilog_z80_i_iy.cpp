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
const std::array<Z80::Instruction, 256> Z80::iy_instr_set{{
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 00 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 01 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 02 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 03 */
    { "INC B",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* FD 04 */ /* Undocumented */
    { "DEC B",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* FD 05 */ /* Undocumented */
    { "LD B, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* FD 06 */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 07 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 08 */
    { "ADD IY, BC",     Z80::i_ADD_IY_rr,   ArgType::None,  15, 2   },  /* FD 09 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 0A */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 0B */
    { "INC C",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* FD 0C */ /* Undocumented */
    { "DEC C",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* FD 0D */ /* Undocumented */
    { "LD C, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* FD 0E */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 0F */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 10 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 11 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 12 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 13 */
    { "IND D",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* FD 14 */ /* Undocumented */
    { "DEC D",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* FD 15 */ /* Undocumented */
    { "LD D, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* FD 16 */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 17 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 18 */
    { "ADD IY, DE",     Z80::i_ADD_IY_rr,   ArgType::None,  15, 2   },  /* FD 19 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 1A */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 1B */
    { "INC E",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* FD 1C */ /* Undocumented */
    { "DEC E",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* FD 1D */ /* Undocumented */
    { "LD E, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* FD 1E */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 1F */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 20 */
    { "LD IY, $^",      Z80::i_LD_IY_nn,    ArgType::A16,   14, 4   },  /* FD 21 */
    { "LD ($^), IY",    Z80::i_LD_mnn_IY,   ArgType::A16,   20, 4   },  /* FD 22 */
    { "INC IY",         Z80::i_INC_IY,      ArgType::None,  10, 2   },  /* FD 23 */
    { "INC IYH",        Z80::i_INC_IYH,     ArgType::None,  8,  2   },  /* FD 24 */ /* Undocumented */
    { "DEC IYH",        Z80::i_DEC_IYH,     ArgType::None,  8,  2   },  /* FD 25 */ /* Undocumented */
    { "LD IYH, $*",     Z80::i_LD_IYH_n,    ArgType::A8,    11, 3   },  /* FD 26 */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 27 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 28 */
    { "ADD IY, IY",     Z80::i_ADD_IY_rr,   ArgType::None,  15, 2   },  /* FD 29 */
    { "LD IY, ($^)",    Z80::i_LD_IY_mnn,   ArgType::A16,   20, 4   },  /* FD 2A */
    { "DEC IY",         Z80::i_DEC_IY,      ArgType::None,  10, 2   },  /* FD 2B */
    { "INC IYL",        Z80::i_INC_IYL,     ArgType::None,  8,  2   },  /* FD 2C */ /* Undocumented */
    { "DEC IYL",        Z80::i_DEC_IYL,     ArgType::None,  8,  2   },  /* FD 2D */ /* Undocumented */
    { "LD IYL, $*",     Z80::i_LD_IYL_n,    ArgType::A8,    11, 3   },  /* FD 2E */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 2F */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 30 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 31 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 32 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 33 */
    { "INC (IY%)",      Z80::i_INC_mIYd,    ArgType::A8,    23, 3   },  /* FD 34 */
    { "DEC (IY%)",      Z80::i_DEC_mIYd,    ArgType::A8,    23, 3   },  /* FD 35 */
    { "LD (IY%), $*",   Z80::i_LD_mIYd_n,   ArgType::A16,   19, 4   },  /* FD 36 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 37 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 38 */
    { "ADD IY, SP",     Z80::i_ADD_IY_rr,   ArgType::None,  15, 2   },  /* FD 39 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 3A */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 3B */
    { "INC A",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* FD 3C */ /* Undocumented */
    { "DEC A",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* FD 3D */ /* Undocumented */
    { "LD A, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* FD 3E */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 3F */

    { "LD B, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 40 */ /* Undocumented */
    { "LD B, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 41 */ /* Undocumented */
    { "LD B, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 42 */ /* Undocumented */
    { "LD B, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 43 */ /* Undocumented */
    { "LD B, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 44 */ /* Undocumented */
    { "LD B, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 45 */ /* Undocumented */
    { "LD B, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 46 */
    { "LD B, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 47 */ /* Undocumented */
    { "LD C, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 48 */ /* Undocumented */
    { "LD C, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 49 */ /* Undocumented */
    { "LD C, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 4A */ /* Undocumented */
    { "LD C, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 4B */ /* Undocumented */
    { "LD C, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 4C */ /* Undocumented */
    { "LD C, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 4D */ /* Undocumented */
    { "LD C, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 4E */
    { "LD C, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 4F */ /* Undocumented */

    { "LD D, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 50 */ /* Undocumented */
    { "LD D, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 51 */ /* Undocumented */
    { "LD D, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 52 */ /* Undocumented */
    { "LD D, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 53 */ /* Undocumented */
    { "LD D, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 54 */ /* Undocumented */
    { "LD D, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 55 */ /* Undocumented */
    { "LD D, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 56 */
    { "LD D, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 57 */ /* Undocumented */
    { "LD E, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 58 */ /* Undocumented */
    { "LD E, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 59 */ /* Undocumented */
    { "LD E, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 5A */ /* Undocumented */
    { "LD E, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 5B */ /* Undocumented */
    { "LD E, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 5C */ /* Undocumented */
    { "LD E, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 5D */ /* Undocumented */
    { "LD E, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 5E */
    { "LD E, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 5F */ /* Undocumented */

    { "LD IYH, B",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 60 */ /* Undocumented */
    { "LD IYH, C",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 61 */ /* Undocumented */
    { "LD IYH, D",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 62 */ /* Undocumented */
    { "LD IYH, E",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 63 */ /* Undocumented */
    { "LD IYH, IYH",    Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 64 */ /* Undocumented */
    { "LD IYH, IYL",    Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 65 */ /* Undocumented */
    { "LD H, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 66 */
    { "LD IYH, A",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 67 */ /* Undocumented */
    { "LD IYL, B",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 68 */ /* Undocumented */
    { "LD IYL, C",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 69 */ /* Undocumented */
    { "LD IYL, D",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 6A */ /* Undocumented */
    { "LD IYL, E",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 6B */ /* Undocumented */
    { "LD IYL, IYH",    Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 6C */ /* Undocumented */
    { "LD IYL, IYL",    Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 6D */ /* Undocumented */
    { "LD L, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 6E */
    { "LD IYL, A",      Z80::i_LD_IYHL_r,   ArgType::None,  8,  2   },  /* FD 6F */ /* Undocumented */

    { "LD (IY%), B",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 70 */
    { "LD (IY%), C",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 71 */
    { "LD (IY%), D",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 72 */
    { "LD (IY%), E",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 73 */
    { "LD (IY%), H",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 74 */
    { "LD (IY%), L",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 75 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD 76 */
    { "LD (IY%), A",    Z80::i_LD_mIYd_r,   ArgType::A8,    19, 3   },  /* FD 77 */
    { "LD A, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 78 */ /* Undocumented */
    { "LD A, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 79 */ /* Undocumented */
    { "LD A, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 7A */ /* Undocumented */
    { "LD A, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 7B */ /* Undocumented */
    { "LD A, IYH",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 7C */ /* Undocumented */
    { "LD A, IYL",      Z80::i_LD_r_IYHL,   ArgType::None,  8,  2   },  /* FD 7D */ /* Undocumented */
    { "LD A, (IY%)",    Z80::i_LD_r_mIYd,   ArgType::A8,    19, 3   },  /* FD 7E */
    { "LD A, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* FD 7F */ /* Undocumented */

    { "ADD A, B",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* FD 80 */ /* Undocumented */
    { "ADD A, C",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* FD 81 */ /* Undocumented */
    { "ADD A, D",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* FD 82 */ /* Undocumented */
    { "ADD A, E",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* FD 83 */ /* Undocumented */
    { "ADD A, IYH",     Z80::i_ADD_A_IYHL,  ArgType::None,  8,  2   },  /* FD 84 */ /* Undocumented */
    { "ADD A, IYL",     Z80::i_ADD_A_IYHL,  ArgType::None,  8,  2   },  /* FD 85 */ /* Undocumented */
    { "ADD A, (IY%)",   Z80::i_ADD_A_mIYd,  ArgType::A8,    19, 3   },  /* FD 86 */
    { "ADD A, A",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* FD 87 */ /* Undocumented */
    { "ADC A, B",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* FD 88 */ /* Undocumented */
    { "ADC A, C",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* FD 89 */ /* Undocumented */
    { "ADC A, D",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* FD 8A */ /* Undocumented */
    { "ADC A, E",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* FD 8B */ /* Undocumented */
    { "ADC A, IYH",     Z80::i_ADC_A_IYHL,  ArgType::None,  8,  2   },  /* FD 8C */ /* Undocumented */
    { "ADC A, IYL",     Z80::i_ADC_A_IYHL,  ArgType::None,  8,  2   },  /* FD 8D */ /* Undocumented */
    { "ADC A, (IY%)",   Z80::i_ADC_A_mIYd,  ArgType::A8,    19, 3   },  /* FD 8E */
    { "ADC A, A",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* FD 8F */ /* Undocumented */

    { "SUB B",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* FD 90 */ /* Undocumented */
    { "SUB C",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* FD 91 */ /* Undocumented */
    { "SUB D",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* FD 92 */ /* Undocumented */
    { "SUB E",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* FD 93 */ /* Undocumented */
    { "SUB IYH",        Z80::i_SUB_A_IYHL,  ArgType::None,  8,  2   },  /* FD 94 */ /* Undocumented */
    { "SUB IYL",        Z80::i_SUB_A_IYHL,  ArgType::None,  8,  2   },  /* FD 95 */ /* Undocumented */
    { "SUB (IY%)",      Z80::i_SUB_A_mIYd,  ArgType::A8,    19, 3   },  /* FD 96 */
    { "SUB A",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* FD 97 */ /* Undocumented */
    { "SBC B",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* FD 98 */ /* Undocumented */
    { "SBC C",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* FD 99 */ /* Undocumented */
    { "SBC D",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* FD 9A */ /* Undocumented */
    { "SBC E",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* FD 9B */ /* Undocumented */
    { "SBC IYH",        Z80::i_SBC_A_IYHL,  ArgType::None,  8,  2   },  /* FD 9C */ /* Undocumented */
    { "SBC IYL",        Z80::i_SBC_A_IYHL,  ArgType::None,  8,  2   },  /* FD 9D */ /* Undocumented */
    { "SBC (IY%)",      Z80::i_SBC_A_mIYd,  ArgType::A8,    19, 3   },  /* FD 9E */
    { "SBC A",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* FD 9F */ /* Undocumented */

    { "AND B",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* FD A0 */ /* Undocumented */
    { "AND C",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* FD A1 */ /* Undocumented */
    { "AND D",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* FD A2 */ /* Undocumented */
    { "AND E",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* FD A3 */ /* Undocumented */
    { "AND IYH",        Z80::i_AND_A_IYHL,  ArgType::None,  8,  2   },  /* FD A4 */ /* Undocumented */
    { "AND IYL",        Z80::i_AND_A_IYHL,  ArgType::None,  8,  2   },  /* FD A5 */ /* Undocumented */
    { "AND (IY%)",      Z80::i_AND_A_mIYd,  ArgType::A8,    19, 3   },  /* FD A6 */
    { "AND A",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* FD A7 */ /* Undocumented */
    { "XOR B",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* FD A8 */ /* Undocumented */
    { "XOR C",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* FD A9 */ /* Undocumented */
    { "XOR D",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* FD AA */ /* Undocumented */
    { "XOR E",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* FD AB */ /* Undocumented */
    { "XOR IYH",        Z80::i_XOR_A_IYHL,  ArgType::None,  8,  2   },  /* FD AC */ /* Undocumented */
    { "XOR IYL",        Z80::i_XOR_A_IYHL,  ArgType::None,  8,  2   },  /* FD AD */ /* Undocumented */
    { "XOR (IY%)",      Z80::i_XOR_A_mIYd,  ArgType::A8,    19, 3   },  /* FD AE */
    { "XOR A",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* FD AF */ /* Undocumented */

    { "OR B",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* FD B0 */ /* Undocumented */
    { "OR C",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* FD B1 */ /* Undocumented */
    { "OR D",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* FD B2 */ /* Undocumented */
    { "OR E",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* FD B3 */ /* Undocumented */
    { "OR IYH",         Z80::i_OR_A_IYHL,   ArgType::None,  8,  2   },  /* FD B4 */ /* Undocumented */
    { "OR IYL",         Z80::i_OR_A_IYHL,   ArgType::None,  8,  2   },  /* FD B5 */ /* Undocumented */
    { "OR (IY%)",       Z80::i_OR_A_mIYd,   ArgType::A8,    19, 3   },  /* FD B6 */
    { "OR A",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* FD B7 */ /* Undocumented */
    { "CP B",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* FD B8 */ /* Undocumented */
    { "CP C",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* FD B9 */ /* Undocumented */
    { "CP D",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* FD BA */ /* Undocumented */
    { "CP E",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* FD BB */ /* Undocumented */
    { "CP IYH",         Z80::i_CP_A_IYHL,   ArgType::None,  8,  2   },  /* FD BC */ /* Undocumented */
    { "CP IYL",         Z80::i_CP_A_IYHL,   ArgType::None,  8,  2   },  /* FD BD */ /* Undocumented */
    { "CP (IY%)",       Z80::i_CP_A_mIYd,   ArgType::A8,    19, 3   },  /* FD BE */
    { "CP A",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* FD BF */ /* Undocumented */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C0 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C2 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C4 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C8 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD C9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD CA */
    { "",               {},                 ArgType::IYBit, 0,  2   },  /* FD CB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD CC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD CD */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD CE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD CF */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D0 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D2 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D4 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D8 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD D9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD DA */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD DB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD DC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD DD */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD DE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD DF */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD E0 */
    { "POP IY",         Z80::i_POP_IY,      ArgType::None,  14, 2   },  /* FD E1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD E2 */
    { "EX (SP), IY",    Z80::i_EX_mSP_IY,   ArgType::None,  23, 2   },  /* FD E3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD E4 */
    { "PUSH IY",        Z80::i_PUSH_IY,     ArgType::None,  15, 2   },  /* FD E5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD E6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD E7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD E8 */
    { "JP (IY)",        Z80::i_JP_IY,       ArgType::None,  8,  2   },  /* FD E9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD EA */
    { "",               {},                 ArgType::IYBit, 0,  2   },  /* FD EB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD EC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD ED */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD EE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD EF */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F0 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F2 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F4 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD F8 */
    { "LD SP, IY",      Z80::i_LD_SP_IY,    ArgType::None,  10, 2   },  /* FD F9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD FA */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD FB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD FC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD FD */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD FE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* FD FF */
}};

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
     *
     * https://clrhome.org/table/#dd
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
     *
     * https://clrhome.org/table/#dd
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
     *
     * https://clrhome.org/table/#dd
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
     *
     * https://clrhome.org/table/#dd
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
    addr_t addr = self._regs.IY + static_cast<int16_t>(arg);
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
    addr_t addr = self._regs.IY + arg;
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
    addr_t addr = self._regs.IY + arg;
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
    addr_t addr = self._regs.IY + arg;
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
    addr_t addr = self._regs.IY + arg;
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
    addr_t addr = self._regs.IY + arg;
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
    addr_t addr = self._regs.IY + arg;
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
    addr_t addr = self._regs.IY + arg;
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
