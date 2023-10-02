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

#include <cstdlib>
#include <exception>
#include <iostream>

#include "logger.hpp"


namespace caio {

template<class MACHINE, class CMDLINE>
class Main_
{
public:
    Main_(int argc, const char** argv) {
        try {
            CMDLINE cmdline{};
            auto sec = config::parse(argc, argv, cmdline);

            log.logfile(sec[config::KEY_LOGFILE]);
            log.loglevel(sec[config::KEY_LOGLEVEL]);

            MACHINE machine{sec};
            machine.run();

            std::exit(EXIT_SUCCESS);

        } catch (const std::exception& err) {
            std::cerr << MACHINE::name() << ": Error: " << err.what() << std::endl;
        }

        std::exit(EXIT_FAILURE);
    }
};

}
