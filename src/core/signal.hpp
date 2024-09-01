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

#include <array>
#include <cmath>
#include <functional>
#include <ostream>

#include "types.hpp"

namespace caio {
namespace signal {

/**
 * Sinc function.
 * @param x Radians.
 * @return sin(x) / x.
 */
constexpr fp_t sinc(fp_t x)
{
    return ((x == 0.0) ? 1.0 : std::sin(x) / x);
}

/**
 * Get the value of an exponential signal.
 * @param A0 DC component;
 * @param A  Exponential maximum value;
 * @param t  Time position;
 * @param T  Time constant.
 * @return The amplitude at the specified time position: A0 + A * e^(-t/T).
 */
constexpr fp_t exp(fp_t A0, fp_t A, fp_t t, fp_t T)
{
    return (A0 + A * std::exp(-t / T));
}

/**
 * Get the value of a triangle signal within the range [-1.0, 1.0].
 * @param t Time positon;
 * @param T Period.
 * @return The amplitude at the specified time position.
 */
constexpr fp_t triangle(fp_t t, fp_t T)
{
    const fp_t slope = 4.0 / T;
    const fp_t T50   = 0.5 * T;
    return (t <= T50 ? slope * t - 1.0 : 1.0 - slope * (t - T50));
}

/**
 * Get the value of a sawtooth signal within the range [-1.0, 1.0].
 * @param t Time position;
 * @param T Period.
 * @return The amplitude at the specified time position.
 */
constexpr fp_t sawtooth(fp_t t, fp_t T)
{
    const fp_t slope = 2.0 / T;
    return (-1.0 + slope * t);
}

/**
 * Get the value of a pulse signal within the range [-1.0, 1.0].
 * @param t  Time position;
 * @param dc Duty cycle.
 * @return 1.0 if t <= dc; -1.0 otherwise.
 */
constexpr fp_t pulse(fp_t t, fp_t dc)
{
    return (t <= dc ? 1.0 : -1.0);
}

/**
 * Compute the modulo of two values.
 * @param a First value;
 * @param m Second value.
 * @return The modulo.
 */
template<typename T, typename = std::enable_if_t<std::is_integral<T>::value>>
constexpr T mod(T a, T m)
{
    T b = a % m;
    if (b < 0) {
        b += m;
    }
    return b;
}

/**
 * Calculate the average of the floating point elements inside a container.
 * @param samples Container.
 * @return The average value.
 */
fp_t mean(samples_fp samples);

/**
 * Transfer function coefficients.
 */
template<size_t M, size_t N>
struct PCoeffs {
    std::array<fp_t, M> num;
    std::array<fp_t, N> den;

    operator p_coeffs() const
    {
        return {num, den};
    }

    operator p_coeffs()
    {
        return {num, den};
    }

    PCoeffs<M, N>& operator=(p_coeffs coeffs)
    {
        const size_t max_m = std::min(std::size(coeffs.first), M);
        const size_t max_n = std::min(std::size(coeffs.second), N);
        std::fill(num.begin(), num.end(), 0.0);
        std::fill(den.begin(), den.end(), 0.0);
        std::copy_n(std::begin(coeffs.first), max_m, num.begin());
        std::copy_n(std::begin(coeffs.second), max_n, den.begin());
        return *this;
    }

    PCoeffs<M, N>& normalize()
    {
        if (den.size() > 0) {
            if (const auto a0 = den[0]; a0 != 1.0) {
                const auto fn = [a0](fp_t value) { return (value / a0); };
                std::transform(num.begin(), num.end(), num.begin(), fn);
                std::transform(den.begin(), den.end(), den.begin(), fn);
            }
        }
        return *this;
    }
};

/**
 * Filter.
 */
template<size_t M, size_t N>
class Filter : public PCoeffs<M, N> {
public:
    Filter()
        : PCoeffs<M, N>{}
    {
    }

