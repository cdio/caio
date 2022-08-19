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
#include "mos_6502.hpp"

#include <chrono>
#include <iomanip>


namespace caio {

const std::array<Mos6502::Instruction, 256> Mos6502::instr_set{{
    { "BRK",            Mos6502::i_BRK,         MODE_NONE,  7,  1   },  /* 00 */
    { "ORA ($*, X)",    Mos6502::i_ORA,         MODE_IND_X, 6,  2   },  /* 01 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 02 */
    { "SLO ($*, X)",    Mos6502::i_SLO,         MODE_IND_X, 8,  2   },  /* 03 */
    { "NOP $*",         Mos6502::i_NOP,         MODE_ZP,    3,  2   },  /* 04 */
    { "ORA $*",         Mos6502::i_ORA,         MODE_ZP,    3,  2   },  /* 05 */
    { "ASL $*",         Mos6502::i_ASL,         MODE_ZP,    5,  2   },  /* 06 */
    { "SLO $*",         Mos6502::i_SLO,         MODE_ZP,    5,  2   },  /* 07 */
    { "PHP",            Mos6502::i_PHP,         MODE_NONE,  3,  1   },  /* 08 */
    { "ORA #$*",        Mos6502::i_ORA_imm,     MODE_IMM,   2,  2   },  /* 09 */
    { "ASL",            Mos6502::i_ASL_acc,     MODE_NONE,  2,  1   },  /* 0A */
    { "ANC #$*",        Mos6502::i_ANC_imm,     MODE_IMM,   2,  2   },  /* 0B */
    { "NOP $^",         Mos6502::i_NOP,         MODE_ABS,   4,  3   },  /* 0C */
    { "ORA $^",         Mos6502::i_ORA,         MODE_ABS,   4,  3   },  /* 0D */
    { "ASL $^",         Mos6502::i_ASL,         MODE_ABS,   6,  3   },  /* 0E */
    { "SLO $^",         Mos6502::i_SLO,         MODE_ABS,   6,  3   },  /* 0F */

    { "BPL $+",         Mos6502::i_BPL,         MODE_REL,   2,  2   },  /* 10 */
    { "ORA ($*), Y",    Mos6502::i_ORA,         MODE_IND_Y, 5,  2   },  /* 11 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 12 */
    { "SLO ($*), Y",    Mos6502::i_SLO,         MODE_IND_Y, 8,  2   },  /* 13 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,  2   },  /* 14 */
    { "ORA $*, X",      Mos6502::i_ORA,         MODE_ZP_X,  4,  2   },  /* 15 */
    { "ASL $*, X",      Mos6502::i_ASL,         MODE_ZP_X,  6,  2   },  /* 16 */
    { "SLO $*, X",      Mos6502::i_SLO,         MODE_ZP_X,  6,  2   },  /* 17 */
    { "CLC",            Mos6502::i_CLC,         MODE_NONE,  2,  1   },  /* 18 */
    { "ORA $^, Y",      Mos6502::i_ORA,         MODE_ABS_Y, 4,  3   },  /* 19 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* 1A */
    { "SLO $^, Y",      Mos6502::i_SLO,         MODE_ABS_Y, 7,  3   },  /* 1B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,  3   },  /* 1C */
    { "ORA $^, X",      Mos6502::i_ORA,         MODE_ABS_X, 4,  3   },  /* 1D */
    { "ASL $^, X",      Mos6502::i_ASL,         MODE_ABS_X, 7,  3   },  /* 1E */
    { "SLO $^, X",      Mos6502::i_SLO,         MODE_ABS_X, 7,  3   },  /* 1F */

    { "JSR $^",         Mos6502::i_JSR,         MODE_ABS,   6,  3   },  /* 20 */
    { "AND ($*, X)",    Mos6502::i_AND,         MODE_IND_X, 6,  2   },  /* 21 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 22 */
    { "RLA ($*, X)",    Mos6502::i_RLA,         MODE_IND_X, 8,  2   },  /* 23 */
    { "BIT $*",         Mos6502::i_BIT,         MODE_ZP,    3,  2   },  /* 24 */
    { "AND $*",         Mos6502::i_AND,         MODE_ZP,    3,  2   },  /* 25 */
    { "ROL $*",         Mos6502::i_ROL,         MODE_ZP,    5,  2   },  /* 26 */
    { "RLA $*",         Mos6502::i_RLA,         MODE_ZP,    5,  2   },  /* 27 */
    { "PLP",            Mos6502::i_PLP,         MODE_NONE,  4,  1   },  /* 28 */
    { "AND #$*",        Mos6502::i_AND_imm,     MODE_IMM,   2,  2   },  /* 29 */
    { "ROL",            Mos6502::i_ROL_acc,     MODE_NONE,  2,  1   },  /* 2A */
    { "ANC #$*",        Mos6502::i_ANC_imm,     MODE_IMM,   2,  2   },  /* 2B */
    { "BIT $^",         Mos6502::i_BIT,         MODE_ABS,   4,  3   },  /* 2C */
    { "AND $^",         Mos6502::i_AND,         MODE_ABS,   4,  3   },  /* 2D */
    { "ROL $^",         Mos6502::i_ROL,         MODE_ABS,   6,  3   },  /* 2E */
    { "RLA $^",         Mos6502::i_RLA,         MODE_ABS,   6,  3   },  /* 2F */

    { "BMI $+",         Mos6502::i_BMI,         MODE_REL,   2,  2   },  /* 30 */
    { "AND ($*), Y",    Mos6502::i_AND,         MODE_IND_Y, 5,  2   },  /* 31 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 32 */
    { "RLA ($*), Y",    Mos6502::i_RLA,         MODE_IND_Y, 8,  2   },  /* 33 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,  2   },  /* 34 */
    { "AND $*, X",      Mos6502::i_AND,         MODE_ZP_X,  4,  2   },  /* 35 */
    { "ROL $*, X",      Mos6502::i_ROL,         MODE_ZP_X,  6,  2   },  /* 36 */
    { "RLA $*, X",      Mos6502::i_RLA,         MODE_ZP_X,  6,  2   },  /* 37 */
    { "SEC",            Mos6502::i_SEC,         MODE_NONE,  2,  1   },  /* 38 */
    { "AND $^, Y",      Mos6502::i_AND,         MODE_ABS_Y, 4,  3   },  /* 39 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* 3A */
    { "RLA $^, Y",      Mos6502::i_RLA,         MODE_ABS_Y, 7,  3   },  /* 3B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,  3   },  /* 3C */
    { "AND $^, X",      Mos6502::i_AND,         MODE_ABS_X, 4,  3   },  /* 3D */
    { "ROL $^, X",      Mos6502::i_ROL,         MODE_ABS_X, 7,  3   },  /* 3E */
    { "RLA $^, X",      Mos6502::i_RLA,         MODE_ABS_X, 7,  3   },  /* 3F */

    { "RTI",            Mos6502::i_RTI,         MODE_NONE,  6,  1   },  /* 40 */
    { "EOR ($*, X)",    Mos6502::i_EOR,         MODE_IND_X, 6,  2   },  /* 41 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 42 */
    { "SRE ($*, X)",    Mos6502::i_SRE,         MODE_IND_X, 8,  2   },  /* 43 */
    { "NOP $*",         Mos6502::i_NOP,         MODE_ZP,    3,  2   },  /* 44 */
    { "EOR $*",         Mos6502::i_EOR,         MODE_ZP,    3,  2   },  /* 45 */
    { "LSR $*",         Mos6502::i_LSR,         MODE_ZP,    5,  2   },  /* 46 */
    { "SRE $*",         Mos6502::i_SRE,         MODE_ZP,    5,  2   },  /* 47 */
    { "PHA",            Mos6502::i_PHA,         MODE_NONE,  3,  1   },  /* 48 */
    { "EOR #$*",        Mos6502::i_EOR_imm,     MODE_IMM,   2,  2   },  /* 49 */
    { "LSR",            Mos6502::i_LSR_acc,     MODE_NONE,  2,  1   },  /* 4A */
    { "ALR #$*",        Mos6502::i_ALR_imm,     MODE_IMM,   2,  2   },  /* 4B */
    { "JMP $^",         Mos6502::i_JMP,         MODE_ABS,   3,  3   },  /* 4C */
    { "EOR $^",         Mos6502::i_EOR,         MODE_ABS,   4,  3   },  /* 4D */
    { "LSR $^",         Mos6502::i_LSR,         MODE_ABS,   6,  3   },  /* 4E */
    { "SRE $^",         Mos6502::i_SRE,         MODE_ABS,   6,  3   },  /* 4F */

    { "BVC $+",         Mos6502::i_BVC,         MODE_REL,   2,  2   },  /* 50 */
    { "EOR ($*), Y",    Mos6502::i_EOR,         MODE_IND_Y, 5,  2   },  /* 51 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 52 */
    { "SRE ($*), Y",    Mos6502::i_SRE,         MODE_IND_Y, 8,  2   },  /* 53 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,  2   },  /* 54 */
    { "EOR $*, X",      Mos6502::i_EOR,         MODE_ZP_X,  4,  2   },  /* 55 */
    { "LSR $*, X",      Mos6502::i_LSR,         MODE_ZP_X,  6,  2   },  /* 56 */
    { "SRE $*, X",      Mos6502::i_SRE,         MODE_ZP_X,  6,  2   },  /* 57 */
    { "CLI",            Mos6502::i_CLI,         MODE_NONE,  2,  1   },  /* 58 */
    { "EOR $^, Y",      Mos6502::i_EOR,         MODE_ABS_Y, 4,  3   },  /* 59 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* 5A */
    { "SRE $^, Y",      Mos6502::i_SRE,         MODE_ABS_Y, 7,  3   },  /* 5B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,  3   },  /* 5C */
    { "EOR $^, X",      Mos6502::i_EOR,         MODE_ABS_X, 4,  3   },  /* 5D */
    { "LSR $^, X",      Mos6502::i_LSR,         MODE_ABS_X, 7,  3   },  /* 5E */
    { "SRE $^, X",      Mos6502::i_SRE,         MODE_ABS_X, 7,  3   },  /* 5F */

    { "RTS",            Mos6502::i_RTS,         MODE_NONE,  6,  1   },  /* 60 */
    { "ADC ($*, X)",    Mos6502::i_ADC,         MODE_IND_X, 6,  2   },  /* 61 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 62 */
    { "RRA ($*, X)",    Mos6502::i_RRA,         MODE_IND_X, 8,  2   },  /* 63 */
    { "NOP $*",         Mos6502::i_NOP,         MODE_ZP,    3,  2   },  /* 64 */
    { "ADC $*",         Mos6502::i_ADC,         MODE_ZP,    3,  2   },  /* 65 */
    { "ROR $*",         Mos6502::i_ROR,         MODE_ZP,    5,  2   },  /* 66 */
    { "RRA $*",         Mos6502::i_RRA,         MODE_ZP,    5,  2   },  /* 67 */
    { "PLA",            Mos6502::i_PLA,         MODE_NONE,  4,  1   },  /* 68 */
    { "ADC #$*",        Mos6502::i_ADC_imm,     MODE_IMM,   2,  2   },  /* 69 */
    { "ROR",            Mos6502::i_ROR_acc,     MODE_NONE,  2,  1   },  /* 6A */
    { "ARR #$*",        Mos6502::i_ARR_imm,     MODE_IMM,   2,  2   },  /* 6B */
    { "JMP ($^)",       Mos6502::i_JMP,         MODE_IND,   5,  3   },  /* 6C */
    { "ADC $^",         Mos6502::i_ADC,         MODE_ABS,   4,  3   },  /* 6D */
    { "ROR $^",         Mos6502::i_ROR,         MODE_ABS,   6,  3   },  /* 6E */
    { "RRA $^",         Mos6502::i_RRA,         MODE_ABS,   6,  3   },  /* 6F */

    { "BVS $+",         Mos6502::i_BVS,         MODE_REL,   2,  2   },  /* 70 */
    { "ADC ($*), Y",    Mos6502::i_ADC,         MODE_IND_Y, 5,  2   },  /* 71 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 72 */
    { "RRA ($*), Y",    Mos6502::i_RRA,         MODE_IND_Y, 8,  2   },  /* 73 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,  2   },  /* 74 */
    { "ADC $*, X",      Mos6502::i_ADC,         MODE_ZP_X,  4,  2   },  /* 75 */
    { "ROR $*, X",      Mos6502::i_ROR,         MODE_ZP_X,  6,  2   },  /* 76 */
    { "RRA $*, X",      Mos6502::i_RRA,         MODE_ZP_X,  6,  2   },  /* 77 */
    { "SEI",            Mos6502::i_SEI,         MODE_NONE,  2,  1   },  /* 78 */
    { "ADC $^, Y",      Mos6502::i_ADC,         MODE_ABS_Y, 4,  3   },  /* 79 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* 7A */
    { "RRA $^, Y",      Mos6502::i_RRA,         MODE_ABS_Y, 7,  3   },  /* 7B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,  3   },  /* 7C */
    { "ADC $^, X",      Mos6502::i_ADC,         MODE_ABS_X, 4,  3   },  /* 7D */
    { "ROR $^, X",      Mos6502::i_ROR,         MODE_ABS_X, 7,  3   },  /* 7E */
    { "RRA $^, X",      Mos6502::i_RRA,         MODE_ABS_X, 7,  3   },  /* 7F */

    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,  2   },  /* 80 */
    { "STA ($*, X)",    Mos6502::i_STA,         MODE_IND_X, 6,  2   },  /* 81 */
    { "NOP $#*",        Mos6502::i_NOP,         MODE_IMM,   2,  2   },  /* 82 */
    { "SAX ($*, X)",    Mos6502::i_SAX,         MODE_IND_X, 6,  2   },  /* 83 */
    { "STY $*",         Mos6502::i_STY,         MODE_ZP,    3,  2   },  /* 84 */
    { "STA $*",         Mos6502::i_STA,         MODE_ZP,    3,  2   },  /* 85 */
    { "STX $*",         Mos6502::i_STX,         MODE_ZP,    3,  2   },  /* 86 */
    { "SAX $*",         Mos6502::i_SAX,         MODE_ZP,    3,  2   },  /* 87 */
    { "DEY",            Mos6502::i_DEY,         MODE_NONE,  2,  1   },  /* 88 */
    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,  2   },  /* 89 */
    { "TXA",            Mos6502::i_TXA,         MODE_NONE,  2,  1   },  /* 8A */
    { "XAA #$*",        Mos6502::i_XAA_imm,     MODE_IMM,   2,  2   },  /* 8B */
    { "STY $^",         Mos6502::i_STY,         MODE_ABS,   4,  3   },  /* 8C */
    { "STA $^",         Mos6502::i_STA,         MODE_ABS,   4,  3   },  /* 8D */
    { "STX $^",         Mos6502::i_STX,         MODE_ABS,   4,  3   },  /* 8E */
    { "SAX $^",         Mos6502::i_SAX,         MODE_ABS,   4,  3   },  /* 8F */

