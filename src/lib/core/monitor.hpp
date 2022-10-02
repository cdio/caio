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

#include <cstdint>
#include <functional>
#include <iostream>
#include <map>
#include <memory>
#include <string>
#include <utility>
#include <vector>

#include "aspace.hpp"
#include "logger.hpp"
#include "types.hpp"


namespace caio {

/**
 * Monitored CPU callbacks.
 */
struct MonitoredCpu {
    std::function<void(std::ostream &)>                       regs{};
    std::function<addr_t &()>                                 pc{};
    std::function<uint8_t(addr_t)>                            read{};
    std::function<void(addr_t, uint8_t)>                      write{};
    std::function<void(std::ostream &, addr_t, size_t, bool)> disass{};
    std::function<std::shared_ptr<ASpace>()>                  mmap{};
    std::function<void()>                                     ebreak{};
    std::function<addr_t(const std::string &, addr_t &)>      load{};
    std::function<void(const std::string &, addr_t, addr_t)>  save{};
    std::function<Logger::Level(const std::string &)>         loglevel{};
    std::function<uint16_t(const std::string &)>              regvalue{};
};

/**
 * CPU Monitor.
 */
class Monitor {
public:
    /**
     * Conditional Breakpoint expression compiler.
     * A conditional expression is a string formatted as "value1 operator value2"
     * where operator is one of: "<", ">", "<=", ">=", "==", "!=", "&", "|"
     * value1 and value2 are expressions containing one of:
     * - Literal value:      "$D020", "D020", "#53280", "#$A7"
     * - Memory reference:   "*D020", "*$D020", "*#53280"
     * - Register reference: "ra", "rx", "ry", "rs", "rp", "rp.n" "rp.v", "rp.b", "rp.d", "rp.i", "rp.z", "rp.c"
     *   Register references can be used along with "*" to get the content of a zero-page address (for exmaple: "*ra").
     *
     * Examples of a breakpoint using the monitor command line:
     *  b $8011 ra >= $20
     *  b $8011 *d020 >= #15
     */
    class Expr {
    public:
        using fn_t = std::function<int(MonitoredCpu &)>;
        using ex_t = std::function<int(MonitoredCpu &cpu, fn_t &, fn_t &)>;

        Expr() = delete;

        ~Expr() = delete;

        /**
         * Compile a user defined condition.
         * @param cpu  Monitored CPU;
         * @param expr Expression to compile.
         * @return On success the first element contains the compiled expression; on error the second element
         * contains the exception. To detect an error condition the bool method of the first element must be called.
         */
        static std::pair<std::function<int()>, std::exception> compile(MonitoredCpu &cpu, const std::string &line);

    private:
        /**
         * Compile an argument.
         * The argument must be a literal value, a memory reference or a register reference.
         * @param cpu  Monitored CPU;
         * @param expr The argument to compile.
         * @return The compiled argument.
         */
        static fn_t compile_argument(MonitoredCpu &cpu, const std::string &line);

        static const std::vector<std::pair<std::string, ex_t>> operators;
    };

    using cond_t = std::pair<std::function<int()>, std::string>;

    /**
     * Initialise this monitor.
     * @param is  Input stream used to communicate with the user;
     * @param os  Output stream used to communicate with the user;
     * @param cpu CPU callbacks.
     * @see MonitoredCpu
     */
    Monitor(std::istream &ios, std::ostream &os, MonitoredCpu &&cpu);

    virtual ~Monitor();

    /**
     * Start this monitor.
     * @return true if the system must be terminated; otherwise false.
     */
    bool run();

    /**
     * Add a breakpoint.
     * @param addr Breakpoint address;
     * @param cond Condition that triggers the breakpoint (emppy for unconditional breakpoints).
     */
    void add_breakpoint(addr_t addr, const cond_t &cond = {});

    /**
     * Remove a breakpoint.
     * @param addr Addres to remove from the breakpoint list.
     */
    void del_breakpoint(addr_t addr);

    /**
     * Detect whether there is a breakpoint at a specified address.
     * @param addr Address to detect.
     * @return true if there is breakpoint; false otherwise.
     */
    bool is_breakpoint(addr_t addr) const;

private:
    constexpr static const char *PROMPT_PREFIX = "";
    constexpr static const char *PROMPT_SUFFIX = "> ";

    using cmdargs_t = std::vector<std::string>;
    using fn_t = std::function<bool(const cmdargs_t &)>;

    struct Command {
        const std::string command{};
        const std::string short_command{};
        const std::string args{};
        const std::string help{};
        fn_t              fn{};
    };

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
    addr_t to_addr(const std::string &str, addr_t defval);

    /**
     * Convert a string formatted as "[$#]xxx" to an unsigned number.
     * @param str The string to convert.
     * @return The converted unsigned integner.
     * @exception InvalidNumber
     * @see utils::to_number()
     */
    //FIXME: move to utils
    size_t to_count(const std::string &str);

    bool assemble    (const cmdargs_t &args);
    bool disassemble (const cmdargs_t &args);
    bool dump        (const cmdargs_t &args);
    bool registers   (const cmdargs_t &args);
    bool mmap        (const cmdargs_t &args);

    bool bp_add      (const cmdargs_t &args);
    bool bp_del      (const cmdargs_t &args);
    bool bp_clear    (const cmdargs_t &args);
    bool bp_list     (const cmdargs_t &args);

    bool go          (const cmdargs_t &args);
    bool step        (const cmdargs_t &args);

    bool load        (const cmdargs_t &args);
    bool save        (const cmdargs_t &args);

    bool loglevel    (const cmdargs_t &args);

    bool quit        (const cmdargs_t &args);
    bool help        (const cmdargs_t &args);

    std::istream            &_is;
    std::ostream            &_os;
    MonitoredCpu             _cpu;
    std::vector<Command>     _commands;

    bool                     _is_running{};
    std::string              _prev_line{};
    std::string              _prev_fn{};
    std::map<addr_t, cond_t> _breakpoints{};

};

}
