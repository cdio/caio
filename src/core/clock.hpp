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
#pragma once

#include "name.hpp"
#include "types.hpp"
#include "utils.hpp"

#include <atomic>
#include <memory>
#include <string>
#include <utility>
#include <vector>

namespace caio {

/**
 * Clockable.
 * A clockable is implemented by any class that must be scheduled by a clock.
 */
class Clockable {
public:
    constexpr static const size_t HALT = 0;

    virtual ~Clockable() = default;

private:
    /**
     * Clockable tick method.
     * This method is called by Clock::tick() at specific clock intervals.
     * @param clk The caller clock.
     * @return The next call interval in cycles; HALT to terminate the clock emulation.
     * @see Clock::tick()
     */
    virtual size_t tick(const class Clock& clk) = 0;

    friend class Clock;
};

/**
 * Clock.
 */
class Clock : public Name {
public:
    constexpr static const char* TYPE = "CLK";
    constexpr static const float SYNC_TIME = 1.0f / 60.0f;  /* Default sync time */

    using ClockablePair = std::pair<sptr_t<Clockable>, size_t>;

    /**
     * Initialise this clock.
     * @param label Label assigned to this clock;
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     */
    Clock(const std::string& label = {}, size_t freq = {}, float delay = 1.0f);

    /**
     * Initialise this clock.
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     */
    Clock(size_t freq, float delay = 1.0f);

    virtual ~Clock() = default;

    /**
     * Synchronise the emulated system with the real time.
     * By default the clock self synchronises each SYNC_TIME seconds;
     * devices can call this method to change this value.
     * @param stime Time since the last sync (seconds).
     */
    void sync(float stime);

    /**
     * @see sync(float)
     */
    void sync(float stime) const
    {
        const_cast<Clock*>(this)->sync(stime);
    }

    /**
     * Return the frequency of this clock (Hz).
     * @return The frequency of this clock (Hz).
     * @see freq(size_t)
     */
    size_t freq() const
    {
        return _freq;
    }

    /**
     * Set the frequency of this clock.
     * @param freq Frequency (Hz).
     * @see freq()
     */
    void freq(size_t freq)
    {
        _freq = freq;
    }

    /**
     * Get the speed delay factor.
     * @return The speed delay factor (1.0 is normal speed, 2.0 is half the speed, etc.).
     * @see delay(float)
     */
    float delay() const
    {
        return _delay;
    }

    /**
     * Set the speed delay factor.
     * The speed delay is a factor applied to the emulated clock frequecy,
     * The actual emulated frequency is "freq() / delay".
     * @param speed Speed delay (1.0 is normal speed, 2.0 is half the speed, etc.).
     * @see delay()
     * @see freq(size_t)
     */
    void delay(float delay)
    {
        _delay = delay;
    }

    /**
     * Enable/disable full-speed mode.
     * When the full-speed mode is enabled the clock runs at host speed.
     * (it does not emulate the actual time expected from the emulated system).
     * @param on true to activate; false to deactivate.
     */
    void fullspeed(bool on)
    {
        _fullspeed = on;
    }

    /**
     * Get the full-speed mode.
     * @return true if full-speed mode is active; false otherwise.
     */
    bool fullspeed() const
    {
        return _fullspeed;
    }

    /**
     * Add a clockable.
     * @param clkb Clockable to register.
     * @see del(const sptr_t<Clockable>&)
     * @see Clockable
     */
    void add(const sptr_t<Clockable>& clkb);

    /**
     * Remove a clockable.
     * @param clkb Clockable to de-register.
     * @see add(const sptr_t<Clockable>&)
     */
    void del(const sptr_t<Clockable>& clkb);

    /**
     * Start this clock.
     * This method returns when the clock is stopped or when
     * at least one of the registered clockables returns clockable::HALT.
     * @see stop()
     * @see tick()
     */
    void run();

