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
#include <string_view>

namespace caio {
namespace ui {

/**
 * Aspect ratios.
 */
enum class AspectRatio {
    System  = 0,
    _16_9   = 178,
    _8_7    = 114,
    _6_5    = 120,
    _5_3    = 167,
    _4_3    = 133
};

/**
 * Convert an aspect ratio string to AspectRatio type.
 * @param str String to convert.
 * @return The AspectRatio value type.
 * @exception InvalidArgument if the specified string is not recognised.
 * @see AspectRatio
 */
AspectRatio to_aspect_ratio(std::string_view str);

/**
 * Convert an aspect ratio to a floating point value.
 * @param ratio         The aspect ratio to convert;
 * @param system_ratio  The system aspect ratio floating point value.
 * @return The floating point aspect ratio value.
 */
static inline float to_value(AspectRatio ratio, float system_ratio)
{
    return (ratio == AspectRatio::System ? system_ratio : static_cast<int>(ratio) / 100.0f);
}

/**
 * Convert an aspect ratio type to string.
 * @param ratio Aspect ratio to convert.
 * @return The aspect ratio converted to string.
 */
std::string to_string(AspectRatio ratio);

/**
 * Scanline effects.
 */
enum class SLEffect {
    None            = +'n',
    Horizontal      = +'h',
    Vertical        = +'v',
    Adv_Horizontal  = +'H',
    Adv_Vertical    = +'V'
};

/**
 * Convert a scanline effect string to SLEffect type.
 * @param str String to convert: "v", "h", "n" or "" (the last two meaning "no scanline effect").
 * @return SLEffect value.
 * @exception InvalidArgument if the string to convert is invalid.
 * @see SLEffect
 */
SLEffect to_sleffect(std::string_view str);

/**
 * Convert a scanline effect to string.
 * @param effect Scanline effect to convert.
 * @return The scanline effect converted to string.
 */
std::string to_string(SLEffect effect);

/**
 * Video configuration.
 */
struct VideoConfig {
    std::string title{};                        /**< Main window title.         */
    unsigned    width{};                        /**< Emulated screen width.     */
    unsigned    height{};                       /**< Emulated screen height.    */
    unsigned    scale{1};                       /**< Screen scale factor.       */
    AspectRatio aspect{AspectRatio::System};    /**< Screen aspect ratio.       */
    SLEffect    sleffect{SLEffect::None};       /**< Scanlines effect.          */
    bool        fullscreen{};                   /**< Start in fullscreen mode.  */
    bool        sresize{};                      /**< Smooth window resize.      */
    std::string statusbar{};                    /**< Status bar position.       */
    std::string screenshotdir{};                /**< Screenshot directory.      */
};

/**
 * Audio configuration.
 */
struct AudioConfig {
    bool        enabled{};                      /**< Enable audio flag.         */
    unsigned    srate{};                        /**< Sampling rate.             */
    unsigned    channels{};                     /**< Audio channels.            */
    unsigned    samples{};                      /**< No. of samples x buffer.   */
};

/**
 * UI Configuration.
 * @see AudioConfig
 * @see VideoConfig
 */
struct Config {
    std::string name{};                         /**< Session name.              */
    std::string snapshotdir{};                  /**< Snapshot directory.        */
    struct AudioConfig audio{};                 /**< Audio configuration.       */
    struct VideoConfig video{};                 /**< Video configuration.       */
};

}
}
