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

#include <cstdint>
#include <cstdlib>
#include <ctime>
#include <limits>
#include <numeric>
#include <ostream>
#include <random>
#include <sstream>
#include <string>
#include <type_traits>
#include <vector>

#include <gsl/span>

#include "utils.hpp"


namespace caio {
namespace signal {

extern std::random_device rd;
extern std::uniform_real_distribution<float> uni_random;

using samples_fp = gsl::span<float>;
using samples_i16 = gsl::span<int16_t>;


/**
 * Convolution product shapes.
 * @see conv()
 */
enum class ConvShape {
    /**
     * Full convolution size.
     * If C = A * B, then the size of the returned buffer
     * is C.size which equals to A.size + B.size - 1.
     */
    Full,

    /**
     * Central (useful) part.
     * If C = A * B, then the size of the returned buffer
     * is the central part of C with size A.size.
     * Used when A contains the sampled data and B the kernel.
     */
    Central
};

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
    const float slope = 4.0f / T;
    const float T50   = 0.5f * T;
    return (t < T50 ? slope * t - 1.0f : 1.0f - slope * (t - T50));
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
    const float slope = 4.0f / T;
    const float T50   = 0.5f * T;
    return (-1.0f + slope * (t < T50 ? t : t - T50));
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
 * Calculate the average of the floating point elements inside a container.
 * @param samples Container.
 * @return The average value.
 */
template <typename C, typename = std::enable_if<utils::is_container<C>::value>>
float mean(const C& samples)
{
    float sum = std::accumulate(samples.begin(), samples.end(), 0.0f);
    return (sum / samples.size());
}

/**
 * Calculate the (optimal) size of a kernel.
 * @return 4.0 * fs / fc.
 */
constexpr size_t kernel_size(float fc, float fs)
{
    size_t size = utils::ceil(4.0f * fs / fc);
    if ((size & 1) == 0) {
        ++size;
    }
    return size;
}

/**
 * Blackman window.
 * @param pos Position;
 * @param N   Size of the window (in samples).
 * @return The value of the blackman window at the specified position.
 */
float blackman(size_t pos, size_t N);

/**
 * Invert the frequency spectrum of a kernel.
 * @param krn Kernel to invert.
 */
void spectral_inversion(samples_fp& krn);

/**
 * Convolution product.
 * @param dst   Destination buffer;
 * @param sig   Signal buffer;
 * @param krn   Kernel buffer;
 * @param shape Desired output shape (see ConvShape).
 * @return A subspan of the destination buffer containing the convolution product, the size of this buffer
 * depends on the specified shape.
 * @note The destination buffer is expected to be large enough (see ConvShape)
 * @see ConvShape
 */
samples_fp conv(samples_fp& dst, const samples_fp& sig, const samples_fp& krn, enum ConvShape shape);

/**
 * Generate a Low-pass filter kernel.
 * @param krn  Destination buffer;
 * @param fc   Cutoff frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp lopass(samples_fp& krn, float fc, float fs, bool osiz = true);

/**
 * Generate a High-pass filter kernel.
 * @param krn  Destination buffer;
 * @param fc   Cutoff frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp hipass(samples_fp& buf, float fc, float fs, bool osiz = true);

/**
 * Generate a Band-pass filter kernel.
 * @param krn  Destination buffer;
 * @param fcl  Low cutoff frequency;
 * @param fch  High cutoff frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp bapass(samples_fp& krn, float fcl, float fch, float fs, bool osiz = true);

/**
 * Generate a Band-stop filter kernel.
 * If the osiz parameter is set and the calculated size is bigger
 * than the received buffer, the size of the buffer is used.
 * @param buf  Destination buffer;
 * @param fcl  Low cutoff frequency;
 * @param fch  High cutoff frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp bastop(samples_fp& krn, float fcl, float fch, float fs, bool osiz = true);

/**
 * Generate a Low-pass filter kernel using a second order pole.
 * A second order pole forms a filter with an attenuation rate of 40dB/dec.
 * @param krn  Destination buffer;
 * @param f0   Resonance frequency;
 * @param Q    Q factor;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp lopass_40(samples_fp& krn, float f0, float Q, float fs, bool osiz = true);

/**
 * Generate a High-pass filter kernel using an inverted second order pole.
 * An inverted second order pole forms a filter with an attenuation rate of 40dB/dec.
 * @param krn  Destination buffer;
 * @param f0   Resonance frequency;
 * @param Q    Q factor;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp hipass_40(samples_fp& krn, float f0, float Q, float fs, bool osiz = true);

/**
 * Generate a Low-pass filter kernel using a simple pole.
 * An simple pole forms a filter with an attenuation rate of 20dB/dec.
 * @param krn  Destination buffer;
 * @param f0   Resonance frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp lopass_20(samples_fp& krn, float f0, float fs, bool osiz = true);

/**
 * Generate a Hi-pass filter kernel using an inverted pole.
 * An inverted pole forms a filter with an attenuation rate of 20dB/dec.
 * @param krn  Destination buffer;
 * @param f0   Resonance frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp hipass_20(samples_fp& krn, float f0, float fs, bool osiz = true);

/**
 * Generate a "triangular" Band-pass filter kernel using a zero and a pole.
 * The filter is based on a zero at the resonance frequency and two poles at the
 * same frequency. The attenuation rate is 20dB/dec.
 * @param krn  Destination buffer;
 * @param f0   Resonance frequency;
 * @param fs   Sampling frequency;
 * @param osiz If true, calculate the optimal size of the kernel; otherwise use the specified buffer size.
 * @return A subspan of the destination buffer containing the actual kernel.
 * @note If osiz is true, the destination buffer is expected to be large enough.
 * @see kernel_size()
 * TODO: use bilinear transform
 */
samples_fp bapass_20(samples_fp& krn, float f0, float fs, bool osiz = true);

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
std::string to_string(const T& samples)
{
    std::stringstream os{};

    os << "[";

    for (const auto& sample : samples) {
        os << +sample << ", ";
    }

    os << "]";

    return os.str();
}

/**
 * Send the content of a samples container to an output stream formatted as an octave structure.
 * @param os      The output stream;
 * @param name    Name for the structure;
 * @param samples Samples;
 * @param fs      Sampling rate;
 * @param fc1     Cutoff frequency 1;
 * @param fc2     Cutoff frequency 2 or 0;
 * @param Q       Q factor or 0.
 * @return os.
 */
std::ostream& dump(std::ostream& os, const samples_fp& samples, const std::string& name, float fs, float fc1,
    float fc2, float Q);

}

using samples_fp = signal::samples_fp;
using samples_i16 = signal::samples_i16;

}
