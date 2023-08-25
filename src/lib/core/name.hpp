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

#include <string>


namespace caio {

/**
 * Anything with a type and a label.
 */
class Name {
public:
    constexpr static const char *TYPE_UNKNOWN = "UNK";

    Name() {
    }

    explicit Name(const std::string &type, const std::string &label = {})
        : _type{(type.empty() ? TYPE_UNKNOWN : type)},
          _label{(label.empty() ? "" : label)} {
    }

    /**
     * @return The type of this instance.
     */
    const std::string &type() const {
        return _type;
    }

    /**
     * Change the type of this instance.
     * @param type New type to set.
     */
    void type(const std::string &type) {
        _type = type;
    }

    /**
     * @return The label of this instance.
     */
    const std::string &label() const {
        return _label;
    }

    /**
     * Change the label of this instance.
     * @param label New label to set.
     */
    void label(const std::string &label) {
        _label = label;
    }

    /**
     * @return A string representation of this instance.
     */
    virtual std::string to_string() const;

    /**
     * Cast to std::string.
     */
    virtual operator std::string() const {
        return to_string();
    }

private:
    std::string _type{};
    std::string _label{};
};

}
