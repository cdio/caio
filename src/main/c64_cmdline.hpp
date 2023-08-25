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

#include "c64_config.hpp"
#include "caio_cmdline.hpp"


namespace caio {
namespace c64 {

/**
 * Command line and configuration file C64 parser.
 */
class C64Cmdline : public CaioCmdline {
public:
    enum C64Options {
        C64_OPTION_CARTFILE = Options::OPTION_MAX,
        C64_OPTION_PRGFILE,
        C64_OPTION_UNIT_8,
        C64_OPTION_UNIT_9,
        C64_OPTION_RESID,
        C64_OPTION_SWAPJOY,

        C64_OPTION_MAX
    };

    C64Cmdline()
        : CaioCmdline{lopts} {
    }

    virtual ~C64Cmdline() {
    }

    Confile parse(int argc, char* const* argv) override {
        return CaioCmdline::parse(argc, argv);
    }

private:
    void usage() override;

    bool parse(Confile& conf, int opt, const std::string& arg) override;

    static const std::vector<::option> lopts;
};

}
}