    { "BCC $+",         Mos6502::i_BCC,         MODE_REL,   2,  2   },  /* 90 */
    { "STA ($*), Y",    Mos6502::i_STA,         MODE_IND_Y, 6,  2   },  /* 91 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* 92 */
    { "AHX ($*), Y",    Mos6502::i_AHX,         MODE_IND_Y, 6,  2   },  /* 93 */
    { "STY $*, X",      Mos6502::i_STY,         MODE_ZP_X,  4,  2   },  /* 94 */
    { "STA $*, X",      Mos6502::i_STA,         MODE_ZP_X,  4,  2   },  /* 95 */
    { "STX $*, Y",      Mos6502::i_STX,         MODE_ZP_Y,  4,  2   },  /* 96 */
    { "SAX $*, Y",      Mos6502::i_SAX,         MODE_ZP_Y,  4,  2   },  /* 97 */
    { "TYA",            Mos6502::i_TYA,         MODE_NONE,  2,  1   },  /* 98 */
    { "STA $^, Y",      Mos6502::i_STA,         MODE_ABS_Y, 4,  3   },  /* 99 */
    { "TXS",            Mos6502::i_TXS,         MODE_NONE,  2,  1   },  /* 9A */
    { "TAS $^, Y",      Mos6502::i_TAS,         MODE_ABS_Y, 5,  3   },  /* 9B */
    { "SHY $^, X",      Mos6502::i_SHY,         MODE_ABS_X, 5,  3   },  /* 9C */
    { "STA $^, X",      Mos6502::i_STA,         MODE_ABS_X, 5,  3   },  /* 9D */
    { "SHX $^, Y",      Mos6502::i_SHX,         MODE_ABS_Y, 5,  3   },  /* 9E */
    { "AHX $^, Y",      Mos6502::i_AHX,         MODE_ABS_Y, 5,  3   },  /* 9F */

