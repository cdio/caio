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
#pragma once

#include <array>
#include <atomic>
#include <functional>
#include <map>
#include <memory>
#include <utility>

#include "aspace.hpp"
#include "clock.hpp"
#include "latch.hpp"
#include "logger.hpp"
#include "monitor.hpp"
#include "pin.hpp"
#include "types.hpp"
#include "utils.hpp"


namespace caio {
namespace zilog {

/**
 * Zilog Z80 emulator.
 *
 * @see https://gist.github.com/drhelius/8497817
 */
class Z80 : public Clockable, public Name {
public:
    constexpr static const char* TYPE    = "Z80";
    constexpr static const char* LABEL   = "CPU";

    constexpr static const addr_t vRESET = 0x0000;
    constexpr static const addr_t vNMI   = 0x0066;
    constexpr static const addr_t vINT   = 0x0038;

    constexpr static const uint8_t I_BIT = 0xCB;
    constexpr static const uint8_t I_IX  = 0xDD;
    constexpr static const uint8_t I_MI  = 0xED;
    constexpr static const uint8_t I_IY  = 0xFD;

    constexpr static const uint8_t I_NOP = 0x00;
    constexpr static const uint8_t I_EI  = 0xFB;

    using breakpoint_cb_t = std::function<void(Z80&, void*)>;

    enum ClockCycle {
        T1 = 0,
        T2,
        T3,
        Tn
    };

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
        IMODE_0,            /* Device puts 8-bit instruction on data bus            */
        IMODE_1,            /* ISR at $0038 (vINT)                                  */
        IMODE_2             /* Device provides LO 8 bits of interrupt vector table  */
    };

    enum Flags {
        S = 0x80,           /* Sign             */
        Z = 0x40,           /* Zero             */
        Y = 0x20,           /* Y or F5          */
        H = 0x10,           /* Half Carry       */
        X = 0x08,           /* X or F3          */
        V = 0x04,           /* Parity/Overflow  */
        N = 0x02,           /* Add/Substract    */
        C = 0x01            /* Carry            */
    };

    struct Instruction {
        const char* format;                     /* Mnemonic format string           */
        int (*fn)(Z80&, uint8_t, addr_t);       /* Instruction callback             */
        ArgType type;                           /* Type of argument                 */
        size_t cycles;                          /* Consumed clock cycles            */
        size_t size;                            /* Size of the instruction (bytes)  */
    };

    struct Registers {
        uint8_t A;
        uint8_t B;
        uint8_t C;
        uint8_t D;
        uint8_t E;
        uint8_t H;
        uint8_t L;
        uint8_t F;

        uint8_t aA;
        uint8_t aB;
        uint8_t aC;
        uint8_t aD;
        uint8_t aE;
        uint8_t aH;
        uint8_t aL;
        uint8_t aF;

        uint8_t  I;             /* Interrupt vector */
        uint8_t  R;             /* Memory refresh   */
        uint16_t IX;            /* Index X          */
        uint16_t IY;            /* Index Y          */
        uint16_t SP;            /* Stack Pointer    */
        uint16_t PC;            /* Program Counter  */

        uint16_t memptr;        /* Undocumented pseudo register */

        uint16_t AF() const {
            return ((static_cast<uint16_t>(A) << 8) | F);
        }

        uint16_t BC() const {
            return ((static_cast<uint16_t>(B) << 8) | C);
        }

        uint16_t DE() const {
            return ((static_cast<uint16_t>(D) << 8) | E);
        }

        uint16_t HL() const {
            return ((static_cast<uint16_t>(H) << 8) | L);
        }

        uint16_t aAF() const {
            return ((static_cast<uint16_t>(aA) << 8) | aF);
        }

        uint16_t aBC() const {
            return ((static_cast<uint16_t>(aB) << 8) | aC);
        }

        uint16_t aDE() const {
            return ((static_cast<uint16_t>(aD) << 8) | aE);
        }

        uint16_t aHL() const {
            return ((static_cast<uint16_t>(aH) << 8) | aL);
        }

