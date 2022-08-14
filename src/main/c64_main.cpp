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
#include "c64_main.hpp"

#include <exception>
#include <iostream>

#include "logger.hpp"

#include "c64_cmdline.hpp"
#include "c64_config.hpp"
#include "c64.hpp"


namespace caio {
namespace c64 {

int main(int argc, char *const *argv)
{
    try {
        C64Cmdline cmdline{};
        C64Confile cfile{cmdline.parse(argc, argv)};
        C64Config conf{cfile};

        log.logfile(conf.logfile);
        log.loglevel(conf.loglevel);

        C64 c64{conf};
        c64.run();

        return EXIT_SUCCESS;

    } catch (const std::exception &err) {
        std::cerr << "C64: Error: " << err.what() << std::endl;
    }

    return EXIT_FAILURE;
}

}
}
