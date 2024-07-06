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

#include <atomic>
#include <memory>
#include <utility>
#include <vector>

#include "name.hpp"
#include "types.hpp"

namespace caio {

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
     * @return The number of clock cycles before calling this instance again; HALT to terminate the clock emulation.
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
    constexpr static const int64_t SYNC_TIME = 20000;

    using clockable_pair_t = std::pair<sptr_t<Clockable>, size_t>;

    /**
     * Initialise this clock.
     * @param label Label assigned to this clock;
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     * @see delay(float)
     */
    Clock(std::string_view label = {}, size_t freq = {}, float delay = 1.0f);

    /**
     * Initialise this clock.
     * @param freq  Frequency (in Hz);
     * @param delay The speed delay (1.0f is normal speed).
     * @see delay(float)
     */
    Clock(size_t freq, float delay = 1.0f);

    virtual ~Clock() {
    }

    /**
     * Return the frequency of this clock in Hz.
     * @return The frequency of this clock in Hz.
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
     * Return the speed delay for this clock.
     * @return The speed delay for this clock (1.0 is normal speed, 2.0 is half the speed. etc.).
     * @see delay(float)
     */
    float delay() const {
        return _delay;
    }

    /**
     * Set the speed delay for this clock.
     * The speed delay is a factor applied to the emulated clock frequecy,
     * The actual emulated frequency is "freq() / delay".
     * @param speed Speed delay (1.0 is normal speed).
     * @see delay()
     * @see freq(size_t)
     */
    void delay(float delay) {
        _delay = delay;
    }

    /**
     * Enable/disable full-speed mode.
     * When the full-speed mode is enabled the clock runs at host speed
     * (does not emulate the actual time expected from the emulated system).
     * @param on true to activate; false to deactivate.
     */
    void fullspeed(bool on) {
        _fullspeed = on;
    }

    /**
     * Return the status of the full-speed mode.
     * @return true if full-speed mode is active; false otherwise.
     */
    bool fullspeed() const {
        return _fullspeed;
    }

    /**
     * Add a clockable to this clock.
     * @param clkb Clockable to register.
     * @see del()
     */
    void add(const sptr_t<Clockable>& clkb);

    /**
     * Remove a clockable from this clock.
     * @param clkb Clockable to de-register.
     * @see add()
     */
    void del(const sptr_t<Clockable>& clkb);

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
     * @return Clockable::HALT if at least one of the clockables returned Clockable::HALT.
     */
    size_t tick();

    /**
     * Reset this clock.
     * Before calling this method the clock must be paused otherwise it does nothing.
     * @see pause()
     */
    void reset();

    /**
     * Stop this clock.
     * Instruct the clock to stop and return immediably.
     * This method does not wait for a running clock thread
     * to return back from the run() method.
     * @see run()
     */
    void stop() {
        _stop = true;
    }

    /**
     * Pause/Unpause this clock.
     * Instruct the clock to pause/unpause and return immediately.
     * This method does not wait for the clock thread to actually pause/resume.
     * @param susp true to pause; false to unpause.
     * @see paused()
     * @see toggle_pause()
     */
    void pause(bool susp = true) {
        _suspend = susp;
    }

    /**
     * Pause/Unpause this clock and wait until it takes effect.
     * Instruct the clock to pause/unpause and wait for it to actually do the change.
     * This method must be called by another thread that is not running this clock.
     * @param susp true to pause; false to unpause.
     * @see paused()
     */
    void pause_wait(bool susp = true);

    /**
     * Toggle the pause/unpause status of this clock.
     * Instruct the clock to toggle its running status and return immediately.
     * This method does not wait for the clock thread to actually pause/resume.
     * @see pause()
     * @see paused()
     */
    void toggle_pause() {
        _suspend = (_suspend ? false : true);
    }

    /**
     * Get the running status of this clock.
     * @return true if this clock is suspended; false otherwise.
     * @see pause()
     * @see toggle_pause()
     */
    bool paused() const {
        return _suspend;
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
     */
    size_t cycles(float secs) const {
        return cycles(secs, _freq);
    }

    /**
     * Get the time interval corresponding to a given number of clock cycles.
     * @param cycles Cycles.
     * @return The time interval corresponding to the specified clock cycles.
     */
    float time(size_t cycles) const {
        return time(cycles, _freq);
    }

    /**
     * Return the elapsed emulated time since the clock was started (in microseconds).
     * @return The elapsed time in microseconds.
     * @see run()
     */
    uint64_t time() const;

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
    size_t                        _freq;
    float                         _delay;
    bool                          _fullspeed{};
    uint64_t                      _ticks{};
    std::atomic_bool              _stop{};
    std::atomic_bool              _suspend{};
    std::vector<clockable_pair_t> _clockables{};
};

}