        void AF(uint16_t);
        void BC(uint16_t);
        void DE(uint16_t);
        void HL(uint16_t);

        void aAF(uint16_t);
        void aBC(uint16_t);
        void aDE(uint16_t);
        void aHL(uint16_t);

        std::string to_string() const;

        static std::string to_string(Flags fl);
    };

    /**
     * Initialise this CPU.
     * @param type  CPU type;
     * @param label CPU label.
     */
    Z80(const std::string& type = TYPE, const std::string& label = LABEL)
        : Name{type, label} {
    }

    /**
     * Initialise this CPU.
     * @param mmap  System mappings;
     * @param type  CPU type;
     * @param label CPU label.
     * @see ASpace
     */
    Z80(const sptr_t<ASpace>& mmap, const std::string& type = TYPE, const std::string& label = LABEL)
        : Name{type, (label.empty() ? LABEL : label)} {
        init(mmap);
    }

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
    void init_monitor(std::istream& is, std::ostream& os);
    void init_monitor(int ifd, int ofd);

    /**
     * Set the single-step log level.
     * @param lvs Log level string to set.
     * @see Logger::loglevel(const std::string&)
     */
    void loglevel(const std::string& lvs);

    /**
     * @return The current single-step log level.
     */
    Loglevel loglevel() const;

    /**
     * Restart this CPU.
     */
    void reset();

    /**
     * Set the /HALT output pin callback.
     * This callback is called each time the CPU is halted.
     * @param halt_cb Callback.
     */
    void halt_pin(const OutputPinCb& halt_cb);

    /**
     * Set the /IORQ output pin callback.
     * This callback is called each time the /IORQ output pin is changed.
     * @param iorq_cb Callback.
     */
    void iorq_pin(const OutputPinCb& iorq_cb);

    /**
     * Set the /M1 output pin callback.
     * This callback is called each time the /M1 output pin is changed.
     * @param m1_cb Callback.
     */
    void m1_pin(const OutputPinCb& m1_cb);

    /**
     * Set the /RFSH output pin callback.
     * This callback is called each time the /RFSH output pin is changed.
     * @param rfsh_cb Callback.
     */
    void rfsh_pin(const OutputPinCb& rfsh_cb);

    /**
     * @return The status of the /HALT output pin.
     */
    bool halt_pin() const;

    /**
     * @return The status of the /IORQ output pin.
     */
    bool iorq_pin() const;

    /**
     * @return The status of the /M1 output pin.
     */
    bool m1_pin() const;

    /**
     * @return The status of the /RFSH output pin.
     */
    bool rfsh_pin() const;

    /**
     * @return The status of the /WAIT input pin.
     */
    bool wait_pin() const;

    /**
     * Trigger an IRQ.
     * This method must be called twice by external peripherals, the first time to activate
     * the /INT input pin and a second time to de-activate it when the interrupt is served.
     * @param active true to generate an interrupt; false to de-activate a previous interrupt request.
     */
    void int_pin(bool active);

    /**
     * Trigger an NMI interrupt.
     * This method must be called twice by external peripherals, the first time to activate
     * the /NMI input pin and a second time to de-activate it when the interrupt is served.
     * @param active true to generate an interrupt; false to de-activate.
     */
    void nmi_pin(bool active);

    /**
     * Trigger a CPU reset.
     * This method must be called twice by external peripherals, the first time to activate
     * the /RESET input pin and a second time to de-active it.
     * As soon as this pin is deactivated the reset cycle is started.
     * @param active true to trigger a reset; false otherwise.
     */
    void reset_pin(bool active);

    /**
     * Trigger a wait state.
     * This method must be called twice by external peripherals, the first time to activate
     * the /WAIT input pin and a second time to de-activate it when the device is ready.
     * @param active true or false.
     */
    void wait_pin(bool active);

    /**
     * External breakpoint.
     * Force a return back to the monitor on the next M1 cycle.
     * If the monitor is not active a system halt is requested.
     * This method is intended to be used by a cpu monitor to implement
     * a single-step command.
     * @see MonitoredCPU
     */
    void ebreak();