    { "LDY #$*",        Mos6502::i_LDY_imm,     MODE_IMM,   2,  2   },  /* A0 */
    { "LDA ($*, X)",    Mos6502::i_LDA,         MODE_IND_X, 6,  2   },  /* A1 */
    { "LDX #$*",        Mos6502::i_LDX_imm,     MODE_IMM,   2,  2   },  /* A2 */
    { "LAX ($*, X)",    Mos6502::i_LAX,         MODE_IND_X, 6,  2   },  /* A3 */
    { "LDY $*",         Mos6502::i_LDY,         MODE_ZP,    3,  2   },  /* A4 */
    { "LDA $*",         Mos6502::i_LDA,         MODE_ZP,    3,  2   },  /* A5 */
    { "LDX $*",         Mos6502::i_LDX,         MODE_ZP,    3,  2   },  /* A6 */
    { "LAX $*",         Mos6502::i_LAX,         MODE_ZP,    3,  2   },  /* A7 */
    { "TAY",            Mos6502::i_TAY,         MODE_NONE,  2,  1   },  /* A8 */
    { "LDA #$*",        Mos6502::i_LDA_imm,     MODE_IMM,   2,  2   },  /* A9 */
    { "TAX",            Mos6502::i_TAX,         MODE_NONE,  2,  1   },  /* AA */
    { "LAX #$*",        Mos6502::i_LAX_imm,     MODE_IMM,   2,  2   },  /* AB */
    { "LDY $^",         Mos6502::i_LDY,         MODE_ABS,   4,  3   },  /* AC */
    { "LDA $^",         Mos6502::i_LDA,         MODE_ABS,   4,  3   },  /* AD */
    { "LDX $^",         Mos6502::i_LDX,         MODE_ABS,   4,  3   },  /* AE */
    { "LAX $^",         Mos6502::i_LAX,         MODE_ABS,   4,  3   },  /* AF */

