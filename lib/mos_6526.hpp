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

#include <cstdint>
#include <functional>
#include <string>

#include "clock.hpp"
#include "device.hpp"
#include "gpio.hpp"


namespace cemu {

/**
 * MOS6526 (CIA) emulator.
 * @see mos_6526_cia_preliminary_mar_1981.pdf
 */
class Mos6526 : public Device, public Gpio, public Clockable {
public:
    constexpr static const char *TYPE = "MOS6526";

    enum Registers {
        PRA         = 0,    /* Port A                       */
        PRB         = 1,    /* Port B                       */
        DDRA        = 2,    /* Data Direction Regsiter A    */
        DDRB        = 3,    /* Data Direction Regsiter B    */
        TALO        = 4,    /* Timer A Low Register         */
        TAHI        = 5,    /* Timer A High Register        */
        TBLO        = 6,    /* Timer B Low Register         */
        TBHI        = 7,    /* Timer B High Register        */
        TOD_10THS   = 8,    /* 10ths of seconds register    */
        TOD_SEC     = 9,    /* Seconds register             */
        TOD_MIN     = 10,   /* Minutes register             */
        TOD_HR      = 11,   /* Hours AM/PM register         */
        SDR         = 12,   /* Serial date register         */
        ICR         = 13,   /* Interrupt control register   */
        CRA         = 14,   /* Control register A           */
        CRB         = 15,   /* Control register B           */

        REGMAX
    };

    constexpr static uint8_t ICR_TA        = 0x01;
    constexpr static uint8_t ICR_TB        = 0x02;
    constexpr static uint8_t ICR_ALRM      = 0x04;
    constexpr static uint8_t ICR_SP        = 0x08;
    constexpr static uint8_t ICR_FLG       = 0x10;
    constexpr static uint8_t ICR_IR        = 0x80;

    constexpr static uint8_t CRx_START     = 0x01;
    constexpr static uint8_t CRx_PBON      = 0x02;
    constexpr static uint8_t CRx_OUTTOGGLE = 0x04;
    constexpr static uint8_t CRx_RUNMODE   = 0x08;
    constexpr static uint8_t CRx_FORCELOAD = 0x10;
    constexpr static uint8_t CRx_INMODE    = 0x20;
    constexpr static uint8_t CRx_SPMODE    = 0x40;
    constexpr static uint8_t CRx_TODIN     = 0x80;
    constexpr static uint8_t CRB_INMODE    = 0x20 | 0x40;
    constexpr static uint8_t CRB_ALARM     = 0x80;

    constexpr static uint8_t PB6           = 0x40;
    constexpr static uint8_t PB7           = 0x80;

    /**
     * Initalise this CIA instance.
     * @param label Label assigned to this device.
     */
    Mos6526(const std::string &label = {})
        : Device{TYPE, label},
          _timer_A{_port_B, PB6},
          _timer_B{_port_B, PB7} {
    }

    virtual ~Mos6526() {
    }

    /**
     * @see Device::size()
     */
    size_t size() const override {
        return REGMAX;
    }

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr) const override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream &dump(std::ostream &os, addr_t base = 0) const override;

    /**
     * Set the IRQ pin callback.
     * The IRQ pin callback is called when the status of the IRQ output pin of this device is changed.
     * @param trigger_irq IRQ pin callback.
     */
    void irq(std::function<void(bool)> trigger_irq) {
        _trigger_irq = trigger_irq;
    }

private:
    /**
     * Interval timer.
     */
    class Timer {
    public:
        Timer(uint8_t &portb, uint8_t pbit)
            : _portb{portb},
              _pbit{pbit} {
        }

        ~Timer() {
        }

        uint8_t counter_hi() const {
            return (_counter >> 8);
        }

        uint8_t counter_lo() const {
            return (_counter & 0x00FF);
        }

        uint16_t counter() const {
            return _counter;
        }

        void reload() {
            _counter = _prescaler;
        }

        void prescaler_hi(uint8_t prehi) {
            _prescaler = (_prescaler & 0x00FF) | (static_cast<uint16_t>(prehi) << 8);
            if (!is_running() || is_forceload()) {
                _counter = _prescaler;
            }
        }

        void prescaler_lo(uint8_t prelo) {
            _prescaler = (_prescaler & 0xFF00) | prelo;
        }

        bool is_running() const {
            return (_cr & CRx_START);
        }

        bool is_forceload() const {
            return (_cr & CRx_FORCELOAD);
        }

        bool is_oneshot() const {
            return (_cr & CRx_RUNMODE);
        }

        bool is_pbon() const {
            return (_cr & CRx_PBON);
        }

        uint8_t cr() const {
            return _cr;
        }

        void cr(uint8_t data) {
            _cr = data;
        }

