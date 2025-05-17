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

namespace caio {
namespace mos {

int Mos6502::i_SLO(Mos6502& self, addr_t addr)
{
    /*
     * Shift left memory then OR with accumulator
     * SLO ($00, X)     - 03 - 8 cycles
     * SLO $00          - 07 - 5 cycles
     * SLO $0000        - 0F - 6 cycles
     * SLO ($00), Y     - 13 - 8 cycles
     * SLO $00, X       - 17 - 6 cycles
     * SLO $0000, Y     - 1B - 7 cycles
     * SLO $0000, X     - 1F - 7 cycles
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_shl(value);
    self.write(addr, value);
    self._regs.A = self.logic_or(self._regs.A, value);
    return 0;
}

int Mos6502::i_RLA(Mos6502& self, addr_t addr)
{
    /*
     * Rotate left memory with carry then AND with accumulator
     * RLA ($00, X)     - 23 - 8 cycles
     * RLA $00          - 27 - 5 cycles
     * RLA $0000        - 2F - 6 cycles
     * RLA ($00), Y     - 33 - 8 cycles
     * RLA $00, X       - 37 - 6 cycles
     * RLA $0000, Y     - 3B - 7 cycles
     * RLA $0000, X     - 3F - 7 cycles
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_rol(value);
    self.write(addr, value);
    self._regs.A = self.logic_and(self._regs.A, value);
    return 0;
}

int Mos6502::i_SRE(Mos6502& self, addr_t addr)
{
    /*
     * SRE ($00, X)     - 43 - 8 cycles - Shift right memory then EOR with accumulator
     * SRE $00          - 47 - 5 cycles
     * SRE $0000        - 4F - 6 cycles
     * SRE ($00), Y     - 53 - 8 cycles
     * SRE $00, X       - 57 - 6 cycles
     * SRE $0000, Y     - 5B - 7 cycles
     * SRE $0000, X     - 5F - 7 cycles
     * Alias: LSE
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_shr(value);
    self.write(addr, value);
    self._regs.A = self.logic_eor(self._regs.A, value);
    return 0;
}

int Mos6502::i_RRA(Mos6502& self, addr_t addr)
{
    /*
     * RRA ($00, X)     - 63 - 8 cycles - Rotate right memory (carry shifted) then add with carry accumulator
     * RRA $00          - 67 - 5 cycles
     * RRA $0000        - 6F - 6 cycles
     * RRA ($00), Y     - 73 - 8 cycles
     * RRA $00, X       - 77 - 6 cycles
     * RRA $0000, Y     - 7B - 7 cycles
     * RRA $0000, X     - 00 - 7 cycles
     * Flags: Same as ROR before ADC
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_ror(value);
    self.write(addr, value);
    self._regs.A = self.adc(self._regs.A, value);
    return 0;
}

int Mos6502::i_SAX(Mos6502& self, addr_t addr)
{
    /*
     * Store A AND X
     * SAX ($00, X)     - 83 - 6 cycles
     * SAX $00          - 87 - 3 cycles
     * SAX $0000        - 8F - 4 cycles
     * SAX $00, Y       - 97 - 4 cycles
     * Flags: -
     */
    const uint8_t value = self._regs.A & self._regs.X;
    self.write(addr, value);
    return 0;
}

int Mos6502::i_LXA(Mos6502& self, addr_t value)
{
    /*
     * Load A and X with same value
     * LXA #$00         - AB - 2 cycles
     * Alias: ATX, LAX immediate
     * Flags: N Z
     *
     * "This opcode ORs the A register with CONST, ANDs the result with an
     * immediate value, and then stores the result in both A and X.
     * N and Z are set according to the value of the accumulator before the
     * instruction executed.
     * Instability: CONST is chip- and/or temperature dependent (common values
     * may be $ee, $00, $ff, ...). Some dependency on the RDY line. Bit 0 and
     * Bit 4 are “weaker” than the other bits, and may drop to 0 in the first
     * cycle of DMA when RDY goes low."
     *
     * see NMOS 6510 - Unintended Opcodes
     */
    constexpr static const uint8_t CONST = 0xFF; /* Common values are 00, EE, or FF */
    const uint8_t res = (self._regs.A | CONST) & value;
    self._regs.A = self._regs.X = res;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_LAX(Mos6502& self, addr_t addr)
{
    /*
     * Load A and X with same value
     * LAX ($00, X)     - A3 - 6 cycles
     * LAX $00          - A7 - 3 cycles
     * LAX $0000        - AF - 3 cycles
     * LAX $00, Y       - B7 - 3 cycles
     * LAX $0000, Y     - BF - 4 cycles + 1 (page boundary crossed)
     * LAX ($00), Y     - B3 - 5 cycles + 1 (page boundary crossed)
     * Flags: N Z
     */
    self._regs.A = self._regs.X = self.read(addr);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_DCP(Mos6502& self, addr_t addr)
{
    /*
     * Decrement memory then compare.
     * DCP ($00, X)     - C3 - 8 cycles
     * DCP $00          - C7 - 5 cycles
     * DCP $0000        - CF - 6 cycles
     * DCP ($00), Y     - D3 - 8 cycles
     * DCP $00, X       - D7 - 6 cycles
     * DCP $0000, Y     - DB - 7 cycles
     * DCP $0000, X     - DF - 7 cycles
     * Alias: DCM
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    --value;
    self.write(addr, value);
    self.cmp(self._regs.A, value);
    return 0;
}

int Mos6502::i_ISC(Mos6502& self, addr_t addr)
{
    /*
     * Increment memory then subtract accumulator.
     * ISC ($00, X)     - E3 - 8 cycles
     * ISC $00          - E7 - 5 cycles
     * ISC $0000        - EF - 6 cycles
     * ISC ($00), Y     - F3 - 8 cycles
     * ISC $00, X       - F7 - 6 cycles
     * ISC $0000, X     - FB - 7 cycles
     * ISC $0000, Y     - FF - 7 cycles
     * Alias: INS, ISB
     * Flags: N V Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    ++value;
    self.write(addr, value);
    self._regs.A = self.sbc(self._regs.A, value);
    return 0;
}

int Mos6502::i_ANC_imm(Mos6502& self, addr_t value)
{
    /*
     * AND immediate value and put bit 7 on carry.
     * ANC #$00         - 0B - 2 cycles
     * Alias: AAC
     * Flags: N Z C
     *
     * "This instruction affects the accumulator;
     * sets the zero flag if the result in the accumulator is 0, otherwise resets the zero flag;
     * sets the negative flag and the carry flag if the result in the accumulator has bit 7 on,
     * otherwise resets the negative flag and the carry flag."
     */
    i_AND_imm(self, value);
    self.flag_C(self._regs.A & 0x80);
    return 0;
}

int Mos6502::i_ALR_imm(Mos6502& self, addr_t value)
{
    /*
     * AND immediate value then shift right with carry.
     * ALR #$00         - 4B - 2 cycles
     * Alias: ASR
     * Flags: N Z C
     */
    self.i_AND_imm(self,  value);
    return self.i_LSR_acc(self, 0);
}

int Mos6502::i_ARR_imm(Mos6502& self, addr_t value)
{
    /*
     * AND immediate value then rotate right.
     * ARR #$00         - 6B - 2 cycles
     * Flags: N V Z C
     *
     * "This instruction seems to be a harmless combination of AND and ROR at
     * first sight, but it turns out that it affects the V flag and also has
     * a special kind of decimal mode. This is because the instruction has
     * inherited some properties of the ADC instruction ($69) in addition to
     * the ROR ($6A).
     * In Binary mode (D flag clear), the instruction effectively does an AND
     * between the accumulator and the immediate parameter, and then shifts
     * the accumulator to the right, copying the C flag to the 8th bit. It
     * sets the Negative and Zero flags just like the ROR would. The ADC code
     * shows up in the Carry and oVerflow flags. The C flag will be copied
     * from the bit 6 of the result (which doesn't seem too logical), and the
     * V flag is the result of an Exclusive OR operation between the bit 6
     * and the bit 5 of the result.  This makes sense, since the V flag will
     * be normally set by an Exclusive OR, too.
     * In Decimal mode (D flag set), the ARR instruction first performs the
     * AND and ROR, just like in Binary mode. The N flag will be copied from
     * the initial C flag, and the Z flag will be set according to the ROR
     * result, as expected. The V flag will be set if the bit 6 of the
     * accumulator changed its state between the AND and the ROR, cleared
     * otherwise.
     * Now comes the funny part. If the low nybble of the AND result,
     * incremented by its lowmost bit, is greater than 5, the low nybble in
     * the ROR result will be incremented by 6. The low nybble may overflow
     * as a consequence of this BCD fixup, but the high nybble won't be
     * adjusted. The high nybble will be BCD fixed in a similar way. If the
     * high nybble of the AND result, incremented by its lowmost bit, is
     * greater than 5, the high nybble in the ROR result will be incremented
     * by 6, and the Carry flag will be set. Otherwise the C flag will be
     * cleared."
     *
     * See https://www.nesdev.org/6502_cpu.txt
     */
    self._regs.A &= (value & 255);

    const uint8_t hiA = self._regs.A >> 4;
    const uint8_t loA = self._regs.A & 15;
    const bool prev_b6 = self._regs.A & D6;

    self._regs.A = self.logic_ror(self._regs.A);

    const bool b6 = self._regs.A & D6;
    const bool b5 = self._regs.A & D5;

    if (self.decimal_mode()) {
        self.flag_V(prev_b6 != b6);

        if (loA + (loA & 1) > 5) {
            self._regs.A = (self._regs.A & 0xF0) | ((self._regs.A + 6) & 0x0F);
        }

        if (hiA + (hiA & 1) > 5) {
            self.flag_C(true);
            self._regs.A = (self._regs.A + 0x60) & 0xFF;
        } else {
            self.flag_C(false);
        }

    } else {
        self.flag_C(b6);
        self.flag_V(b6 ^ b5);
    }

    return 0;
}

int Mos6502::i_XAA_imm(Mos6502& self, addr_t value)
{
    /*
     * X AND immediate value then move into A.
     * XAA #$00         - 8B - 2 cycles
     * Alias: ANE, AXA
     *
     * "A = (A | #$EE) & X & #byte
     * same as
     * A = ((A & #$11 & X) | ( #$EE & X)) & #byte
     * In real 6510/8502 the internal parameter #$11
     * may occasionally be #$10, #$01 or even #$00.
     * This occurs when the video chip starts DMA
     * between the opcode fetch and the parameter fetch
     * of the instruction.  The value probably depends
     * on the data that was left on the bus by the VIC-II."
     *
     * See https://www.nesdev.org/6502_cpu.txt
     */
    return i_LDA_imm(self, (self._regs.A | 0xEE) & self._regs.X & value & 255);
}

int Mos6502::i_SBX_imm(Mos6502& self, addr_t value)
{
    /*
     * A AND X then substract immediate, move result into X.
     * SBX #$00         - CB - 2 cycles
     * Alias: AXS, ASX, SAX
     * Flags: N Z C
     * X = (A & X) - imm
     *
     * "This undocumented instruction performs a bit-by-bit "AND" of the value of the accumulator
     * and the index register X and subtracts the value of memory from this result, using two's
     * complement arithmetic, and stores the result in the index register X.
     * This instruction affects the index register X.
     * The carry flag is set if the result is greater than or equal to 0.
     * The carry flag is reset when the result is less than 0, indicating a borrow.
     * The negative flag is set if the result in index register X has bit 7 on, otherwise it is reset.
     * The Z flag is set if the result in index register X is 0, otherwise it is reset."
     *
     * See https://www.pagetable.com/c64ref/6502/?tab=2#SBX
     */
    const int result = static_cast<int>(self._regs.A & self._regs.X) - static_cast<int>(value);
    self._regs.X = result & 0xFF;
    self.flag_C(result >= 0);
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}

int Mos6502::i_SHA(Mos6502& self, addr_t addr)
{
    /*
     * Store A AND X AND (high-byte of addr + 1) at addr
     * SHA ($00), Y     - 93 - 6 cycles
     * SHA $0000, Y     - 9F - 5 cycles
     * Alias: SAH, AXA, AHY
     * Flags: -
     *
     * "Stores A AND X AND (high-byte of addr + 1) at addr.
     * unstable: sometimes 'AND (H+1)' is dropped, page boundary crossings
     * may not work (with the high-byte of the value used as the high-byte of the address)
     * A AND X AND (H+1) -> M"
     *
     * See https://www.masswerk.at/nowgobang/2021/6502-illegal-opcodes
     */
    const uint8_t value = self._regs.A & self._regs.X & ((addr >> 8) + 1);
    self.write(addr, value);
    return 0;
}

int Mos6502::i_SHY(Mos6502& self, addr_t addr)
{
    /*
     * Store Y AND (high-byte of addr + 1) at addr
     * SHY $0000, X     - 9C - 5 cycles
     *
     * See https://www.masswerk.at/nowgobang/2021/6502-illegal-opcodes
     *
     * The code below is the only way to pass the gblargg instruction test.
     */
    const addr_t dst = addr + self._regs.X;
    const bool crossed = self.page_crossed(addr, dst);
    if (!crossed) {
        const uint8_t hi = (addr + 1) >> 8;
        const uint8_t value = self._regs.Y & hi;
        self.write(dst, value);
    }
    return 0;
}

int Mos6502::i_SHX(Mos6502& self, addr_t addr)
{
    /*
     * Store X AND (high-byte of addr + 1) at addr
     * SHX $0000, Y     - 9E - 5 cycles
     * Alias: SXA, SXH, XAS
     *
     * "Stores X AND (high-byte of addr + 1) at addr.
     * unstable: sometimes 'AND (H+1)' is dropped, page boundary
     * crossings may not work (with the high-byte of the value used
     * as the high-byte of the address)
     * X AND (H+1) -> M"
     *
     * See https://www.masswerk.at/nowgobang/2021/6502-illegal-opcodes
     *
     * The code below is the only way to pass the gblargg instruction test.
     */
    const addr_t dst = addr + self._regs.Y;
    const bool crossed = self.page_crossed(addr, dst);
    if (!crossed) {
        const uint8_t hi = (addr + 1) >> 8;
        const uint8_t value = self._regs.X & hi;
        self.write(dst, value);
    }
    return 0;
}

int Mos6502::i_SHS(Mos6502& self, addr_t addr)
{
    /*
     * Put A AND X in SP and store A AND X AND (high-byte of addr + 1) at addr
     * SHS $0000, Y     - 9B - 5 cycles
     * Alias: SSH, TAS, XAS
     *
     * "Puts A AND X in SP and stores A AND X AND (high-byte of addr. + 1) at addr.
     * unstable: sometimes 'AND (H+1)' is dropped, page boundary crossings may not
     * work (with the high-byte of the value used as the high-byte of the address)
     * A AND X -> SP, A AND X AND (H+1) -> M"
     *
     * See https://www.masswerk.at/nowgobang/2021/6502-illegal-opcodes
     */
    self._regs.S = self._regs.A & self._regs.X;
    const addr_t base = addr - self._regs.Y;
    const uint8_t value = self._regs.S & ((base >> 8) + 1);
    self.write(addr, value);
    return 0;
}

int Mos6502::i_LAS(Mos6502& self, addr_t addr)
{
    /*
     * AND memory with stack pointer put result in stack pointer, accumulator and X
     * LAS $0000, Y     - BB - cycles 4 + 1 (page boundary crossed)
     * Alias: LAE, LAR, AST
     * Flags: N Z
     */
    const uint8_t value = self.read(addr);
    self._regs.A = self._regs.X = self._regs.S = self._regs.S & value;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_KIL(Mos6502& self, addr_t addr)
{
    /*
     * Do nothing until RESET
     * KIL              - 02 12 22 32 42 52 62 72 92 B2 D2 F2 - 2 cycles
     * Alias: HLT, JAM, CIM
     */
    self._halted = true;
    log.debug("KIL instruction at ${:04X}, CPU halted\n", self._regs.PC);
    return 0;
}

}
}