    { "BCS $+",         Mos6502::i_BCS,         MODE_REL,   2,  2   },  /* B0 */
    { "LDA ($*), Y",    Mos6502::i_LDA,         MODE_IND_Y, 5,  2   },  /* B1 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* B2 */
    { "LAX ($*), Y",    Mos6502::i_LAX,         MODE_IND_Y, 5,  2   },  /* B3 */
    { "LDY $*, X",      Mos6502::i_LDY,         MODE_ZP_X,  4,  2   },  /* B4 */
    { "LDA $*, X",      Mos6502::i_LDA,         MODE_ZP_X,  4,  2   },  /* B5 */
    { "LDX $*, Y",      Mos6502::i_LDX,         MODE_ZP_Y,  4,  2   },  /* B6 */
    { "LAX $*, Y",      Mos6502::i_LAX,         MODE_ZP_Y,  4,  2   },  /* B7 */
    { "CLV",            Mos6502::i_CLV,         MODE_NONE,  2,  1   },  /* B8 */
    { "LDA $^, Y",      Mos6502::i_LDA,         MODE_ABS_Y, 4,  3   },  /* B9 */
    { "TSX",            Mos6502::i_TSX,         MODE_NONE,  2,  1   },  /* BA */
    { "LAS $^, Y",      Mos6502::i_LAS,         MODE_ABS_Y, 4,  3   },  /* BB */
    { "LDY $^, X",      Mos6502::i_LDY,         MODE_ABS_X, 4,  3   },  /* BC */
    { "LDA $^, X",      Mos6502::i_LDA,         MODE_ABS_X, 4,  3   },  /* BD */
    { "LDX $^, Y",      Mos6502::i_LDX,         MODE_ABS_Y, 4,  3   },  /* BE */
    { "LAX $^, Y",      Mos6502::i_LAX,         MODE_ABS_Y, 4,  3   },  /* BF */

