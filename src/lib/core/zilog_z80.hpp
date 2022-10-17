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
#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "aspace.hpp"
#include "clock.hpp"
#include "logger.hpp"
#include "monitor.hpp"
#include "pin.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {

/**
 * Zilog Z80 emulator.
 */
class ZilogZ80 : public Clockable, public Name {
public:
    constexpr static const char *TYPE    = "Z80";
    constexpr static const char *LABEL   = "CPU";

    constexpr static const addr_t vRESET = 0x0000;
    constexpr static const addr_t vNMI   = 0x0066;
    constexpr static const addr_t vIRQ   = 0x0038;  /* IMODE_1 */

    using breakpoint_cb_t = std::function<void(ZilogZ80 &, void *)>;

//XXX remove
#if 0
    enum class AddrMode {
        Immediate,
        ImmediateEx,
        PageZero,
        Relative,
        Extended,
        Indexed,
        Register,
        Implied,
        RegisterInd,
        None
    };
#endif

    enum class ArgType {
        None,
        A8,
        A16,
        Bit,
        IX,
        IXBit,
        IY,
        IYBit,
        MI,
        Inv
    };

    enum InterruptMode {
        IMODE_0,            /* Device puts 8-bit instruction on data bus                */
        IMODE_1,            /* ISR at $0038                                             */
        IMODE_2             /* Device provides LO 8-bit of interrupt vector table, bit 1 cleared XXX      */
    };

    enum Flags {
        S  = 0x80,          /* Sign             */
        Z  = 0x40,          /* Zero             */
        _1 = 0x20,          /* Unused           */
        H  = 0x10,          /* Half Carry       */
        _2 = 0x08,          /* Unused           */
        V  = 0x04,          /* Parity/Overflow  */
        N  = 0x02,          /* Add/Substract    */
        C  = 0x01           /* Carry            */
    };

    struct Instruction {
        const char *format;                     /* Mnemonic format string               */
        int (*fn)(ZilogZ80 &, uint8_t, addr_t); /* Instruction callback                 */
        ArgType type;                           /* Type of argument                     */
        size_t M_cycles;                        /* Consumed M cycles                    */
        size_t T_states;                        /* Consumed clock T states              */
        size_t size;                            /* Size of the instruction (in bytes)   */
    };

    struct Registers {
        union {
            uint8_t  A;
            uint8_t  F;
            uint16_t AF;
        };

        union {
            uint8_t  B;
            uint8_t  C;
            uint16_t BC;
        };

        union {
            uint8_t  D;
            uint8_t  E;
            uint16_t DE;
        };

        union {
            uint8_t  H;
            uint8_t  L;
            uint16_t HL;
        };

        union {
            uint8_t  aA;
            uint8_t  aF;
            uint16_t aAF;
        };

        union {
            uint8_t  aB;
            uint8_t  aC;
            uint16_t aBC;
        };

        union {
            uint8_t  aD;
            uint8_t  aE;
            uint16_t aDE;
        };

        union {
            uint8_t  aH;
            uint8_t  aL;
            uint16_t aHL;
        };

        uint8_t  I;             /* Interrupt vector */
        uint8_t  R;             /* Memory refresh   */
        uint16_t IX;            /* Index X          */
        uint16_t IY;            /* Index Y          */
        uint16_t SP;            /* Stack Pointer    */
        uint16_t PC;            /* Program Counter  */

        std::string to_string() const;

        static std::string to_string(Flags fl);
    };

    /**
     * Initialise this CPU.
     * @param type  CPU type;
     * @param label CPU label.
     */
    ZilogZ80(const std::string &type = TYPE, const std::string &label = LABEL);

    /**
     * Initialise this CPU.
     * @param mmap  System mappings;
     * @param type  CPU type;
     * @param label CPU label.
     * @see ASpace
     */
    ZilogZ80(const std::shared_ptr<ASpace> &mmap, const std::string &type = TYPE, const std::string &label = LABEL);

    virtual ~ZilogZ80();

    /**
     * Initialise a monitor for this CPU.
     * This CPU must be properly initialised (system mappings set) before this method can be called.
     * The CPU monitor is initialised and a breakpoint is added at the reset address (vRESET),
     * the monitor takes control as soon as this CPU is started.
     * @param is Input stream used to communicate with the user;
     * @param os Output stream used to communicate with the user.
     * @exception InvalidArgument if the system mappings are not set.
     * @see reset()
     */
    void init_monitor(std::istream &is, std::ostream &os);

