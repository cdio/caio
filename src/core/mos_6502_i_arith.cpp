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

int Mos6502::i_AND_imm(Mos6502& self, addr_t value)
{
    /*
     * AND accumulator
     * AND #$00         - 29
     */
    self._regs.A = self.logic_and(self._regs.A, value);
    return 0;
}

int Mos6502::i_AND(Mos6502& self, addr_t addr)
{
    /*
     * AND accumulator with memory
     * AND ($00, X)     - 21
     * AND $00          - 25
     * AND $0000        - 2D
     * AND ($00), Y     - 31
     * AND $00, X       - 35
     * AND $0000, Y     - 39
     * AND $0000, X     - 3D
     */
    uint8_t value = self.read(addr);
    return i_AND_imm(self, value);
}

int Mos6502::i_BIT(Mos6502& self, addr_t addr)
{
    /*
     * AND accumulator without store (modify flags only)
     * BIT $00          - 24 - 3 cycles
     * BIT $0000        - 2C - 4 cycles
     * Flags: N V Z
     */
    uint8_t value = self.read(addr);
    self.set_N(value);
    self.flag_V(value & 0x40);
    value &= self._regs.A;
    self.set_Z(value);
    return 0;
}

int Mos6502::i_ORA_imm(Mos6502& self, addr_t value)
{
    /*
     * OR accumulator
     * ORA #$00         - 09
     */
    self._regs.A = self.logic_or(self._regs.A, value);
    return 0;
}

int Mos6502::i_ORA(Mos6502& self, addr_t addr)
{
    /*
     * OR accumulator with memory
     * ORA ($00, X)     - 01
     * ORA $00          - 05
     * ORA $0000        - 0D
     * ORA ($00), Y     - 11
     * ORA $00, X       - 15
     * ORA $0000, Y     - 19
     * ORA $0000, X     - 1D
     */
    uint8_t value = self.read(addr);
    return i_ORA_imm(self, value);
}

int Mos6502::i_EOR_imm(Mos6502& self, addr_t value)
{
    /*
     * XOR accumulator
     * EOR #$00         - 49
     */
    self._regs.A = self.logic_eor(self._regs.A, value);
    return 0;
}

int Mos6502::i_EOR(Mos6502& self, addr_t addr)
{
    /*
     * XOR accumulator with memory
     * EOR ($00, X)     - 41
     * EOR $00          - 45
     * EOR $0000        - 4D
     * EOR ($00, Y)     - 51
     * EOR $0000, X     - 55
     * EOR $0000, Y     - 59
     * EOR $00, X       - 5D
     */
    uint8_t value = self.read(addr);
    return i_EOR_imm(self, value);
}

uint8_t Mos6502::adc_bin(uint8_t v1, uint8_t v2)
{
    unsigned r = v1 + v2 + test_C();
    bool s1 = v1 & 0x80;
    bool s2 = v2 & 0x80;
    bool sr = r & 0x80;
    flag_V((s1 && s2 && !sr) || (!s1 && !s2 && sr));
    flag_C(r & 0x100);
    set_N(r);
    set_Z(r & 0xFF);
    return (r & 0xFF);
}

uint8_t Mos6502::adc_bcd(uint8_t v1, uint8_t v2)
{
    /*
     * See "65xx Processor Data" by Mark Ormston
     */
    unsigned r = (v1 & 0x0F) + (v2 & 0x0F) + test_C();
    if (r > 0x09) {
        r += 0x06;
    }

    r = (r & 0x0F) + (v1 & 0xF0) + (v2 & 0xF0) + (r <= 0x0F ? 0 : 0x10);

    set_Z((v1 + v2 + test_C()) & 0xFF);
    flag_N(r & 0x80);
    flag_V(((v1 ^ r) & 0x80) && !((v1 ^ v2) & 0x80));

    if ((r & 0x1F0) > 0x90) {
        r += 0x60;
    }

    flag_C((r & 0x0FF0) > 0xF0);

    return (r & 0xFF);
}

uint8_t Mos6502::adc(uint8_t v1, uint8_t v2)
{
    return (decimal_mode() ? adc_bcd(v1, v2) : adc_bin(v1, v2));
}

int Mos6502::i_ADC_imm(Mos6502& self, addr_t value)
{
    /*
     * ADD accumulator with carry
     * ADC #$00         - 69
     */
    self._regs.A = self.adc(self._regs.A, value);
    return 0;
}