        void stop() {
            _cr &= ~CRx_START;
        }

        void tick() {
            if (_counter > 0) {
                --_counter;
            }
        }

        void setpb() {
            if (is_pbon()) {
                if (_cr & CRx_OUTTOGGLE) {
                    /* Toggle port-B bit */
                    _portb ^= _pbit;
                } else {
                    /* Set port-B bit active for one cycle. See unsetpb() */
                    _portb |= _pbit;
                }
            }
        }

        void unsetpb() {
            /* Unset the port-B bit. This must be called 1 cycle after setpb() */
            if (is_pbon() && !(_cr & CRx_OUTTOGGLE) && (_portb & _pbit)) {
                _portb &= ~_pbit;
            }
        }

    private:
        uint8_t &_portb;        /* Output port      */
        uint8_t  _pbit;         /* Output pin       */

        uint8_t  _cr{};         /* Control register */
        uint16_t _counter{};    /* Counter          */
        uint16_t _prescaler{};  /* Prescaler        */
    };


    /**
     * Time Of Day (TOD) RT clock.
     */
    class Tod {
    public:
        constexpr static const float TICK_INTERVAL = 0.1f;     /* TOD resolution is 1/10th seconds */

        struct TodData {
            constexpr static const uint8_t TTH_MASK  = 0x0F;
            constexpr static const uint8_t SEC_MASK  = 0x7F;
            constexpr static const uint8_t MIN_MASK  = 0x7F;
            constexpr static const uint8_t HOUR_MASK = 0x1F;
            constexpr static const uint8_t PM_BIT    = 0x80;

            uint8_t hour{};
            uint8_t min{};
            uint8_t sec{};
            uint8_t tth{};

            bool operator==(const TodData &tod) const {
                return (tth == tod.tth && sec == tod.sec && min == tod.min && hour == tod.hour);
            }

            TodData &operator=(const TodData &tod);

            TodData &operator++();
        };


        void tod_hour(uint8_t hour) {
            stop();
            _tod.hour = (hour & TodData::HOUR_MASK) | (hour & TodData::PM_BIT);
        }

        void tod_min(uint8_t min) {
            _tod.min = min & TodData::MIN_MASK;
        }

        void tod_sec(uint8_t sec) {
            _tod.sec = sec & TodData::SEC_MASK;
        }

        void tod_tth(uint8_t tth) {
            _tod.tth = tth & TodData::TTH_MASK;
            start();
        }

        uint8_t tod_hour() const {
            _latch = _tod;
            return _latch.hour;
        }

        uint8_t tod_min() const {
            return _latch.min;
        }

        uint8_t tod_sec() const {
            return _latch.sec;
        }

        uint8_t tod_tth() const {
            return _latch.tth;
        }

        void alarm_hour(uint8_t hour) {
            _alarm.hour = (hour & TodData::HOUR_MASK) | (hour & TodData::PM_BIT);
        }

        void alarm_min(uint8_t min) {
            _alarm.min = min & TodData::MIN_MASK;
        }

        void alarm_sec(uint8_t sec) {
            _alarm.sec = sec & TodData::SEC_MASK;
        }

        void alarm_tth(uint8_t tth) {
            _alarm.tth = tth & TodData::TTH_MASK;
        }

        void start() {
            _is_running = true;
        }

        void stop() {
            _is_running = false;
        }

        bool is_running() const {
            return _is_running;
        }

        bool is_alarm() const {
            return (_tod == _alarm);
        }

        bool tick(const Clock &clk);

    private:
        std::atomic_bool _is_running{true};
        TodData          _tod{};
        TodData          _alarm{};
        mutable TodData  _latch{};
        size_t           _cycles{};
    };


    /**
     * @see Clockable::tick()
     */
    size_t tick(const Clock &clk) override;

    /**
     * Tick event for a specified timer.
     * @param t Timer to tick.
     * @return true if the timer expired; false otherwise.
     */
    bool tick(Timer &t);

    /**
     * Set the status of the IRQ output pin.
     * @param active true if the IRQ pin must be activated; false otherwise.
     * @see _trigger_irq
     */
    void irq_out(bool active);

    /**
     * IRQ output trigger.
     */
    std::function<void(bool)> _trigger_irq{};

    uint8_t _port_A{};
    uint8_t _port_A_dir{};      /* 1=Input/Output; 0=Input  */

    uint8_t _port_B{};
    uint8_t _port_B_dir{};      /* 1=Input/Output; 0=Input  */

    Timer   _timer_A;
    Timer   _timer_B;

    Tod     _tod{};

    uint8_t _icr_data{};        /* ICR read register        */
    uint8_t _icr_mask{};        /* ICR write register       */

    bool    _irq_pin{};         /* IRQ output pin           */
};

}
