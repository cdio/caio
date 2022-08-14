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

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

#include "name.hpp"


namespace caio {

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
    constexpr static const int64_t MIN_SLEEP_TIME = 20000;  /* us */

    using clockable_pair_t = std::pair<std::shared_ptr<Clockable>, size_t>;

    /**
     * Initialise this clock.
     * @param label Label assigned to this clock;
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     * @see delay(float)
     */
    Clock(const std::string &label = {}, size_t freq = {}, float delay = 1.0f);

    /**
     * Initialise this clock.
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     * @see delay(float)
     */
    explicit Clock(size_t freq, float delay = 1.0f);

    virtual ~Clock();

    /**
     * @return The frequency (in Hz) of this clock.
     * @see freq(size_t)
     */
    size_t freq() const;

    /**
     * Set the frequency of this clock.
     * @param freq Frequency (in Hz).
     * @see freq()
     */
    void freq(size_t freq);

    /**
     * @return The speed delay for this clock (1.0 is normal speed, 2.0 is half the speed. etc.).
     * @see delay(float)
     */
    float delay() const;

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
    void delay(float delay);

    /**
     * Add a clockable to this clock.
     * @param clkb Clockable to register.
     * @see del()
     */
    void add(const std::shared_ptr<Clockable> &clkb);

    /**
     * Remove a clockable from this clock.
     * @param clkb Clockable to de-register.
     * @see add()
     */
    void del(const std::shared_ptr<Clockable> &clkb);

    /**
     * Execute a clock tick loop.
     * This method returns when the clock is stopped or
     * one of the registered clockables returns clockable::HALT.
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
    void stop();

    /**
     * Suspend/Unsuspend this clock;
     * @param susp true to suspend this clock; false to unsuspend it.
     * @see is_suspended()
     * @see toggle_suspend()
     */
    void suspend(bool susp = true);

    /**
     * Toggle suspend status of this clock.
     * @see is_suspended()
     * @see suspend()
     */
    void toggle_suspend();

    /**
     * @return true if this clock is suspended; false otherwise.
     * @see suspend()
     * @see toggle_suspend()
     */
    bool is_suspended() const;

    /**
     * Synchronise this clock.
     * This method must be called periodically by one of the clockables in order to
     * synchronise the speed of the emulated system with the speed of the host processor.
     * Usually, a video controller device is the clockable in charge of calling this
     * method every time a vertical screen refresh is performed (at a rate of approx 50Hz).
     * @param cycles Emulated clock cycles that have been passed since the last call to this method.
     */
    void sync(unsigned cycles);

    /**
     * @return A human readable string representation of this clock.
     */
    std::string to_string() const override;

    /**
     * Get the number of clock cycles correspoinding to a given time interval.
     * @param secs Time interval (seconds).
     * @return The clock cycles corresponding to the specified time interval.
     */
    size_t cycles(float secs) const;

    /**
     * Get the time interval corresponding to a given number of clock cycles.
     * @param cycles Cycles.
     * @return The time interval corresponding to the specified clock cycles.
     */
    float time(size_t cycles) const;

    /**
     * Get the number of clock cycles correspoinding to a given time interval.
     * @param secs Time interval (seconds);
     * @param freq Clock frequency (Hz).
     * @return The clock cycles corresponding to the specified time interval.
     */
    constexpr static size_t cycles(float secs, size_t freq) {
        return static_cast<size_t>(secs * freq);
    }

    /**
     * Get the time corresponding to a given number of cycles.
     * @param cycles Cycles;
     * @param freq   Clock frequency (Hz).
     * @return The time interval corresponding to the specified cycles.
     */
    constexpr static float time(size_t cycles, size_t freq) {
        return (cycles / static_cast<float>(freq));
    }

private:
    size_t                        _freq{};
    float                         _delay{};
    int64_t                       _sync_us{};
    std::atomic_bool              _stop{};
    std::atomic_bool              _suspend{};
    std::vector<clockable_pair_t> _clockables{};
};

}
