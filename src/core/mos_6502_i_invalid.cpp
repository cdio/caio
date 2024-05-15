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

int Mos6502::i_SLO(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SLO - Shift Left memory then OR with Accumulator
     * alias: ASO
     *
     *  SLO $00         5 cycles
     *  SLO $00, X      6 cycles
     *  SLO $0000       6 cycles
     *  SLO $0000, X    7 cycles
     *  SLO $0000, Y    7 cycles
     *  SLO ($00, X)    8 cycles
     *  SLO ($00), Y    8 cycles
     *
     * - Get value from memory;
     * - Shift left one bit;
     * - Store value back on memory;
     * - OR the shifted value with the Accumulator.
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    value = self.logic_shl(value);
    self.write(addr, value);
    self._regs.A = self.logic_or(self._regs.A, value);
    return 0;
}

int Mos6502::i_RLA(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: RLA - Rotate left memory with carry then AND with Accumulator
     *
     *  RLA ($00, X)    8 cycles
     *  RLA $00         5 cycles
     *  RLA $0000       6 cycles
     *  RLA ($00), Y    8 cycles
     *  RLA $00, X      6 cycles
     *  RLA $0000, Y    7 cycles
     *  RLA $0000, X    7 cycles
     *
     * - Get value from memory;
     * - Rotate left one bit with carry;
     * - Store value back on memory;
     * - AND the rotated value with the Accumulator.
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    value = self.logic_rol(value);
    self.write(addr, value);
    self._regs.A = self.logic_and(self._regs.A, value);
    return 0;
}

int Mos6502::i_SRE(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SRE - Shift right memory then EOR with Accumulator
     * alias: LSE
     *
     *  SRE ($00, X)    8 cycles
     *  SRE $00         5 cycles
     *  SRE $0000       6 cycles
     *  SRE ($00), Y    8 cycles
     *  SRE $00, X      6 cycles
     *  SRE $0000, Y    7 cycles
     *  SRE $0000, X    7 cycles
     *
     * - Get value from memory;
     * - Shift right one bit;
     * - Store value back on memory, previous bit 1 moved to C;
     * - OR the shifted value with the Accumulator.
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    value = self.logic_shr(value);
    self.write(addr, value);
    self._regs.A = self.logic_eor(self._regs.A, value);
    return 0;
}

int Mos6502::i_RRA(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: RRA - Rotate right memory then add with carry Accumulator.
     *
     *  RRA ($00, X)    8 cycles
     *  RRA $00         5 cycles
     *  RRA $0000       6 cycles
     *  RRA ($00), Y    8 cycles
     *  RRA $00, X      6 cycles
     *  RRA $0000, Y    7 cycles
     *  RRA $0000, X    7 cycles
     *
     * - Get value from memory;
     * - Rotate right one bit (carry shifted to bit 7 and bit 0 moved to carry);
     * - Store value back on memory;
     * - ADC the new value with the Accumulator.
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    value = self.logic_ror(value);
    self.write(addr, value);
    self._regs.A = self.adc(self._regs.A, value);
    return 0;
}

int Mos6502::i_SAX(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SAX - Store A AND X.
     *
     *  SAX ($00, X)    6 cycles
     *  SAX $00         3 cycles
     *  SAX $0000       4 cycles
     *  SAX $00, Y      4 cycles
     *
     *  *addr = A & X
     *
     * Flags: -
     */
    uint8_t value = self._regs.A & self._regs.X;
    self.write(addr, value);
    return 0;
}

