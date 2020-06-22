/*
 * Copyright (C) 2020 Claudio Castiglia
 *
 * This file is part of CEMU.
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

#include <cmath>
#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include "utils.hpp"


namespace cemu {
namespace signal {

using samples_fp = std::vector<float>;
using samples_i16 = std::vector<int16_t>;

extern std::random_device rd;
extern std::uniform_real_distribution<float> uni_random;


/**
 * @return A positive random value between 0.0 and 1.0.
 */
__attribute__((always_inline))
static inline float prand()
{
    return std::abs(rand());
}


/**
 * @return A random value between -1.0 and 1.0.
 */
__attribute__((always_inline))
static inline float rand()
{
    return uni_random(rd);
}


/**
 * Get the value of an exponential signal.
 * @param A0 DC component;
 * @param A  Exponential maximum value;
 * @param t  Time position;
 * @param T  Time constant.
 * @return The amplitude at the specified time position: A0 + A * e^(-t/T).
 */
__attribute__((always_inline))
static inline float exp(float A0, float A, float t, float T)
{
    return A0 + A * std::exp(-t / T);
}


/**
 * Get the value of a triangle signal.
 * @param t Time positon;
 * @param T Period.
 * @return The amplitude at the specified time position.
 */
__attribute__((always_inline))
static inline float triangle(float t, float T)
{
    const float hhT = 0.25f * T;
    const float slope = 1.0f / hhT;
    return ((t > hhT && t < 0.75f * T) ? (2.0f - slope * t) : slope * (t - ((t <= hhT) ? 0.0f : T)));
}


/**
 * Get the value of a sawtooth signal.
 * @param t Time position;
 * @param T Period.
 * @return The amplitude at the specified time position.
 */
__attribute__((always_inline))
static inline float sawtooth(float t, float T)
{
    const float hT = 0.5f * T;
    const float slope = 2.0f / T;
    return slope * (t - ((t > hT) ? T : 0.0f));
}


/**
 * Generate a pulse signal value.
 * @param t  Time position;
 * @param dc Duty cycle.
 * @return 1.0 if t <= dc: 0.0f otherwise.
 */
__attribute__((always_inline))
static inline float pulse(float t, float dc)
{
    return (t < dc ? 1.0f : 0.0f);
}


/**
 * Generate a square signal value.
 * @param t  Time position;
 * @param dc Duty cycle.
 * @return 1.0 if t <= dc: -1.0f otherwise.
 */
__attribute__((always_inline))
static inline float square(float t, float dc)
{
    return (t < dc ? 1.0f : -1.0f);
}


/**
 * Sinc function.
 * @param x Radians.
 * @return sin(x) / x.
 */
__attribute__((always_inline))
static inline float sinc(float x)
{
    return ((x == 0.0f) ? 1.0f : std::sin(x) / x);
}


/**
 * Calculate the mean of the floating point elements inside a container.
 * @param samples Container.
 * @return The mean value.
 */
template <typename C, typename = std::enable_if<utils::is_container<C>::value>>
float mean(const C &samples)
{
    float sum = 0.0f;

    for (const auto &sample : samples) {
        sum += sample;
    }

    return (sum / samples.size());
}


/**
 * Calculate the size of a kernel.
 * @return 4.0 * fs / fc.
 */
size_t kernel_size(float fc, float fs);


/**
 * Blackman window.
 * @param pos Position;
 * @param N   Size of the window (in samples).
 * @return The value of the blackman window at the specified position.
 */
float blackman(size_t pos, size_t N);


/**
 * Invert the frequency spectrum of a signal.
 * @param v Signal to invert.
 * @return v.
 */
gsl::span<float> &spectral_inversion(gsl::span<float> &v);


/**
 * Convolution product.
 * @param x First samples buffer;
 * @param y Second samples buffer.
 * @return The convolution product of the two buffers.
 */
samples_fp conv(const samples_fp &x, const samples_fp &y);


/**
 * Apply a filter kernel to a buffer.
 * @param v Samples buffer to filter;
 * @param k Filter kernel.
 */
void conv_kernel(samples_fp &v, const gsl::span<float> &k);