int Mos6502::i_ADC(Mos6502& self, addr_t addr)
{
    /*
     * ADD accumultaor and memory with carry
     * ADC ($00, X)     - 61
     * ADC $00          - 65
     * ADC $0000        - 6D
     * ADC ($00), Y     - 71
     * ADC $00, X       - 75
     * ADC $0000, Y     - 79
     * ADC $0000, X     - 7D
     */
    uint8_t value = self.read(addr);
    return i_ADC_imm(self, value);
}

uint8_t Mos6502::sbc_bin(uint8_t v1, uint8_t v2)
{
    uint16_t b = (test_C() ? 0x0000 : 0xFFFF);
    uint16_t r = v1 - v2 + b;
    bool s1 = v1 & 0x80;
    bool s2 = v2 & 0x80;
    bool sr = r & 0x80;
    flag_V((s1 && !s2 && !sr) || (!s1 && s2 && sr));
    flag_C(!(r & 0x100));
    set_N(r);
    set_Z(r);
    return (r & 255);
}

uint8_t Mos6502::sbc_bcd(uint8_t v1, uint8_t v2)
{
    /*
     * See "65xx Processor Data" by Mark Ormston
     */
    unsigned t  = v1 - v2 - (test_C() ^ 1);
    unsigned t2 = (v1 & 0x0F) - (v2 & 0x0F) - (test_C() ^ 1);

    flag_C(t < 0x100);
    set_N(t);
    set_Z(t & 0xFF);
    flag_V(((v1 ^ t) & 0x80) && ((v1 ^ v2) & 0x80));

    if (t2 & 0x10) {
        t2 = ((t2 - 6) & 0x0F) | ((v1 & 0xF0) - (v2 & 0xF0) - 0x10);
    } else {
        t2 = (t2 & 0x0F) | ((v1 & 0xF0) - (v2 & 0xF0));
    }

    if (t2 & 0x100) {
        t2 -= 0x60;
    }

    return (t2 & 0xFF);
}

uint8_t Mos6502::sbc(uint8_t v1, uint8_t v2)
{
    return (decimal_mode() ? sbc_bcd(v1, v2) : sbc_bin(v1, v2));
}

int Mos6502::i_SBC_imm(Mos6502& self, addr_t value)
{
    /*
     * Substract accumulator with borrow
     * SBC #$00         - E9
     * SBC #$00         - EB (undocumented or invalid instruction)
     */
    self._regs.A = self.sbc(self._regs.A, value);
    return 0;
}

int Mos6502::i_SBC(Mos6502& self, addr_t addr)
{
    /*
     * Substract accumulator and memory with borrow
     * SBC ($00, X)     - E1
     * SBC $00          - E5
     * SBC $0000        - ED
     * SBC ($00), Y     - F1
     * SBC $00, X       - F5
     * SBC $0000, Y     - F9
     * SBC $0000, X     - FD
     */
    uint8_t value = self.read(addr);
    return i_SBC_imm(self, value);
}

int Mos6502::i_CMP_imm(Mos6502& self, addr_t value)
{
    /*
     * Compare accumulator and value
     * CMP #$00         - C9
     */
    self.cmp(self._regs.A, value);
    return 0;
}

int Mos6502::i_CMP(Mos6502& self, addr_t addr)
{
    /*
     * Compare accumulator and memory
     * CPM ($00, X)     - C1
     * CMP $00          - C5
     * CMP $0000        - CD
     * CMP ($00, Y)     - D1
     * CMP $00, X       - D5
     * CMP $0000, Y     - D9
     * CMP $0000, X     - DD
     */
    uint8_t value = self.read(addr);
    return i_CMP_imm(self, value);
}

int Mos6502::i_CPX_imm(Mos6502& self, addr_t value)
{
    /*
     * Compary X and value
     * CPX #$00         - E0
     */
    self.cmp(self._regs.X, value);
    return 0;
}

int Mos6502::i_CPX(Mos6502& self, addr_t addr)
{
    /*
     * Compare X and memory
     * CPX $00          - E4
     * CPX $0000        - EC
     */
    uint8_t value = self.read(addr);
    return i_CPX_imm(self, value);
}

int Mos6502::i_CPY_imm(Mos6502& self, addr_t value)
{
    /*
     * Compary Y and value
     * CPY #$00         - C0
     */
    self.cmp(self._regs.Y, value);
    return 0;
}

