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
#include "serializer.hpp"

#include "logger.hpp"
#include "utils.hpp"

#include <fstream>

namespace caio {

Serializer::ObjSep Serializer::objsep{};

Serializer::Serializer(uptr_t<std::istream>&& is)
    : _is{std::move(is)}
{
}

Serializer::Serializer(uptr_t<std::ostream>&& os)
    : _os{std::move(os)}
{
}

std::istream& Serializer::input()
{
    if (!_is) {
        throw IOError{"Input stream is null"};
    }

    return *_is;
}

std::ostream& Serializer::output()
{
    if (!_os) {
        throw IOError{"Output stream is null"};
    }

    /* Not the best place but... */
    if (_objsep) {
        _objsep = false;
        (*_os) << OBJECT_SEP;
    }

    return *_os;
}

Serializer Serializer::create_serializer(const fs::Path& fname)
{
    auto os = std::make_unique<std::ofstream>(fname);
    if (os->fail()) {
        throw IOError{"Can't open snapshot image: {}: {}", fname.string(), Error::to_string()};
    }

    return Serializer{std::move(os)};
}

Serializer& operator&(Serializer& ser, Serializer::ObjSep&)
{
    ser._objsep = true;
    return ser;
}

Serializer Serializer::create_deserializer(const fs::Path& fname)
{
    auto is = std::make_unique<std::ifstream>(fname);
    if (is->fail()) {
        throw IOError{"Can't open snapshot image: {}: {}", fname.string(), Error::to_string()};
    }

    return Serializer{std::move(is)};
}

Serializer& operator&(Serializer& ser, std::string& value)
{
    try {
        if (ser.is_serializer()) {
            auto& os = ser.output();

            os << std::hex << value.size() << Serializer::FIELD_SEP;

            if (value.size() != 0) {
                os << value << Serializer::FIELD_SEP;
            }

        } else if (ser.is_deserializer()) {
            auto& is = ser.input();

            size_t size{};
            is >> std::hex >> size;

            value.resize(size);

            if (size != 0) {
                is.seekg(1, std::ios_base::cur);
                is.read(value.data(), size);
            }
        }
    } catch (const std::exception& err) {
        throw IOError{"Can't string serdes: {}", err.what()};
    }

    return ser;
}

Serializer& operator&(Serializer& ser, uint64_t& value)
{
    try {
        if (ser.is_serializer()) {
            ser.output() << std::hex << value << Serializer::FIELD_SEP;
        } else if (ser.is_deserializer()) {
            ser.input() >> std::hex >> value;
        }
    } catch (const std::exception& err) {
        throw IOError{"Can't uint64 serdes: {}", err.what()};
    }

    return ser;
}

Serializer& operator&(Serializer& ser, double& value)
{
    if (ser.is_serializer()) {
        uint64_t data = htobe64(*reinterpret_cast<uint64_t*>(&value));
        ser & data;
    } else if (ser.is_deserializer()) {
        uint64_t data{};
        ser & data;
        data = be64toh(data);
        value = *reinterpret_cast<double*>(&data);
    }

    return ser;
}

Serializer& operator&(Serializer& ser, float& value)
{
    if (ser.is_serializer()) {
        uint32_t data = htobe32(*reinterpret_cast<uint32_t*>(&value));
        ser & data;

    } else if (ser.is_deserializer()) {
        uint32_t data{};
        ser & data;
        data = be32toh(data);
        value = *reinterpret_cast<float*>(&data);
    }

    return ser;
}

Serializer& operator&(Serializer& ser, std::span<uint8_t>& buf)
{
    try {
        if (ser.is_serializer()) {
            const auto encoded = utils::to_string(utils::base64_encode(buf));
            ser.output() << encoded << Serializer::FIELD_SEP;

        } else if (ser.is_deserializer()) {
            Buffer decoded{};
            ser & decoded;

            const size_t count = std::min(buf.size(), decoded.size());
            std::copy_n(decoded.begin(), count, buf.begin());
        }
    } catch (const std::exception& err) {
        throw IOError{"Can't serdes 8-bit span: {}", err.what()};
    }

    return ser;
}

Serializer& operator&(Serializer& ser, Buffer& buf)
{
    try {
        if (ser.is_serializer()) {
            auto sbuf = std::span<uint8_t>{buf.data(), buf.size()};
            ser & sbuf;

        } else if (ser.is_deserializer()) {
            std::string encoded{};
            ser.input() >> encoded;
            buf = utils::base64_decode(encoded);
        }
    } catch (const std::exception& err) {
        throw IOError{"Can't serdes 8-bit buffer: {}", err.what()};
    }

    return ser;
}

}
