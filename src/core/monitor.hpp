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
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "aspace.hpp"
#include "logger.hpp"
#include "readline.hpp"
#include "types.hpp"


namespace caio {
namespace monitor {

class Monitor;

/**
 * Monitored CPU.
 */
struct MonitoredCPU {
    std::function<std::ostream&(std::ostream&)>                             regs{};
    std::function<addr_t&()>                                                pc{};
    std::function<uint8_t(addr_t)>                                          peek{};
    std::function<void(addr_t, uint8_t)>                                    write{};
    std::function<void(std::ostream&, addr_t, size_t, bool)>                disass{};
    std::function<sptr_t<ASpace>()>                                         mmap{};
    std::function<void()>                                                   ebreak{};
    std::function<std::pair<addr_t, addr_t>(const std::string&, addr_t)>    load{};
    std::function<void(const std::string&, addr_t, addr_t)>                 save{};
    std::function<Loglevel(const std::string&)>                             loglevel{};
    std::function<uint16_t(const std::string&)>                             regvalue{};

    operator bool() const {
        return (regs && pc && peek && write && disass && mmap && ebreak && load && save && loglevel && regvalue);
    }
};

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
 *  b $8011 ra >= $20       Breakpoint at $8011 only when the value of register A is greater than $20
 *  b $8011 *d020 >= #15    Breakpoint at $8011 only when the content of memory address $D020 is greater
 *                          or equal to 15.
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
    static std::function<int()> compile(MonitoredCPU& cpu, const std::string& line);

private:
    /**
     * Compile an argument.
     * The argument must be a literal value, a register value, a memory pointer or a register pointer.
     * @param cpu  Monitored CPU;
     * @param line The argument to compile.
     * @return The compiled argument.
     */
    static fn_t compile_argument(MonitoredCPU& cpu, const std::string& line);

    static const std::pair<std::string, ex_t> operators[];
};

/**
 * Monitor command.
 */
struct Command {
    using args_t = std::vector<std::string>;
    using fn_t   = std::function<bool(Monitor&, const args_t&)>;

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
     * TODO: replace with iostream and native_handle() (C++26)
     */
    Monitor(int ifd, int ofd, MonitoredCPU&& cpu)
        : _rd{ifd, ofd},
          _cpu{cpu} {
    }

    /**
     * Enter this monitor.
     * @return true if the system must be terminated; otherwise false.
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
     * @param addr Addres to remove from the breakpoint list.
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
     * @return The next prompt string.
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
    addr_t to_addr(const std::string& str, addr_t defval);

    /**
     * Convert a string formatted as "[$#]xxx" to an unsigned number.
     * @param str The string to convert.
     * @return The converted unsigned integner.
     * @exception InvalidNumber
     * @see utils::to_number()
     */
    //FIXME: move to utils
    size_t to_count(const std::string& str);

    Readline                 _rd{};
    MonitoredCPU             _cpu{};

    bool                     _is_running{};
    std::string              _prev_line{};
    std::string              _prev_fn{};
    std::map<addr_t, cond_t> _breakpoints{};

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