    /**
     * Add a breakpoint on a memory address.
     * @param addr Address;
     * @parma cb   Method to call when the breakpoint hits;
     * @param arg  Argument sent to the callback.
     */
    void bpadd(addr_t addr, const breakpoint_cb_t& cb, void* arg);

    /**
     * Delete a breakpoint on a memory address.
     * @param addr Address.
     */
    void bpdel(addr_t addr);

    /**
     * @return The register values.
     */
    const Registers& regs() const;

    /**
     * Disassembler.
     * @param os      Output stream;
     * @param addr    Start address with the machine code to disassemble;
     * @param count   Number of instructions to disassemble.
     * @param show_pc true if the position of the PC must be marked in the disassembled code; false otherwise (default).
     * @see MonitoredCPU
     */
    void disass(std::ostream& os, addr_t start, size_t count, bool show_pc = false);

    /**
     * Read an address from an address.
     * @param addr Address to read from.
     * @return The 16 bits value.
     * @exception InvalidReadAddress
     * @see read()
     */
    addr_t read_addr(size_t addr);

    /**
     * Write an address into an address.
     * @param addr  Address to write to;
     * @param value 16 bits value to write.
     * @exception InvalidWriteAddress
     * @see write()
     */
    void write_addr(addr_t addr, addr_t data);

    /**
     * @see Device::read()
     */
    virtual uint8_t read(addr_t addr, Device::ReadMode mode = Device::ReadMode::Read);

    /**
     * @see Device::peek()
     */
    virtual uint8_t peek(addr_t addr) const {
        return const_cast<Z80*>(this)->read(addr, Device::ReadMode::Peek);
    }

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data);

    /**
     * @return A string containing a human readable string with the status of this CPU (regsiters and other values).
     */
    std::string status() const;

    /**
     * Initialise this CPU.
     * @param mmap System mappings.
     */
//XXX
    void init(const sptr_t<ASpace>& mmap = {});

private:
    /**
     * Set the status of the /HALT output pin.
     * Set the status of the /HALT output pin and, if defined, call the user defined callback.
     * @param active true to activate the pin, false to de-activate it.
     * @see halt_pin(const OutputPinCb&)
     */
    void halt_pin(bool active);

    /**
     * Set the status of the /IORQ output pin.
     * Set the status of the /IORQ output pin and, if defined, call the user defined callback.
     * @param active true to activate the pin, false to de-activate it.
     * @see iorq_pin(const OutputPinCb&)
     */
    void iorq_pin(bool active);

    /**
     * Set the status of the /RFSH output pin.
     * Set the status of the /RFSH output pin and, if defined, call the user defined callback.
     * @param active true to activate the pin, false to de-activate it.
     * @see rfsh_pin(const OutputPinCb&)
     */
    void rfsh_pin(bool active);

    /**
     * Set the status of the /M1 output pin.
     * Set the status of the /M1 output pin and, if defined, call the user defined callback.
     * @param active true to activate the pin, false to de-activate it.
     * @see m1_pin(const OutputPinCb&)
     */
    void m1_pin(bool active);

    /**
     * Tick event method.
     * This method is called by the clock and executes an instruction cycle.
     * If the monitor is not running the current instruction cycle is executed.
     * If the monitor is running and a monitor-breakpoint is set on the current PC address
     * the monitor's run method is called instead.
     * @param clk The caller clock.
     * @return The number of clock cycles consumed (the clock will call this method again after
     * this number of cycles have passed); Clockable::HALT if the clock must be terminated.
     * @see tick()
     * @see Monitor::run()
     */
    size_t tick(const Clock& clk) override;

    /**
     * Tick event method.
     * If the CPU is halted this methods returns immediately.
     * If an interrupt occured an interrupt M1 cycle is started/continued; otherwise a
     * normal M1 cycle is started/continued.
     * @return The number of clock cycles consumed (the clock will call this method again after
     * this number of cycles have passed); Clockable::HALT if the clock must be terminated.
     * @see halt_pin()
     * @see m1_cycle()
     * @see m1_cycle_interrupt()
     */
    size_t tick();

