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

#include "zx80_config.hpp"
#include "caio_cmdline.hpp"


namespace caio {
namespace zx80 {

/**
 * Command line and configuration file ZX80 parser.
 */
class ZX80Cmdline : public CaioCmdline {
public:
    enum ZX80Options {
        //TODO
        ZX80_OPTION_MAX
    };

    ZX80Cmdline();

    virtual ~ZX80Cmdline();

    Confile parse(int argc, char *const *argv) override;

private:
    void usage() override;

    bool parse(Confile &conf, int opt, const std::string &arg) override;

    static const std::vector<::option> lopts;
};

}
}