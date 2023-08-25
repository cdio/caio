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

#include <getopt.h>

#include <string>
#include <vector>

#include "config.hpp"


namespace caio {

/**
 * Command line and configuration file parser.
 */
class CaioCmdline {
public:
    enum Options {
        OPTION_CONF,
        OPTION_ROMDIR,
        OPTION_CARTDIR,
        OPTION_PALETTEDIR,
        OPTION_PALETTE,
        OPTION_KEYMAPSDIR,
        OPTION_KEYMAPS,
        OPTION_FPS,
        OPTION_SCALE,
        OPTION_SCANLINES,
        OPTION_FULLSCREEN,
        OPTION_SRESIZE,
        OPTION_AUDIO,
        OPTION_DELAY,
        OPTION_MONITOR,
        OPTION_LOGFILE,
        OPTION_LOGLEVEL,
        OPTION_VERSION,
        OPTION_HELP,

        OPTION_MAX
    };

    CaioCmdline() {
    }

    virtual ~CaioCmdline() {
    }

    /**
     * Parse the command line options and load the configuration file.
     * @param argc Number of command line arguments;
     * @param argv List of command line arguments.
     * @return The configuration file with values from the actual
     * configuration file on disk overrided by command line arguments.
     * In case of errors this method prints the error message and terminates the current process.
     */
    virtual Confile parse(int argc, char* const* argv);

protected:
    /**
     * Command line and configuration file parser.
     * @param ext_lopts Extended long options.
     */
    CaioCmdline(const std::vector<::option>& ext_lopts);

    /**
     * Print the command line options usage on standard error.
     */
    virtual void usage();

    /**
     * Parse extended long options.
     * @param conf Configuration file to update;
     * @param opt  Long option index;
     * @param arg  Command line argument.
     * @return true if the option was recoginised; false otherwise.
     */
    virtual bool parse(Confile& conf, int opt, const std::string& arg);

    std::string           _progname;
    std::vector<::option> _lopts{};

private:
    static const std::vector<::option> lopts;
};

}
