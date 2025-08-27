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

#include "latch.hpp"
#include "pullup.hpp"

#include <functional>

namespace caio {

/**
 * Input Pin.
 * An Input pin is a 1 bit latch register.
 */
using InputPin = Latch;

/**
 * IRQ input pin.
 * An IRQ pin is a pull-up pin where several interrupt
 * sources can drive the line low.
 */
using IRQPin = PullUp;

/**
 * Output Pin Callback.
 * An output pin is implemented as a callback function that
 * connects an output pin to one or more input pins.
 * In other words: When an output pin is "set" what is set
 * are the input pins connected to that output, this is done
 * by the callback.
 */
using OutputPinCb = std::function<void(bool)>;

}