    /**
     * Set the single-step log level.
     * @param lvs Log level string to set.
     * @see Logger::loglevel(const std::string &)
     */
    void loglevel(const std::string &lvs);

    /**
     * @return The current single-step log level.
     */
    Logger::Level loglevel() const;

    /**
     * Restart this CPU.
     */
    void reset();

    /**
     * Trigger an IRQ.
     * This method must be called twice by external peripherals, the first time to activate
     * the INT pin and a second time to de-activate it when the interrupt is served.
     * @param active true to generate an interrupt; false to de-activate a previous interrupt request.
     * @return The status of the INT pin after this call.
     */
    bool int_pin(bool active);

    /**
     * Trigger an NMI interrupt.
     * @param active true to generate an interrupt; false to de-activate.
     * @return The status of the NMI pin after this call.
     */
    bool nmi_pin(bool active);

    /**
     * Set the WAIT pin.
     * @param active true to activate the WAIT pin; false otherwise.
     * @return The status of the WAIT pin after this call.
     */
    bool wait_pin(bool active);

    /**
     * @return The RESET input pin of this CPU.
     * @param active true to activate the RESET pin; false otherwise.
     * @return The status of the RESET pin after this call.
     */
    bool reset_pin(bool active);

    /**
     * External breakpoint.
     * Force a return back to the monitor on the next clock tick,
     * if the monitor is not active a system halt is requested.
     */
    void ebreak();

    /**
     * Add a breakpoint on a memory address.
     * @param addr Address;
     * @parma cb   Method to call when the breakpoint hits.
     */
    void bpadd(addr_t addr, const breakpoint_cb_t &cb, void *arg);

    /**
     * Delete a breakpoint on a memory address.
     * @param addr Address.
     */
    void bpdel(addr_t addr);

    /**
     * @return The register values.
     */
    const Registers &regs() const;

    /**
     * Disassembler.
     * @param os      Output stream;
     * @param addr    Start address with the machine code to disassemble;
     * @param count   Number of instructions to disassemble.
     * @param show_pc true if the position of the PC must be marked in the disassembled code; false otherwise (default).
     */
    void disass(std::ostream &os, addr_t start, size_t count, bool show_pc = false);

    /**
     * Read an address from an address.
     * @param addr Address to read from.
     * @return The 16 bits value.
     * @exception InvalidReadAddress
     * @see read()
     */
    addr_t read_addr(size_t addr) const;

    /**
     * Write an address into an address.
     * @param addr  Address to write to;
     * @param value 16 bits value to write.
     * @exception InvalidWriteAddress
     * @see write()
     */
    void write_addr(addr_t addr, addr_t data);

    /**
     * @see ASpace::read()
     */
    virtual uint8_t read(addr_t addr) const;

    /**
     * @see ASpace::write()
     */
    virtual void write(addr_t addr, uint8_t data);

private:
    /**
     * Initialise this CPU.
     * @param mmap System mappings.
     */
    void init(const std::shared_ptr<ASpace> &mmap = {});

    /**
     * Tick event method.
     * This method is called by the clock and executes a single CPU instruction.
     * If the monitor is not running the current instruction is executed.
     * If the monitor is running and a monitor-breakpoint is set on the current PC address
     * the monitor's run method is called instead.
     * @param clk The caller clock.
     * @return The number of clock cycles consumed by the executed instruction;
     *         Clockable::HALT if the halt instruction was executed.
     * @see single_step()
     * @see ZilogZ80Monitor::run()
     */
    size_t tick(const Clock &clk) override;

    /**
     * Disassemble a single instruction located at a specified address.
     * @param addr    Address to disassemble (after disassembly this value is
     *                incremented with the position of the next instruction);
     * @param show_pc true if the position of the PC must be marked in the disassembled code;
     *                false otherwise.
     * @return A string with the disassembled instruction.
     */
    std::string disass(addr_t &addr, bool show_pc = false);