    { "CPY #$*",        Mos6502::i_CPY_imm,     MODE_IMM,   2,  2   },  /* C0 */
    { "CMP ($*, X)",    Mos6502::i_CMP,         MODE_IND_X, 6,  2   },  /* C1 */
    { "NOP $#*",        Mos6502::i_NOP,         MODE_IMM,   2,  2   },  /* C2 */
    { "DCP ($*, X)",    Mos6502::i_DCP,         MODE_IND_X, 8,  2   },  /* C3 */
    { "CPY $*",         Mos6502::i_CPY,         MODE_ZP,    3,  2   },  /* C4 */
    { "CMP $*",         Mos6502::i_CMP,         MODE_ZP,    3,  2   },  /* C5 */
    { "DEC $*",         Mos6502::i_DEC,         MODE_ZP,    5,  2   },  /* C6 */
    { "DCP $*",         Mos6502::i_DCP,         MODE_ZP,    5,  2   },  /* C7 */
    { "INY",            Mos6502::i_INY,         MODE_NONE,  2,  1   },  /* C8 */
    { "CMP #$*",        Mos6502::i_CMP_imm,     MODE_IMM,   2,  2   },  /* C9 */
    { "DEX",            Mos6502::i_DEX,         MODE_NONE,  2,  1   },  /* CA */
    { "AXS #$*",        Mos6502::i_AXS_imm,     MODE_IMM,   2,  2   },  /* CB */
    { "CPY $^",         Mos6502::i_CPY,         MODE_ABS,   4,  3   },  /* CC */
    { "CMP $^",         Mos6502::i_CMP,         MODE_ABS,   4,  3   },  /* CD */
    { "DEC $^",         Mos6502::i_DEC,         MODE_ABS,   6,  3   },  /* CE */
    { "DCP $^",         Mos6502::i_DCP,         MODE_ABS,   6,  3   },  /* CF */

