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
#pragma once

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

#include "name.hpp"


namespace cemu {

/**
 * Clock frequency on PAL systems (Hz).
 */
constexpr static const unsigned CLOCK_FREQ_PAL = 985248;


/**
 * Clockable.
 * A clockable is implemented by any class that must be scheduled by a clock.
 */
class Clockable {
public:
    constexpr static const size_t HALT = 0;

    Clockable() {
    }

    virtual ~Clockable() {
    }

private:
    /**
     * Clockable tick method.
     * This method is called by Clock::tick() at specific clock intervals.
     * @param clk The caller clock.
     * @return The number of clock cycles that must pass before this instance is called again;
     * Clockable::HALT if the caller clock must be stopped (usually this happens when the application is terminated).
     * @see Clock::tick()
     */
    virtual size_t tick(const class Clock &clk) = 0;

    friend class Clock;
};


/**
 * Clock.
 */
class Clock : public Name {
public:
    constexpr static const char *TYPE = "CLK";

    using clockable_pair_t = std::pair<std::shared_ptr<Clockable>, size_t>;

    /**
     * Initialise this clock.
     * @param label Label assigned to this clock;
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     * @see delay(float)
     */
    Clock(const std::string &label = {}, size_t freq = {}, float delay = 1.0f)
        : Name{TYPE, label},
          _freq{freq},
          _delay{delay} {
    }

    /**
     * Initialise this clock.
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     * @see delay(float)
     */
    Clock(size_t freq, float delay = 1.0f)
        : Name{TYPE, {}},
          _freq{freq},
          _delay{delay} {
    }

    virtual ~Clock() {
    }

    /**
     * @return The frequency (in Hz) of this clock.
     * @see freq(size_t)
     */
    size_t freq() const {
        return _freq;
    }

    /**
     * Set the frequency of this clock.
     * @param freq Frequency (in Hz).
     * @see freq()
     */
    void freq(size_t freq) {
        _freq = freq;
    }

    /**
     * @return The speed delay for this clock (1.0 is normal speed, 2.0 is half the speed. etc.).
     * @see delay(float)
     */
    float delay() const {
        return _delay;
    }

    /**
     * Set the speed delay for this clock.
     * The speed delay is a factor applied to the emulated clock frequecy,
     * The actual emulated frequency is "freq() / delay", that is
     * a factor of 1 makes the clock to run at nominal frequency,
     * a factor of 2 makes the clock to run at half the frequency, and so on.
     * Useful for debugging purposes.
     * @param speed Speed delay (1.0 is normal speed).
     * @see delay()
     * @see freq(size_t)
     */
    void delay(float delay) {
        _delay = delay;
    }

    /**
     * Add a clockable to this clock.
     * @param clkb Clockable to register.
     * @see del()
     */
    void add(std::shared_ptr<Clockable> clkb);

    /**
     * Remove a clockable from this clock.
     * @param clkb Clockable to de-register.
     * @see add()
     */
    void del(std::shared_ptr<Clockable> clkb);

    /**
     * Execute a clock tick loop.
     * This method returns when one of the registered clockables returns clockable::HALT.
     * @see stop()
     * @see tick()
     */
    void run();

    /**
     * Execute a clock tick cycle.
     * Tick all the registered clockables and return.
     * @return Clockable::HALT if at least one of the clockables returned clockable::HALT.
     */
    size_t tick();

    /**
     * Stop this clock.
     * Instruct the clock to stop and return immediably;
     * this method does not wait for a running clock thread
     * to return back from the run() method.
     * @see run()
     */
    void stop() {
        _stop = true;
    }

    /**
     * Suspend/Unsuspend this clock;
     * @param susp true to suspend this clock; false to unsuspend it.
     * @see is_suspended()
     * @see toggle_suspend()
     */
    void suspend(bool susp = true) {
        _suspend = susp;
    }

    /**
     * Toggle suspend status of this clock.
     * @see is_suspended()
     * @see suspend()
     */
    void toggle_suspend() {
        _suspend = (_suspend ? false : true);
    }

    /**
     * @return true if this clock is suspended; false otherwise.
     * @see suspend()
     * @see toggle_suspend()
     */
    bool is_suspended() const {
        return _suspend;
    }

    /**
     * Synchronise this clock.
     * This method must be called by one of the clockables in order to delay
     * the speed of the emulated system which is running at host processor's speed.
     * When this value is set and a clock tick round is ended this clock is put
     * to sleep for the specified amount of time. After that, this clock restarts its
     * tick round again. In order to achieve synchronisation this method must be called
     * continiously by the clockable.
     * Usually, the video controller device is the clockable in charge of calling this
     * method to synchronise the system clock with the emulated vertical screen refresh
     * (usually at a rate of 50Hz).
     * @param cycles Clock cycles to sleep before starting the next clock tick round.
     */
    void sync(unsigned cycles) {
        _sync_us = static_cast<int64_t>(cycles * 1000000.0 * _delay / static_cast<double>(_freq));
    }

    /**
     * @return A human readable string representation of this clock.
     */
    std::string to_string() const override;

private:
    size_t                        _freq{};
    float                         _delay{};
    uint64_t                      _sync_us{};
    std::atomic_bool              _stop{};
    std::atomic_bool              _suspend{};
    std::vector<clockable_pair_t> _clockables{};
};

}
