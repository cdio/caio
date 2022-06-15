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
#include "types.hpp"

#include <execinfo.h>

#include <climits>
#include <array>
#include <cstdlib>

#include "utils.hpp"


namespace caio {

void stacktrace(std::ostream &os)
{
    std::array<void *, 256> buffer{};
    size_t size = ::backtrace(buffer.data(), buffer.size());
    if (size == 0) {
        os << "Unable to retreive stack trace" << std::endl;
        return;
    }

    os << "Stack Trace:" << std::endl;

    char **symbols = ::backtrace_symbols(buffer.data(), size);
    if (symbols == nullptr) {
        os << "Unable to retreive symbols: " << Error::to_string(errno) << std::endl << "--" << std::endl;
    }

    for (size_t i = 0; i < size; ++i) {
        if (symbols) {
            os << "  " << symbols[i] << std::endl;
        } else {
            os << "  0x" << std::hex << std::setfill('0') << std::setw(sizeof(void *) * 2)
               << reinterpret_cast<long>(buffer[i]) << std::endl;
        }
    }

    if (symbols) {
        ::free(symbols);
    }
}

std::string Error::to_string(int err)
{
    char buf[LINE_MAX];

#ifdef __linux__
    return ::strerror_r(err, buf, sizeof(buf));
#else
    ::strerror_r(err, buf, sizeof(buf));
    return buf;
#endif
}

InvalidAddress::InvalidAddress(const std::string &elem, addr_t addr, bool read)
    : Error{elem, "Invalid " + std::string{(read ? "read" : "write")} + " address: $" + utils::to_string(addr)}
{
}

}
