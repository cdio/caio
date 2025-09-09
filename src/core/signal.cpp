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

#include <iterator>
#include <numeric>
#include <sstream>

namespace caio {
namespace signal {

fp_t mean(samples_fp samples)
{
    const fp_t sum = std::accumulate(samples.begin(), samples.end(), 0.0);
    return (sum / samples.size());
}

std::string to_string(samples_fp samples)
{
    std::stringstream os{};
    os << "[ ";
    for (auto sample : samples) {
        os << sample << " ";
    }
    os << "]";
    return os.str();
}

std::ostream& dump(std::ostream& os, samples_fp samples, std::string_view name, fp_t fc1, fp_t fc2, fp_t Q, fp_t fs)
{
    std::format_to(std::ostream_iterator<char>(os),
        "{} = struct('fs', {}, 'fc1', {}, 'fc2', {}, 'Q', {}, 'v', {});\n",
        name, fs, fc1, fc2, Q, to_string(samples));
    return os;
}

}
}
