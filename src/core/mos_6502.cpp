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
#include "mos_6502.hpp"

#include <chrono>
#include <iomanip>

#include "monitor.hpp"

namespace caio {
namespace mos {

const Mos6502::Instruction Mos6502::instr_set[256] = {
    { "BRK",            Mos6502::i_BRK,         MODE_NONE,  7                   },  /* 00 */
    { "ORA ($*, X)",    Mos6502::i_ORA,         MODE_IND_X, 6                   },  /* 01 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2                   },  /* 02 */
    { "SLO ($*, X)",    Mos6502::i_SLO,         MODE_IND_X, 8                   },  /* 03 */
    { "NOP $*",         Mos6502::i_NOP,         MODE_ZP,    3                   },  /* 04 */
    { "ORA $*",         Mos6502::i_ORA,         MODE_ZP,    3                   },  /* 05 */
    { "ASL $*",         Mos6502::i_ASL,         MODE_ZP,    5,  NO_DUMMY_READ   },  /* 06 */
    { "SLO $*",         Mos6502::i_SLO,         MODE_ZP,    5                   },  /* 07 */
    { "PHP",            Mos6502::i_PHP,         MODE_NONE,  3                   },  /* 08 */
    { "ORA #$*",        Mos6502::i_ORA_imm,     MODE_IMM,   2                   },  /* 09 */
    { "ASL",            Mos6502::i_ASL_acc,     MODE_NONE,  2,  NO_DUMMY_READ   },  /* 0A */
    { "ANC #$*",        Mos6502::i_ANC_imm,     MODE_IMM,   2                   },  /* 0B */
    { "NOP $^",         Mos6502::i_NOP,         MODE_ABS,   4                   },  /* 0C */
    { "ORA $^",         Mos6502::i_ORA,         MODE_ABS,   4                   },  /* 0D */
    { "ASL $^",         Mos6502::i_ASL,         MODE_ABS,   6,  NO_DUMMY_READ   },  /* 0E */
    { "SLO $^",         Mos6502::i_SLO,         MODE_ABS,   6                   },  /* 0F */

    { "BPL $+",         Mos6502::i_BPL,         MODE_REL,   2                   },  /* 10 */
    { "ORA ($*), Y",    Mos6502::i_ORA,         MODE_IND_Y, 5                   },  /* 11 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2                   },  /* 12 */
    { "SLO ($*), Y",    Mos6502::i_SLO,         MODE_IND_Y, 8,  NO_DUMMY_READ   },  /* 13 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,                  },  /* 14 */
    { "ORA $*, X",      Mos6502::i_ORA,         MODE_ZP_X,  4,                  },  /* 15 */
    { "ASL $*, X",      Mos6502::i_ASL,         MODE_ZP_X,  6,  NO_DUMMY_READ   },  /* 16 */
    { "SLO $*, X",      Mos6502::i_SLO,         MODE_ZP_X,  6,                  },  /* 17 */
    { "CLC",            Mos6502::i_CLC,         MODE_NONE,  2,                  },  /* 18 */
    { "ORA $^, Y",      Mos6502::i_ORA,         MODE_ABS_Y, 4,                  },  /* 19 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* 1A */
    { "SLO $^, Y",      Mos6502::i_SLO,         MODE_ABS_Y, 7,  NO_DUMMY_READ   },  /* 1B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,                  },  /* 1C */
    { "ORA $^, X",      Mos6502::i_ORA,         MODE_ABS_X, 4,                  },  /* 1D */
    { "ASL $^, X",      Mos6502::i_ASL,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 1E */
    { "SLO $^, X",      Mos6502::i_SLO,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 1F */

    { "JSR $^",         Mos6502::i_JSR,         MODE_ABS,   6,                  },  /* 20 */
    { "AND ($*, X)",    Mos6502::i_AND,         MODE_IND_X, 6,                  },  /* 21 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 22 */
    { "RLA ($*, X)",    Mos6502::i_RLA,         MODE_IND_X, 8,                  },  /* 23 */
    { "BIT $*",         Mos6502::i_BIT,         MODE_ZP,    3,                  },  /* 24 */
    { "AND $*",         Mos6502::i_AND,         MODE_ZP,    3,                  },  /* 25 */
    { "ROL $*",         Mos6502::i_ROL,         MODE_ZP,    5,  NO_DUMMY_READ   },  /* 26 */
    { "RLA $*",         Mos6502::i_RLA,         MODE_ZP,    5,                  },  /* 27 */
    { "PLP",            Mos6502::i_PLP,         MODE_NONE,  4,                  },  /* 28 */
    { "AND #$*",        Mos6502::i_AND_imm,     MODE_IMM,   2,                  },  /* 29 */
    { "ROL",            Mos6502::i_ROL_acc,     MODE_NONE,  2,  NO_DUMMY_READ   },  /* 2A */
    { "ANC #$*",        Mos6502::i_ANC_imm,     MODE_IMM,   2,                  },  /* 2B */
    { "BIT $^",         Mos6502::i_BIT,         MODE_ABS,   4,                  },  /* 2C */
    { "AND $^",         Mos6502::i_AND,         MODE_ABS,   4,                  },  /* 2D */
    { "ROL $^",         Mos6502::i_ROL,         MODE_ABS,   6,  NO_DUMMY_READ   },  /* 2E */
    { "RLA $^",         Mos6502::i_RLA,         MODE_ABS,   6,                  },  /* 2F */

    { "BMI $+",         Mos6502::i_BMI,         MODE_REL,   2,                  },  /* 30 */
    { "AND ($*), Y",    Mos6502::i_AND,         MODE_IND_Y, 5,                  },  /* 31 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 32 */
    { "RLA ($*), Y",    Mos6502::i_RLA,         MODE_IND_Y, 8,  NO_DUMMY_READ   },  /* 33 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,                  },  /* 34 */
    { "AND $*, X",      Mos6502::i_AND,         MODE_ZP_X,  4,                  },  /* 35 */
    { "ROL $*, X",      Mos6502::i_ROL,         MODE_ZP_X,  6,  NO_DUMMY_READ   },  /* 36 */
    { "RLA $*, X",      Mos6502::i_RLA,         MODE_ZP_X,  6,                  },  /* 37 */
    { "SEC",            Mos6502::i_SEC,         MODE_NONE,  2,                  },  /* 38 */
    { "AND $^, Y",      Mos6502::i_AND,         MODE_ABS_Y, 4,                  },  /* 39 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* 3A */
    { "RLA $^, Y",      Mos6502::i_RLA,         MODE_ABS_Y, 7,  NO_DUMMY_READ   },  /* 3B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,                  },  /* 3C */
    { "AND $^, X",      Mos6502::i_AND,         MODE_ABS_X, 4,                  },  /* 3D */
    { "ROL $^, X",      Mos6502::i_ROL,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 3E */
    { "RLA $^, X",      Mos6502::i_RLA,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 3F */

    { "RTI",            Mos6502::i_RTI,         MODE_NONE,  6,                  },  /* 40 */
    { "EOR ($*, X)",    Mos6502::i_EOR,         MODE_IND_X, 6,                  },  /* 41 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 42 */
    { "SRE ($*, X)",    Mos6502::i_SRE,         MODE_IND_X, 8,                  },  /* 43 */
    { "NOP $*",         Mos6502::i_NOP,         MODE_ZP,    3,                  },  /* 44 */
    { "EOR $*",         Mos6502::i_EOR,         MODE_ZP,    3,                  },  /* 45 */
    { "LSR $*",         Mos6502::i_LSR,         MODE_ZP,    5,  NO_DUMMY_READ   },  /* 46 */
    { "SRE $*",         Mos6502::i_SRE,         MODE_ZP,    5,                  },  /* 47 */
    { "PHA",            Mos6502::i_PHA,         MODE_NONE,  3,                  },  /* 48 */
    { "EOR #$*",        Mos6502::i_EOR_imm,     MODE_IMM,   2,                  },  /* 49 */
    { "LSR",            Mos6502::i_LSR_acc,     MODE_NONE,  2,  NO_DUMMY_READ   },  /* 4A */
    { "ALR #$*",        Mos6502::i_ALR_imm,     MODE_IMM,   2,                  },  /* 4B */
    { "JMP $^",         Mos6502::i_JMP,         MODE_ABS,   3,                  },  /* 4C */
    { "EOR $^",         Mos6502::i_EOR,         MODE_ABS,   4,                  },  /* 4D */
    { "LSR $^",         Mos6502::i_LSR,         MODE_ABS,   6,  NO_DUMMY_READ   },  /* 4E */
    { "SRE $^",         Mos6502::i_SRE,         MODE_ABS,   6,                  },  /* 4F */

    { "BVC $+",         Mos6502::i_BVC,         MODE_REL,   2,                  },  /* 50 */
    { "EOR ($*), Y",    Mos6502::i_EOR,         MODE_IND_Y, 5,                  },  /* 51 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 52 */
    { "SRE ($*), Y",    Mos6502::i_SRE,         MODE_IND_Y, 8,  NO_DUMMY_READ   },  /* 53 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,                  },  /* 54 */
    { "EOR $*, X",      Mos6502::i_EOR,         MODE_ZP_X,  4,                  },  /* 55 */
    { "LSR $*, X",      Mos6502::i_LSR,         MODE_ZP_X,  6,  NO_DUMMY_READ   },  /* 56 */
    { "SRE $*, X",      Mos6502::i_SRE,         MODE_ZP_X,  6,                  },  /* 57 */
    { "CLI",            Mos6502::i_CLI,         MODE_NONE,  2,                  },  /* 58 */
    { "EOR $^, Y",      Mos6502::i_EOR,         MODE_ABS_Y, 4,                  },  /* 59 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* 5A */
    { "SRE $^, Y",      Mos6502::i_SRE,         MODE_ABS_Y, 7,  NO_DUMMY_READ   },  /* 5B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,                  },  /* 5C */
    { "EOR $^, X",      Mos6502::i_EOR,         MODE_ABS_X, 4,                  },  /* 5D */
    { "LSR $^, X",      Mos6502::i_LSR,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 5E */
    { "SRE $^, X",      Mos6502::i_SRE,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 5F */

    { "RTS",            Mos6502::i_RTS,         MODE_NONE,  6,                  },  /* 60 */
    { "ADC ($*, X)",    Mos6502::i_ADC,         MODE_IND_X, 6,                  },  /* 61 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 62 */
    { "RRA ($*, X)",    Mos6502::i_RRA,         MODE_IND_X, 8,                  },  /* 63 */
    { "NOP $*",         Mos6502::i_NOP,         MODE_ZP,    3,                  },  /* 64 */
    { "ADC $*",         Mos6502::i_ADC,         MODE_ZP,    3,                  },  /* 65 */
    { "ROR $*",         Mos6502::i_ROR,         MODE_ZP,    5,  NO_DUMMY_READ   },  /* 66 */
    { "RRA $*",         Mos6502::i_RRA,         MODE_ZP,    5,                  },  /* 67 */
    { "PLA",            Mos6502::i_PLA,         MODE_NONE,  4,                  },  /* 68 */
    { "ADC #$*",        Mos6502::i_ADC_imm,     MODE_IMM,   2,                  },  /* 69 */
    { "ROR",            Mos6502::i_ROR_acc,     MODE_NONE,  2,  NO_DUMMY_READ   },  /* 6A */
    { "ARR #$*",        Mos6502::i_ARR_imm,     MODE_IMM,   2,                  },  /* 6B */
    { "JMP ($^)",       Mos6502::i_JMP,         MODE_IND,   5,                  },  /* 6C */
    { "ADC $^",         Mos6502::i_ADC,         MODE_ABS,   4,                  },  /* 6D */
    { "ROR $^",         Mos6502::i_ROR,         MODE_ABS,   6,  NO_DUMMY_READ   },  /* 6E */
    { "RRA $^",         Mos6502::i_RRA,         MODE_ABS,   6,                  },  /* 6F */

    { "BVS $+",         Mos6502::i_BVS,         MODE_REL,   2,                  },  /* 70 */
    { "ADC ($*), Y",    Mos6502::i_ADC,         MODE_IND_Y, 5,                  },  /* 71 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 72 */
    { "RRA ($*), Y",    Mos6502::i_RRA,         MODE_IND_Y, 8,  NO_DUMMY_READ   },  /* 73 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,                  },  /* 74 */
    { "ADC $*, X",      Mos6502::i_ADC,         MODE_ZP_X,  4,                  },  /* 75 */
    { "ROR $*, X",      Mos6502::i_ROR,         MODE_ZP_X,  6,  NO_DUMMY_READ   },  /* 76 */
    { "RRA $*, X",      Mos6502::i_RRA,         MODE_ZP_X,  6,                  },  /* 77 */
    { "SEI",            Mos6502::i_SEI,         MODE_NONE,  2,                  },  /* 78 */
    { "ADC $^, Y",      Mos6502::i_ADC,         MODE_ABS_Y, 4,                  },  /* 79 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* 7A */
    { "RRA $^, Y",      Mos6502::i_RRA,         MODE_ABS_Y, 7,  NO_DUMMY_READ   },  /* 7B */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,                  },  /* 7C */
    { "ADC $^, X",      Mos6502::i_ADC,         MODE_ABS_X, 4,                  },  /* 7D */
    { "ROR $^, X",      Mos6502::i_ROR,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 7E */
    { "RRA $^, X",      Mos6502::i_RRA,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* 7F */

    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,                  },  /* 80 */
    { "STA ($*, X)",    Mos6502::i_STA,         MODE_IND_X, 6,                  },  /* 81 */
    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,                  },  /* 82 */
    { "SAX ($*, X)",    Mos6502::i_SAX,         MODE_IND_X, 6,                  },  /* 83 */
    { "STY $*",         Mos6502::i_STY,         MODE_ZP,    3,                  },  /* 84 */
    { "STA $*",         Mos6502::i_STA,         MODE_ZP,    3,                  },  /* 85 */
    { "STX $*",         Mos6502::i_STX,         MODE_ZP,    3,                  },  /* 86 */
    { "SAX $*",         Mos6502::i_SAX,         MODE_ZP,    3,                  },  /* 87 */
    { "DEY",            Mos6502::i_DEY,         MODE_NONE,  2,                  },  /* 88 */
    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,                  },  /* 89 */
    { "TXA",            Mos6502::i_TXA,         MODE_NONE,  2,                  },  /* 8A */
    { "XAA #$*",        Mos6502::i_XAA_imm,     MODE_IMM,   2,                  },  /* 8B */
    { "STY $^",         Mos6502::i_STY,         MODE_ABS,   4,                  },  /* 8C */
    { "STA $^",         Mos6502::i_STA,         MODE_ABS,   4,                  },  /* 8D */
    { "STX $^",         Mos6502::i_STX,         MODE_ABS,   4,                  },  /* 8E */
    { "SAX $^",         Mos6502::i_SAX,         MODE_ABS,   4,                  },  /* 8F */

    { "BCC $+",         Mos6502::i_BCC,         MODE_REL,   2,                  },  /* 90 */
    { "STA ($*), Y",    Mos6502::i_STA,         MODE_IND_Y, 6,  NO_DUMMY_READ   },  /* 91 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* 92 */
    { "SHA ($*), Y",    Mos6502::i_SHA,         MODE_IND_Y, 6,  NO_DUMMY_READ   },  /* 93 */
    { "STY $*, X",      Mos6502::i_STY,         MODE_ZP_X,  4,                  },  /* 94 */
    { "STA $*, X",      Mos6502::i_STA,         MODE_ZP_X,  4,                  },  /* 95 */
    { "STX $*, Y",      Mos6502::i_STX,         MODE_ZP_Y,  4,                  },  /* 96 */
    { "SAX $*, Y",      Mos6502::i_SAX,         MODE_ZP_Y,  4,                  },  /* 97 */
    { "TYA",            Mos6502::i_TYA,         MODE_NONE,  2,                  },  /* 98 */
    { "STA $^, Y",      Mos6502::i_STA,         MODE_ABS_Y, 5,  NO_DUMMY_READ   },  /* 99 */
    { "TXS",            Mos6502::i_TXS,         MODE_NONE,  2,                  },  /* 9A */
    { "SHS $^, Y",      Mos6502::i_SHS,         MODE_ABS_Y, 5,  NO_DUMMY_READ   },  /* 9B */
    { "SHY $^, X",      Mos6502::i_SHY,         MODE_ABS,   5,  NO_DUMMY_READ   },  /* 9C */
    { "STA $^, X",      Mos6502::i_STA,         MODE_ABS_X, 5,  NO_DUMMY_READ   },  /* 9D */
    { "SHX $^, Y",      Mos6502::i_SHX,         MODE_ABS,   5,  NO_DUMMY_READ   },  /* 9E */
    { "SHA $^, Y",      Mos6502::i_SHA,         MODE_ABS_Y, 5,  NO_DUMMY_READ   },  /* 9F */

    { "LDY #$*",        Mos6502::i_LDY_imm,     MODE_IMM,   2,                  },  /* A0 */
    { "LDA ($*, X)",    Mos6502::i_LDA,         MODE_IND_X, 6,                  },  /* A1 */
    { "LDX #$*",        Mos6502::i_LDX_imm,     MODE_IMM,   2,                  },  /* A2 */
    { "LAX ($*, X)",    Mos6502::i_LAX,         MODE_IND_X, 6,                  },  /* A3 */
    { "LDY $*",         Mos6502::i_LDY,         MODE_ZP,    3,                  },  /* A4 */
    { "LDA $*",         Mos6502::i_LDA,         MODE_ZP,    3,                  },  /* A5 */
    { "LDX $*",         Mos6502::i_LDX,         MODE_ZP,    3,                  },  /* A6 */
    { "LAX $*",         Mos6502::i_LAX,         MODE_ZP,    3,                  },  /* A7 */
    { "TAY",            Mos6502::i_TAY,         MODE_NONE,  2,                  },  /* A8 */
    { "LDA #$*",        Mos6502::i_LDA_imm,     MODE_IMM,   2,                  },  /* A9 */
    { "TAX",            Mos6502::i_TAX,         MODE_NONE,  2,                  },  /* AA */
    { "LXA #$*",        Mos6502::i_LXA,         MODE_IMM,   2,                  },  /* AB */
    { "LDY $^",         Mos6502::i_LDY,         MODE_ABS,   4,                  },  /* AC */
    { "LDA $^",         Mos6502::i_LDA,         MODE_ABS,   4,                  },  /* AD */
    { "LDX $^",         Mos6502::i_LDX,         MODE_ABS,   4,                  },  /* AE */
    { "LAX $^",         Mos6502::i_LAX,         MODE_ABS,   4,                  },  /* AF */

    { "BCS $+",         Mos6502::i_BCS,         MODE_REL,   2,                  },  /* B0 */
    { "LDA ($*), Y",    Mos6502::i_LDA,         MODE_IND_Y, 5,                  },  /* B1 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* B2 */
    { "LAX ($*), Y",    Mos6502::i_LAX,         MODE_IND_Y, 5,                  },  /* B3 */
    { "LDY $*, X",      Mos6502::i_LDY,         MODE_ZP_X,  4,                  },  /* B4 */
    { "LDA $*, X",      Mos6502::i_LDA,         MODE_ZP_X,  4,                  },  /* B5 */
    { "LDX $*, Y",      Mos6502::i_LDX,         MODE_ZP_Y,  4,                  },  /* B6 */
    { "LAX $*, Y",      Mos6502::i_LAX,         MODE_ZP_Y,  4,                  },  /* B7 */
    { "CLV",            Mos6502::i_CLV,         MODE_NONE,  2,                  },  /* B8 */
    { "LDA $^, Y",      Mos6502::i_LDA,         MODE_ABS_Y, 4,                  },  /* B9 */
    { "TSX",            Mos6502::i_TSX,         MODE_NONE,  2,                  },  /* BA */
    { "LAS $^, Y",      Mos6502::i_LAS,         MODE_ABS_Y, 4,                  },  /* BB */
    { "LDY $^, X",      Mos6502::i_LDY,         MODE_ABS_X, 4,                  },  /* BC */
    { "LDA $^, X",      Mos6502::i_LDA,         MODE_ABS_X, 4,                  },  /* BD */
    { "LDX $^, Y",      Mos6502::i_LDX,         MODE_ABS_Y, 4,                  },  /* BE */
    { "LAX $^, Y",      Mos6502::i_LAX,         MODE_ABS_Y, 4,                  },  /* BF */

    { "CPY #$*",        Mos6502::i_CPY_imm,     MODE_IMM,   2,                  },  /* C0 */
    { "CMP ($*, X)",    Mos6502::i_CMP,         MODE_IND_X, 6,                  },  /* C1 */
    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,                  },  /* C2 */
    { "DCP ($*, X)",    Mos6502::i_DCP,         MODE_IND_X, 8,                  },  /* C3 */
    { "CPY $*",         Mos6502::i_CPY,         MODE_ZP,    3,                  },  /* C4 */
    { "CMP $*",         Mos6502::i_CMP,         MODE_ZP,    3,                  },  /* C5 */
    { "DEC $*",         Mos6502::i_DEC,         MODE_ZP,    5,                  },  /* C6 */
    { "DCP $*",         Mos6502::i_DCP,         MODE_ZP,    5,                  },  /* C7 */
    { "INY",            Mos6502::i_INY,         MODE_NONE,  2,                  },  /* C8 */
    { "CMP #$*",        Mos6502::i_CMP_imm,     MODE_IMM,   2,                  },  /* C9 */
    { "DEX",            Mos6502::i_DEX,         MODE_NONE,  2,                  },  /* CA */
    { "SBX #$*",        Mos6502::i_SBX_imm,     MODE_IMM,   2,                  },  /* CB */
    { "CPY $^",         Mos6502::i_CPY,         MODE_ABS,   4,                  },  /* CC */
    { "CMP $^",         Mos6502::i_CMP,         MODE_ABS,   4,                  },  /* CD */
    { "DEC $^",         Mos6502::i_DEC,         MODE_ABS,   6,                  },  /* CE */
    { "DCP $^",         Mos6502::i_DCP,         MODE_ABS,   6,                  },  /* CF */

    { "BNE $+",         Mos6502::i_BNE,         MODE_REL,   2,                  },  /* D0 */
    { "CMP ($*), Y",    Mos6502::i_CMP,         MODE_IND_Y, 5,                  },  /* D1 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* D2 */
    { "DCP ($*), Y",    Mos6502::i_DCP,         MODE_IND_Y, 8,  NO_DUMMY_READ   },  /* D3 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,                  },  /* D4 */
    { "CMP $*, X",      Mos6502::i_CMP,         MODE_ZP_X,  4,                  },  /* D5 */
    { "DEC $*, X",      Mos6502::i_DEC,         MODE_ZP_X,  6,  NO_DUMMY_READ   },  /* D6 */
    { "DCP $*, X",      Mos6502::i_DCP,         MODE_ZP_X,  6,                  },  /* D7 */
    { "CLD",            Mos6502::i_CLD,         MODE_NONE,  2,                  },  /* D8 */
    { "CMP $^, Y",      Mos6502::i_CMP,         MODE_ABS_Y, 4,                  },  /* D9 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* DA */
    { "DCP $^, Y",      Mos6502::i_DCP,         MODE_ABS_Y, 7,  NO_DUMMY_READ   },  /* DB */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,                  },  /* DC */
    { "CMP $^, X",      Mos6502::i_CMP,         MODE_ABS_X, 4,                  },  /* DD */
    { "DEC $^, X",      Mos6502::i_DEC,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* DE */
    { "DCP $^, X",      Mos6502::i_DCP,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* DF */

    { "CPX #$*",        Mos6502::i_CPX_imm,     MODE_IMM,   2,                  },  /* E0 */
    { "SBC ($*, X)",    Mos6502::i_SBC,         MODE_IND_X, 6,                  },  /* E1 */
    { "NOP #$*",        Mos6502::i_NOP,         MODE_IMM,   2,                  },  /* E2 */
    { "ISC ($*, X)",    Mos6502::i_ISC,         MODE_IND_X, 8,                  },  /* E3 */
    { "CPX $*",         Mos6502::i_CPX,         MODE_ZP,    3,                  },  /* E4 */
    { "SBC $*",         Mos6502::i_SBC,         MODE_ZP,    3,                  },  /* E5 */
    { "INC $*",         Mos6502::i_INC,         MODE_ZP,    5,                  },  /* E6 */
    { "ISC $*",         Mos6502::i_ISC,         MODE_ZP,    5,                  },  /* E7 */
    { "INX",            Mos6502::i_INX,         MODE_NONE,  2,                  },  /* E8 */
    { "SBC #$*",        Mos6502::i_SBC_imm,     MODE_IMM,   2,                  },  /* E9 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* EA */
    { "SBC #$*",        Mos6502::i_SBC_imm,     MODE_IMM,   2,                  },  /* EB */
    { "CPX $^",         Mos6502::i_CPX,         MODE_ABS,   4,                  },  /* EC */
    { "SBC $^",         Mos6502::i_SBC,         MODE_ABS,   4,                  },  /* ED */
    { "INC $^",         Mos6502::i_INC,         MODE_ABS,   6,                  },  /* EE */
    { "ISC $^",         Mos6502::i_ISC,         MODE_ABS,   6,                  },  /* EF */

    { "BEQ $+",         Mos6502::i_BEQ,         MODE_REL,   2,                  },  /* F0 */
    { "SBC ($*), Y",    Mos6502::i_SBC,         MODE_IND_Y, 5,                  },  /* F1 */
    { "KIL",            Mos6502::i_KIL,         MODE_NONE,  2,                  },  /* F2 */
    { "ISC ($*), Y",    Mos6502::i_ISC,         MODE_IND_Y, 8,  NO_DUMMY_READ   },  /* F3 */
    { "NOP $*, X",      Mos6502::i_NOP,         MODE_ZP_X,  4,                  },  /* F4 */
    { "SBC $*, X",      Mos6502::i_SBC,         MODE_ZP_X,  4,                  },  /* F5 */
    { "INC $*, X",      Mos6502::i_INC,         MODE_ZP_X,  6,  NO_DUMMY_READ   },  /* F6 */
    { "ISC $*, X",      Mos6502::i_ISC,         MODE_ZP_X,  6,                  },  /* F7 */
    { "SED",            Mos6502::i_SED,         MODE_NONE,  2,                  },  /* F8 */
    { "SBC $^, Y",      Mos6502::i_SBC,         MODE_ABS_Y, 4,                  },  /* F9 */
    { "NOP",            Mos6502::i_NOP,         MODE_NONE,  2,                  },  /* FA */
    { "ISC $^, Y",      Mos6502::i_ISC,         MODE_ABS_Y, 7,  NO_DUMMY_READ   },  /* FB */
    { "NOP $^, X",      Mos6502::i_NOP,         MODE_ABS_X, 4,                  },  /* FC */
    { "SBC $^, X",      Mos6502::i_SBC,         MODE_ABS_X, 4,                  },  /* FD */
    { "INC $^, X",      Mos6502::i_INC,         MODE_ABS_X, 7,  NO_DUMMY_READ   },  /* FE */
    { "ISC $^, X",      Mos6502::i_ISC,         MODE_ABS_X, 7,  NO_DUMMY_READ   }   /* FF */
};

std::string Mos6502::Registers::to_string(Mos6502::Flags fl)
{
    return std::format("{}{}1{}{}{}{}{}",
        ((fl & Flags::N) ? "N" : "-"),
        ((fl & Flags::V) ? "V" : "-"),
        ((fl & Flags::B) ? "B" : "-"),
        ((fl & Flags::D) ? "D" : "-"),
        ((fl & Flags::I) ? "I" : "-"),
        ((fl & Flags::Z) ? "Z" : "-"),
        ((fl & Flags::C) ? "C" : "-"));
}

std::string Mos6502::Registers::to_string() const
{
    return std::format("A={:02X} X={:02X} Y={:02X} P={:02X} {} S={:02X} PC={:04X}",
        A, X, Y, P, to_string(static_cast<Flags>(P)), S, PC);
}

Mos6502::Mos6502(const sptr_t<ASpace>& mmap)
    : Mos6502{TYPE, LABEL, mmap}
{
}

Mos6502::Mos6502(std::string_view label, const sptr_t<ASpace>& mmap)
    : Mos6502{TYPE, label, mmap}
{
}

Mos6502::Mos6502(std::string_view type, std::string_view label, const sptr_t<ASpace>& mmap)
    : Name{type, label},
      _mmap{mmap}
{
    if (_mmap) {
        Mos6502::reset();
    }
}

Mos6502::~Mos6502()
{
}

void Mos6502::init(const sptr_t<ASpace>& mmap)
{
    if (mmap) {
        _mmap = mmap;
        reset();
    }
}

void Mos6502::init_monitor(int ifd, int ofd, const monitor::LoadCb& load, const monitor::SaveCb& save)
{
    CAIO_ASSERT(ifd >= 0 && ofd >= 0);

    auto getpc = [this]() -> addr_t {
        return _regs.PC;
    };

    auto setpc = [this](addr_t addr) {
        _regs.PC = addr;
    };

    auto mmap = [this]() {
        return this->_mmap;
    };

    auto regvalue = [this](std::string_view rname) -> uint16_t {
        static std::unordered_map<std::string, std::function<int(const Mos6502&)>> regvals{
            { "ra",   [](const Mos6502& cpu) { return cpu._regs.A;  }},
            { "rx",   [](const Mos6502& cpu) { return cpu._regs.X;  }},
            { "ry",   [](const Mos6502& cpu) { return cpu._regs.Y;  }},
            { "rs",   [](const Mos6502& cpu) { return cpu._regs.S;  }},
            { "rp",   [](const Mos6502& cpu) { return cpu._regs.P;  }},
            { "rp.n", [](const Mos6502& cpu) { return cpu.test_N(); }},
            { "rp.v", [](const Mos6502& cpu) { return cpu.test_V(); }},
            { "rp.b", [](const Mos6502& cpu) { return cpu.test_B(); }},
            { "rp.d", [](const Mos6502& cpu) { return cpu.test_D(); }},
            { "rp.i", [](const Mos6502& cpu) { return cpu.test_I(); }},
            { "rp.z", [](const Mos6502& cpu) { return cpu.test_Z(); }},
            { "rp.c", [](const Mos6502& cpu) { return cpu.test_C(); }}
        };
        auto it = regvals.find(std::string{rname});
        if (it != regvals.end()) {
            return it->second(*this);
        }
        throw InvalidArgument{};
    };

    auto bpdoc = [](std::string_view cmd) -> std::string {
        return {
            std::string{cmd} + " help | h | ?\n" +
            std::string{cmd} + " <addr> [<cond>]\n\n"
            "<cond> = <val> <op> <val>\n\n"
            "<val>  = [*] { [#][$]<u16>| ra | rx | ry | rs | rp | rp.n | rp.v | rp.b | rp.i | rp.z | rp.c }\n\n"
            "<op>   = '<' | '>' | '<=' | '>=' | '==' | '!=' | '&' | '|'\n\n"
            "examples:\n"
            "  b $8009 *$fd20 >= #$f0\n"
            "  b $8010 rx >= 80\n"
            "  b $4100 rp.n == 1\n"
        };
    };

    MonitoredCPU monitor_funcs = monitor::monitored_cpu_defaults(this);
    monitor_funcs.getpc = getpc;
    monitor_funcs.setpc = setpc;
    monitor_funcs.mmap = mmap;
    monitor_funcs.regvalue = regvalue;
    monitor_funcs.bpdoc = bpdoc;

    if (load) {
        monitor_funcs.load = load;
    }

    if (save) {
        monitor_funcs.save = save;
    }

    _monitor = std::make_unique<Monitor>(ifd, ofd, std::move(monitor_funcs));
    _monitor->add_breakpoint(read_addr(vRESET));
}

void Mos6502::logfile(int fd)
{
    _log.logfile(fd);
}

void Mos6502::loglevel(std::string_view lvs)
{
    _log.loglevel(lvs);
}

Loglevel Mos6502::loglevel() const
{
    return _log.loglevel();
}

void Mos6502::reset()
{
    const addr_t pc = (_mmap->read(vRESET + 1) << 8) | _mmap->read(vRESET);

    /* Power up */
    _regs = {
        .A  = 0,
        .X  = 0,
        .Y  = 0,
        .PC = pc,
        .S  = S_init,
        .P  = Flags::I | Flags::Z | Flags::_
    };

    _halted = false;
}

bool Mos6502::irq_pin(bool active)
{
    _irq_pin = active;
    return _irq_pin;
}

bool Mos6502::nmi_pin(bool active)
{
    _nmi_pin = active;
    return _nmi_pin;
}

bool Mos6502::rdy_pin(bool active)
{
    _rdy_pin = active;
    return _rdy_pin;
}

bool Mos6502::rdy_pin() const
{
    return _rdy_pin;
}

void Mos6502::ebreak()
{
    _break = true;
}

void Mos6502::bpadd(addr_t addr, const BreakpointCb& cb, void* arg)
{
    _breakpoints[addr] = {cb, arg};
}

void Mos6502::bpdel(addr_t addr)
{
    _breakpoints.erase(addr);
}

const Mos6502::Registers& Mos6502::regs() const
{
    return _regs;
}

void Mos6502::disass(std::ostream& os, addr_t start, size_t count, bool show_pc)
{
    for (addr_t addr = start; count; --count) {
        auto line = disass(addr, show_pc);
        os << line << "\n";
    }
}

std::string Mos6502::disass(addr_t& addr, bool show_pc)
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
    const uint8_t opcode = peek(addr);
    const auto& ins = instr_set[opcode];

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
        const char v = format[pos];
        std::ostringstream ops{};
        addr_t operand{};
        uint8_t ophi{}, oplo{};

        oplo = peek(addr++);

        hex << " " << utils::to_string(oplo);

        switch (v) {
        case '*':
            /* Operand is an 8 bits value, disassembled as $00 or #$00 */
            ops << utils::to_string(oplo);
            break;

        case '^':
            /* Operand is a 16 bit value, disassembled as $0000 */
            ophi = peek(addr++);
            operand = (ophi << 8) | oplo;
            hex << " " << utils::to_string(ophi);
            ops << utils::to_string(operand);
            break;

        case '+':
            /* Operand is a relative address but the disassembled string shows the absolute address */
            operand = addr + oplo;
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
    if (_halted) {
        /*
         * CPU is halted, the only way to exit this condition is by doing a RESET.
         */
        return 1;
    }

    if (!_rdy_pin) {
        /*
         * CPU freezed on read operations.
         * In the real CPU several clock cycles are needed to execute one single instruction,
         * in this emulator, instructions are executed in one single_step() call, the number of
         * cycles is returned back to the caller clock which will call this method again after
         * those cycles have passed.
         * This means that we are not able to discern between read and write cycles, so we assume
         * the next operation will always be a read operation.
         *
         * For the reasons above, this pin acts as the AEC pin.
         */
        return 1;
    }

    std::string line{};
    if (_log.is_debug()) {
        addr_t addr = _regs.PC;
        line = disass(addr);
    }

    const uint8_t opcode = read(_regs.PC++);
    const auto& ins = instr_set[opcode];
    size_t extra_cycles{};
    addr_t arg{};

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
        arg = read(_regs.PC++);
        break;

    case MODE_REL:
        arg = read(_regs.PC++);
        break;

    case MODE_ABS:
    case MODE_ABS_X:
    case MODE_ABS_Y:
    case MODE_IND:
        arg = read_addr(_regs.PC);
        _regs.PC += 2;
        break;
    }

    /*
     * Dummy reads and indexing.
     */
    switch (ins.mode) {
    case MODE_NONE:
    case MODE_IMM:
        if (ins.dummy_read) {
            read(_regs.PC);
        }
        break;

    case MODE_REL:
        if (ins.dummy_read) {
            read(_regs.PC + 0);             /* Dummy read at PC + 1 */
            read(_regs.PC + 1 + arg);       /* Dummy read at PC + 1 + rel */
        }
        break;

    case MODE_ZP:
    case MODE_ABS:
        break;

    case MODE_ZP_X:
        if (ins.dummy_read) {
            read(arg);
        }
        arg = (arg + _regs.X) & 255;        /* Zero page index bug */
        break;

    case MODE_ZP_Y:
        if (ins.dummy_read) {
            read(arg);
        }
        arg = (arg + _regs.Y) & 255;        /* Zero page index bug */
        break;

    case MODE_ABS_X:
        if (ins.dummy_read && page_crossed_rel(arg, _regs.X)) {
            /*
             * Dummy read only if page crossed.
             */
            read(arg + _regs.X - 0x0100);
            ++extra_cycles;
        }
        arg += _regs.X;
        break;

    case MODE_ABS_Y:
        if (ins.dummy_read && page_crossed_rel(arg, _regs.Y)) {
            /*
             * Dummy read only if page crossed.
             */
            read(arg + _regs.Y - 0x0100);
            ++extra_cycles;
        }
        arg += _regs.Y;
        break;

    case MODE_IND_X:
        if (ins.dummy_read) {
            read(arg);
        }
        arg = (arg + _regs.X) & 255;        /* Zero page index bug */
        arg = read_addr_bug(arg);           /* Page boundary bug */
        break;

    case MODE_IND_Y:
        arg = read_addr_bug(arg);
        if (ins.dummy_read && page_crossed_rel(arg, _regs.Y)) {
            /*
             * Dummy read only if page crossed.
             */
            read(arg + _regs.Y - 0x0100);
            ++extra_cycles;
        }
        arg += _regs.Y;
        break;

    case MODE_IND:
        /* JMP ($xxFF) */
        arg = read_addr_bug(arg);           /* Page boundary bug */
        break;
    }

    size_t cycles = ins.fn(*this, arg) + ins.cycles + extra_cycles;

    if (_log.is_debug()) {
        _log.debug("{:35s}{}  cycles={}\n", line, _regs.to_string(), ins.cycles);
    }

    /*
     * A taken branch w/o page crossed delays IRQ sampling.
     * XXX test
     */
    if (_delayed_irq) {
        _delayed_irq = false;

    } else {
        /*
         * Sample interrupts.
         */
        const bool is_nmi = _nmi_pin;
        addr_t isr_addr{};
        if (is_nmi) {
            _nmi_pin.reset();               /* Reset the pin to simulate an edge triggered interrupt */
            isr_addr = read_addr(vNMI);
        } else if (is_irq_enabled() && _irq_pin) {
            isr_addr = read_addr(vIRQ);
        }

        if (isr_addr) {
            /*
             * Prepare to serve the interrupt.
             */
            read_addr(_regs.PC);            /* Dummy reads at PC and PC + 1 */
            push_addr(_regs.PC);
            push(_regs.P | ((_delayed_I && _delayed_I.value()) * Flags::I));    /* SEI */
            _regs.PC = isr_addr;
            flag(Flags::I);
            if (_log.is_debug()) {
                _log.debug("Detected {} interrupt. Extra cycles=7\n", (is_nmi ? "NMI" : "IRQ"));
            }

            cycles += 7;
        }

        if (_delayed_I) {
            /*
             * I flag is set after interrupt sampling.
             */
            flag_I(_delayed_I.value());
            _delayed_I.reset();
        }
    }

    return cycles;
}

size_t Mos6502::tick(const Clock& clk)
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

    const auto bp = _breakpoints.find(_regs.PC);
    if (bp != _breakpoints.end()) {
        /*
         * System breakpoint (from some part of the emulator).
         */
        const auto& [fn, arg] = bp->second;
        fn(*this, arg);
    }

    const size_t cycles = single_step();
    return (cycles == 0 ? Clockable::HALT : cycles);
}

addr_t Mos6502::read_addr(addr_t addr)
{
    const uint8_t lo = read(addr);
    const uint8_t hi = read(addr + 1);
    return ((hi << 8) | lo);
}

addr_t Mos6502::read_addr_bug(addr_t addr)
{
    const addr_t addr_hi = (addr & 0xFF00) | (((addr & 0x00FF) + 1) & 0x00FF);
    const uint8_t lo = read(addr);
    const uint8_t hi = read(addr_hi);
    return ((hi << 8) | lo);
}

void Mos6502::write_addr(addr_t addr, addr_t data)
{
    const uint8_t lo = data & 0xFF;
    const uint8_t hi = data >> 8;
    write(addr, lo);
    write(addr + 1, hi);
}

uint8_t Mos6502::read(addr_t addr, Device::ReadMode mode)
{
    return _mmap->read(addr, mode);
}

void Mos6502::write(addr_t addr, uint8_t data)
{
    _mmap->write(addr, data);
}

}
}