    /**
     * Start/continue a M1 cycle.
     * This method contains a state machine that implements an M1 cycle,
     * which consists of various steps executed in several clock cycles.
     * During the M1 cycle an instruction is fetched and executed.
     * @see m1_cycle_interrupt()
     */
    size_t m1_cycle();

    /**
     * Start/continue an interrupt M1 cycle.
     * This method contains a state machine that implements an M1 cycle that process
     * maskable and non-maskable interrupts. An M1 cycle consists of various steps
     * executed in several clock cycles.
     * During the interrupt M1 cycle INT and NMI interrupts are processed.
     * @see m1_cycle()
     */
    size_t m1_cycle_interrupt();

    /**
     * Execute an instruction.
     * If the opcode is not forced it must be fetched from the address specified by the PC register.
     * @param opcode First opcode of the instruction to execute;
     * @param forced true if the opcode is forced (not fetched from any memory address); false otherwise.
     * @return The number of clock cycles consumed by the instruction.
     * @see execute(const Instruction *ins, uint8_t opcode, bool forced = false)
     */
    size_t execute(uint8_t opcode, bool forced = false);

    /**
     * Execute an instruction.
     * Decode and execute an instruction.
     * @param ins    Instruction table;
     * @param opcode Valid entry in the specified instruction table;
     * @param forced true if the opcode is forced (not fetched from any memory address); false otherwise.
     * @return The number of clock cycles consumed by the instruction.
     */
    size_t execute(const Instruction* ins, uint8_t opcode, bool forced = false);

    /**
     * Disassemble a single instruction located at a specified address.
     * @param addr    Address to disassemble (after disassembly this value is
     *                incremented with the position of the next instruction);
     * @param show_pc true if the position of the PC must be marked in the disassembled code;
     *                false otherwise.
     * @return A string with the disassembled instruction.
     */
    std::string disass(addr_t& addr, bool show_pc = false);

    /**
     * IN helper.
     * @param port I/O port to read from.
     * @return The value at the specifed I/O port.
     */
    uint8_t io_in(uint8_t port);

    /*
     * OUT helper.
     * @param port  I/O port to write to;
     * @param value Value to write.
     */
    void io_out(uint8_t port, uint8_t value);

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

    void flag_Y(bool act) {
        flag(Flags::Y, act);
    }