    /**
     * XXX FIXME
     * Execute a single instruction located at the current PC address.
     * @return The clock cycles that must pass until the clock is allowd to call
     * this method again (clock cycles consumed by the executed instruction);
     */
    size_t single_step();

    /**
     * XXX TODO
     */
    uint8_t in(uint8_t port) { return 0; } //XXX TODO
    void out(uint8_t port, uint8_t value) {} //XXX TODO

    void flag(uint8_t bits, bool act = true) {
        _regs.F = (act ? (_regs.F | bits) : (_regs.F & (~bits)));
    }

    void flag_S(bool act) {
        flag(Flags::S, act);
    }

    void flag_Z(bool act) {
        flag(Flags::Z, act);
    }

    void flag_H(bool act) {
        flag(Flags::H, act);
    }

    void flag_V(bool act) {
        flag(Flags::V, act);
    }

    void flag_N(bool act) {
        flag(Flags::N, act);
    }

    void flag_C(bool act) {
        flag(Flags::C, act);
    }

    bool test_flags(uint8_t bits) const {
        return (_regs.F & bits);
    }

    bool test_a_flags(uint8_t bits) const {
        return (_regs.aF & bits);
    }

    bool test_S() const {
        return test_flags(Flags::S);
    }

    bool test_Z() const {
        return test_flags(Flags::Z);
    }

    bool test_H() const {
        return test_flags(Flags::H);
    }

    bool test_V() const {
        return test_flags(Flags::V);
    }

    bool test_N() const {
        return test_flags(Flags::N);
    }

    bool test_C() const {
        return test_flags(Flags::C);
    }

    bool test_aS() const {
        return test_a_flags(Flags::S);
    }

    bool test_aZ() const {
        return test_a_flags(Flags::Z);
    }

    bool test_aH() const {
        return test_a_flags(Flags::H);
    }

    bool test_aV() const {
        return test_a_flags(Flags::V);
    }

    bool test_aN() const {
        return test_a_flags(Flags::N);
    }

    bool test_aC() const {
        return test_a_flags(Flags::C);
    }

    Logger                   _log{};
    std::unique_ptr<Monitor> _monitor{};
    bool                     _IFF1{};
    bool                     _IFF2{};
    bool                     _is_halted{};
    InterruptMode            _IM{};             //XXX default?
    Registers                _regs{};
    std::shared_ptr<ASpace>  _mmap{};
    IRQPin                   _int_pin{};
    IRQPin                   _nmi_pin{};
    InputPin                 _wait_pin{};
    InputPin                 _reset_pin{};
//TODO    OutputPinCb _iorq_pin{};  Gpio?
//TODO    OutputPinCb _halt_pin{};  Gpio?

    std::atomic_bool _break{};
    std::map<addr_t, std::pair<breakpoint_cb_t, void *>> _breakpoints{};

    static const std::array<Instruction, 256> instr_set;
    static const std::array<Instruction, 256> bit_instr_set;
    static const std::array<Instruction, 256> ix_instr_set;
    static const std::array<Instruction, 256> ix_bit_instr_set;
    static const std::array<Instruction, 256> mi_instr_set;
    static const std::array<Instruction, 256> iy_instr_set;
    static const std::array<Instruction, 256> iy_bit_instr_set;

    /**
     * Relative branch helper.
     * @param rel Address to jump relative to the current PC position.
     */
    void take_branch(int8_t rel) {
        _regs.PC += rel;
    }

    void push(uint8_t value) {
        --_regs.SP;
        write(_regs.SP, value);
    }

    void push_addr(addr_t addr) {
        push(addr >> 4);
        push(addr & 0xFF);
    }

    uint8_t pop() {
        uint8_t value = read(_regs.SP);
        ++_regs.SP;
        return value;
    }

    addr_t pop_addr() {
        addr_t addr = pop() | (static_cast<uint16_t>(pop()) << 4);
        return addr;
    }

    bool parity(uint8_t value) {
        return !(((value & 0x80) == 0x80) ^
                 ((value & 0x40) == 0x40) ^
                 ((value & 0x20) == 0x20) ^
                 ((value & 0x10) == 0x10) ^
                 ((value & 0x08) == 0x08) ^
                 ((value & 0x04) == 0x04) ^
                 ((value & 0x02) == 0x02) ^
                 ((value & 0x01) == 0x01));
    }