/**
 * Generate a Low-pass filter kernel.
 * If the osiz parameter is true and the calculated size is bigger
 * than the received buffer, the size of the buffer is used.
 * @param buf  The buffer to fill with the generated kernel;
 * @param fc   Cutoff frequency;
 * @param fs   Sampling frequency;
 * @param rs   Aplitude of the resonance component;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the buffer size.
 * @return A subspan of the buffer containing the actual kernel.
 * @see kernel_size()
 */
gsl::span<float> lopass(gsl::span<float> &buf, float fc, float fs, float rs, bool osize = true);


/**
 * Generate a High-pass filter kernel.
 * If the osiz parameter is set and the calculated size is bigger
 * than the received buffer, the size of the buffer is used.
 * @param buf  The buffer to fill with the generated kernel;
 * @param fc   Cutoff frequency;
 * @param fs   Sampling frequency;
 * @param rs   Aplitude of the resonance component;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the buffer size.
 * @return A subspan of the buffer containing the actual kernel.
 * @see kernel_size()
 */
__attribute__((always_inline))
static inline gsl::span<float> hipass(gsl::span<float> &buf, float fc, float fs, float rs, bool osiz = true)
{
    auto hi = lopass(buf, fc, fs, rs, osiz);
    return spectral_inversion(hi);
}


/**
 * Generate a Stop-band filter kernel.
 * If the osiz parameter is set and the calculated size is bigger
 * than the received buffer, the size of the buffer is used.
 * @param buf  The buffer to fill with the generated kernel;
 * @param fcl  Low cutoff frequency;
 * @param fch  High cutoff frequency;
 * @param fs   Sampling frequency;
 * @param rs   Aplitude of the resonance component;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the buffer size.
 * @return A subspan of the buffer containing the actual kernel.
 * @see kernel_size()
 */
gsl::span<float> stopband(gsl::span<float> &buf, float fcl, float fch, float fs, float rs, bool osiz = true);


/**
 * Generate a Band-pass filter kernel.
 * If the osiz parameter is set and the calculated size is bigger
 * than the received buffer, the size of the buffer is used.
 * @param buf  The buffer to fill with the generated kernel;
 * @param fcl  Low cutoff frequency;
 * @param fch  High cutoff frequency;
 * @param fs   Sampling frequency;
 * @param rs   Aplitude of the resonance component;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the buffer size.
 * @return A subspan of the buffer containing the actual kernel.
 * @see kernel_size()
 */
__attribute__((always_inline))
static inline gsl::span<float> bapass(gsl::span<float> &buf, float fcl, float fch, float fs, float rs, bool osiz = true)
{
    auto bp = stopband(buf, fcl, fch, fs, osiz);
    return spectral_inversion(bp);
}


/**
 * Convert an integer value to floating point.
 * @param value Value to convert;
 * @return The floating point value.
 */
template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
float to_fp(T value)
{
    return static_cast<float>(value) / static_cast<float>(std::numeric_limits<T>::max());
}


/**
 * Convert a floating point value to integer type.
 * @param value Floating point value to convert ([0.0, 1.0] if T is unsigned or [-1.0, 1.0] if T is signed).
 * @return The integer value.
 */
template <typename T, typename = std::enable_if<std::is_integral<T>::value>>
T to_integer(float value)
{
    return static_cast<T>(value * std::numeric_limits<T>::max());
}


/**
 * Convert a floating point value to signed 16 bits.
 * @param value Value to convert (between 0 and 1).
 * @return The 16 bits unsigned value.
 * @see to_integer()
 */
__attribute__((always_inline))
static inline int16_t to_i16(float value)
{
    return to_integer<int16_t>(value);
}


/**
 * @return An octave compatible representation of a samples buffer.
 */
template<typename T, typename = std::enable_if<utils::is_container<T>::value>>
std::string to_string(const T &samples)
{
    std::stringstream os{};

    os << "[";

    for (const auto &sample : samples) {
        os << +sample << ", ";
    }

    os << "]";

    return os.str();
}

}

using samples_fp = signal::samples_fp;
using samples_i16 = signal::samples_i16;

}
