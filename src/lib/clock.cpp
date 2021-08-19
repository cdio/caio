/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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
#include "clock.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

#include "logger.hpp"


namespace cemu {

using namespace std::chrono_literals;


void Clock::add(std::shared_ptr<Clockable> c)
{
    auto it = std::find_if(_clockables.begin(), _clockables.end(), [&c](const clockable_pair_t &pair) -> bool {
        return (pair.first.get() == c.get());
    });

    if (it == _clockables.end()) {
        _clockables.push_back({c, 0});
    }
}

void Clock::del(std::shared_ptr<Clockable> c)
{
    auto it = std::find_if(_clockables.begin(), _clockables.end(), [&c](const clockable_pair_t &pair) -> bool {
        return (pair.first.get() == c.get());
    });

    if (it != _clockables.end()) {
        _clockables.erase(it);
    }
}

void Clock::run()
{
    auto start = std::chrono::steady_clock::now();

    while (!_stop)  {
        while (_suspend && !_stop) {
            std::this_thread::sleep_for(200ms);
            start = std::chrono::steady_clock::now();
        }

        if (tick() == Clockable::HALT) {
            return;
        }

        /*
         * An emulated clock should be running at the specified frequency,
         * unfortunatelly the clock granularity of the host system does not allow this.
         *
         * The workaround used here lets the emulated system run at host speed for a number
         * of clock ticks, then it sleeps for some time until the correct emulated clock
         * cycles has passed.
         * This synchronisation is managed by a clockable device (like a video controller).
         */
        if (_sync_us != 0) {
            auto end = std::chrono::steady_clock::now();
            auto elapsed = std::chrono::duration_cast<std::chrono::microseconds>(end - start).count();
            auto diff = std::chrono::microseconds(_sync_us - elapsed);

            if (diff > 0ms) {
                std::this_thread::sleep_for(diff);
#if 0
            } else if (diff < -1ms) {
                log.warn("%s(%s): Host system too slow: Delay of %lldus\n", type().c_str(), label().c_str(),
                    -diff.count());
#endif
            }

            _sync_us = 0;
            start = std::chrono::steady_clock::now();
        }
    }
}

size_t Clock::tick()
{
    for (auto &pair : _clockables) {
        auto &cycles = pair.second;
        if (cycles == 0) {
            cycles = pair.first->tick(*this);
            if (cycles == Clockable::HALT) {
                /*
                 * System Halt is requested.
                 */
                return Clockable::HALT;
            }
        }

        --cycles;
    }

    return ~Clockable::HALT;
}

std::string Clock::to_string() const
{
    std::stringstream os{};

    os << Name::to_string()
       << ", freq " << _freq << "Hz"
       << ", delay " << std::setprecision(3) << _delay;

    return os.str();
}

}
