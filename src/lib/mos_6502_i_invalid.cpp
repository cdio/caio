/*
 * Copyright (C) 2020-2022 Claudio Castiglia
 *
 * This file is part of CAIO.
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

void Mos6502::i_SLO(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: SLO - Shift Left memory the OR with Accumulator
     *
     *  SLO $00
     *  SLO $00, X
     *  SLO $00, Y
     *  SLO $0000
     *  SLO $0000, X
     *  SLO $0000, Y
     *  SLO ($00, X)
     *  SLO ($00), Y
     *
     * - Get value from memory;
     * - Shift left one bit;
     * - Store value back on memory;
     * - OR the shifted value with the Accumulator.
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    value  = self.logic_shl(value);
    self.write(addr, value);
    self._regs.A = self.logic_or(self._regs.A, value);
}

void Mos6502::i_RLA(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: RLA - Rotate left memory then AND with Accumulator
     *
     *  RLA $00
     *  RLA $00, X
     *  RLA $00, Y
     *  RLA $0000
     *  RLA $0000, X
     *  RLA $0000, Y
     *  RLA ($00, X)
     *  RLA ($00), Y
     *
     * - Get value from memory;
     * - Rotate left one bit;
     * - Store value back on memory;
     * - AND the rotated value with the Accumulator.
     *
     * Flags: N Z C
     *
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    value = self.logic_rol(value);
    self.write(addr, value);
    self._regs.A = self.logic_and(self._regs.A, value);
}

void Mos6502::i_SRE(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: SRE - Shift right memory then EOR with Accumulator
     *
     *  SRE $00
     *  SRE $00, X
     *  SRE $00, Y
     *  SRE $0000
     *  SRE $0000, X
     *  SRE $0000, Y
     *  SRE ($00, X)
     *  SRE ($00), Y
     *
     * - Get value from memory;
     * - Shift right one bit;
     * - Store value back on memory;
     * - OR the shifted value with the Accumulator.
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    value  = self.logic_shl(value);
    self.write(addr, value);
    self._regs.A = self.logic_eor(self._regs.A, value);
}

void Mos6502::i_RRA(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: RRA - Rotate right memory then add with carry Accumulator.
     *
     *  RRA $00
     *  RRA $00, X
     *  RRA $00, Y
     *  RRA $0000
     *  RRA $0000, X
     *  RRA $0000, Y
     *  RRA ($00, X)
     *  RRA ($00), Y
     *
     * - Get value from memory;
     * - Rotate right one bit;
     * - Store value back on memory;
     * - ADC the shifted value with the Accumulator.
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    value  = self.logic_ror(value);
    self.write(addr, value);
    self._regs.A = self.adc(self._regs.A, value);
}

void Mos6502::i_SAX(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: SAX - Store A AND X.
     *
     *  SAX $00
     *  SAX $00, Y
     *  SAX $0000
     *  SAX ($00, X)
     *
     *  *addr = A & X
     *
     * Flags: -
     */
    uint8_t value = self._regs.A & self._regs.X;
    self.write(addr, value);
}

void Mos6502::i_LAX_imm(Mos6502 &self, addr_t value)
{
    /*
     * Illegal Instruction: LAX - Load A and X with same value.
     *
     * A = X = imm
     *
     * Flags: N Z
     *
     * Unstable, does not work on some machines.
     *
     * 2 cycles
     */
    self._regs.A = self._regs.X = static_cast<uint8_t>(value);
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
}

void Mos6502::i_LAX(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: LAX - Load A and X with same value.
     *
     * A = X = *addr
     *
     * Flags: N Z
     */
    addr_t value = self.read(addr);
    i_LAX_imm(self, value);
}

void Mos6502::i_DCP(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: DCP - Decrement memory then compare.
     *
     * --(*addr)
     * CMP(A, *addr)
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    --value;
    self.write(addr, value);
    self.cmp(self._regs.A, value);
}

void Mos6502::i_ISC(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: ISC - Increment memory then subtract accumulator.
     *
     * ++(*addr)
     * A = A - *addr
     *
     * Flags: N V Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    ++value;
    self.write(addr, value);
    //XXX self.flag_C(true);
    self._regs.A = self.sbc(self._regs.A, value);
}

