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
#include "clock.hpp"

#include <algorithm>
#include <chrono>
#include <sstream>
#include <thread>

#include "logger.hpp"


namespace caio {

using namespace std::chrono_literals;

Clock::Clock(const std::string &label, size_t freq, float delay)
    : Name{TYPE, label},
      _freq{freq},
      _delay{delay}
{
}

Clock::Clock(size_t freq, float delay)
    : Name{TYPE, {}},
      _freq{freq},
      _delay{delay}
{
}

Clock::~Clock()
{
}

size_t Clock::freq() const
{
    return _freq;
}

void Clock::freq(size_t freq)
{
    _freq = freq;
}

float Clock::delay() const
{
    return _delay;
}

void Clock::delay(float delay)
{
    _delay = delay;
}

void Clock::add(const std::shared_ptr<Clockable> &c)
{
    if (c) {
        auto it = std::find_if(_clockables.begin(), _clockables.end(), [&c](const clockable_pair_t &pair) -> bool {
            return (pair.first.get() == c.get());
        });

        if (it == _clockables.end()) {
            _clockables.push_back({c, 0});
        }
    }
}

void Clock::del(const std::shared_ptr<Clockable> &c)
{
    if (c) {
        auto it = std::find_if(_clockables.begin(), _clockables.end(), [&c](const clockable_pair_t &pair) -> bool {
            return (pair.first.get() == c.get());
        });

        if (it != _clockables.end()) {
            _clockables.erase(it);
        }
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
         * cycles have passed.
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

void Clock::stop()
{
    _stop = true;
}

void Clock::suspend(bool susp)
{
    _suspend = susp;
}

void Clock::toggle_suspend()
{
    _suspend = (_suspend ? false : true);
}

bool Clock::is_suspended() const
{
    return _suspend;
}

void Clock::sync(unsigned cycles)
{
    _sync_us = static_cast<int64_t>(cycles * 1000000.0 * _delay / static_cast<double>(_freq));
}

std::string Clock::to_string() const
{
    std::stringstream os{};

    os << Name::to_string()
       << ", freq " << _freq << "Hz"
       << ", delay " << std::setprecision(3) << _delay;

    return os.str();
}

size_t Clock::cycles(float secs) const
{
    return cycles(secs, _freq);
}

float Clock::time(size_t cycles) const
{
    return time(cycles, _freq);
}

}
