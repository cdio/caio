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
#include "mos_6526.hpp"

#include <array>

#include "utils.hpp"


namespace cemu {

uint8_t Mos6526::read(addr_t addr) const
{
    switch (addr) {
    case PRA:
        return ior(PRA);

    case PRB:
        return ior(PRB);

    case DDRA:
        return _port_A_dir;

    case DDRB:
        return _port_B_dir;

    case TALO:
        return _timer_A.counter_lo();

    case TAHI:
        return _timer_A.counter_hi();

    case TBLO:
        return _timer_B.counter_lo();

    case TBHI:
        return _timer_B.counter_hi();

    case TOD_10THS:
        return _tod.tod_tth();

    case TOD_SEC:
        return _tod.tod_sec();

    case TOD_MIN:
        return _tod.tod_min();

    case TOD_HR:
        return _tod.tod_hour();

    case SDR:
        //TODO
        return 0;

    case ICR: {
        /*
         * ICR DATA register is cleared after read.
         */
        uint8_t data = _icr_data;
        const_cast<Mos6526 *>(this)->_icr_data = 0;
        const_cast<Mos6526 *>(this)->irq_out(false);
        return data;
    }

    case CRA:
        return _timer_A.cr();

    case CRB:
        return _timer_B.cr();

    default:;
    }

    return 0;
}

void Mos6526::write(addr_t addr, uint8_t data)
{
    uint8_t odata;

    switch (addr) {
    case PRA:
        odata = (data & _port_A_dir);
        _port_A = (_port_A & ~_port_A_dir) | odata;
        iow(PRA, odata);
        break;

    case PRB:
        odata = (data & _port_B_dir);
        _port_B = (_port_B & ~_port_B_dir) | odata;
        iow(PRB, odata);
        break;

    case DDRA:
        _port_A_dir = data;
        break;

    case DDRB:
        _port_B_dir = data;
        break;

    case TALO:
        _timer_A.prescaler_lo(data);
        break;

    case TAHI:
        _timer_A.prescaler_hi(data);
        break;

    case TBLO:
        _timer_B.prescaler_lo(data);
        break;

    case TBHI:
        _timer_B.prescaler_hi(data);
        break;

    case TOD_10THS:
        return ((_timer_B.cr() & CRB_ALARM) ? _tod.alarm_tth(data) : _tod.tod_tth(data));

    case TOD_SEC:
        return ((_timer_B.cr() & CRB_ALARM) ? _tod.alarm_sec(data) : _tod.tod_sec(data));

    case TOD_MIN:
        return ((_timer_B.cr() & CRB_ALARM) ? _tod.alarm_min(data) : _tod.tod_min(data));

    case TOD_HR:
        return ((_timer_B.cr() & CRB_ALARM) ? _tod.alarm_hour(data) : _tod.tod_hour(data));

    case SDR:
        //TODO
        break;

    case ICR:
        /*
         * mos_6526_cia_preliminary_mar_1981.pdf, page 7:
         * "The MASK register provides convenient control of individual mask bits.
         * When writing to the MASK regis­ter, if bit 7 (SET/CLEAR) of the data written
         * is a ZERO, any mask bit written with a one will be cleared, while those mask
         * bits written with a zero will be unaffected. If bit 7 of the data written is
         * a ON E, any mask bit written with a one will be set, while those mask bits
         * written with a zero will be unaffected. In order for an interrupt flag to set
         * IR and generate an Interrupt Request, the corresponding MASK bit must be set."
         */
        if (data & ICR_IR) {
            _icr_mask |= (data & ~ICR_IR);
        } else {
            _icr_mask &= ~data;
        }
        break;

    case CRA:
        _timer_A.cr(data);
        break;

    case CRB:
        _timer_B.cr(data);
        break;

    default:;
    }
}


Mos6526::Tod::TodData &Mos6526::Tod::TodData::operator=(const TodData &tod)
{
    tth  = tod.tth;
    sec  = tod.sec;
    min  = tod.min;
    hour = tod.hour;

    return *this;
}

Mos6526::Tod::TodData &Mos6526::Tod::TodData::operator++()
{
    ++tth;

    if (tth == 10) {
        tth = 0;
        auto bsec = utils::bcd_to_bin(sec) + 1;

        if (bsec == 60) {
            bsec = 0;
            auto bmin = utils::bcd_to_bin(min) + 1;

            if (bmin == 60) {
                bmin = 0;
                auto pm = ((hour & PM_BIT) ? 12 : 0);
                auto bhour = utils::bcd_to_bin(hour & HOUR_MASK) + pm + 1;

                if (bhour == 24) {
                    bhour = 0;
                    pm = 0;
                } else if (bhour == 12) {
                    pm = 12;
                }

                hour = utils::bin_to_bcd(bhour - pm) | (pm == 0 ? 0 : PM_BIT);
            }

            min = utils::bin_to_bcd(bmin);
        }

        sec = utils::bin_to_bcd(bsec);
    }

    return *this;
}


bool Mos6526::Tod::tick(const Clock &clk)
{
    if (!is_running()) {
        return false;
    }

    if (_cycles == 0) {
        ++_tod;
        _cycles = static_cast<size_t>(clk.freq() * TICK_INTERVAL);

    } else {
        --_cycles;
    }

    return is_alarm();
}


size_t Mos6526::tick(const Clock &clk)
{
    if (tick(_timer_A)) {
        _icr_data |= ICR_TA;
    }

    if (tick(_timer_B)) {
        _icr_data |= ICR_TB;
    }

    if (_tod.tick(clk)) {
        _icr_data |= ICR_ALRM;
    }

    if ((_icr_data & ICR_IR) == 0 && (_icr_data & _icr_mask) != 0) {
        _icr_data |= ICR_IR;
        irq_out(true);
    }

    return 1;
}

bool Mos6526::tick(Timer &t)
{
    if (t.is_running()) {
        if (t.counter() == 0) {
            t.reload();
            t.setpb();
            if (t.is_oneshot()) {
                t.stop();
            }

            return true;

        } else {
            t.unsetpb();
            t.tick();
        }
    }

    return false;
}

void Mos6526::irq_out(bool active)
{
    if (_irq_pin != active) {
        _irq_pin = active;
        if (_trigger_irq) {
            _trigger_irq(active);
        }
    }
}

std::ostream &Mos6526::dump(std::ostream &os, addr_t base) const
{
    std::array<uint8_t, REGMAX> regs{
        read(PRA),
        read(PRB),
        read(DDRA),
        read(DDRB),
        read(TALO),
        read(TAHI),
        read(TBLO),
        read(TBHI),
        read(TOD_10THS),
        read(TOD_SEC),
        read(TOD_MIN),
        read(TOD_HR),
        read(SDR),
        read(ICR),
        read(CRA),
        read(CRB)
    };

    return utils::dump(os, regs, base);
}

}
