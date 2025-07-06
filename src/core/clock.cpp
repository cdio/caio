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
#include "clock.hpp"

#include <algorithm>
#include <chrono>
#include <cinttypes>
#include <thread>

#include "logger.hpp"
#include "utils.hpp"

namespace caio {

using namespace std::chrono_literals;

Clock::Clock(std::string_view label, size_t freq, float delay)
    : Name{TYPE, label},
      _freq{freq},
      _delay{delay}
{
    sync(SYNC_TIME);
}

Clock::Clock(size_t freq, float delay)
    : Clock{"", freq, delay}
{
}

Clock::~Clock()
{
}

void Clock::add(const sptr_t<Clockable>& clkb)
{
    if (clkb) {
        auto it = std::find_if(_clockables.begin(), _clockables.end(), [&clkb](const ClockablePair& pair) -> bool {
            return (pair.first.get() == clkb.get());
        });

        if (it == _clockables.end()) {
            _clockables.push_back({clkb, 0});
        }
    }
}

void Clock::del(const sptr_t<Clockable>& clkb)
{
    if (clkb) {
        auto it = std::find_if(_clockables.begin(), _clockables.end(), [&clkb](const ClockablePair& pair) -> bool {
            return (pair.first.get() == clkb.get());
        });

        if (it != _clockables.end()) {
            _clockables.erase(it);
        }
    }
}

void Clock::run()
{
    ssize_t sched_cycle = 0;
    int64_t start = utils::now();

    _stop = false;

    while (!_stop)  {
        while (_suspend && !_stop) {
            /*
             * The emulated system is paused, wait for 200ms and check again.
             */
            std::this_thread::sleep_for(200ms);
            start = utils::now();
        }

        ++_ticks;

        if (tick() == Clockable::HALT) {
            return;
        }

        if (_fullspeed) {
            continue;
        }

        ++sched_cycle;

        if (sched_cycle < _sync_cycles) {
            continue;
        }

        /*
         * Calculate the time required for the host system
         * to execute _sync_cycles emulated clock cycles.
         */
        const int64_t end = utils::now();
        const int64_t run_time = end - start;
        const int64_t wait_time = _sync_time - run_time;
        if (wait_time < 0) {
            /*
             * Slow host system.
             */
//            log.warn("{}: Slow host or late sync, delay of {} us\n", Name::to_string(), wait_time);
            start = end;
            sched_cycle = 0;
            continue;
        }

        /*
         * Suspend the execution of this clock until the
         * time expected by the emulated system is reached.
         */
        std::this_thread::sleep_for(std::chrono::microseconds{static_cast<int64_t>(wait_time * _delay)});

        /*
         * This thread probably slept far more than requested.
         * Adjust for this situation.
         */
        start = utils::now();
        const ssize_t sleep_cycles = cycles((start - end) / (_delay * 1'000'000.0f));
        const ssize_t wait_cycles = cycles(wait_time / 1'000'000.0f);
        const ssize_t extra_cycles = sleep_cycles - wait_cycles;

        sched_cycle = -extra_cycles;
    }
}

size_t Clock::tick()
{
    for (auto& pair : _clockables) {
        auto& cycles = pair.second;
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

void Clock::reset()
{
    if (paused()) {
        for (auto& pair : _clockables) {
            auto& cycles = pair.second;
            cycles = 0;
        }
    }
}

void Clock::pause_wait(bool susp)
{
    if (susp != paused()) {
        pause(susp);
        while (paused() != susp) {
            std::this_thread::yield();
        }
    }
}

std::string Clock::to_string() const
{
    return std::format("{}, freq {} Hz, delay {:1.1f}", Name::to_string(), _freq, _delay);
}

uint64_t Clock::time() const
{
    return (time(_ticks) * 1'000'000);
}

}