    /**
     * Execute a clock tick cycle.
     * Tick all the registered clockables and return.
     * @return Clockable::HALT if at least one of the clockables returned Clockable::HALT.
     */
    size_t tick();

    /**
     * Reset this clock.
     * This method must be called with this clock paused
     * otherwise it does nothing.
     * @see pause()
     */
    void reset();

    /**
     * Get the running state.
     * @return true if this clock is running (the run() method was called); false otherwise.
     */
    bool is_running() const
    {
        return _is_running;
    }

    /**
     * Stop this clock.
     * Set the stop flag and return immediably.
     * This method does not wait for the clock thread
     * to return back from the run() method.
     * @see run()
     */
    void stop()
    {
        _stop = true;
    }

    /**
     * Pause/Unpause this clock.
     * Set the pause/unpause flag and return immediately.
     * This method does not wait for the clock thread to actually pause/unpause.
     * @param susp true to pause; false to unpause.
     * @see paused()
     * @see toggle_pause()
     */
    void pause(bool susp = true);

    /**
     * Toggle the pause/unpause flag.
     * Toggle the pause flag and return immediately.
     * This method does not wait for the clock thread to actually pause/unpause.
     * @see pause()
     * @see paused()
     */
    void toggle_pause();

    /**
     * Pause/Unpause this clock.
     * Set the pause/unpause flag and wait for this commadn to take effect.
     * This method must be called from a thread that is not running this clock.
     * @param suspend true to pause; false to unpause.
     * @see paused()
     */
    void pause_wait(bool suspend = true);

    /**
     * Get the pause status of this clock.
     * @return true if this clock is paused; false otherwise.
     * @see pause()
     * @see toggle_pause()
     */
    bool paused() const
    {
        return _paused;
    }

    /**
     * Return a human readable string representation of this clock.
     * @return A string representation of this clock.
     */
    std::string to_string() const override;

    /**
     * Get the number of clock cycles corresponding to a given time interval.
     * @param secs Time interval (seconds).
     * @return The clock cycles corresponding to the specified time interval.
     * @see cycles(float, size_t)
     * @see time(size_t) const
     */
    size_t cycles(float secs) const
    {
        return cycles(secs, _freq);
    }

    /**
     * Get the time interval corresponding to a given number of clock cycles.
     * @param cycles Cycles.
     * @return The time interval corresponding to the specified clock cycles.
     * @see cycles(float) const
     * @see time(size_t, size_t)
     */
    float time(size_t cycles) const
    {
        return time(cycles, _freq);
    }

    /**
     * Get the elapsed emulated time since this clock was started.
     * @return The elapsed time in microseconds.
     * @see run()
     */
    uint64_t time() const;

    /**
     * Get the number of clock cycles that corresponds to a given time interval.
     * @param secs Time interval (seconds);
     * @param freq Clock frequency (Hz).
     * @return The clock cycles that corresponds to the specified time interval.
     * @see time(size_t, size_t)
     */
    constexpr static size_t cycles(float secs, size_t freq)
    {
        return static_cast<size_t>(secs * freq);
    }

    /**
     * Get the time that corresponds to a given number of cycles.
     * @param cycles Cycles;
     * @param freq   Clock frequency (Hz).
     * @return The time interval that corresponds to the specified cycles.
     * @see cycles(float, size_t)
     */
    constexpr static float time(size_t cycles, size_t freq)
    {
        return (cycles / static_cast<float>(freq));
    }

private:
    size_t                      _freq;
    float                       _delay;
    int64_t                     _sync_time;
    ssize_t                     _sync_cycles;
    bool                        _fullspeed{};
    uint64_t                    _ticks{};
    std::atomic_bool            _stop{};
    std::atomic_bool            _suspend{};         /* Pause/Unpause command */
    std::atomic_bool            _paused{};          /* Pause/Unpause state   */
    std::atomic_bool            _is_running{};
    std::vector<ClockablePair>  _clockables{};
};

}