    { "BNE $+",         Mos6502::i_BNE,         MODE_REL,   2,  2   },  /* D0 */
    { "CMP ($*), Y",    Mos6502::i_CMP,         MODE_IND_Y, 5,  2   },  /* D1 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* D2 */
    { "DCP ($*), Y",    Mos6502::i_DCP,         MODE_IND_Y, 8,  2   },  /* D3 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,  2   },  /* D4 */
    { "CMP $*, X",      Mos6502::i_CMP,         MODE_ZP_X,  4,  2   },  /* D5 */
    { "DEC $*, X",      Mos6502::i_DEC,         MODE_ZP_X,  4,  2   },  /* D6 */
    { "DCP $*, X",      Mos6502::i_DCP,         MODE_ZP_X,  4,  2   },  /* D7 */
    { "CLD",            Mos6502::i_CLD,         MODE_NONE,  2,  1   },  /* D8 */
    { "CMP $^, Y",      Mos6502::i_CMP,         MODE_ABS_Y, 4,  3   },  /* D9 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* DA */
    { "DCP $^, Y",      Mos6502::i_DCP,         MODE_ABS_Y, 7,  3   },  /* DB */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,  3   },  /* DC */
    { "CMP $^, X",      Mos6502::i_CMP,         MODE_ABS_X, 4,  3   },  /* DD */
    { "DEC $^, X",      Mos6502::i_DEC,         MODE_ABS_X, 7,  3   },  /* DE */
    { "DCP $^, X",      Mos6502::i_DCP,         MODE_ABS_X, 7,  3   },  /* DF */

    { "CPX #$*",        Mos6502::i_CPX_imm,     MODE_IMM,   2,  2   },  /* E0 */
    { "SBC ($*, X)",    Mos6502::i_SBC,         MODE_IND_X, 6,  2   },  /* E1 */
    { "NOP $#*",        Mos6502::i_NOP,         MODE_IMM,   2,  2   },  /* E2 */
    { "ISC ($*, X)",    Mos6502::i_ISC,         MODE_IND_X, 8,  2   },  /* E3 */
    { "CPX $*",         Mos6502::i_CPX,         MODE_ZP,    3,  2   },  /* E4 */
    { "SBC $*",         Mos6502::i_SBC,         MODE_ZP,    3,  2   },  /* E5 */
    { "INC $*",         Mos6502::i_INC,         MODE_ZP,    5,  2   },  /* E6 */
    { "ISC $*",         Mos6502::i_ISC,         MODE_ZP,    5,  2   },  /* E7 */
    { "INX",            Mos6502::i_INX,         MODE_NONE,  2,  1   },  /* E8 */
    { "SBC #$*",        Mos6502::i_SBC_imm,     MODE_IMM,   2,  2   },  /* E9 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* EA */
    { "SBC #$*",        Mos6502::i_SBC_imm,     MODE_IMM,   2,  2   },  /* EB */
    { "CPX $^",         Mos6502::i_CPX,         MODE_ABS,   4,  3   },  /* EC */
    { "SBC $^",         Mos6502::i_SBC,         MODE_ABS,   4,  3   },  /* ED */
    { "INC $^",         Mos6502::i_INC,         MODE_ABS,   6,  3   },  /* EE */
    { "ISC $^",         Mos6502::i_ISC,         MODE_ABS,   6,  3   },  /* EF */

    { "BEQ $+",         Mos6502::i_BEQ,         MODE_REL,   2,  2   },  /* F0 */
    { "SBC ($*), Y",    Mos6502::i_SBC,         MODE_IND_Y, 5,  2   },  /* F1 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  0,  1   },  /* F2 */
    { "ISC ($*), Y",    Mos6502::i_ISC,         MODE_IND_Y, 8,  2   },  /* F3 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,  2   },  /* F4 */
    { "SBC $*, X",      Mos6502::i_SBC,         MODE_ZP_X,  4,  2   },  /* F5 */
    { "INC $*, X",      Mos6502::i_INC,         MODE_ZP_X,  4,  2   },  /* F6 */
    { "ISC $*, X",      Mos6502::i_ISC,         MODE_ZP_X,  4,  2   },  /* F7 */
    { "SED",            Mos6502::i_SED,         MODE_NONE,  2,  1   },  /* F8 */
    { "SBC $^, Y",      Mos6502::i_SBC,         MODE_ABS_Y, 4,  3   },  /* F9 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,  1   },  /* FA */
    { "ISC $^, Y",      Mos6502::i_ISC,         MODE_ABS_Y, 7,  3   },  /* FB */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,  3   },  /* FC */
    { "SBC $^, X",      Mos6502::i_SBC,         MODE_ABS_X, 4,  3   },  /* FD */
    { "INC $^, X",      Mos6502::i_INC,         MODE_ABS_X, 7,  3   },  /* FE */
    { "ISC $^, X",      Mos6502::i_ISC,         MODE_ABS_X, 7,  3   }   /* FF */
}};


