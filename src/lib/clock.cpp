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
#include "clock.hpp"

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <sstream>
#include <thread>

#include "logger.hpp"
#include "utils.hpp"


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
    ssize_t sched_cycle = 0;
    ssize_t sync_cycles = cycles(SYNC_TIME / 1000000.0f);
    int64_t start = utils::now();

    while (!_stop)  {
        while (_suspend && !_stop) {
            /*
             * The emulated system is paused, wait for 200ms and check again.
             */
            std::this_thread::sleep_for(200ms);
            start = utils::now();
        }

        if (tick() == Clockable::HALT) {
            return;
        }

        ++sched_cycle;

        if (sched_cycle == sync_cycles) {
            /*
             * Calculate the time required to the host system
             * to execute sync_cycles emulated clock cycles.
             */
            int64_t end = utils::now();
            int64_t elapsed = end - start;
            int64_t wait_time = SYNC_TIME - elapsed;
            if (wait_time < 0) {
                /*
                 * Slow host system.
                 */
#if 0
                log.warn("%s: Slow host system, delayed of %" PRIu64 "us\n",
                    Name::to_string().c_str(),
                    -wait_time);
#endif
                sched_cycle = 0;
                start = utils::now();
                continue;
            }

            /*
             * Suspend the execution of this clock until the
             * time expected by the emulated system is reached.
             */
            std::this_thread::sleep_for(std::chrono::microseconds{static_cast<int64_t>(wait_time * _delay)});

            /*
             * Don't expect the operating system's scheduler to be real-time,
             * this thread probably slept far more than requested.
             * Adjust for this condition.
             */
            start = utils::now();
            ssize_t delayed_cycles = cycles((start - end) / (_delay * 1000000.0f));
            ssize_t wait_cycles = cycles(wait_time / 1000000.0f);
            ssize_t extra_cycles = delayed_cycles - wait_cycles;
            sched_cycle = -extra_cycles;
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
