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

#include "endian.hpp"
#include "fs.hpp"
#include "utils.hpp"

#include <array>
#include <concepts>
#include <iostream>
#include <optional>
#include <string_view>
#include <type_traits>

namespace caio {

/**
 * Serializer/Deserializer.
 */
class Serializer {
public:
    /**
     * Create a serializer.
     * @param fname Output file name.
     * @return The serializer.
     * @exception IOError
     */
    static Serializer create_serializer(const fs::Path& fname);

    /**
     * Create a deserializer.
     * @param fname Input file name.
     * @return The deserializer.
     * @exception IOError
     */
    static Serializer create_deserializer(const fs::Path& fname);

    /**
     * Get the type of this instance.
     * @return true if this instance is a serializer; false otherwise.
     */
    bool is_serializer() const
    {
        return (_os.get() != nullptr);
    }

    /**
     * Get the type of this instance.
     * @return true if this instance is a deserializer; false otherwise.
     */
    bool is_deserializer() const
    {
        return (_is.get() != nullptr);
    }

    /**
     * Object separation marker (debugging).
     */
    static struct ObjSep {
    } objsep;

private:
    constexpr static const char OBJECT_SEP = '\n';
    constexpr static const char FIELD_SEP = ' ';

    Serializer(uptr_t<std::istream>&& is);
    Serializer(uptr_t<std::ostream>&& os);

    /**
     * Get the serializer output stream.
     * @return The output stream.
     * @exception IOError if this instance is not a serializer.
     */
    std::ostream& output();

    /**
     * Get the deserializer input stream.
     * @return The input stream.
     * @exception IOError if this instance is not a deserializer.
     */
    std::istream& input();

    uptr_t<std::istream> _is{};
    uptr_t<std::ostream> _os{};
    bool                 _objsep{};

    friend Serializer& operator&(Serializer&, ObjSep&);
    friend Serializer& operator&(Serializer&, std::string&);
    friend Serializer& operator&(Serializer&, uint64_t&);
    friend Serializer& operator&(Serializer&, std::span<uint8_t>&);
    friend Serializer& operator&(Serializer&, Buffer&);
};

/**
 * Set a serialization/deserialization attribute.
 * There is only one attribute 'objsep' used o mark
 * the beginning of an object (useful for debugging).
 * @param ser   Serializer instance;
 * @param attr  The attribute to set.
 * @return The serializer instance.
 * @see Serializer::objsep
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, Serializer::ObjSep& attr);

/**
 * String serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param value String to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, std::string& value);

/**
 * 64-bit value serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param value 64-bit value to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, uint64_t& value);

/**
 * 64-bit floating point value serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param value 64-bit floating point value to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, double& value);

/**
 * 32-bit floating point value serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param value 32-bit floating point value to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, float& value);

/**
 * Integer/enum value serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param value Value to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
template<typename T>
requires (std::is_integral_v<T> || std::is_enum_v<T>) && (not std::is_same_v<uint64_t, T>)
Serializer& operator&(Serializer& ser, T& value)
{
    if (ser.is_serializer()) {
        auto data = static_cast<uint64_t>(value);
        ser & data;
    } else if (ser.is_deserializer()) {
        uint64_t data{};
        ser & data;
        value = static_cast<T>(data);
    }

    return ser;
}

/**
 * Optional serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param value Optional to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
template<typename T>
Serializer& operator&(Serializer& ser, std::optional<T>& opt)
{
    if (ser.is_serializer()) {
        bool has_value = opt.has_value();
        T value = (has_value ? opt.value() : T{});
        ser & has_value & value;

    } else if (ser.is_deserializer()) {
        bool has_value{};
        T value{};
        ser & has_value & value;
        if (has_value) {
            opt = value;
        } else {
            opt.reset();
        }
    }

    return ser;
}

/**
 * Array of numbers/enum values serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param arr   Array to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
template<typename T, size_t N>
requires std::is_arithmetic_v<T> || std::is_enum_v<T>
Serializer& operator&(Serializer& ser, std::array<T, N>& arr)
{
    std::for_each(arr.begin(), arr.end(), [&ser](auto& value) { ser & value; });
    return ser;
}

/**
 * Buffer of 8-bit values serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param buf   Buffer of 8-bit values.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, std::span<uint8_t>& buf);

/**
 * Buffer of 8-bit values serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param buf   Buffer of 8-bit values.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
Serializer& operator&(Serializer& ser, Buffer& buf);

/**
 * Buffer of (non 8-bit) integers serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param buf   Buffer of (non 8-bit) integers.
 * @return The serializer instance.
 * @exception IOError
 * @see Serializer
 */
template<typename T>
requires std::is_integral_v<T> && (not std::is_same_v<uint8_t, T>)
Serializer& operator&(Serializer& ser, std::span<T>& buf)
{
    std::for_each(buf.begin(), buf.end(), [&ser](auto& value) { ser & value; });
    return ser;
}

/**
 * Pointer to object serialization/deserialization operator.
 * @param ser   Serializer instance;
 * @param pinst Pointer the object to serialize/deserialize.
 * @return The serializer instance.
 * @exception IOError
 * @note The object must implement: Serializer& operator&(Serializer&, OBJECT&)
 */
template<typename T>
requires std::is_class_v<T>
Serializer& operator&(Serializer& ser, const sptr_t<T>& pinst)
{
    if (pinst) {
        ser & *pinst;
    }
    return ser;
}

}
