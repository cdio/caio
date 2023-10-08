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
 * Most of the information in the following table come from:
 * - Z80 CPU User Manual UM008011-0816.
 * - https://clrhome.org/table/
 */
const std::array<Z80::Instruction, 256> Z80::ix_instr_set{{
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 00 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 01 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 02 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 03 */
    { "INC B",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* DD 04 */ /* Undocumented */
    { "DEC B",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* DD 05 */ /* Undocumented */
    { "LD B, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* DD 06 */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 07 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 08 */
    { "ADD IX, BC",     Z80::i_ADD_IX_rr,   ArgType::None,  15, 2   },  /* DD 09 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 0A */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 0B */
    { "INC C",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* DD 0C */ /* Undocumented */
    { "DEC C",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* DD 0D */ /* Undocumented */
    { "LD C, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* DD 0E */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 0F */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 10 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 11 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 12 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 13 */
    { "INC D",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* DD 14 */ /* Unodcumented */
    { "DEC D",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* DD 15 */ /* Unodcumented */
    { "LD D, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* DD 16 */ /* Unodcumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 17 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 18 */
    { "ADD IX, DE",     Z80::i_ADD_IX_rr,   ArgType::None,  15, 2   },  /* DD 19 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 1A */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 1B */
    { "INC E",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* DD 1C */ /* Undocumented */
    { "DEC E",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* DD 1D */ /* Undocumented */
    { "LD E, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* DD 1E */ /* Undocumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 1F */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 20 */
    { "LD IX, $^",      Z80::i_LD_IX_nn,    ArgType::A16,   14, 4   },  /* DD 21 */
    { "LD ($^), IX",    Z80::i_LD_mnn_IX,   ArgType::A16,   20, 4   },  /* DD 22 */
    { "INC IX",         Z80::i_INC_IX,      ArgType::None,  10, 2   },  /* DD 23 */
    { "INC IXH",        Z80::i_INC_IXH,     ArgType::None,  8,  2   },  /* DD 24 */ /* Undocumented */
    { "DEC IXH",        Z80::i_DEC_IXH,     ArgType::None,  8,  2   },  /* DD 25 */ /* Unodcumented */
    { "LD IXH, $*",     Z80::i_LD_IXH_n,    ArgType::A8,    11, 3   },  /* DD 26 */ /* Unodcumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 27 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 28 */
    { "ADD IX, IX",     Z80::i_ADD_IX_rr,   ArgType::None,  15, 2   },  /* DD 29 */
    { "LD IX, ($^)",    Z80::i_LD_IX_mnn,   ArgType::A16,   20, 4   },  /* DD 2A */
    { "DEC IX",         Z80::i_DEC_IX,      ArgType::None,  10, 2   },  /* DD 2B */
    { "INC IXL",        Z80::i_INC_IXL,     ArgType::None,  8,  2   },  /* DD 2C */ /* Undocumented */
    { "DEC IXL",        Z80::i_DEC_IXL,     ArgType::None,  8,  2   },  /* DD 2D */ /* Unodcumented */
    { "LD IXL, $*",     Z80::i_LD_IXL_n,    ArgType::A8,    11, 3   },  /* DD 2E */ /* Unodcumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 2F */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 30 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 31 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 32 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 33 */
    { "INC (IX%)" ,     Z80::i_INC_mIXd,    ArgType::A8,    23, 3   },  /* DD 34 */
    { "DEC (IX%)",      Z80::i_DEC_mIXd,    ArgType::A8,    23, 3   },  /* DD 35 */
    { "LD (IX%), $*",   Z80::i_LD_mIXd_n,   ArgType::A16,   19, 4   },  /* DD 36 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 37 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 38 */
    { "ADD IX, SP",     Z80::i_ADD_IX_rr,   ArgType::None,  15, 2   },  /* DD 39 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 3A */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 3B */
    { "INC A",          Z80::i_INC_r,       ArgType::None,  8,  2   },  /* DD 3C */ /* Undocumented */
    { "DEC A",          Z80::i_DEC_r,       ArgType::None,  8,  2   },  /* DD 3D */ /* Unodcumented */
    { "LD A, $*",       Z80::i_LD_r_n,      ArgType::A8,    11, 3   },  /* DD 3E */ /* Unodcumented */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 3F */

    { "LD B, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 40 */ /* Undocumented */
    { "LD B, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 41 */ /* Undocumented */
    { "LD B, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 42 */ /* Undocumented */
    { "LD B, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 43 */ /* Undocumented */
    { "LD B, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 44 */ /* Undocumented */
    { "LD B, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 45 */ /* Undocumented */
    { "LD B, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 46 */
    { "LD B, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 47 */ /* Undocumented */
    { "LD C, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 48 */ /* Undocumented */
    { "LD C, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 49 */ /* Undocumented */
    { "LD C, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 4A */ /* Undocumented */
    { "LD C, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 4B */ /* Undocumented */
    { "LD C, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 4C */ /* Undocumented */
    { "LD C, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 4D */ /* Undocumented */
    { "LD C, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 4E */
    { "LD C, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 4F */ /* Undocumented */

    { "LD D, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 50 */ /* Undocumented */
    { "LD D, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 51 */ /* Undocumented */
    { "LD D, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 52 */ /* Undocumented */
    { "LD D, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 53 */ /* Undocumented */
    { "LD D, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 54 */ /* Undocumented */
    { "LD D, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 55 */ /* Undocumented */
    { "LD D, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 56 */
    { "LD D, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 57 */ /* Undocumented */
    { "LD E, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 58 */ /* Undocumented */
    { "LD E, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 59 */ /* Undocumented */
    { "LD E, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 5A */ /* Undocumented */
    { "LD E, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 5B */ /* Undocumented */
    { "LD E, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 5C */ /* Undocumented */
    { "LD E, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 5D */ /* Undocumented */
    { "LD E, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 5E */
    { "LD E, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 5F */ /* Undocumented */

    { "LD IXH, B",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 60 */ /* Undocumented */
    { "LD IXH, C",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 61 */ /* Undocumented */
    { "LD IXH, D",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 62 */ /* Undocumented */
    { "LD IXH, E",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 63 */ /* Undocumented */
    { "LD IXH, IXH",    Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 64 */ /* Undocumented */
    { "LD IXH, IXL",    Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 65 */ /* Undocumented */
    { "LD H, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 66 */
    { "LD IXH, A",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 67 */ /* Undocumented */
    { "LD IXL, B",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 68 */ /* Undocumented */
    { "LD IXL, C",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 69 */ /* Undocumented */
    { "LD IXL, D",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 6A */ /* Undocumented */
    { "LD IXL, E",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 6B */ /* Undocumented */
    { "LD IXL, IXH",    Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 6C */ /* Undocumented */
    { "LD IXL, IXL",    Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 6D */ /* Undocumented */
    { "LD L, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 6E */
    { "LD IXL, A",      Z80::i_LD_IXHL_r,   ArgType::None,  8,  2   },  /* DD 6F */ /* Undocumented */

    { "LD (IX%), B",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 70 */
    { "LD (IX%), C",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 71 */
    { "LD (IX%), D",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 72 */
    { "LD (IX%), E",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 73 */
    { "LD (IX%), H",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 74 */
    { "LD (IX%), L",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 75 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD 76 */
    { "LD (IX%), A",    Z80::i_LD_mIXd_r,   ArgType::A8,    19, 3   },  /* DD 77 */
    { "LD A, B",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 78 */ /* Undocumented */
    { "LD A, C",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 79 */ /* Undocumented */
    { "LD A, D",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 7A */ /* Undocumented */
    { "LD A, E",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 7B */ /* Undocumented */
    { "LD A, IXH",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 7C */ /* Undocumented */
    { "LD A, IXL",      Z80::i_LD_r_IXHL,   ArgType::None,  8,  2   },  /* DD 7D */ /* Undocumented */
    { "LD A, (IX%)",    Z80::i_LD_r_mIXd,   ArgType::A8,    19, 3   },  /* DD 7E */
    { "LD A, A",        Z80::i_LD_r_r,      ArgType::None,  8,  2   },  /* DD 7F */ /* Undocumented */

    { "ADD A, B",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* DD 80 */ /* Undocumented */
    { "ADD A, C",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* DD 81 */ /* Undocumented */
    { "ADD A, D",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* DD 82 */ /* Undocumented */
    { "ADD A, E",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* DD 83 */ /* Undocumented */
    { "ADD A, IXH",     Z80::i_ADD_A_IXHL,  ArgType::None,  8,  2   },  /* DD 84 */ /* Undocumented */
    { "ADD A, IXL",     Z80::i_ADD_A_IXHL,  ArgType::None,  8,  2   },  /* DD 85 */ /* Undocumented */
    { "ADD A, (IX%)",   Z80::i_ADD_A_mIXd,  ArgType::A8,    19, 3   },  /* DD 86 */
    { "ADD A, A",       Z80::i_ADD_A_r,     ArgType::None,  8,  2   },  /* DD 87 */ /* Undocumented */
    { "ADC A, B",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* DD 88 */ /* Undocumented */
    { "ADC A, C",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* DD 89 */ /* Undocumented */
    { "ADC A, D",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* DD 8A */ /* Undocumented */
    { "ADC A, E",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* DD 8B */ /* Undocumented */
    { "ADC A, IXH",     Z80::i_ADC_A_IXHL,  ArgType::None,  8,  2   },  /* DD 8C */ /* Undocumented */
    { "ADC A, IXL",     Z80::i_ADC_A_IXHL,  ArgType::None,  8,  2   },  /* DD 8D */ /* Undocumented */
    { "ADC A, (IX%)",   Z80::i_ADC_A_mIXd,  ArgType::A8,    19, 3   },  /* DD 8E */
    { "ADC A, A",       Z80::i_ADC_A_r,     ArgType::None,  8,  2   },  /* DD 8F */ /* Undocumented */

    { "SUB B",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* DD 90 */ /* Undocumented */
    { "SUB C",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* DD 91 */ /* Undocumented */
    { "SUB D",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* DD 92 */ /* Undocumented */
    { "SUB E",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* DD 93 */ /* Undocumented */
    { "SUB IXH",        Z80::i_SUB_A_IXHL,  ArgType::None,  8,  2   },  /* DD 94 */ /* Undocumented */
    { "SUB IXL",        Z80::i_SUB_A_IXHL,  ArgType::None,  8,  2   },  /* DD 95 */ /* Undocumented */
    { "SUB (IX%)",      Z80::i_SUB_A_mIXd,  ArgType::A8,    19, 3   },  /* DD 96 */
    { "SUB A",          Z80::i_SUB_A_r,     ArgType::None,  8,  2   },  /* DD 97 */ /* Undocumented */
    { "SBC B",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* DD 98 */ /* Undocumented */
    { "SBC C",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* DD 99 */ /* Undocumented */
    { "SBC D",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* DD 9A */ /* Undocumented */
    { "SBC E",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* DD 9B */ /* Undocumented */
    { "SBC IXH",        Z80::i_SBC_A_IXHL,  ArgType::None,  8,  2   },  /* DD 9C */ /* Undocumented */
    { "SBC IXL",        Z80::i_SBC_A_IXHL,  ArgType::None,  8,  2   },  /* DD 9D */ /* Undocumented */
    { "SBC (IX%)",      Z80::i_SBC_A_mIXd,  ArgType::A8,    19, 3   },  /* DD 9E */
    { "SBC A",          Z80::i_SBC_A_r,     ArgType::None,  8,  2   },  /* DD 9F */ /* Undocumented */

    { "AND B",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* DD A0 */ /* Undocumented */
    { "AND C",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* DD A1 */ /* Undocumented */
    { "AND D",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* DD A2 */ /* Undocumented */
    { "AND E",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* DD A3 */ /* Undocumented */
    { "AND IXH",        Z80::i_AND_A_IXHL,  ArgType::None,  8,  2   },  /* DD A4 */ /* Undocumented */
    { "AND IXL",        Z80::i_AND_A_IXHL,  ArgType::None,  8,  2   },  /* DD A5 */ /* Undocumented */
    { "AND (IX%)",      Z80::i_AND_A_mIXd,  ArgType::A8,    19, 3   },  /* DD A6 */
    { "AND A",          Z80::i_AND_A_r,     ArgType::None,  8,  2   },  /* DD A7 */ /* Undocumented */
    { "XOR B",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* DD A8 */ /* Undocumented */
    { "XOR C",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* DD A9 */ /* Undocumented */
    { "XOR D",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* DD AA */ /* Undocumented */
    { "XOR E",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* DD AB */ /* Undocumented */
    { "XOR IXH",        Z80::i_XOR_A_IXHL,  ArgType::None,  8,  2   },  /* DD AC */ /* Undocumented */
    { "XOR IXL",        Z80::i_XOR_A_IXHL,  ArgType::None,  8,  2   },  /* DD AD */ /* Undocumented */
    { "XOR (IX%)",      Z80::i_XOR_A_mIXd,  ArgType::A8,    19, 3   },  /* DD AE */
    { "XOR A",          Z80::i_XOR_A_r,     ArgType::None,  8,  2   },  /* DD AF */ /* Undocumented */

    { "OR B",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* DD B0 */ /* Undocumented */
    { "OR C",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* DD B1 */ /* Undocumented */
    { "OR D",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* DD B2 */ /* Undocumented */
    { "OR E",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* DD B3 */ /* Undocumented */
    { "OR IXH",         Z80::i_OR_A_IXHL,   ArgType::None,  8,  2   },  /* DD B4 */ /* Undocumented */
    { "OR IXL",         Z80::i_OR_A_IXHL,   ArgType::None,  8,  2   },  /* DD B5 */ /* Undocumented */
    { "OR (IX%)",       Z80::i_OR_A_mIXd,   ArgType::A8,    19, 3   },  /* DD B6 */
    { "OR A",           Z80::i_OR_A_r,      ArgType::None,  8,  2   },  /* DD B7 */ /* Undocumented */
    { "CP B",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* DD B8 */ /* Undocumented */
    { "CP C",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* DD B9 */ /* Undocumented */
    { "CP D",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* DD BA */ /* Undocumented */
    { "CP E",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* DD BB */ /* Undocumented */
    { "CP IXH",         Z80::i_CP_A_IXHL,   ArgType::None,  8,  2   },  /* DD BC */ /* Undocumented */
    { "CP IXL",         Z80::i_CP_A_IXHL,   ArgType::None,  8,  2   },  /* DD BD */ /* Undocumented */
    { "CP (IX%)",       Z80::i_CP_A_mIXd,   ArgType::A8,    19, 3   },  /* DD BE */
    { "CP A",           Z80::i_CP_A_r,      ArgType::None,  8,  2   },  /* DD BF */ /* Undocumented */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C0 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C2 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C4 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C8 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD C9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD CA */
    { "",               {},                 ArgType::IXBit, 0,  2   },  /* DD CB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD CC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD CD */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD CE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD CF */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D0 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D2 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D4 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D8 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD D9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD DA */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD DB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD DC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD DD */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD DE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD DF */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD E0 */
    { "POP IX",         Z80::i_POP_IX,      ArgType::None,  14, 2   },  /* DD E1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD E2 */
    { "EX (SP), IX",    Z80::i_EX_mSP_IX,   ArgType::None,  23, 2   },  /* DD E3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD E4 */
    { "PUSH IX",        Z80::i_PUSH_IX,     ArgType::None,  15, 2   },  /* DD E5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD E6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD E7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD E8 */
    { "JP (IX)",        Z80::i_JP_IX,       ArgType::None,  8,  2   },  /* DD E9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD EA */
    { "",               {},                 ArgType::IXBit, 0,  2   },  /* DD EB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD EC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD ED */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD EE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD EF */

    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F0 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F1 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F2 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F3 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F4 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F5 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F6 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F7 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD F8 */
    { "LD SP, IX",      Z80::i_LD_SP_IX,    ArgType::None,  10, 2   },  /* DD F9 */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD FA */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD FB */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD FC */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD FD */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD FE */
    { "",               {},                 ArgType::Inv,   0,  2   },  /* DD FF */
}};

uint8_t Z80::reg_hilo(uint16_t reg, uint8_t op)
{
    return ((op & 1) == 0 ? (reg >> 8) : (reg & 0xFF));
}

int Z80::add_xx_rr(uint16_t& reg, uint8_t op)
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
    uint16_t src_reg = (r == 0x00 ? _regs.BC() :
                       (r == 0x10 ? _regs.DE() :
                       (r == 0x20 ? reg        : _regs.SP)));

    _regs.memptr = reg + 1;
    add16(reg, src_reg);
    return 0;
}

int Z80::ld_r_xxHL(uint16_t& reg, uint8_t op)
{
    /*
     * LD {A,B,C,D,E}, IXH - DD 7C, DD 44, DD 4C, DD 54, DD 5C - Undocumented
     * LD {A,B,C,D,E}, IXL - DD 7D, DD 45, DD 4D, DD 55, DD 5D - Undocumented
     *
     * LD {A,B,C,D,E}, IYH - FD 7C, FD 44, FD 4C, FD 54, FD 5C - Undocumented
     * LD {A,B,C,D,E}, IYL - FD 7D, FD 45, FD 4D, FD 55, FD 5D - Undocumented
     */
    uint8_t value = reg_hilo(reg, op);
    uint8_t& dst_reg = reg8_from_opcode(op);
    dst_reg = value;
    return 0;
}

int Z80::ld_xxHL_r(uint16_t& reg, uint8_t op)
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
        reg = (reg << 8) | (reg & 0x00FF);
        break;
    case 0x6C: /* LD IYL,IYH  LD IXL,IXH */
        reg = (reg & 0xFF00) | (reg >> 8);
        break;
    default: {
        uint16_t src_reg = reg8_src_from_opcode(op);
        bool lo = (op & 8);
        if (lo) {
            reg = (reg & 0xFF00) | src_reg;
        } else {
            reg = (reg & 0x00FF) | (src_reg << 8);
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
    addr_t addr = reg + static_cast<int16_t>(arg & 0xFF);
    uint8_t value = static_cast<uint8_t>(arg >> 8);
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
    write(addr, src_reg);
    _regs.memptr = addr;
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

int Z80::inc_xxH(uint16_t& reg)
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
     *
     * https://clrhome.org/table/#dd
     */
    bool C = test_C();
    uint16_t result = add8(reg >> 8, 1, 0);
    reg = (result << 8) | (reg & 0x00FF);
    flag_C(C);
    return 0;
}

int Z80::inc_xxL(uint16_t& reg)
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
     *
     * https://clrhome.org/table/#dd
     */
    bool C = test_C();
    uint8_t result = add8(reg & 0x00FF, 1, 0);
    reg = (reg & 0xFF00) | result;
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
    addr_t addr = reg + arg;
    _regs.memptr = addr;
    uint8_t value = read(addr);
    bool C = test_C();
    value = add8(value, 1, 0);
    flag_C(C);
    write(addr, value);
    return 0;
}

int Z80::dec_xxH(uint16_t& reg)
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
    uint16_t result = sub8(reg >> 8, 1, 0);
    reg = (result << 8) | (reg & 0x00FF);
    flag_C(C);
    return 0;
}

int Z80::dec_xxL(uint16_t& reg)
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
    uint8_t result = sub8(reg & 0x00FF, 1, 0);
    reg = (reg & 0xFF00) | result;
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
    addr_t addr = reg + arg;
    uint8_t value = read(addr);
    bool C = test_C();
    value = sub8(value, 1, 0);
    flag_C(C);
    write(addr, value);
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
     *
     * https://clrhome.org/table/#dd
     */
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
    self._regs.IX = (arg << 8) | (self._regs.IX & 0xFF);
    return 0;
}

int Z80::i_LD_IXL_n(Z80& self, uint8_t op, addr_t arg)
{
    /*
     * LD IXL, n - Undocumented
     */
    self._regs.IX = (self._regs.IX & 0xFF00) | (arg & 0x00FF);
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
     *
     * https://clrhome.org/table/#dd
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
     *
     * https://clrhome.org/table/#dd
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
     *
     * https://clrhome.org/table/#dd
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
    addr_t addr = self._regs.IX + arg;
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
    addr_t addr = self._regs.IX + arg;
    int value = self.read(addr);
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
    addr_t addr = self._regs.IX + arg;
    uint8_t value = self.read(addr);
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
    addr_t addr = self._regs.IX + arg;
    uint8_t value = self.read(addr);
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
    addr_t addr = self._regs.IX + arg;
    uint8_t value = self.read(addr);
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
    addr_t addr = self._regs.IX + arg;
    uint8_t value = self.read(addr);
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
    addr_t addr = self._regs.IX + arg;
    uint8_t value = self.read(addr);
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
