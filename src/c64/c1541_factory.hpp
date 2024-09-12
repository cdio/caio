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
#include <memory>

#include "fs.hpp"

#include "cbm_bus.hpp"
#include "c1541.hpp"

namespace caio {
namespace commodore {
namespace c1541 {

/**
 * Create a C1541 unit based on the path to attach.
 * @param path Directory, file or device to attach to;
 * @param unit Unit number assigned to the device;
 * @param bus  Bus to connect to.
 * @return A pointer to the C1541 implementation.
 * @exception InvalidArgument
 * @exception IOError
 * @see C1541::C1541()
 * @see C1541::attach()
 */
sptr_t<C1541> instance(const fs::Path& path, uint8_t unit, const sptr_t<cbm_bus::Bus>& bus);

}
}
}
