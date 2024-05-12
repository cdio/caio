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

#include <cstdint>
#include <functional>
#include <string>

#include "clock.hpp"
#include "device.hpp"
#include "gpio.hpp"
#include "pin.hpp"

namespace caio {

/**
 * MOS6526 (CIA) emulator.
 * @see mos_6526_cia_preliminary_mar_1981.pdf
 */
class Mos6526 : public Device, public Gpio, public Clockable {
public:
    constexpr static const char* TYPE = "MOS6526";

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
        SDR         = 12,   /* Serial data register         */
        ICR         = 13,   /* Interrupt control register   */
        CRA         = 14,   /* Control register A           */
        CRB         = 15,   /* Control register B           */

        REGMAX
    };

    enum class TimerMode {
        PHI2        = 0,    /* CLK                          */
        CNT         = 1,    /* /CNT negative transition     */
        TA          = 2,    /* Timer A underflow            */
        TA_CNT      = 3     /* Timer A underflow + /CNT low */
    };

    constexpr static uint8_t ICR_TA        = 0x01;
    constexpr static uint8_t ICR_TB        = 0x02;
    constexpr static uint8_t ICR_ALRM      = 0x04;
    constexpr static uint8_t ICR_SP        = 0x08;
    constexpr static uint8_t ICR_FLG       = 0x10;
    constexpr static uint8_t ICR_IR        = 0x80;
    constexpr static uint8_t ICR_SRC_MASK  = ICR_FLG | ICR_SP | ICR_ALRM | ICR_TB | ICR_TA;

    constexpr static uint8_t CRx_START     = 0x01;
    constexpr static uint8_t CRx_PBON      = 0x02;
    constexpr static uint8_t CRx_PBTOGGLE  = 0x04;          /* 0: Pulse Port B bit; 1: Toggle Port B bit    */
    constexpr static uint8_t CRx_ONESHOT   = 0x08;          /* 0: Continuous, 1: One-shot                   */
    constexpr static uint8_t CRx_FORCELOAD = 0x10;          /* Timer force load. No storage for this bit    */

    constexpr static uint8_t CRA_INMODE    = 0x20;          /* Timer A mode                                 */
    constexpr static uint8_t CRA_SPMODE    = 0x40;
    constexpr static uint8_t CRA_TODIN     = 0x80;

    constexpr static uint8_t CRB_INMODE    = 0x20 | 0x40;   /* Timer B mode                                 */
    constexpr static uint8_t CRB_ALARM     = 0x80;          /* 0: Set ToD clock; 1: Set ToD alarm           */

    constexpr static uint8_t P0            = 0x01;
    constexpr static uint8_t P1            = 0x02;
    constexpr static uint8_t P2            = 0x04;
    constexpr static uint8_t P3            = 0x08;
    constexpr static uint8_t P4            = 0x10;
    constexpr static uint8_t P5            = 0x20;
    constexpr static uint8_t P6            = 0x40;
    constexpr static uint8_t P7            = 0x80;

    constexpr static uint8_t PB6           = P6;            /* Port B bit for timer A                       */
    constexpr static uint8_t PB7           = P7;            /* Port B bit for timer B                       */

    class Timer {
    public:
        Timer(Mos6526& dev, uint8_t pbit);

        void     reset();
        uint8_t  counter_hi() const;
        uint8_t  counter_lo() const;
        uint16_t counter() const;
        void     reload();
        void     prescaler_hi(uint8_t prehi);
        void     prescaler_lo(uint8_t prelo);
        bool     is_started() const;
        bool     is_oneshot() const;
        bool     is_pbon() const;
        bool     is_pbtoggle() const;
        bool     is_underflow() const;
        uint8_t  cr() const;
        void     cr(uint8_t data);
        void     stop();
        void     tick();
        void     setpb();
        void     unsetpb();

    private:
        Mos6526& _dev;
        uint8_t  _pbit;

        uint8_t  _cr;
        uint16_t _counter;
        uint16_t _prescaler;
        bool     _is_underflow;
    };

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

            bool     operator==(const TodData& tod) const;
            TodData& operator=(const TodData& tod);
            TodData& operator++();
        };

        Tod();

        void    tod_hour(uint8_t hour);
        void    tod_min(uint8_t min);
        void    tod_sec(uint8_t sec);
        void    tod_tth(uint8_t tth);
        uint8_t tod_hour() const;
        uint8_t tod_min() const;
        uint8_t tod_sec() const;
        uint8_t tod_tth() const;
        void    alarm_hour(uint8_t hour);
        void    alarm_min(uint8_t min);
        void    alarm_sec(uint8_t sec);
        void    alarm_tth(uint8_t tth);
        void    start();
        void    stop();
        bool    tick(const Clock& clk);

    private:
        bool             _is_running{};
        TodData          _tod{};
        TodData          _alarm{};
        mutable TodData  _latch{};
        size_t           _cycles{};
    };

    /**
     * Initalise this CIA instance.
     * @param label Label assigned to this device.
     */
    explicit Mos6526(const std::string& label = {});

    virtual ~Mos6526();

    /**
     * @see Device::reset()
     */
    void reset() override;

    /**
     * @see Device::size()
     */
    size_t size() const override;

    /**
     * @see Device::read()
     */
    uint8_t read(addr_t addr, ReadMode mode = ReadMode::Read) override;

    /**
     * @see Device::write()
     */
    void write(addr_t addr, uint8_t data) override;

    /**
     * @see Device::dump()
     */
    std::ostream& dump(std::ostream& os, addr_t base = 0) const override;

    /**
     * Set the IRQ pin callback.
     * The IRQ pin callback is called when the status of the IRQ output pin of this device is changed.
     * @param irq_out IRQ output pin callback.
     */
    void irq(const OutputPinCb& irq_out);

private:
    /**
     * @see Clockable::tick()
     */
    size_t tick(const Clock& clk) override;

    bool tick(Timer& timer, TimerMode mode);

    TimerMode timer_A_mode() const;

    TimerMode timer_B_mode() const;

    void irq_out(bool active);

    OutputPinCb _irq_out{};

    Timer       _timer_A;
    Timer       _timer_B;

    Tod         _tod{};

    uint8_t     _port_A_dir{};  /* 0: Input, 1: Output  */
    uint8_t     _port_B_dir{};  /* 0: Input, 1: Output  */

    uint8_t     _icr_data{};
    uint8_t     _icr_mask{};
};

}