std::string Mos6502::Registers::to_string(Mos6502::Flags fl)
{
    std::ostringstream ss{};

    ss << ((fl & Flags::N) ? "N" : "-")
       << ((fl & Flags::V) ? "V" : "-")
       << "-"
       << ((fl & Flags::B) ? "B" : "-")
       << ((fl & Flags::D) ? "D" : "-")
       << ((fl & Flags::I) ? "I" : "-")
       << ((fl & Flags::Z) ? "Z" : "-")
       << ((fl & Flags::C) ? "C" : "-");

    return ss.str();
}

std::string Mos6502::Registers::to_string() const
{
    std::ostringstream ss;

    ss << "A=" << utils::to_string(A)
       << "  X=" << utils::to_string(X)
       << "  Y=" << utils::to_string(Y)
       << "  P=" << utils::to_string(P) << " " << to_string(static_cast<Flags>(P))
       << "  S=" << utils::to_string(S)
       << "  PC=" << utils::to_string(PC);

    return ss.str();
}

Mos6502::Mos6502(const std::string &type, const std::string &label)
    : Name{type, label}
{
}

Mos6502::Mos6502(const std::shared_ptr<ASpace> &mmap, const std::string &type, const std::string &label)
    : Name{type, (label.empty() ? LABEL : label)}
{
    reset(mmap);
}