    /**
     * Initialise this filter.
     * @param coeffs Moved normalized filter coefficients.
     * @see PCoeffs<M, N>
     */
    Filter(PCoeffs<M, N>&& coeffs)
        : PCoeffs<M, N>{coeffs}
    {
    }

    /**
     * Initialise this filter.
     * @param coeffs Normalized filter coefficients.
     * @see PCoeffs<M, N>
     */
    Filter(const PCoeffs<M, N>& coeffs)
        : PCoeffs<M, N>{coeffs}
    {
    }

    /**
     * Initialise this filter.
     * @param coeffs Normalized filter coefficients.
     * @see p_coeffs
     */
    Filter(p_coeffs coeffs)
    {
        static_cast<PCoeffs<M, N>&>(*this) = coeffs;
    }

    /**
     * Apply this filter to a stream of samples.
     * @param sample Next input sample.
     * @return A filtered output sample.
     */
    fp_t operator()(fp_t sample)
    {
        _x[_xpos] = sample;
        fp_t y = 0;

        for (ssize_t m = 0; m < static_cast<ssize_t>(M); ++m) {
            auto pos = mod<ssize_t>(_xpos - m, M);
            y += _x[pos] * PCoeffs<M, N>::num[m];
        }

        for (ssize_t n = 1; n < static_cast<ssize_t>(N); ++n) {
            auto pos = mod<ssize_t>(_ypos - n, N);
            y -= _y[pos] * PCoeffs<M, N>::den[n];
        }

        _y[_ypos] = y;

        _xpos = mod<ssize_t>(_xpos + 1, M);
        _ypos = mod<ssize_t>(_ypos + 1, N);

        return y;
    }

private:
    std::array<fp_t, M> _x{};
    std::array<fp_t, N> _y{};
    ssize_t             _xpos{};
    ssize_t             _ypos{};
};

/**
 * Add two polynomials.
 * @param a1 First polynomial coefficients;
 * @param a2 Second polynomial coefficients.
 * @return The resulting polynomial coefficients.
 */
template<size_t R, size_t C>
std::array<fp_t, std::max(R, C)> poly_add(const std::array<fp_t, R>& a1, const std::array<fp_t, C>& a2)
{
    static_assert((R != 0) && (C != 0), "Invalid input array size");
    constexpr size_t N = std::max(R, C);
    std::array<fp_t, N> arr{(R > C ? a1 : a2)};
    if (R > C) {
        for (size_t i = 0; i < C; ++i) {
            arr[i] += a2[i];
        }
    } else {
        for (size_t i = 0; i < C; ++i) {
            arr[i] += a1[i];
        }
    }
    return arr;
}

/**
 * Add two pairs of coefficients.
 * @param c1 First pair of coefficients;
 * @param c2 Second pair of coefficeints.
 * @return The resulting coefficient pair.
 */
template<size_t M1, size_t N1, size_t M2, size_t N2>
constexpr PCoeffs<std::max(M1, M2), std::max(N1, N2)> add(const PCoeffs<M1, N1>& c1, const PCoeffs<M2, N2>& c2)
{
    PCoeffs<std::max(M1, M2), std::max(N1, N2)> result{
        .num = poly_add(c1.num, c2.num),
        .den = poly_add(c1.den, c2.den)
    };
    return result.normalize();
}

/**
 * Return the set of coefficients for a second order IIR low-pass filter.
 * Roll-off is 40 dB/dec (12 dB/oct).
 *
 *                       1
 *   H(s) = -----------------------------
 *            1 + s / Q w0 + s^2 / w0^2
 *
 * @param f0 Resonance frequency;
 * @param Q  Q factor;
 * @param fs Sampling frequency.
 * @return The normalized filter coefficients.
 */
constexpr PCoeffs<3, 3> iir_lopass40(fp_t f0, fp_t Q, fp_t fs)
{
    const fp_t w  = Pi * f0 / fs;
    const fp_t ww = w * w;
    const fp_t wq = w * Q;

    const fp_t a0 = 1.0 + 1.0 / wq + 1.0 / ww;
    const fp_t a1 = (2.0 - 2.0 / ww) / a0;
    const fp_t a2 = (1.0 - 1.0 / wq + 1.0 / ww) / a0;

    const fp_t b0 = 1.0 / a0;
    const fp_t b1 = 2.0 / a0;
    const fp_t b2 = 1.0 / a0;

    return { { b0, b1, b2 }, { 1.0, a1, a2 } };
}

/**
 * Return the set of coefficients for a second order high-pass filter.
 * Roll-off is 40 dB/dec (12 dB/oct).
 *
 *                  (s / w0)^2
 *   H(s) = -----------------------------
 *            1 + s / Q w0 + (s / w0)^2
 *
 * @param f0 Resonance frequency;
 * @param Q  Q factor;
 * @param fs Sampling frequency.
 * @return The normalized filter coefficients.
 */
constexpr PCoeffs<3, 3> iir_hipass40(fp_t f0, fp_t Q, fp_t fs)
{
    const fp_t w  = Pi * f0 / fs;
    const fp_t ww = w * w;
    const fp_t wq = w / Q;

    const fp_t a0 = 1.0 + wq + ww;
    const fp_t a1 = (2.0 * ww - 2.0) / a0;
    const fp_t a2 = (1.0 - wq + ww) / a0;

    const fp_t b0 =  1.0 / a0;
    const fp_t b1 = -2.0 / a0;
    const fp_t b2 =  1.0 / a0;

    return { { b0, b1, b2 }, { 1.0, a1, a2 } };
}

/**
 * Return the set of coefficients for a first order band-pass filter.
 * Roll-off is 20 dB/dec (6 dB/oct).
 *
 *                     s / wh
 *   H(s) = -----------------------------
 *            (1 + s / wh) (1 + s / wl)
 *
 * @param fc Cut-off frequency;
 * @param Q  Q factor;
 * @param fs Sampling frequency.
 * @return The normalized filter coefficients.
 */
constexpr PCoeffs<3, 3> iir_bapass20(fp_t fc, fp_t Q, fp_t fs)
{
    fp_t df = fc / (2.0 * Q);
    fp_t fh = std::max<fp_t>(fc - df, 0.0);
    fp_t fl = std::min<fp_t>(fc + df, fs / 2.0);

    const fp_t wh = Pi * fh / fs;
    const fp_t wl = Pi * fl / fs;

    const fp_t b0 = wl;
    const fp_t b1 = 0.0;
    const fp_t b2 = -wl;

    const fp_t a0 = (wh + 1.0) * (wl + 1.0);
    const fp_t a1 = (wh + 1.0) * (wl - 1.0) + (wh - 1.0) * (wl + 1.0);
    const fp_t a2 = (wl - 1.0) * (wh - 1.0);

    return { { b0 / a0, b1 / a0, b2 / a0 }, { 1.0, a1 / a0, a2 / a0 } };
}

/**
 * Generate an octave transposed vector containing a set of samples.
 * @param samples Samples buffer.
 * @return A transposed vector string.
 * @see https://octave.org
 */
std::string to_string(samples_fp samples);

/**
 * Send the content of a samples buffer to an output stream formatted as an octave structure.
 * @param os      The output stream;
 * @param name    Name for the structure;
 * @param samples Samples;
 * @param fc1     Cutoff frequency 1;
 * @param fc2     Cutoff frequency 2 or 0;
 * @param Q       Q factor or 0;
 * @param fs      Sampling rate.
 * @return os.
 * @see https://octave.org
 */
std::ostream& dump(std::ostream& os, samples_fp samples, std::string_view name, fp_t fc1, fp_t fc2, fp_t Q, fp_t fs);

}

template<size_t M1, size_t N1, size_t M2, size_t N2>
constexpr signal::PCoeffs<std::max(M1, M2), std::max(N1, N2)> operator+(const signal::PCoeffs<M1, N1>& c1,
    const signal::PCoeffs<M2, N2>& c2)
{
    return signal::add(c1, c2);
}

}