int Mos6502::i_CPY(Mos6502& self, addr_t addr)
{
    /*
     * Compary Y and memory
     * CPY $00          - C4
     * CPY $0000        - CC
     */
    uint8_t value = self.read(addr);
    return i_CPY_imm(self, value);
}

int Mos6502::i_DEC(Mos6502& self, addr_t addr)
{
    /*
     * Decrement memory
     * DEC $00          - C6
     * DEC $0000        - CE
     * DEC $00, X       - D6
     * DEC $0000, X     - DE
     * Flags: N Z
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    --value;
    self.set_N(value);
    self.set_Z(value);
    self.write(addr, value);
    return 0;
}

int Mos6502::i_DEX(Mos6502& self, addr_t)
{
    /*
     * Decrement X
     * DEX              - CA
     * Flags: N Z
     */
    --self._regs.X;
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}

int Mos6502::i_DEY(Mos6502& self, addr_t)
{
    /*
     * Decrement Y
     * DEY              - 88
     * Flags: N Z
     */
    --self._regs.Y;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}

int Mos6502::i_INC(Mos6502& self, addr_t addr)
{
    /*
     * Increment memory
     * INC $00          - E6
     * INC $0000        - EE
     * INC $00, X       - F6
     * INC $0000, X     - FE
     * Flags: N Z
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    ++value;
    self.write(addr, value);
    self.set_N(value);
    self.set_Z(value);
    return 0;
}

int Mos6502::i_INX(Mos6502& self, addr_t)
{
    /*
     * Increment X
     * INX              - E8
     * Flags: N Z
     */
    ++self._regs.X;
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}

int Mos6502::i_INY(Mos6502& self, addr_t _)
{
    /*
     * Increment Y
     * INY              - C8
     * Flags: N Z
     */
    ++self._regs.Y;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}

int Mos6502::i_ASL_acc(Mos6502& self, addr_t)
{
    /*
     * Shift left accumulator 1 bit
     * ASL              - 0A
     * Flags: N Z C
     */
    self._regs.A = self.logic_shl(self._regs.A);
    return 0;
}

int Mos6502::i_ASL(Mos6502& self, addr_t addr)
{
    /*
     * Shift left memory 1 bit
     * ASL $00          - 06
     * ASL $0000        - 0E
     * ASL $00, X       - 16
     * ASL $0000, X     - 1E
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_shl(value);
    self.write(addr, value);
    return 0;
}

int Mos6502::i_ROL_acc(Mos6502& self, addr_t)
{
    /*
     * Rotate left accumulator 1 bit
     * ROL              - 2A
     * Flags: N Z C
     */
    self._regs.A = self.logic_rol(self._regs.A);
    return 0;
}

int Mos6502::i_ROL(Mos6502& self, addr_t addr)
{
    /*
     * Rotate left memory 1 bit
     * ROL $00          - 26
     * ROL $0000        - 2E
     * ROL $00, X       - 36
     * ROL $0000, X     - 3E
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_rol(value);
    self.write(addr, value);
    return 0;
}

int Mos6502::i_LSR_acc(Mos6502& self, addr_t)
{
    /*
     * Shift right accumulator 1 bit
     * LSR              - 4A
     * Flags: N Z C
     */
    self._regs.A = self.logic_shr(self._regs.A);
    return 0;
}

int Mos6502::i_LSR(Mos6502& self, addr_t addr)
{
    /*
     * Shift right memory 1 bit
     * LSR $00          - 46
     * LSR $0000        - 4E
     * LSR $00, X       - 56
     * LSR $0000, X     - 5E
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_shr(value);
    self.write(addr, value);
    return 0;
}

int Mos6502::i_ROR_acc(Mos6502& self, addr_t)
{
    /*
     * Rotate right accumulator 1 bit
     * ROR              - 6A
     * Flags: N Z C
     */
    self._regs.A = self.logic_ror(self._regs.A);
    return 0;
}

int Mos6502::i_ROR(Mos6502& self, addr_t addr)
{
    /*
     * Rotate memory right 1 bit
     * ROR $00          - 66
     * ROR $0000        - 6E
     * ROR $00, X       - 76
     * ROR $0000, X     - 7E
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    /* Read-Write-Modify instruction */
    value = self.logic_ror(value);
    self.write(addr, value);
    return 0;
}

}