    uint8_t &reg8_code(uint8_t code, uint8_t &noreg) {
        switch (code) {
        case 0x0: return _regs.B;
        case 0x1: return _regs.C;
        case 0x2: return _regs.D;
        case 0x3: return _regs.E;
        case 0x4: return _regs.H;
        case 0x5: return _regs.L;
        case 0x7: return _regs.A;
        default:  return noreg;
        }
    }

    uint8_t &reg8_from_opcode(uint8_t op, uint8_t &noreg) {
        constexpr static uint8_t REG8_MASK = 0x38;
        constexpr static uint8_t REG8_SHIFT = 3;
        uint8_t rcode = (op & REG8_MASK) >> REG8_SHIFT;
        return reg8_code(rcode, noreg);
    }

    uint8_t &reg8_from_opcode(uint8_t op) {
        static uint8_t dummy{};
        return reg8_from_opcode(op, dummy);
    }

    uint8_t &reg8_src_from_opcode(uint8_t op, uint8_t &noreg) {
        constexpr static uint8_t REG8_MASK = 0x07;
        uint8_t rcode = op & REG8_MASK;
        return reg8_code(rcode, noreg);
    }

    uint8_t &reg8_src_from_opcode(uint8_t op) {
        static uint8_t dummy{};
        return reg8_src_from_opcode(op, dummy);
    }

    uint16_t &reg16_from_opcode(uint8_t op, bool nosp = false) {
        constexpr static uint8_t REG16_MASK = 0x30;
        switch (op & REG16_MASK) {
        case 0x00:
            return _regs.BC;
        case 0x10:
            return _regs.DE;
        case 0x20:
            return _regs.HL;
        case 0x30:
        default:
            return (nosp ? _regs.AF : _regs.SP);
        }
    }

    bool test_cond_from_opcode(uint8_t op);
    uint8_t bit_from_opcode(uint8_t);

    int call(addr_t addr);

    int add_A(int value);
    int sub_A(int value);
    int and_A(uint8_t value);
    int xor_A(uint8_t value);
    int or_A(uint8_t value);
    int cp_A(int value);

    void rlc(uint8_t &reg);
    void rrc(uint8_t &reg);
    void rl(uint8_t &reg);
    void rr(uint8_t &reg);
    void sla(uint8_t &reg);
    void sra(uint8_t &reg);
    void sll(uint8_t &reg);
    void srl(uint8_t &reg);

    void add16(uint16_t &reg, uint16_t value);
    void adc16(uint16_t &reg, uint16_t value);
    void sbc16(uint16_t &reg, uint16_t value);

    uint8_t reg_HL(uint16_t&reg, uint8_t op);
    int add_xx_rr(uint16_t &reg, uint8_t op);
    int ld_r_xxHL(uint16_t &reg, uint8_t op);
    int ld_xxHL_r(uint16_t &reg, uint8_t op);
    int ld_mxxd_n(uint16_t &reg, addr_t arg);
    int ld_mxxd_r(uint16_t &reg, uint8_t op, addr_t arg);
    int ld_r_mxxd(uint16_t &reg, uint8_t op, addr_t arg);
    int inc_xxH(uint16_t &reg);
    int inc_xxL(uint16_t &reg);
    int inc_mxxd(uint16_t &reg, addr_t arg);
    int dec_xxH(uint16_t &reg);
    int dec_xxL(uint16_t &reg);
    int dec_mxxd(uint16_t &reg, addr_t arg);

