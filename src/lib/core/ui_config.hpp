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

#include <string>


namespace caio {
namespace ui {

/**
 * Scanline effects.
 */
enum class SLEffect {
    NONE        = +'n',
    HORIZONTAL  = +'h',
    VERTICAL    = +'v'
};

/**
 * Convert a scanline effect string to SLEffect type.
 * @param str String to convert: "v", "h", "n" or "" (the last two meaning "no scanline effect").
 * @return SLEffect value.
 * @exception InvalidArgument if the string to convert is invalid.
 * @see SLEffect
 */
SLEffect to_sleffect(const std::string& str);

/**
 * Video configuration.
 */
struct VideoConfig {
    std::string title{};                    /**< Main window title.         */
    unsigned    width{};                    /**< Emulated screen width.     */
    unsigned    height{};                   /**< Emulated screen height.    */
    unsigned    fps{};                      /**< Frames per second.         */
    float       scale{1.0f};                /**< Screen scale factor.       */
    SLEffect    sleffect{SLEffect::NONE};   /**< Scanlines effect.          */
    bool        fullscreen{};               /**< Start in fullscreen mode.  */
    bool        smooth_resize{};            /**< Smooth window resize.      */
};

/**
 * Audio configuration.
 */
struct AudioConfig {
    bool     enabled{};                     /**< Audio output enabled.      */
    unsigned srate{};                       /**< Sampling rate.             */
    unsigned channels{};                    /**< Audio channels.            */
    unsigned samples{};                     /**< No. of samples x buffer.   */
};

/**
 * UI Configuration.
 * @see AudioConfig
 * @see VideoConfig
 */
struct Config {
    struct AudioConfig audio{};
    struct VideoConfig video{};
};

}
}
