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

#include <cstdint>
#include <functional>
#include <memory>
#include <string>
#include <string_view>
#include <unordered_map>
#include <utility>
#include <vector>

#include "aspace.hpp"
#include "fs.hpp"
#include "logger.hpp"
#include "readline.hpp"
#include "types.hpp"

namespace caio {
namespace monitor {

using RegsCb     = std::function<std::string()>;
using GetpcCb    = std::function<addr_t()>;
using SetpcCb    = std::function<void(addr_t)>;
using PeekCb     = std::function<uint8_t(addr_t)>;
using WriteCb    = std::function<void(addr_t, uint8_t)>;
using DisassCb   = std::function<void(std::ostream&, addr_t, size_t, bool)>;
using MmapCb     = std::function<sptr_t<ASpace>()>;
using EbreakCb   = std::function<void()>;
using LoadCb     = std::function<std::pair<addr_t, addr_t>(std::string_view, addr_t)>;
using SaveCb     = std::function<void(std::string_view, addr_t, addr_t)>;
using LogfileCb  = std::function<void(int)>;
using LoglevelCb = std::function<Loglevel(std::string_view)>;
using RegvalueCb = std::function<uint16_t(std::string_view)>;
using BpdocCb    = std::function<std::string(std::string_view)>;

/**
 * Monitored CPU.
 * Set of callbacks that must be provided
 * by any CPU that wants to be monitored.
 */
struct MonitoredCPU {
    RegsCb      regs{};         /* Registers as string                      */
    GetpcCb     getpc{};        /* Get the program counter                  */
    SetpcCb     setpc{};        /* Set the program counter                  */
    PeekCb      peek{};         /* Peek memory                              */
    WriteCb     write{};        /* Write memory                             */
    DisassCb    disass{};       /* Disassemble a memory address             */
    MmapCb      mmap{};         /* Get the memory mappings (address space)  */
    EbreakCb    ebreak{};       /* Set a breakpoint on next instruction     */
    LoadCb      load{};         /* Inject content of a file into memory     */
    SaveCb      save{};         /* Write a memory area into a file          */
    LogfileCb   logfile{};      /* Set the logfile file descriptor          */
    LoglevelCb  loglevel{};     /* Set/get the loglevel                     */
    RegvalueCb  regvalue{};     /* Get a register's value given its name    */
    BpdocCb     bpdoc{};        /* Documentation on how to set breakpoints  */

    operator bool() const {
        return (regs && getpc && setpc && peek && write && disass && mmap &&
            ebreak && load && save && loglevel && regvalue && bpdoc);
    }
};

/**
 * Default monitored CPU methods.
 * This method returns a MonitoredCPU structure filled
 * with default callbacks.
 * The CPU *must* provide the following callbacks (which are not set):
 * - getpc
 * - setpc
 * - mmap
 * - regvalue
 * @param cpu A raw pointer to the actual monitored CPU.
 * @return A monitored cpu structure with default callbacks.
 * @see MonitoredCPU
 */
template<typename CPU>
MonitoredCPU monitored_cpu_defaults(CPU* cpu)
{
    return MonitoredCPU{
        .regs = [cpu]() {
            return cpu->regs().to_string();
        },

        .getpc = {
            /*
             * CPU dependent.
             */
        },

        .setpc = {
            /*
             * CPU dependent.
             */
        },

        .peek = [cpu](addr_t addr) {
            return cpu->peek(addr);
        },

        .write = [cpu](addr_t addr, uint8_t data) {
            cpu->write(addr, data);
        },

        .disass = [cpu](std::ostream& os, addr_t addr, size_t count, bool show_pc) {
            cpu->disass(os, addr, count, show_pc);
        },

        .mmap = {
            /*
             * CPU (Platform) dependent.
             */
        },

        .ebreak = [cpu]() {
            return cpu->ebreak();
        },

        .load = [cpu](std::string_view fname, addr_t start) -> std::pair<addr_t, addr_t> {
            auto buf = fs::load(fname);
            addr_t addr = start;
            for (auto c : buf) {
                cpu->write(addr++, c);
            }
            return {start, buf.size()};
        },

        .save = [cpu](std::string_view fname, addr_t start, addr_t end) {
            ssize_t size = end - start;
            if (size > 0) {
                std::vector<uint8_t> buf{};
                for (auto addr = start; addr <= end; ++addr) {
                    uint8_t c = cpu->read(addr);
                    buf.push_back(c);
                }
                fs::save(fname, buf);
            }
        },

        .logfile = [cpu](int fd) {
            cpu->logfile(fd);
        },

        .loglevel = [cpu](std::string_view lv) {
            if (!empty(lv)) {
                cpu->loglevel(lv);
            }
            return cpu->loglevel();
        },

        .regvalue = {
            /*
             * CPU dependent.
             */
        },

        .bpdoc = [](std::string_view) {
            return "";
        }
    };
}

/**
 * Conditional breakpoint expression compiler.
 * A conditional breakpoint expression is a string formatted as "value1 operator value2"
 * where operator is one of: "<", ">", "<=", ">=", "==", "!=", "&", "|"
 * value1 and value2 are expressions containing one of:
 * - Literal value:     "$D020", "D020", "#53280", "#$A7"
 * - Register value:    "ra", "rx", "ry", "rs", "rp", "rp.n" "rp.v", "rp.b", "rp.d", "rp.i", "rp.z", "rp.c"
 * - Memory pointer:    "*D020", "*$D020", "*#53280"
 * - Register pointer:  "*ra", "*rx"
 *   Register pointers retrieve the content at the address specified by the register.
 *
 * Examples of a breakpoint using the monitor command line:
 *  b $8011 ra > $20        Breakpoint at $8011 only when the value of register A is greater than hexadecimal 20.
 *  b $8011 *d020 >= #15    Breakpoint at $8011 only when the content of memory address $D020 is greater
 *                          or equal to decimal 15.
 */
class Expr {
public:
    using fn_t = std::function<int(MonitoredCPU&)>;
    using ex_t = std::function<int(MonitoredCPU* cpu, fn_t&, fn_t&)>;

