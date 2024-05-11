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
#include "signal.hpp"

namespace caio {
namespace signal {

std::ostream& dump(std::ostream& os, const samples_fp& samples, const std::string& name, fp_t fc1, fp_t fc2,
    fp_t Q, fp_t fs)
{
    os << name << " = struct('fs', "   << fs  << ", "
               <<           "'fc1', "  << fc1 << ", "
               <<           "'fc2', "  << fc2 << ", "
               <<           "'Q', "    << Q   << ", "
               <<           "'v', [ ";

    for (fp_t value : samples) {
        os << value << " ";
    }

    os << " ]);\n";

    return os;
}

}
}
