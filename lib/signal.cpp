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
#include "signal.hpp"

#include <gsl/span>


namespace cemu {
namespace signal {

std::random_device rd{};
std::uniform_real_distribution<float> uni_random{-1.0f, 1.0f};


size_t kernel_size(float fc, float fs)
{
    size_t size = std::ceil(4.0f * fs / fc);

    if ((size & 1) == 0) {
        ++size;
    }

    return size;
}


float blackman(size_t pos, size_t N)
{
    if (N <= 1) {
        return 1.0f;
    }

    const float k = static_cast<float>(pos) / static_cast<float>(N - 1);
    return (0.42f - 0.5f * std::cos(2.0f * M_PI * k) + 0.08f * std::cos(4.0f * M_PI * k));
}


gsl::span<float> &spectral_inversion(gsl::span<float> &v)
{
    for (auto &value : v) {
        value *= -1.0f;
    }

    const size_t centre = v.size() >> 1;
    v[centre] += 1.0f;

    return v;
}


gsl::span<float> lopass(gsl::span<float> &buf, float fc, float fs, float rs, bool osiz)
{
    size_t N;
    if (osiz) {
        N = kernel_size(fc, fs);
        if (N > buf.size()) {
            N = buf.size();
        }
    } else {
        N = buf.size();
    }

    float sum = 0.0f;
    const float w = 2.0f * M_PI * fc;

    const float ts = 1.0f / fs;
    float t = -ts * static_cast<float>(N >> 1);

    /*
     * The "resonance" that is artificially generated here is completely guessed
     * and it probably does not alter the spectrum as the audiophiles want.
     */
    for (size_t k = 0; k < N; ++k, t += ts) {
        float value = (sinc(w * t) + rs * std::sin(w * t)) * blackman(k, N);
        buf[k] = value;
        sum += value;
    }

    for (auto &value : buf) {
        value /= sum;
    }

    return buf.subspan(0, N);
}


gsl::span<float> stopband(gsl::span<float> &buf, float fcl, float fch, float fs, float rs, bool osiz)
{
    float lodata[buf.size()];
    gsl::span lo{lodata, buf.size()};
    lo = lopass(lo, fcl, fs, rs, osiz);

    const size_t N = lo.size();

    float hidata[buf.size()];
    gsl::span hi{hidata, N};
    hi = hipass(hi, fch, fs, rs, false);

    for (size_t k = 0; k < N; ++k) {
        buf[k] = lo[k] + hi[k];
    }

    return buf.subspan(0, N);
}


samples_fp conv(const samples_fp &x, const samples_fp &y)
{
    const size_t Nx = x.size();
    const size_t Ny = y.size();
    const size_t Nc = Nx + Ny - 1;

    samples_fp c(Nc, 0.0f);

    for (size_t ix = 0; ix < Nx; ++ix) {
        for (size_t iy = 0; iy < Ny; ++iy) {
            c[ix + iy] += x[ix] * y[iy];
        }
    }

    return c;
}


void conv_kernel(samples_fp &v, const gsl::span<float> &k)
{
    const size_t Nv = v.size();
    const size_t Nk = k.size();
    const size_t Nc = Nv + Nk - 1;

    float buf[Nc];
    gsl::span c{buf, Nc};
    std::fill_n(c.begin(), Nc, 0.0f);

    for (size_t iv = 0; iv < Nv; ++iv) {
        for (size_t ik = 0; ik < Nk; ++ik) {
            c[iv + ik] += v[iv] * k[ik];
        }
    }

    auto from = c.begin() + ((Nc - Nv) >> 1);
    auto to = from + Nv;
    std::copy(from, to, v.begin());
}

}
}