void Mos6502::i_ANC_imm(Mos6502 &self, addr_t value)
{
    /*
     * Illegal Instruction: ANC - AND immediate value put bit 7 on carry (?).
     *
     * Equivalent:
     *      AND #i8
     *      ASL
     *
     * This command performs an AND operation only, but bit 7 is put into the carry,
     * as if the ASL/ROL would have been executed (source: http://www.oxyron.de/html/opcodes02.html).
     *
     * Flags: N Z C
     *
     * 2 cycles
     */
    i_AND_imm(self, value);
    self.flag_C(self._regs.A & 0x80);
}

void Mos6502::i_ALR_imm(Mos6502 &self, addr_t value)
{
    /*
     * Illegal Instruction: ALR - AND immediate value then shift right.
     *
     * Equivalent:
     *      AND #$00
     *      LSR
     *
     * Flags: N Z C
     *
     * 2 cycles
     */
    self.i_AND_imm(self,  value);
    self.i_LSR(self, 0);
}

void Mos6502::i_ARR_imm(Mos6502 &self, addr_t value)
{
    /*
     * Illegal Instruction: ARR - AND immediate value then rotate right.
     *
     * Equivalent:
     *      AND #$00
     *      ROR
     *
     * 2 cycles
     */
    self.i_AND_imm(self,  value);
    self.i_ROR(self, 0);
}

void Mos6502::i_XAA_imm(Mos6502 &self, addr_t value)
{
    /*
     * Illegal Instruction: XAA - X AND immediate value then move into A.
     *
     * A = X & imm
     *
     * Unstable, does not work on some machines.
     *
     * Flags: N Z
     *
     * 2 cycles
     */
    i_LDA_imm(self, self._regs.X & value);
}

void Mos6502::i_AXS_imm(Mos6502 &self, addr_t value)
{
    /*
     * Illegal Instruction: AXS - A AND X then substract immediate, move result into X.
     *
     * X = (A & X) - imm
     *
     * Flags: N Z C
     *
     * 2 cycles
     *
     * "performs CMP and DEX at the same time, so that the MINUS sets the flag like CMP, not SBC."
     * Source: http://www.oxyron.de/html/opcodes02.html
     */
    uint8_t r = self.logic_and(self._regs.A, self._regs.X);
    self.cmp(r, static_cast<uint8_t>(value));
    self._regs.X = r - static_cast<uint8_t>(value);
}

void Mos6502::i_AHX(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: AXH
     *
     * *addr = A & X & HI(addr)
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = (addr >> 8) & self._regs.A & self._regs.X;
    self.write(addr, value);
}

void Mos6502::i_SHY(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: SHY
     *
     * *addr = Y & HI(addr)
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = (addr >> 8) & self._regs.Y;
    self.write(addr, value);
}

void Mos6502::i_SHX(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: SHX
     *
     * *addr = X & HI(addr)
     *
     * Unstable, does not work on some machines.
     */
    uint8_t value = (addr >> 8) & self._regs.X;
    self.write(addr, value);
}

void Mos6502::i_TAS(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: TAS
     *
     * TAS $0000, Y
     *
     * S = A & X
     * *addr = A & X & HI(addr)
     *
     * Unstable, does not work on some machines.
     *
     * 5 cycles
     */
    self._regs.S = self._regs.A & self._regs.X;
    uint8_t value = self._regs.S & (addr >> 8);
    self.write(addr, value);
}

void Mos6502::i_LAS(Mos6502 &self, addr_t addr)
{
    /*
     * Illegal Instruction: LAS
     *
     * LAS $0000, Y     - Invalid Command
     *
     * A = X = S = (*addr & S)
     *
     * Flags: N Z
     *
     * 4 cycles (5 if page boundary is crossed)
     */
    uint8_t value = self.read(addr);
    self._regs.S &= value;
    self._regs.A = self._regs.X = self._regs.S;
    self.set_N(self._regs.A);
    self.set_Z(self._regs.A);
}

void Mos6502::i_KIL(Mos6502 &self, addr_t addr)
{
    log.debug("KIL instruction at $" + utils::to_string(self._regs.PC) + "\n");
}

}