    void flag_X(bool act) {
        flag(Flags::X, act);
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

    bool test_Y() const {
        return test_flags(Flags::Y);
    }

    bool test_H() const {
        return test_flags(Flags::H);
    }

    bool test_X() const {
        return test_flags(Flags::X);
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

    Logger           _log{};
    uptr_t<Monitor>  _monitor{};
    bool             _IFF1{};
    bool             _IFF2{};
    InterruptMode    _imode{IMODE_0};
    Registers        _regs{};
    sptr_t<ASpace>   _mmap{};
    IRQPin           _int_pin{};
    IRQPin           _nmi_pin{};
    PullUp           _reset_pin{};
    PullUp           _wait_pin{};
    bool             _halted{};
    OutputPinCb      _halt_cb{};
    bool             _iorq_pin{};
    OutputPinCb      _iorq_cb{};
    bool             _m1_pin{};
    OutputPinCb      _m1_cb{};
    bool             _rfsh_pin{};
    OutputPinCb      _rfsh_cb{};
    bool             _is_int{};
    bool             _is_nmi{};
    int              _tx{T1};
    std::atomic_bool _break{};
    std::map<addr_t, std::pair<breakpoint_cb_t, void*>> _breakpoints{};

    static const std::array<Instruction, 256> instr_set;
    static const std::array<Instruction, 256> bit_instr_set;
    static const std::array<Instruction, 256> ix_instr_set;
    static const std::array<Instruction, 256> ix_bit_instr_set;
    static const std::array<Instruction, 256> mi_instr_set;
    static const std::array<Instruction, 256> iy_instr_set;
    static const std::array<Instruction, 256> iy_bit_instr_set;

    void take_branch(int8_t rel, bool memptr = true) {
        _regs.PC += rel;
        if (memptr) {
            _regs.memptr = _regs.PC;
        }
    }

    void push(uint8_t value) {
        --_regs.SP;
        write(_regs.SP, value);
    }

    void push_addr(addr_t addr) {
        push(addr >> 8);
        push(addr & 0xFF);
    }

    uint8_t pop() {
        uint8_t value = read(_regs.SP);
        ++_regs.SP;
        return value;
    }

    addr_t pop_addr() {
        addr_t addr = pop() | (static_cast<uint16_t>(pop()) << 8);
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

    uint8_t& reg8_code(uint8_t code, uint8_t& noreg) {
        switch (code) {
        case 0x00: return _regs.B;
        case 0x01: return _regs.C;
        case 0x02: return _regs.D;
        case 0x03: return _regs.E;
        case 0x04: return _regs.H;
        case 0x05: return _regs.L;
        case 0x07: return _regs.A;
        default:   return noreg;
        }
    }

    uint8_t& reg8_from_opcode(uint8_t op, uint8_t& noreg) {
        constexpr static uint8_t REG8_MASK = 0x07;
        constexpr static uint8_t REG8_SHIFT = 3;
        uint8_t rcode = (op >> REG8_SHIFT) & REG8_MASK;
        return reg8_code(rcode, noreg);
    }

    uint8_t& reg8_from_opcode(uint8_t op) {
        static uint8_t dummy{};
        return reg8_from_opcode(op, dummy);
    }

    uint8_t& reg8_src_from_opcode(uint8_t op, uint8_t& noreg) {
        constexpr static uint8_t REG8_MASK = 0x07;
        uint8_t rcode = op & REG8_MASK;
        return reg8_code(rcode, noreg);
    }

    uint8_t& reg8_src_from_opcode(uint8_t op) {
        static uint8_t dummy{};
        return reg8_src_from_opcode(op, dummy);
    }

    std::pair<std::function<uint16_t()>, std::function<void(uint16_t)>>
    reg16_from_opcode(uint8_t op, bool nosp = false) {
        constexpr static uint8_t REG16_MASK = 0x30;
        switch (op & REG16_MASK) {
        case 0x00:
            return {{[this]() { return _regs.BC(); }}, {[this](uint16_t val) { _regs.BC(val); }}};
        case 0x10:
            return {{[this]() { return _regs.DE(); }}, {[this](uint16_t val) { _regs.DE(val); }}};
        case 0x20:
            return {{[this]() { return _regs.HL(); }}, {[this](uint16_t val) { _regs.HL(val); }}};
        case 0x30:
        default:
            if (nosp) {
                return {{[this]() { return _regs.AF(); }}, {[this](uint16_t val) { _regs.AF(val); }}};
            } else {
                return {{[this]() { return _regs.SP; }}, {[this](uint16_t val) { _regs.SP = val; }}};
            }
        }
    }

    bool test_cond_from_opcode(uint8_t op);
    uint8_t bit_from_opcode(uint8_t);

    int call(addr_t addr);

    uint8_t add8(uint8_t v1, uint8_t v2, uint8_t carry);
    uint8_t sub8(uint8_t v1, uint8_t v2, bool borrow);

    int add_A(uint8_t value, bool carry);
    int sub_A(uint8_t value, bool borrow);
    int cp_A(uint8_t value);
    int and_A(uint8_t value);
    int xor_A(uint8_t value);
    int or_A(uint8_t value);

    void rlc(uint8_t& reg);
    void rrc(uint8_t& reg);
    void rl(uint8_t& reg);
    void rr(uint8_t& reg);
    void sla(uint8_t& reg);
    void sra(uint8_t& reg);
    void sll(uint8_t& reg);
    void srl(uint8_t& reg);

    void add16(uint16_t& reg, uint16_t value, bool carry = false);
    void adc16(uint16_t& reg, uint16_t value);
    void sbc16(uint16_t& reg, uint16_t value);

    uint8_t reg_hilo(uint16_t reg, uint8_t op);
    int add_xx_rr(uint16_t& reg, uint8_t op);
    int ld_r_xxHL(uint16_t& reg, uint8_t op);
    int ld_xxHL_r(uint16_t& reg, uint8_t op);
    int ld_mxxd_n(uint16_t& reg, addr_t arg);
    int ld_mxxd_r(uint16_t& reg, uint8_t op, addr_t arg);
    int ld_r_mxxd(uint16_t& reg, uint8_t op, addr_t arg);
    int inc_xxH(uint16_t& reg);
    int inc_xxL(uint16_t& reg);
    int inc_mxxd(uint16_t& reg, addr_t arg);
    int dec_xxH(uint16_t& reg);
    int dec_xxL(uint16_t& reg);
    int dec_mxxd(uint16_t& reg, addr_t arg);

    /*
     * Instruction callbacks: Load operations.
     */
    static int i_LD_r_n     (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_r_r     (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_r_mHL   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mHL_r   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_rr_nn   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_A_mdd   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mdd_A   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mnn_HL  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_HL_mnn  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mHL_n   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_SP_HL   (Z80& self, uint8_t op, addr_t arg);
    static int i_POP_rr     (Z80& self, uint8_t op, addr_t arg);
    static int i_PUSH_rr    (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: 16-bits arithmetic.
     */
    static int i_INC_rr     (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_rr     (Z80& self, uint8_t op, addr_t arg);
    static int i_ADD_HL_rr  (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: 8-bits arithmetic.
     */
    static int i_INC_r      (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_mHL    (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_r      (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_mHL    (Z80& self, uint8_t op, addr_t arg);

    static int i_DAA        (Z80& self, uint8_t op, addr_t arg);
    static int i_CPL        (Z80& self, uint8_t op, addr_t arg);
    static int i_SCF        (Z80& self, uint8_t op, addr_t arg);
    static int i_CCF        (Z80& self, uint8_t op, addr_t arg);

    static int i_ADD_A_r    (Z80& self, uint8_t op, addr_t arg);
    static int i_ADD_A_n    (Z80& self, uint8_t op, addr_t arg);
    static int i_ADD_A_mHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_r    (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_n    (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_mHL  (Z80& self, uint8_t op, addr_t arg);

    static int i_SUB_A_r    (Z80& self, uint8_t op, addr_t arg);
    static int i_SUB_A_n    (Z80& self, uint8_t op, addr_t arg);
    static int i_SUB_A_mHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_r    (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_n    (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_mHL  (Z80& self, uint8_t op, addr_t arg);

    static int i_AND_A_r    (Z80& self, uint8_t op, addr_t arg);
    static int i_AND_A_n    (Z80& self, uint8_t op, addr_t arg);
    static int i_AND_A_mHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_XOR_A_r    (Z80& self, uint8_t op, addr_t arg);
    static int i_XOR_A_n    (Z80& self, uint8_t op, addr_t arg);
    static int i_XOR_A_mHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_OR_A_r     (Z80& self, uint8_t op, addr_t arg);
    static int i_OR_A_n     (Z80& self, uint8_t op, addr_t arg);
    static int i_OR_A_mHL   (Z80& self, uint8_t op, addr_t arg);
    static int i_CP_A_r     (Z80& self, uint8_t op, addr_t arg);
    static int i_CP_A_n     (Z80& self, uint8_t op, addr_t arg);
    static int i_CP_A_mHL   (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Rotate and Shift operations.
     */
    static int i_RLCA       (Z80& self, uint8_t op, addr_t arg);
    static int i_RRCA       (Z80& self, uint8_t op, addr_t arg);
    static int i_RLA        (Z80& self, uint8_t op, addr_t arg);
    static int i_RRA        (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Exchange operations.
     */
    static int i_EX_AF_sAF  (Z80& self, uint8_t op, addr_t arg);
    static int i_EX_DE_HL   (Z80& self, uint8_t op, addr_t arg);
    static int i_EXX        (Z80& self, uint8_t op, addr_t arg);
    static int i_EX_mSP_HL  (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Branch/Jump operations.
     */
    static int i_NOP        (Z80& self, uint8_t op, addr_t arg);
    static int i_DJNZ       (Z80& self, uint8_t op, addr_t arg);
    static int i_JR         (Z80& self, uint8_t op, addr_t arg);
    static int i_JR_NZ      (Z80& self, uint8_t op, addr_t arg);
    static int i_JR_Z       (Z80& self, uint8_t op, addr_t arg);
    static int i_JR_NC      (Z80& self, uint8_t op, addr_t arg);
    static int i_JR_C       (Z80& self, uint8_t op, addr_t arg);
    static int i_JP_nn      (Z80& self, uint8_t op, addr_t arg);
    static int i_JP_cc_nn   (Z80& self, uint8_t op, addr_t arg);
    static int i_JP_HL      (Z80& self, uint8_t op, addr_t arg);
    static int i_CALL_nn    (Z80& self, uint8_t op, addr_t arg);
    static int i_CALL_cc_nn (Z80& self, uint8_t op, addr_t arg);
    static int i_RET        (Z80& self, uint8_t op, addr_t arg);
    static int i_RET_cc     (Z80& self, uint8_t op, addr_t arg);
    static int i_RST_p      (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: I/O ports.
     */
    static int i_IN_A_n     (Z80& self, uint8_t op, addr_t arg);
    static int i_OUT_n_A    (Z80& self, uint8_t op, addr_t arg);

    /*
     * Instruction callbacks: Flags.
     */
    static int i_HALT       (Z80& self, uint8_t op, addr_t arg);
    static int i_DI         (Z80& self, uint8_t op, addr_t arg);
    static int i_EI         (Z80& self, uint8_t op, addr_t arg);

    /*
     * Bit Instructions (CB).
     */
    static int i_bit_sr     (Z80& self, uint8_t op, addr_t arg);
    static int i_bit_b      (Z80& self, uint8_t op, addr_t arg);

    /*
     * IX Instructions (DD).
     */
    static int i_ADD_IX_rr  (Z80& self, uint8_t op, addr_t arg);

    static int i_LD_IX_nn   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mnn_IX  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_IX_mnn  (Z80& self, uint8_t op, addr_t arg);

    static int i_LD_IXH_n   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_IXL_n   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_r_IXHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_IXHL_r  (Z80& self, uint8_t op, addr_t arg);

    static int i_LD_mIXd_n  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mIXd_r  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_r_mIXd  (Z80& self, uint8_t op, addr_t arg);

    static int i_INC_IX     (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_IXH    (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_IXL    (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_mIXd   (Z80& self, uint8_t op, addr_t arg);

    static int i_DEC_IX     (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_IXH    (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_IXL    (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_mIXd   (Z80& self, uint8_t op, addr_t arg);

    static int i_ADD_A_IXHL (Z80& self, uint8_t op, addr_t arg);
    static int i_ADD_A_mIXd (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_IXHL (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_mIXd (Z80& self, uint8_t op, addr_t arg);

    static int i_SUB_A_IXHL (Z80& self, uint8_t op, addr_t arg);
    static int i_SUB_A_mIXd (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_IXHL (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_mIXd (Z80& self, uint8_t op, addr_t arg);

    static int i_AND_A_IXHL (Z80& self, uint8_t op, addr_t arg);
    static int i_AND_A_mIXd (Z80& self, uint8_t op, addr_t arg);

    static int i_XOR_A_IXHL (Z80& self, uint8_t op, addr_t arg);
    static int i_XOR_A_mIXd (Z80& self, uint8_t op, addr_t arg);

    static int i_OR_A_IXHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_OR_A_mIXd  (Z80& self, uint8_t op, addr_t arg);

    static int i_CP_A_IXHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_CP_A_mIXd  (Z80& self, uint8_t op, addr_t arg);

    static int i_POP_IX     (Z80& self, uint8_t op, addr_t arg);
    static int i_EX_mSP_IX  (Z80& self, uint8_t op, addr_t arg);
    static int i_PUSH_IX    (Z80& self, uint8_t op, addr_t arg);
    static int i_JP_IX      (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_SP_IX   (Z80& self, uint8_t op, addr_t arg);

    /*
     * IY Instructions (FD).
     */
    static int i_ADD_IY_rr  (Z80& self, uint8_t op, addr_t arg);

    static int i_LD_IY_nn   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mnn_IY  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_IY_mnn  (Z80& self, uint8_t op, addr_t arg);

    static int i_LD_IYH_n   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_IYL_n   (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_r_IYHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_IYHL_r  (Z80& self, uint8_t op, addr_t arg);

    static int i_LD_mIYd_n  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_mIYd_r  (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_r_mIYd  (Z80& self, uint8_t op, addr_t arg);

    static int i_INC_IY     (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_IYH    (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_IYL    (Z80& self, uint8_t op, addr_t arg);
    static int i_INC_mIYd   (Z80& self, uint8_t op, addr_t arg);

    static int i_DEC_IY     (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_IYH    (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_IYL    (Z80& self, uint8_t op, addr_t arg);
    static int i_DEC_mIYd   (Z80& self, uint8_t op, addr_t arg);

    static int i_ADD_A_IYHL (Z80& self, uint8_t op, addr_t arg);
    static int i_ADD_A_mIYd (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_IYHL (Z80& self, uint8_t op, addr_t arg);
    static int i_ADC_A_mIYd (Z80& self, uint8_t op, addr_t arg);

    static int i_SUB_A_IYHL (Z80& self, uint8_t op, addr_t arg);
    static int i_SUB_A_mIYd (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_IYHL (Z80& self, uint8_t op, addr_t arg);
    static int i_SBC_A_mIYd (Z80& self, uint8_t op, addr_t arg);

    static int i_AND_A_IYHL (Z80& self, uint8_t op, addr_t arg);
    static int i_AND_A_mIYd (Z80& self, uint8_t op, addr_t arg);

    static int i_XOR_A_IYHL (Z80& self, uint8_t op, addr_t arg);
    static int i_XOR_A_mIYd (Z80& self, uint8_t op, addr_t arg);

    static int i_OR_A_IYHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_OR_A_mIYd  (Z80& self, uint8_t op, addr_t arg);

    static int i_CP_A_IYHL  (Z80& self, uint8_t op, addr_t arg);
    static int i_CP_A_mIYd  (Z80& self, uint8_t op, addr_t arg);

    static int i_POP_IY     (Z80& self, uint8_t op, addr_t arg);
    static int i_EX_mSP_IY  (Z80& self, uint8_t op, addr_t arg);
    static int i_PUSH_IY    (Z80& self, uint8_t op, addr_t arg);
    static int i_JP_IY      (Z80& self, uint8_t op, addr_t arg);
    static int i_LD_SP_IY   (Z80& self, uint8_t op, addr_t arg);

    /*
     * IX-Bit Instructions (DD CB).
     * IY-Bit Instructions (DF CB).
     */
    int xx_bit_sr(uint16_t& reg, uint8_t op, addr_t arg);
    int xx_bit(uint16_t& reg, uint8_t op, addr_t arg);

    static int i_ix_bit_sr  (Z80& self, uint8_t op, addr_t arg);
    static int i_ix_bit     (Z80& self, uint8_t op, addr_t arg);

    static int i_iy_bit_sr  (Z80& self, uint8_t op, addr_t arg);
    static int i_iy_bit     (Z80& self, uint8_t op, addr_t arg);

    /*
     * MI (Misc) Instructions (ED).
     */
    static int i_mi_io      (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_adcsbc  (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_LD_m_rr (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_LD_rr_m (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_NEG     (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_RETN    (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_RETI    (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_IM_x    (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_LD_x_A  (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_LD_A_x  (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_RRD     (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_RLD     (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_copy    (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_cmp     (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_in      (Z80& self, uint8_t op, addr_t arg);
    static int i_mi_out     (Z80& self, uint8_t op, addr_t arg);
};

}

using Z80 = zilog::Z80;

}