    /*
     * Instruction callbacks: Load operations.
     */
    static int i_LD_r_n     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_r_r     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_r_mHL   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mHL_r   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_rr_nn   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_A_mdd   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mdd_A   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mnn_HL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_HL_mnn  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mHL_n   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_SP_HL   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_POP_rr     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_PUSH_rr    (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: 16-bits arithmetic.
     */
    static int i_INC_rr     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_rr     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADD_HL_rr  (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: 8-bits arithmetic.
     */
    static int i_INC_r      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_mHL    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_r      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_mHL    (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_DAA        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CPL        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SCF        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CCF        (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_ADD_A_r    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADD_A_n    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADD_A_mHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_r    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_n    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_mHL  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_SUB_A_r    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SUB_A_n    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SUB_A_mHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_r    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_n    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_mHL  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_AND_A_r    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_AND_A_n    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_AND_A_mHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_XOR_A_r    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_XOR_A_n    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_XOR_A_mHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_OR_A_r     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_OR_A_n     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_OR_A_mHL   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CP_A_r     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CP_A_n     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CP_A_mHL   (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Rotate and Shift operations.
     */
    static int i_RLCA       (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_RRCA       (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_RLA        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_RRA        (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Exchange operations.
     */
    static int i_EX_AF_sAF  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_EX_DE_HL   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_EXX        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_EX_mSP_HL  (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Branch/Jump operations.
     */
    static int i_NOP        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DJNZ       (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JR         (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JR_NZ      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JR_Z       (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JR_NC      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JR_C       (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JP_nn      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JP_cc_nn   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JP_HL      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CALL_nn    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CALL_cc_nn (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_RET        (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_RET_cc     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_RST_p      (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: I/O ports.
     */
    static int i_IN_A_n     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_OUT_n_A    (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Flags.
     */
    static int i_HALT       (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DI         (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_EI         (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * Bit Instructions (CB).
     */
    static int i_bit_sr     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_bit_b      (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * IX Instructions (DD).
     */
    static int i_ADD_IX_rr  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_LD_IX_nn   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mnn_IX  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_IX_mnn  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_LD_IXH_n   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_IXL_n   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_r_IXHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_IXHL_r  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_LD_mIXd_n  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mIXd_r  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_r_mIXd  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_INC_IX     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_IXH    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_IXL    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_mIXd   (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_DEC_IX     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_IXH    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_IXL    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_mIXd   (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_ADD_A_IXHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADD_A_mIXd (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_IXHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_mIXd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_SUB_A_IXHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SUB_A_mIXd (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_IXHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_mIXd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_AND_A_IXHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_AND_A_mIXd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_XOR_A_IXHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_XOR_A_mIXd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_OR_A_IXHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_OR_A_mIXd  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_CP_A_IXHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CP_A_mIXd  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_POP_IX     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_EX_mSP_IX  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_PUSH_IX    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JP_IX      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_SP_IX   (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * IY Instructions (FD).
     */
    static int i_ADD_IY_rr  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_LD_IY_nn   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mnn_IY  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_IY_mnn  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_LD_IYH_n   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_IYL_n   (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_r_IYHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_IYHL_r  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_LD_mIYd_n  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_mIYd_r  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_r_mIYd  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_INC_IY     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_IYH    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_IYL    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_INC_mIYd   (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_DEC_IY     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_IYH    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_IYL    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_DEC_mIYd   (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_ADD_A_IYHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADD_A_mIYd (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_IYHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ADC_A_mIYd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_SUB_A_IYHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SUB_A_mIYd (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_IYHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_SBC_A_mIYd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_AND_A_IYHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_AND_A_mIYd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_XOR_A_IYHL (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_XOR_A_mIYd (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_OR_A_IYHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_OR_A_mIYd  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_CP_A_IYHL  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_CP_A_mIYd  (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_POP_IY     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_EX_mSP_IY  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_PUSH_IY    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_JP_IY      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_LD_SP_IY   (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * IX-Bit Instructions (DD CB).
     * IY-Bit Instructions (DF CB).
     */
    int xx_bit_sr(uint16_t &reg, uint8_t op, addr_t arg);
    int xx_bit(uint16_t &reg, uint8_t op, addr_t arg);

    static int i_ix_bit_sr  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_ix_bit     (ZilogZ80 &self, uint8_t op, addr_t arg);

    static int i_iy_bit_sr  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_iy_bit     (ZilogZ80 &self, uint8_t op, addr_t arg);

    /*
     * MI (Misc) Instructions (ED).
     */
    static int i_mi_io      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_adcsbc  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_LD_m_rr (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_LD_rr_m (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_NEG     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_RETN    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_RETI    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_IM_x    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_LD_x_A  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_LD_A_x  (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_RRD     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_RLD     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_copy    (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_cmp     (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_in      (ZilogZ80 &self, uint8_t op, addr_t arg);
    static int i_mi_out     (ZilogZ80 &self, uint8_t op, addr_t arg);
};

}
