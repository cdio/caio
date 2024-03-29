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

/********************************************************************************
 * AND
 ********************************************************************************/
int Mos6502::i_AND_imm(Mos6502& self, addr_t value)
{
    /*
     * AND #$00
     */
    self._regs.A = self.logic_and(self._regs.A, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_AND(Mos6502& self, addr_t addr)
{
    /*
     * AND $00
     * AND $00, X
     * AND $0000
     * AND $0000, Y
     * AND $0000, X
     * AND ($00, X)
     * AND ($00), Y
     */
    uint8_t value = self.read(addr);
    return i_AND_imm(self, value);
}


/********************************************************************************
 * BIT (AND Accumulator without store. Modify flags only)
 ********************************************************************************/
int Mos6502::i_BIT(Mos6502& self, addr_t addr)
{
    /*
     * BIT $00
     * BIT $0000
     *
     * Flags: N V Z
     *
     * 3 cycles
     */
    uint8_t value = self.read(addr);
    self.set_N(value);
    self.flag_V(value & 0x40);
    value &= self._regs.A;
    self.set_Z(value);
    return 0;
}


/********************************************************************************
 * ORA
 ********************************************************************************/
int Mos6502::i_ORA_imm(Mos6502& self, addr_t value)
{
    /*
     * ORA #$00
     */
    self._regs.A = self.logic_or(self._regs.A, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_ORA(Mos6502& self, addr_t addr)
{
    /*
     * ORA $00
     * ORA $00, X
     * ORA $0000
     * ORA $0000, X
     * ORA $0000, Y
     * ORA ($00, X)
     * ORA ($00), Y
     */
    uint8_t value = self.read(addr);
    return i_ORA_imm(self, value);
}


/********************************************************************************
 * EOR
 ********************************************************************************/
int Mos6502::i_EOR_imm(Mos6502& self, addr_t value)
{
    /*
     * EOR #$00
     */
    self._regs.A = self.logic_eor(self._regs.A, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_EOR(Mos6502& self, addr_t addr)
{
    /*
     * EOR $00
     * EOR $00, X
     * EOR $0000
     * EOR $0000, Y
     * EOR $0000, X
     * EOR ($00, X)
     * EOR ($00, Y)
     */
    uint8_t value = self.read(addr);
    return i_EOR_imm(self, value);
}


/********************************************************************************
 * ADC
 ********************************************************************************/
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
     * See https://sourceforge.net/p/vice-emu/code/HEAD/tree/testprogs/decimalmode.c
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
    return (test_D() ? adc_bcd(v1, v2) : adc_bin(v1, v2));
}

int Mos6502::i_ADC_imm(Mos6502& self, addr_t value)
{
    /*
     * ADC #$00
     */
    self._regs.A = self.adc(self._regs.A, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_ADC(Mos6502& self, addr_t addr)
{
    /*
     * ADC $00
     * ADC $00, X
     * ADC $0000
     * ADC $0000, X
     * ADC $0000, Y
     * ADC ($00, X)
     * ADC ($00), Y
     */
    uint8_t value = self.read(addr);
    return i_ADC_imm(self, value);
}


/********************************************************************************
 * SBC
 ********************************************************************************/
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
     * See https://sourceforge.net/p/vice-emu/code/HEAD/tree/testprogs/decimalmode.c
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
    return (test_D() ? sbc_bcd(v1, v2) : sbc_bin(v1, v2));
}

int Mos6502::i_SBC_imm(Mos6502& self, addr_t value)
{
    /*
     * SBC #$00
     */
    self._regs.A = self.sbc(self._regs.A, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_SBC(Mos6502& self, addr_t addr)
{
    /*
     * SBC $00
     * SBC $00, X
     * SBC $0000
     * SBC $0000, X
     * SBC $0000, Y
     * SBC ($00, X)
     * SBC ($00), Y
     */
    uint8_t value = self.read(addr);
    return i_SBC_imm(self, value);
}


/********************************************************************************
 * CMP
 ********************************************************************************/
int Mos6502::i_CMP_imm(Mos6502& self, addr_t value)
{
    /*
     * CMP #$00
     */
    self.cmp(self._regs.A, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_CMP(Mos6502& self, addr_t addr)
{
    /*
     * CMP $00
     * CMP $00, X
     * CMP $0000
     * CMP $0000, X
     * CMP $0000, Y
     * CPM ($00, X)
     * CPM ($00, Y)
     */
    uint8_t value = self.read(addr);
    return i_CMP_imm(self, value);
}


/********************************************************************************
 * CPX
 ********************************************************************************/
int Mos6502::i_CPX_imm(Mos6502& self, addr_t value)
{
    /*
     * CPX #$00
     */
    self.cmp(self._regs.X, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_CPX(Mos6502& self, addr_t addr)
{
    /*
     * CPX $00
     * CPX $0000
     */
    uint8_t value = self.read(addr);
    return i_CPX_imm(self, value);
}


/********************************************************************************
 * CPY
 ********************************************************************************/
int Mos6502::i_CPY_imm(Mos6502& self, addr_t value)
{
    /*
     * CPY #$00
     */
    self.cmp(self._regs.Y, static_cast<uint8_t>(value));
    return 0;
}

int Mos6502::i_CPY(Mos6502& self, addr_t addr)
{
    /*
     * CPY $00
     * CPY $0000
     */
    uint8_t value = self.read(addr);
    return i_CPY_imm(self, value);
}


/********************************************************************************
 * DEC
 ********************************************************************************/
int Mos6502::i_DEC(Mos6502& self, addr_t addr)
{
    /*
     * DEC $00
     * DEC $00, X
     * DEC $0000
     * DEC $0000, X
     *
     * Flags: N Z
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    --value;
    self.set_N(value);
    self.set_Z(value);
    self.write(addr, value);
    return 0;
}


/********************************************************************************
 * DEX
 ********************************************************************************/
int Mos6502::i_DEX(Mos6502& self, addr_t)
{
    /*
     * DEX
     *
     * Flags: N Z
     */
    --self._regs.X;
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}


/********************************************************************************
 * DEY
 ********************************************************************************/
int Mos6502::i_DEY(Mos6502& self, addr_t)
{
    /*
     * DEY
     *
     * Flags: N Z
     */
    --self._regs.Y;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}


/********************************************************************************
 * INC
 ********************************************************************************/
int Mos6502::i_INC(Mos6502& self, addr_t addr)
{
    /*
     * INC $00
     * INC $00, X
     * INC $0000
     * INC $0000, X
     *
     * Flags: N Z
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    ++value;
    self.write(addr, value);
    self.set_N(value);
    self.set_Z(value);
    return 0;
}


/********************************************************************************
 * INX
 ********************************************************************************/
int Mos6502::i_INX(Mos6502& self, addr_t)
{
    /*
     * INX
     *
     * Flags: N Z
     */
    ++self._regs.X;
    self.set_N(self._regs.X);
    self.set_Z(self._regs.X);
    return 0;
}


/********************************************************************************
 * INY
 ********************************************************************************/
int Mos6502::i_INY(Mos6502& self, addr_t _)
{
    /*
     * INY
     *
     * Flags: N Z
     */
    ++self._regs.Y;
    self.set_N(self._regs.Y);
    self.set_Z(self._regs.Y);
    return 0;
}


/********************************************************************************
 * ASL (shift left 1 bit)
 ********************************************************************************/
int Mos6502::i_ASL_acc(Mos6502& self, addr_t)
{
    /*
     * ASL
     *
     * Flags: N Z C
     */
    self._regs.A = self.logic_shl(self._regs.A);
    return 0;
}

int Mos6502::i_ASL(Mos6502& self, addr_t addr)
{
    /*
     * ASL $00
     * ASL $00, X
     * ASL $0000
     * ASL $0000, X
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    value = self.logic_shl(value);
    self.write(addr, value);
    return 0;
}


/********************************************************************************
 * ROL (rotate left 1 bit)
 ********************************************************************************/
int Mos6502::i_ROL_acc(Mos6502& self, addr_t)
{
    /*
     * ROL $00
     *
     * Flags: N Z C
     */
    self._regs.A = self.logic_rol(self._regs.A);
    return 0;
}

int Mos6502::i_ROL(Mos6502& self, addr_t addr)
{
    /*
     * ROL $00
     * ROL $00, X
     * ROL $0000
     * ROL $0000, X
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value); // Read-Write-Modify instruction.
    value = self.logic_rol(value);
    self.write(addr, value);
    return 0;
}


/********************************************************************************
 * LSR (shift right 1 bit)
 ********************************************************************************/
int Mos6502::i_LSR_acc(Mos6502& self, addr_t)
{
    /*
     * LSR
     *
     * Flags: N Z C
     */
    self._regs.A = self.logic_shr(self._regs.A);
    return 0;
}

int Mos6502::i_LSR(Mos6502& self, addr_t addr)
{
    /*
     * LSR $00
     * LSR $00, X
     * LSR $0000
     * LSR $0000, X
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    value = self.logic_shr(value);
    self.write(addr, value);
    return 0;
}


/********************************************************************************
 * ROR (rotate right 1 bit)
 ********************************************************************************/
int Mos6502::i_ROR_acc(Mos6502& self, addr_t)
{
    /*
     * ROR
     *
     * Flags: N Z C
     */
    self._regs.A = self.logic_ror(self._regs.A);
    return 0;
}

int Mos6502::i_ROR(Mos6502& self, addr_t addr)
{
    /*
     * ROR $00
     * ROR $00, X
     * ROR $0000
     * ROR $0000, X
     *
     * Flags: N Z C
     */
    uint8_t value = self.read(addr);
    self.write(addr, value);    // Read-Write-Modify instruction.
    value = self.logic_ror(value);
    self.write(addr, value);
    return 0;
}

}