Mos6502::~Mos6502()
{
}

void Mos6502::init_monitor(std::istream &is, std::ostream &os)
{
    if (!_mmap) {
        throw InvalidArgument{*this, "System mappings not defined"};
    }

    _monitor = std::make_unique<Mos6502Monitor>(is, os, *this);
    _monitor->add_breakpoint(_regs.PC);
}

void Mos6502::reset(const std::shared_ptr<ASpace> &mmap)
{
    if (mmap) {
        _mmap = mmap;
    }

    _regs = {
        .PC = read_addr(vRESET),
        .S  = S_init
    };

    flag(0);
}

void Mos6502::loglevel(const std::string &lvs)
{
    _log.loglevel(lvs);
}

Logger::Level Mos6502::loglevel() const
{
    return _log.loglevel();
}

IRQPin &Mos6502::irq_pin()
{
    return _irq_pin;
}

IRQPin &Mos6502::nmi_pin()
{
    return _nmi_pin;
}

InputPin &Mos6502::rdy_pin()
{
    return _rdy_pin;
}

void Mos6502::ebreak()
{
    _break = true;
}

void Mos6502::bpadd(addr_t addr, const breakpoint_cb_t &cb, void *arg)
{
    _breakpoints[addr] = {cb, arg};
}

void Mos6502::bpdel(addr_t addr)
{
    _breakpoints.erase(addr);
}

const Mos6502::Registers &Mos6502::regs() const
{
    return _regs;
}

void Mos6502::disass(std::ostream &os, addr_t start, size_t count, bool show_pc)
{
    for (addr_t addr = start; count; --count) {
        const std::string &line = disass(addr, show_pc);
        os << line << std::endl;
    }
}

std::string Mos6502::disass(addr_t &addr, bool show_pc)
{
    /*
     * Output format:
     *  8000: A9 00      LDA #$00
     *  8002: 8D 21 D0   STA $D021
     *  8005: 60         RTS
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
         * '^', '*', or '+' is present in the format string: Find the operands.
         */
        char v = format[pos];
        if ((v == '*' && ins.size != 2) ||
            (v == '+' && ins.size != 2) ||
            (v == '^' && ins.size != 3)) {
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

size_t Mos6502::single_step()
{
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
        case MODE_NONE:
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
            arg += _regs.X;
            arg %= 0x0100;                      /* Zero page index bug */
            break;

        case MODE_ABS_X:
            arg += _regs.X;
            break;

        case MODE_ZP_Y:
            arg += _regs.Y;
            arg %= 0x0100;                     /* Zero page index bug */
            break;

        case MODE_ABS_Y:
            arg += _regs.Y;
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
}

size_t Mos6502::tick(const Clock &clk)
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
        if (_monitor->run() == Clockable::HALT) {
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

addr_t Mos6502::read_addr(size_t addr) const
{
    uint8_t lo = read(addr);
    uint8_t hi = read(addr + 1);

    return (static_cast<addr_t>(hi) << 8 | lo);
}

void Mos6502::write_addr(addr_t addr, addr_t data)
{
    uint8_t lo = static_cast<uint8_t>(data & 0xFF);
    uint8_t hi = static_cast<uint8_t>(data >> 8);

    write(addr, lo);
    write(addr + 1, hi);
}

uint8_t Mos6502::read(addr_t addr) const
{
    return _mmap->read(addr);
}

void Mos6502::write(addr_t addr, uint8_t data)
{
    _mmap->write(addr, data);
}

}
