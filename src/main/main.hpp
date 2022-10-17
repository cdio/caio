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

#include <cstdlib>
#include <exception>
#include <iostream>

#include "logger.hpp"


namespace caio {

template<class CMDLINE, class CONFILE, class CONFIG, class MACHINE>
class Main_
{
public:
    Main_(int argc, char *const *argv) {
        try {
            CMDLINE cmdline{};
            CONFILE cfile{cmdline.parse(argc, argv)};
            CONFIG  conf{cfile};

            log.logfile(conf.logfile);
            log.loglevel(conf.loglevel);

            MACHINE machine{conf};
            machine.run();

            std::exit(EXIT_SUCCESS);

        } catch (const std::exception &err) {
            std::cerr << MACHINE::name() << ": Error: " << err.what() << std::endl;
        }

        std::exit(EXIT_FAILURE);
    }
};

}