int Mos6502::i_LAX_imm(Mos6502& self, addr_t value)
{
    /*
     * Illegal Instruction: LAX - Load A and X with same value.
     *
     *  LAX #$00    2 cycles
     *
     * A = X = imm
     *
     * Flags: N Z
     *
     * Unstable, does not work on some machines.
     */
    self._regs.A = self._regs.X = static_cast<uint8_t>(value);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_LAX(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: LAX - Load A and X with same value.
     *
     *  LAX ($00, X)    6 cycles
     *  LAX $00         3 cycles
     *  LAX $0000       4 cycles
     *  LAX ($00), Y    5 cycles + 1 (page boundary crossed)
     *  LAX $00, Y      4 cycles
     *  LAX $0000, Y    4 cycles + 1 (page boundary crossed)
     *
     * A = X = *addr
     *
     * Flags: N Z
     */
    addr_t value = self.read(addr);
    i_LAX_imm(self, value);
    return 0;
}

int Mos6502::i_DCP(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: DCP - Decrement memory then compare.
     * alias: DCM
     *
     *  DCP ($00, X)    8 cycles
     *  DCP $00         5 cycles
     *  DCP $0000       6 cycles
     *  DCP ($00), Y    8 cycles
     *  DCP $00, X      6 cycles
     *  DCP $0000, Y    7 cycles
     *  DCP $0000, X    7 cycles
     *
     * --(*addr)
     * CMP(A, *addr)
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    --value;
    self.write(addr, value);
    self.cmp(self._regs.A, value);
    return 0;
}

int Mos6502::i_ISC(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: ISC - Increment memory then subtract accumulator.
     * alias: INS
     * alias: ISB
     *
     *  ISC ($00, X)    8 cycles
     *  ISC $00         5 cycles
     *  ISC $0000       6 cycles
     *  ISC ($00), Y    8 cycles
     *  ISC $00, X      6 cycles
     *  ISC $0000, X    7 cycles
     *  ISC $0000, Y    7 cycles
     *
     * ++(*addr)
     * A = A - *addr
     *
     * Flags: N V Z C
     *
     * See https://sourceforge.net/p/vice-emu/code/HEAD/tree/testprogs/decimalmode.c
     * See "65xx Processor Data" by Mark Ormston
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    ++value;
    self.write(addr, value);
    self._regs.A = self.sbc(self._regs.A, value);
    return 0;
}

int Mos6502::i_ANC_imm(Mos6502& self, addr_t value)
{
    /*
     * Illegal Instruction: ANC - AND immediate value and put bit 7 on carry.
     * alias: AAC
     *
     *  ANC #$00    2 cycles
     *
     * A = A & value
     * C = A & 0x80
     *
     * "This instruction affects the accumulator;
     * sets the zero flag if the result in the accumulator is 0, otherwise resets the zero flag;
     * sets the negative flag and the carry flag if the result in the accumulator has bit 7 on,
     * otherwise resets the negative flag and the carry flag."
     *
     * Flags: N Z C
     */
    i_AND_imm(self, value);
    self.flag_C(self._regs.A & 0x80);
    return 0;
}

int Mos6502::i_ALR_imm(Mos6502& self, addr_t value)
{
    /*
     * Illegal Instruction: ALR - AND immediate value then shift right.
     * alias: ASR
     *
     *  ALR #$00    2 cycles
     *
     * A = A & value
     * C = A & 1
     * A >>= 1
     *
     * Flags: N Z C
     */
    self.i_AND_imm(self,  value);
    return self.i_LSR_acc(self, 0);
}

int Mos6502::i_ARR_imm(Mos6502& self, addr_t value)
{
    /*
     * Illegal Instruction: ARR - AND immediate value then rotate right.
     *
     *  ARR #$00    2 cycles
     *
     * value = A & value
     * A = (value >> 1) | (C * 0x80)
     * C = value & 1
     *
     * Flags: N V Z C
     *
     * See https://www.pagetable.com/c64ref/6502/?tab=2#ARR
     * "The V and C flags depends on the Decimal Mode Flag:
     * In decimal mode, the V flag is set if bit 6 is different than
     * the original data's bit 6, otherwise the V flag is reset.
     * The C flag is set if (operand & 0xF0) + (operand & 0x10) is
     * greater than 0x50, otherwise the C flag is reset.
     * In binary mode, the V flag is set if bit 6 of the result is
     * different than bit 5 of the result, otherwise the V flag is reset.
     * The C flag is set if the result in the accumulator has bit 6 on, otherwise it is reset."
     *
     * See 65xx Processor Data, by Mark Ormston (aka Zolaerla or MeMSO)
     * If the CPU is the Ricoh-2A03 (Nintendo) then the binary mode is implemented no matter the value of D.
     */
    /*
     * The code below comes from "65xx Processor Data" from Mark Ormston.
     */
    uint8_t op = self._regs.A & static_cast<uint8_t>(value & 0xFF);
    uint8_t r  = (op >> 1) | (self.test_C() ? 0x80 : 0x00);

    self.flag_N(self.test_C());
    self.set_Z(r);

//XXX
#if 1   /* CPU != 2A03 */
    self.flag_V((r ^ op) & 0x40);
    if (((op & 0x0F) + (op & 0x01)) > 0x05) {
        r = (r & 0xF0) | ((r + 0x06) & 0x0F);
    }

    if (((op & 0xF0) + (op & 0x10)) > 0x50) {
        r = (r & 0x0F) | ((r + 0x60) & 0xF0);
        self.flag_C(1);
    } else {
        self.flag_C(0);
    }
#else
    self.flag_C(r & 0x40);
    self.flag_V(((r >> 6) ^ (r >> 5)) & 1);
#endif

    self._regs.A = static_cast<uint8_t>(r & 0xFF);
    return 0;
}

int Mos6502::i_XAA_imm(Mos6502& self, addr_t value)
{
    /*
     * Illegal Instruction: XAA - X AND immediate value then move into A.
     * alias: ANE
     * alias: AXA
     *
     *  XAA #$00    2 cycles
     *
     * A = X & imm
     *
     * Unstable, does not work on some machines.
     *
     * Flags: N Z
     *
     * See https://www.pagetable.com/c64ref/6502/?tab=2#SBX
     * "The operation of the undocumented XAA instruction depends on the individual microprocessor.
     * On most machines, it performs a bit-by-bit AND operation of the following three operands:
     * The first two are the index register X and memory.
     * The third operand is the result of a bit-by-bit AND operation of the accumulator and a magic component.
     * This magic component depends on the individual microprocessor and is usually one of $00, $EE, $EF, $FE
     * and $FF, and may be influenced by the RDY pin, leftover contents of the data bus, the temperature of
     * the microprocessor, the supplied voltage, and other factors.
     * On some machines, additional bits of the result may be set or reset depending on non-deterministic
     * factors.  It then transfers the result to the accumulator.
     * XAA does not affect the C or V flags; sets Z if the value loaded was zero, otherwise resets it;
     * sets N if the result in bit 7 is a 1; otherwise N is reset."
     */
    return i_LDA_imm(self, self._regs.X & value);
}

int Mos6502::i_SBX_imm(Mos6502& self, addr_t value)
{
    /*
     * Illegal Instruction: SBX - A AND X then substract immediate, move result into X.
     * alias: AXS
     * alias: ASX
     * alias: SAX
     *
     *  SBX #$00    2 cycles
     *
     * X = (A & X) - imm
     *
     * Flags: N Z C
     *
     * See https://www.pagetable.com/c64ref/6502/?tab=2#SBX
     * "This undocumented instruction performs a bit-by-bit "AND" of the value of the accumulator
     * and the index register X and subtracts the value of memory from this result, using two's
     * complement arithmetic, and stores the result in the index register X.
     * This instruction affects the index register X.
     * The carry flag is set if the result is greater than or equal to 0.
     * The carry flag is reset when the result is less than 0, indicating a borrow.
     * The negative flag is set if the result in index register X has bit 7 on, otherwise it is reset.
     * The Z flag is set if the result in index register X is 0, otherwise it is reset."
     */
    int result = static_cast<int>(self._regs.A & self._regs.X) - static_cast<int>(value);
    self._regs.X = static_cast<uint8_t>(result & 0xFF);
    self.flag_C(result >= 0);
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}

int Mos6502::i_SHA_zp(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SHA
     * alias: SAH
     * alias: AXA
     *
     *  SHA ($00), Y     6 cycles
     *
     *  v = *(addr + 1) & A & X
     *  a = *((u16 *)(addr)) + Y
     * *a = v
     *
     * Flags: -
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = self.read(static_cast<uint8_t>(addr + 1));
    value &= self._regs.A & self._regs.X;
    addr = self.read_addr(addr) + self._regs.Y;
    self.write(addr, value);
    return 0;
}

int Mos6502::i_SHA(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SHA
     * alias: SAH
     * alias: AXA
     *
     *  SHA $0000, Y    6 cycles
     *
     * *(addr + Y) = A & X & ((addr >> 8) + 1)
     *
     * Flags: -
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = static_cast<uint8_t>((addr >> 8) + 1) & self._regs.A & self._regs.X;
    self.write(addr + self._regs.Y, value);
    return 0;
}

int Mos6502::i_SHY(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SHY
     *
     *  SHY $0000, X
     *
     * *(addr + X) = Y & ((addr >> 8) + 1)
     *
     * Flags: -
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = static_cast<uint8_t>((addr >> 8) + 1) & self._regs.Y;
    self.write(addr + self._regs.X, value);
    return 0;
}

int Mos6502::i_SHX(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SHX
     * alias: SXA
     * alias: SXH
     * alias: XAS
     *
     *  SHX $0000, Y
     *
     * *(addr + Y) = X & ((addr >> 8) + 1)
     *
     * Flags: -
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = static_cast<uint8_t>((addr >> 8) + 1) & self._regs.X;
    self.write(addr + self._regs.Y, value);
    return 0;
}

int Mos6502::i_SHS(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: SHS
     * alias: SSH
     * alias: TAS
     * alias: XAS
     *
     *  SHS $0000, Y    5 cycles
     *
     * S = A & X
     * *(addr + Y) = S & ((addr >> 8) + 1)
     *
     * Flags: -
     *
     * Unstable, does not work on some machines.
     */
    self._regs.S = self._regs.A & self._regs.X;
    uint8_t value = static_cast<uint8_t>((addr >> 8) + 1) & self._regs.S;
    self.write(addr + self._regs.Y, value);
    return 0;
}

int Mos6502::i_LAS(Mos6502& self, addr_t addr)
{
    /*
     * Illegal Instruction: LAS
     * alias: LAE
     * alias: LAR
     * alias: AST
     *
     *  LAS $0000, Y        4 + 1 (page boundary crossed)
     *
     * A = X = S = (*addr & S)
     *
     * Flags: N Z
     */
    uint8_t value = self.read(addr);
    self._regs.S &= value;
    self._regs.A = self._regs.X = self._regs.S;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
    return 0;
}

int Mos6502::i_KIL(Mos6502& self, addr_t addr)
{
    /*
     * Illegal instruction: KIL - Do nothing until RESET
     * alias: HLT
     * alias: JAM
     * alias: CIM
     */
    self._halted = true;
    log.debug("KIL instruction at ${}, CPU halted\n", caio::to_string(self._regs.PC));
    return 0;
}

}