    Expr() = delete;
    ~Expr() = delete;

    /**
     * Compile a user defined condition.
     * @param cpu  Monitored CPU;
     * @param line Expression to compile.
     * @return The compiled expression as a function call.
     * @exception InvalidArgument
     */
    static std::function<int()> compile(MonitoredCPU& cpu, std::string_view line);

private:
    /**
     * Compile an argument.
     * The argument must be a literal value, a register value, a memory pointer or a register pointer.
     * @param cpu  Monitored CPU;
     * @param line The argument to compile.
     * @return The compiled argument.
     */
    static fn_t compile_argument(MonitoredCPU& cpu, std::string_view line);

    static const std::pair<std::string, ex_t> operators[];
};

/**
 * Monitor command.
 */
struct Command {
    using args_t = std::vector<std::string>;
    using fn_t   = std::function<bool(class Monitor&, const args_t&)>;

    std::string command{};
    std::string short_command{};
    std::string args{};
    std::string help{};
    fn_t        fn{};
};

/**
 * CPU Monitor.
 */
class Monitor {
public:
    constexpr static const char* HISTFILE      = "monitor.hist";
    constexpr static const char* PROMPT_PREFIX = "";
    constexpr static const char* PROMPT_SUFFIX = "> ";

    using cond_t = std::pair<std::function<int()>, std::string>;

    /**
     * Initialise this monitor.
     * @param ifd Input file descriptor used to communicate with the user;
     * @param ofd Output file descriptor used to communicate with the user;
     * @param cpu Monitored CPU.
     * @see MonitoredCPU
     * TODO: replace ifd and ofd with streams and native_handle() (C++26)
     */
    Monitor(int ifd, int ofd, MonitoredCPU&& cpu);

    /**
     * Enter this monitor.
     * @return true to continue the CPU; false to terminate the CPU.
     */
    bool run();

    /**
     * Add a breakpoint.
     * @param addr Breakpoint address;
     * @param cond Condition that triggers the breakpoint (empty for unconditional breakpoints).
     */
    void add_breakpoint(addr_t addr, const cond_t& cond = {}) {
        _breakpoints[addr] = cond;
    }

    /**
     * Remove a breakpoint.
     * @param addr Address to remove from the breakpoint list.
     */
    void del_breakpoint(addr_t addr) {
        _breakpoints.erase(addr);
    }

    /**
     * Detect whether there is a breakpoint at a specified address.
     * @param addr Address.
     * @return true if there is breakpoint at the specified address; false otherwise.
     */
    bool is_breakpoint(addr_t addr) const;

private:
    /**
     * Return the current prompt string.
     * @return The prompt string.
     */
    std::string prompt();

    /**
     * Convert a string formatted as "[$]xxxx" or "." to an address.
     * @param str    The string to convert;
     * @param devfal Default value (set in case the string is ".").
     * @return The address.
     * @exception InvalidNumber
     * @see to_number()
     */
    //FIXME: move to utils
    addr_t to_addr(std::string_view str, addr_t defval);

    /**
     * Convert a string formatted as "[$#]xxx" to an unsigned number.
     * @param str The string to convert.
     * @return The converted unsigned integner.
     * @exception InvalidNumber
     * @see utils::to_number()
     */
    //FIXME: move to utils
    size_t to_count(std::string_view str);

    Readline                           _rd{};
    MonitoredCPU                       _cpu{};
    bool                               _is_running{};
    std::string                        _prev_line{};
    std::string                        _prev_fn{};
    std::unordered_map<addr_t, cond_t> _breakpoints{};

    static bool assemble    (Monitor& mon, const Command::args_t& args);
    static bool disassemble (Monitor& mon, const Command::args_t& args);
    static bool dump        (Monitor& mon, const Command::args_t& args);
    static bool registers   (Monitor& mon, const Command::args_t& args);
    static bool mmap        (Monitor& mon, const Command::args_t& args);

    static bool bp_add      (Monitor& mon, const Command::args_t& args);
    static bool bp_del      (Monitor& mon, const Command::args_t& args);
    static bool bp_clear    (Monitor& mon, const Command::args_t& args);
    static bool bp_list     (Monitor& mon, const Command::args_t& args);

    static bool go          (Monitor& mon, const Command::args_t& args);
    static bool step        (Monitor& mon, const Command::args_t& args);

    static bool load        (Monitor& mon, const Command::args_t& args);
    static bool save        (Monitor& mon, const Command::args_t& args);

    static bool loglevel    (Monitor& mon, const Command::args_t& args);

    static bool history     (Monitor& mon, const Command::args_t& args);

    static bool quit        (Monitor& mon, const Command::args_t& args);
    static bool help        (Monitor& mon, const Command::args_t& args);

    static Command commands[];
};

}

using Monitor = monitor::Monitor;
using MonitoredCPU = monitor::MonitoredCPU;

}
